#include <stdio.h>
#include "NodeBlockClass.h"
#include "NodeBlockFunctionDefinition.h"
#include "CompilerState.h"
#include "SymbolFunctionName.h"


namespace MFM {

  static const char * CModeForHeaderFiles = "/**                                        -*- mode:C++ -*/\n\n";

  NodeBlockClass::NodeBlockClass(NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s) : NodeBlock(prevBlockNode, state, s), m_functionST(state)
  {}

  NodeBlockClass::~NodeBlockClass()
  {}


  void NodeBlockClass::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)    
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    if(m_nextNode)
      m_nextNode->print(fp);  //datamember var decls

    NodeBlockFunctionDefinition * func = findTestFunctionNode();
    if(func)
      func->print(fp);

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }


  void NodeBlockClass::printPostfix(File * fp)
  {
    fp->write(m_state.getUlamTypeByIndex(getNodeType())->getUlamTypeUPrefix().c_str());  //e.g. Ue_Foo
    fp->write(getName());  //unmangled

    fp->write(" {");
    // has no m_node! 
    if(m_nextNode)
      m_nextNode->printPostfix(fp);  //datamember vardecls


    NodeBlockFunctionDefinition * func = findTestFunctionNode();
    if(func)
      func->printPostfix(fp);
    else
      fp->write(" <NOMAIN>");  //not an error

    fp->write(" }");
  }


  const char * NodeBlockClass::getName()
  {
    return m_state.getUlamTypeByIndex(getNodeType())->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state).c_str(); 
    //return "{}";
  }


  const std::string NodeBlockClass::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeBlockClass::checkAndLabelType()
  { 
    //side-effect DataMember VAR DECLS
    if(m_nextNode)
      m_nextNode->checkAndLabelType();   


    // label all the function definition bodies first. why?
    m_functionST.labelTableOfFunctions();

    // check that a 'test' function returns Int (ulam convention)
    NodeBlockFunctionDefinition * funcNode = findTestFunctionNode();
    if(funcNode)
      {
	UTI funcType = funcNode->getNodeType();
	if(funcType != Int)
	  {
	    std::ostringstream msg;
	    msg << "By convention, Function '" << funcNode->getName() << "''s Return type must be <Int>, not <" << m_state.getUlamTypeNameBriefByIndex(funcType).c_str() << ">";
	    MSG(funcNode->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    setNodeType(Void);     //will be reset to reflect the Class type
    return getNodeType();
  }
  

  EvalStatus NodeBlockClass::eval()
  {
#if 0
    //determine size of stackframe (enable for test t3116)
    u32 stackframetotal = getSizeOfFuncSymbolsInTable();
    u32 numberoffuncs = getNumberOfFuncSymbolsInTable();
    {
      std::ostringstream msg;
      msg << stackframetotal << " is the total stackframe size required for " << numberoffuncs << " functions";
      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
    }
#endif

    evalNodeProlog(0);       //new current frame pointer for nodeeval stack

    EvalStatus evs = ERROR;  //init

#if 0
    // NodeVarDecl's make UlamValue storage now, so don't want their
    // side-effects for the class definition, rather the instance.
    if(m_nextNode)
      {
	m_nextNode->eval();  //side-effect for datamember vardecls
      }
#endif

    NodeBlockFunctionDefinition * funcNode = findTestFunctionNode();
    if(funcNode)
      {
	UTI saveClassType = getNodeType();  //temp!!
	setNodeType(Int);   //for testing WHY? clobbers element/quark type
	UTI funcType = funcNode->getNodeType();
	
	makeRoomForNodeType(funcType);  //Int return

	evs = funcNode->eval();
	if(evs == NORMAL)
	  {
	    UlamValue testUV = m_state.m_nodeEvalStack.popArg();
	    assignReturnValueToStack(testUV);
	  }

	setNodeType(saveClassType); //temp, restore
      }

    evalNodeEpilog();
    return evs;
  }


  //override to check both variables and function names.
  bool NodeBlockClass::isIdInScope(u32 id, Symbol * & symptrref)
  {
    return (m_ST.isInTable(id, symptrref) || isFuncIdInScope(id, symptrref));
  }


  bool NodeBlockClass::isFuncIdInScope(u32 id, Symbol * & symptrref)
  {
    return m_functionST.isInTable(id, symptrref);
  }


  void NodeBlockClass::addFuncIdToScope(u32 id, Symbol * symptr)
  {
    m_functionST.addToTable(id, symptr);
  }


  u32 NodeBlockClass::getNumberOfFuncSymbolsInTable()
  {
    return m_functionST.getTableSize();
  }


  u32 NodeBlockClass::getSizeOfFuncSymbolsInTable()
  {
    return m_functionST.getTotalSymbolSize();
  }


  //don't set nextNode since it'll get deleted with program.
  NodeBlockFunctionDefinition * NodeBlockClass::findTestFunctionNode()
  {
    Symbol * fnSym;
    NodeBlockFunctionDefinition * func = NULL;
    u32 testid = m_state.m_pool.getIndexForDataString("test");
    if(isFuncIdInScope(testid, fnSym))
      {
	SymbolFunction * funcSymbol = NULL;
	std::vector<UTI> voidVector;
	if(((SymbolFunctionName *) fnSym)->findMatchingFunction(voidVector, funcSymbol))
	  {
	    func = funcSymbol->getFunctionNode();
	  }
      }

    return func;
  }


  void NodeBlockClass::packBitsForVariableDataMembers()
  {
    u32 offset = 0;
    //m_ST.packBitsForTableOfVariableDataMembers();
    m_nextNode->packBitsInOrderOfDeclaration(offset);
  }

  //header .h file
  void NodeBlockClass::genCode(File * fp)
  {
    UlamType * cut = m_state.getUlamTypeByIndex(getNodeType());
    ULAMCLASSTYPE classtype = cut->getUlamClass();
    assert(cut->getUlamTypeEnum() == Class);

    m_state.m_currentIndentLevel = 0;
    fp->write(CModeForHeaderFiles);

    //generate includes for all the other classes that have appeared
    m_state.m_programDefST.generateIncludesForTableOfClasses(fp);

    m_state.indent(fp);
    fp->write("#include \"");
    fp->write(m_state.getFileNameForThisTypesHeader().c_str());  
    fp->write("\"\n");
    fp->write("\n");

    m_state.indent(fp);
    fp->write("#ifndef ");
    fp->write(allCAPS(cut->getUlamTypeMangledName(&m_state).c_str()).c_str());
    fp->write("_H\n");

    m_state.indent(fp);
    fp->write("#define ");
    fp->write(allCAPS(cut->getUlamTypeMangledName(&m_state).c_str()).c_str());
    fp->write("_H\n\n");

    m_state.indent(fp);
    fp->write("namespace MFM{\n\n");

    m_state.m_currentIndentLevel++;

    if(classtype == UC_QUARK)
      {
	m_state.indent(fp);
	fp->write("template <u32 POS>\n");
      }

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(cut->getUlamTypeMangledName(&m_state).c_str());

    fp->write("\n");

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    if(classtype == UC_ELEMENT)
      {
	//DataMember VAR DECLS
	m_nextNode->genCode(fp);
	//NodeBlock::genCodeDeclsForVariableDataMembers(fp, classtype); //not in order declared
	fp->write("\n");
      }

    //default constructor/destructor
    if(classtype == UC_ELEMENT)
      {
	m_state.indent(fp);
	fp->write(cut->getUlamTypeMangledName(&m_state).c_str());
	fp->write("();\n");
	
	m_state.indent(fp);
	fp->write("~");
	fp->write(cut->getUlamTypeMangledName(&m_state).c_str());
	fp->write("();\n\n");
      }

    // if this 'element' contains more than one template (quark) data members, 
    // we need vector of offsets to generate a separate function decl/dfn for each one's POS
    // in case a function has one as a return value and/or parameter.


    //gencode declarations only for all the function definitions
    //bool declOnly = (m_state.m_compileThisId != getNodeType()->getUlamKeyTypeSignature()->getUlamKeyTypeSignatureNameId());
    m_functionST.genCodeForTableOfFunctions(fp, (classtype == UC_ELEMENT), classtype);

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("};\n");

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //MFM\n\n");

    m_state.indent(fp);
    fp->write("#endif //");
    fp->write(allCAPS(cut->getUlamTypeMangledName(&m_state).c_str()).c_str());
    fp->write("_H\n");
  }


  //Body for This Class only; practically empty if quark (.cpp)
  void NodeBlockClass::genCodeBody(File * fp)
  {
    UlamType * cut = m_state.getUlamTypeByIndex(getNodeType());
    ULAMCLASSTYPE classtype = cut->getUlamClass();

    m_state.m_currentIndentLevel = 0;

    m_state.indent(fp);
    fp->write("#include \"");
    fp->write(m_state.getFileNameForThisClassHeader().c_str());  
    fp->write("\"\n");
    fp->write("\n");

    m_state.indent(fp);
    fp->write("namespace MFM{\n\n");

    if(classtype == UC_ELEMENT)
      {
	m_state.m_currentIndentLevel++;

	//default constructor
	m_state.indent(fp);
	fp->write(cut->getUlamTypeMangledName(&m_state).c_str());
	fp->write("::");
	fp->write(cut->getUlamTypeMangledName(&m_state).c_str());
	fp->write("(){}\n\n");
	
	//default destructor
	m_state.indent(fp);
	fp->write(cut->getUlamTypeMangledName(&m_state).c_str());
	fp->write("::~");
	fp->write(cut->getUlamTypeMangledName(&m_state).c_str());
	fp->write("(){}\n\n");
	
	assert(m_state.m_compileThisId == cut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId());
	m_functionST.genCodeForTableOfFunctions(fp, false, classtype);

	m_state.m_currentIndentLevel--;
      }
    
    m_state.indent(fp);
    fp->write("} //MFM\n\n");
  }


  std::string NodeBlockClass::allCAPS(const char * s)
  {
    u32 len = strlen(s);
    std::ostringstream up;
    
    for(unsigned int i = 0; i < len; ++i)
      {
      std::string c(1,(s[i] <= 'z' && s[i] >= 'a') ? s[i]-('a'-'A') : s[i]);
      up << c;
    }
    
    return up.str();
  }


} //end MFM

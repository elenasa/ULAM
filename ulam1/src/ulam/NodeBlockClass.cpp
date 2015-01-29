#include <stdio.h>
#include "NodeBlockClass.h"
#include "NodeBlockFunctionDefinition.h"
#include "CompilerState.h"
#include "SymbolFunctionName.h"


namespace MFM {

  //static const char * CModeForHeaderFiles = "/**                                        -*- mode:C++ -*/\n\n";

  NodeBlockClass::NodeBlockClass(NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s) : NodeBlock(prevBlockNode, state, s), m_functionST(state), m_isEmpty(false), m_templateClassParent(Nav) {}

  NodeBlockClass::NodeBlockClass(const NodeBlockClass& ref) : NodeBlock(ref), m_functionST(ref.m_functionST) /* deep copy */, m_isEmpty(ref.m_isEmpty), m_templateClassParent(Nav) /* no longer needed? */{}

  NodeBlockClass::~NodeBlockClass() {}

  Node * NodeBlockClass::clone()
  {
    return new NodeBlockClass(*this);
  }

  bool NodeBlockClass::isEmpty()
  {
    return m_isEmpty;
  }

  void NodeBlockClass::setEmpty()
  {
    m_isEmpty = true;
  }

  void NodeBlockClass::setClassTemplateParent(UTI cuti)
  {
    m_templateClassParent = cuti;
  }

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
    fp->write("\n");
  } //print (debug)


  void NodeBlockClass::printPostfix(File * fp)
  {
    fp->write(m_state.getUlamTypeByIndex(getNodeType())->getUlamTypeUPrefix().c_str());  //e.g. Ue_Foo
    fp->write(getName());  //unmangled

    if(isEmpty())
      {
	fp->write(" { /* empty class block */ }");
	return;
      }

    fp->write(" {");
    // has no m_node!
    // use Symbol Table of variables instead of parse tree; only want the UEventWindow storage
    // since the two stack-type storage are all gone by now.
    //    if(m_nextNode)
    //  m_nextNode->printPostfix(fp);  //datamember vardecls
    ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(getNodeType())->getUlamClass(); //may not need classtype
    //simplifying assumption for testing purposes: center site
    Coord c0(0,0);
    s32 slot = c0.convertCoordToIndex();

    m_ST.printPostfixValuesForTableOfVariableDataMembers(fp, slot, ATOMFIRSTSTATEBITPOS, classtype);

    NodeBlockFunctionDefinition * func = findTestFunctionNode();
    if(func)
      func->printPostfix(fp);
    else
      fp->write(" <NOMAIN>");  //not an error

    fp->write(" }");
    fp->write("\n");
  } //printPostfix


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
    if(isEmpty())
      return getNodeType();

#if 0
    // redo checkAndLabel with class arg values?
    if(m_templateClassParent != Nav)
      {
	return getPreviousBlockPointer()->checkAndLabelType();
      }
#endif

    //side-effect DataMember VAR DECLS
    if(m_nextNode)
      m_nextNode->checkAndLabelType();

    // label all the function definition bodies
    m_functionST.labelTableOfFunctions();

    // check that a 'test' function returns Int (ulam convention)
    NodeBlockFunctionDefinition * funcNode = findTestFunctionNode();
    if(funcNode)
      {
	UTI funcType = funcNode->getNodeType();
	if(funcType != Int)
	  {
	    std::ostringstream msg;
	    msg << "By convention, Function '" << funcNode->getName() << "''s Return type must be 'Int', not " << m_state.getUlamTypeNameBriefByIndex(funcType).c_str();
	    MSG(funcNode->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    // type already set during parsing
    return getNodeType();
  } //checkAndLabelType


  void NodeBlockClass::countNavNodes(u32& cnt)
  {
    if(m_nextNode) //may not have data members
      NodeBlock::countNavNodes(cnt);
    m_functionST.countNavNodesAcrossTableOfFunctions();
  }


  void NodeBlockClass::checkCustomArrayTypeFunctions()
  {
    if(!isEmpty())
      m_functionST.checkCustomArrayTypeFuncs();
  }


  EvalStatus NodeBlockClass::eval()
  {
    if(isEmpty())
      return NORMAL;

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
      m_nextNode->eval();  //side-effect for datamember vardecls
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
	    //rtnValue = rtnUV.getImmediateData(32);
	  }
	setNodeType(saveClassType); //temp, restore
      }
    evalNodeEpilog();
    return evs;
  } //eval


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
  } //findTestFunctionNode()

  void NodeBlockClass::packBitsForVariableDataMembers()
  {
    if(m_ST.getTableSize() == 0) return;
    u32 offset = 0; //relative to ATOMFIRSTSTATEBITPOS
    //m_ST.packBitsForTableOfVariableDataMembers();
    assert(m_nextNode);
    m_nextNode->packBitsInOrderOfDeclaration(offset);
  }

  u32 NodeBlockClass::countNativeFuncDecls()
  {
    if(isEmpty())
      return 0;
    return m_functionST.countNativeFuncDeclsForTableOfFunctions();
  }

  void NodeBlockClass::generateCodeForFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    // check all the function names for duplicate definitions
    m_functionST.checkTableOfFunctions(); //returns prob counts, outputs errs
    m_functionST.genCodeForTableOfFunctions(fp, declOnly, classtype);
  }

  //header .h file
  void NodeBlockClass::genCode(File * fp, UlamValue& uvpass)
  {
    //use the instance UTI instead of the node's original type
    //UlamType * cut = m_state.getUlamTypeByIndex(getNodeType());
    UlamType * cut = m_state.getUlamTypeByIndex(m_state.m_compileThisIdx);
    ULAMCLASSTYPE classtype = cut->getUlamClass();
    assert(cut->getUlamTypeEnum() == Class);

    m_state.m_currentIndentLevel = 0;

    fp->write("\n");
    m_state.indent(fp);
    fp->write("namespace MFM{\n\n");

    m_state.m_currentIndentLevel++;

    if(classtype == UC_QUARK)
      {
	genCodeHeaderQuark(fp);
      }
    else  //element
      {
	genCodeHeaderElement(fp);
      }

    //gencode declarations only for all the function definitions
    bool declOnly = true;
    generateCodeForFunctions(fp, declOnly, classtype);

    generateCodeForBuiltInClassFunctions(fp, declOnly, classtype);

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("};\n");

    //declaration of THE_INSTANCE for ELEMENT
    if(classtype == UC_ELEMENT)
      {
	fp->write("\n");
	m_state.indent(fp);
	fp->write("template<class CC>\n");
	m_state.indent(fp);
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<CC> ");
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<CC>::THE_INSTANCE;\n\n");
      }
    m_state.m_currentIndentLevel = 0;
    fp->write("} //MFM\n\n");

    //leave class' endif to caller
  } //genCode


  void NodeBlockClass::genCodeHeaderQuark(File * fp)
  {
    //use the instance UTI instead of the node's original type
    //UlamType * cut = m_state.getUlamTypeByIndex(getNodeType());
    UlamType * cut = m_state.getUlamTypeByIndex(m_state.m_compileThisIdx);

    m_state.indent(fp);
    fp->write("template <class CC, u32 POS>\n");

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(cut->getUlamTypeMangledName().c_str());

    //tbd inheritance

    fp->write("\n");

    m_state.indent(fp);
    fp->write("{\n");
    fp->write("\n");

    m_state.m_currentIndentLevel++;

    genShortNameParameterTypesExtractedForHeaderFile(fp);

    // output quark size enum
    m_state.indent(fp);
    fp->write("enum { \n");
    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("QUARK_SIZE = ");
    fp->write_decimal(cut->getBitSize());
    fp->write(" /* Requiring quarks to advertise their size in a std way.) */\n");
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("};\n");

    fp->write("\n");

    // where to put these???
    //genImmediateMangledTypesForHeaderFile(fp);
    m_state.indent(fp);
    fp->write("typedef AtomicParameterType <CC, VD::BITS, QUARK_SIZE, POS> Up_Us; //entire quark\n");

    fp->write("\n");

    //DataMember VAR DECLS
    if(m_nextNode)
      {
	UlamValue uvpass;
	m_nextNode->genCode(fp, uvpass);  //output the BitField typedefs
	//NodeBlock::genCodeDeclsForVariableDataMembers(fp, classtype); //not in order declared
	fp->write("\n");
      }
    //default constructor/destructor
  } //genCodeHeaderQuark


  void NodeBlockClass::genCodeHeaderElement(File * fp)
  {
    //use the instance UTI instead of the node's original type
    //UlamType * cut = m_state.getUlamTypeByIndex(getNodeType());
    UlamType * cut = m_state.getUlamTypeByIndex(m_state.m_compileThisIdx);

    m_state.indent(fp);
    fp->write("template<class CC>\n");

    m_state.indent(fp);
    fp->write("class ");
    fp->write(cut->getUlamTypeMangledName().c_str());

    fp->write(" : public UlamElement<CC>\n");

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    genShortNameParameterTypesExtractedForHeaderFile(fp);

    // where to put these???
    //genImmediateMangledTypesForHeaderFile(fp);

    fp->write("\n");
    m_state.m_currentIndentLevel--;
    //m_state.m_currentIndentLevel = 0;
    m_state.indent(fp);
    fp->write("public:\n\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("static ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write(" THE_INSTANCE;\n");

    //DataMember VAR DECLS
    if(m_nextNode)
      {
	UlamValue uvpass;
	m_nextNode->genCode(fp, uvpass);  //output the BitField typedefs
	//NodeBlock::genCodeDeclsForVariableDataMembers(fp, classtype); //not in order declared
	fp->write("\n");
      }

    //default constructor/destructor
    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("();\n");

    m_state.indent(fp);
    fp->write("~");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("();\n\n");

    // if this 'element' contains more than one template (quark) data members,
    // we need vector of offsets to generate a separate function decl/dfn for each one's POS
    // in case a function has one as a return value and/or parameter.
  } //genCodeHeaderElement


  void NodeBlockClass::genImmediateMangledTypesForHeaderFile(File * fp)
  {
    m_state.indent(fp);
    fp->write("// immediate values\n");

    //skip Nav type (0)
    std::map<UlamKeyTypeSignature, UlamType *, less_than_key>::iterator it = m_state.m_definedUlamTypes.begin();
    while(it != m_state.m_definedUlamTypes.end())
      {
	UlamType * ut = it->second;
	//skip constants, atoms, ptrs, elements, void and nav
	if(ut->needsImmediateType())
	  ut->genUlamTypeMangledImmediateDefinitionForC(fp);
	it++;
      }
  } //genImmediateMangledTypesForHeaderFile


  void NodeBlockClass::genShortNameParameterTypesExtractedForHeaderFile(File * fp)
  {
    m_state.indent(fp);
    fp->write("// Extract short names for parameter types\n");
    m_state.indent(fp);
    fp->write("typedef typename CC::ATOM_TYPE T;\n");
    m_state.indent(fp);
    fp->write("typedef typename CC::PARAM_CONFIG P;\n");
    fp->write("\n");

    m_state.indent(fp);
    fp->write("enum { BPA = P::BITS_PER_ATOM };\n");

    m_state.indent(fp);
    fp->write("typedef BitVector<BPA> BV;\n");

    fp->write("\n");
  } //genShortNameParameterTypesExtractedForHeaderFile


  //Body for This Class only; practically empty if quark (.tcc)
  void NodeBlockClass::genCodeBody(File * fp, UlamValue& uvpass)
  {
    //use the instance UTI instead of the node's original type
    //UlamType * cut = m_state.getUlamTypeByIndex(getNodeType());
    UlamType * cut = m_state.getUlamTypeByIndex(m_state.m_compileThisIdx);
    ULAMCLASSTYPE classtype = cut->getUlamClass();

    m_state.m_currentIndentLevel = 0;

    //generate include statements for all the other classes that have appeared
    m_state.m_programDefST.generateIncludesForTableOfClasses(fp);
    fp->write("\n");


    m_state.indent(fp);
    fp->write("namespace MFM{\n\n");

    m_state.m_currentIndentLevel++;

    if(classtype == UC_ELEMENT)
      {
	//default constructor
	m_state.indent(fp);
	fp->write("template<class CC>\n");

	m_state.indent(fp);
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<CC>");
	fp->write("::");
	fp->write(cut->getUlamTypeMangledName().c_str());

	std::string namestr = cut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state);
	fp->write("() : UlamElement<CC>(MFM_UUID_FOR(\"");
	fp->write(namestr.c_str());
	fp->write("\", 0))\n");
	m_state.indent(fp);
	fp->write("{\n");

	m_state.m_currentIndentLevel++;

	m_state.indent(fp);
	fp->write("//XXXX  Element<CC>::SetAtomicSymbol(\"");
	fp->write(namestr.substr(0,1).c_str());
	fp->write("\");  // figure this out later\n");

	m_state.indent(fp);
	fp->write("Element<CC>::SetName(\"");
	fp->write(namestr.c_str());
	fp->write("\");\n");

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("}\n\n");

	//default destructor
	m_state.indent(fp);
	fp->write("template<class CC>\n");

	m_state.indent(fp);
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<CC>");
	fp->write("::~");
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("(){}\n\n");

	assert(m_state.m_compileThisId == cut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId());
      }

    generateCodeForFunctions(fp, false, classtype);

    generateCodeForBuiltInClassFunctions(fp, false, classtype);

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //MFM\n\n");
  } //genCodeBody


  void NodeBlockClass::generateCodeForBuiltInClassFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    // 'has' is for both class types
    NodeBlock::generateCodeForBuiltInClassFunctions(fp, declOnly, classtype);

    // 'is' is only for element/classes
    if(classtype != UC_ELEMENT)
      return;

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("//helper method not called directly\n");

	m_state.indent(fp);
	fp->write("bool ");
	fp->write(m_state.getIsMangledFunctionName());
	fp->write("(const T& targ) const;\n\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class CC>\n");
    m_state.indent(fp);
    fp->write("bool ");  //return pos offset, or -1 if not found

    //UTI cuti = getNodeType();
    UTI cuti = m_state.m_compileThisIdx;
    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());

    fp->write("<CC>::");
    fp->write(m_state.getIsMangledFunctionName());
    fp->write("(const T& targ) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("return ");
    fp->write("(THE_INSTANCE.GetType() == targ.GetType());\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}   //is\n\n");
  } //generateCodeForBuiltInClassFunctions

} //end MFM

#include <stdio.h>
#include "NodeBlockClass.h"
#include "NodeBlockFunctionDefinition.h"
#include "CompilerState.h"
#include "SymbolFunctionName.h"


namespace MFM {

  NodeBlockClass::NodeBlockClass(NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s) : NodeBlock(prevBlockNode, state, s), m_functionST(state), m_isEmpty(false)
  {
    m_nodeParameterList = new NodeList(state);
    assert(m_nodeParameterList);
  }

  NodeBlockClass::NodeBlockClass(const NodeBlockClass& ref) : NodeBlock(ref), m_functionST(ref.m_functionST) /* deep copy */, m_isEmpty(ref.m_isEmpty), m_nodeParameterList(NULL)
  {
    setNodeType(m_state.getCompileThisIdx());
    m_nodeParameterList = (NodeList *) ref.m_nodeParameterList->instantiate(); //instances don't need this; its got symbols
  }

  NodeBlockClass::~NodeBlockClass()
  {
    delete m_nodeParameterList;
    m_nodeParameterList = NULL;
  }

  Node * NodeBlockClass::instantiate()
  {
    return new NodeBlockClass(*this);
  }

  bool NodeBlockClass::isEmpty()
  {
    return m_isEmpty;
  }

  void NodeBlockClass::setEmpty()
  {
    m_isEmpty = true; //may have parameters!
  }

  void NodeBlockClass::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    if(m_node)
      m_node->updateLineage(getNodeNo());
    if(m_nodeNext)
      m_nodeNext->updateLineage(getNodeNo());
    if(m_nodeParameterList)
      m_nodeParameterList->updateLineage(getNodeNo());
    m_functionST.linkToParentNodesAcrossTableOfFunctions(this); //all the function defs
  } //updateLineage

  bool NodeBlockClass::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeBlock::findNodeNo(n, foundNode))
      return true;

    UTI cuti = getNodeType();
    if(cuti == Int) //kludge nodetype clobbered to test
      cuti = m_state.getCompileThisIdx();

    if(m_state.isClassASubclass(cuti))
      {
	NodeBlockClass * superblock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superblock);
	if(superblock->findNodeNo(n, foundNode))
	  return true;
      }

    if(m_functionST.findNodeNoAcrossTableOfFunctions(n, foundNode)) //all the function defs
      return true;
    if(m_nodeParameterList && m_nodeParameterList->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeBlockClass::setNodeLocation(Locator loc)
  {
    if(m_nodeParameterList)
      m_nodeParameterList->setNodeLocation(loc);
    Node::setNodeLocation(loc);
  }

  void NodeBlockClass::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myuti = getNodeType();

    if(myuti == Int) //kludge nodetype clobbered to test
      myuti = m_state.getCompileThisIdx();

    char id[255];
    if(myuti == Nav)
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myuti).c_str());
    fp->write(id);

    if(m_nodeNext)
      m_nodeNext->print(fp);  //datamember var decls

    NodeBlockFunctionDefinition * func = findTestFunctionNode();
    if(func)
      func->print(fp);

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
    fp->write("\n");
  } //print (debug)

  void NodeBlockClass::printPostfix(File * fp)
  {
    UTI cuti = getNodeType();
    if(cuti == Int) //kludge nodetype clobbered to test
      cuti = m_state.getCompileThisIdx();

    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeUPrefix().c_str());  //e.g. Ue_Foo
    fp->write(getName());  //unmangled

    if(m_state.isClassASubclass(cuti))
      {
	NodeBlockClass * superblock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superblock);
	fp->write(" : ");
	fp->write(m_state.getUlamTypeByIndex(superblock->getNodeType())->getUlamTypeUPrefix().c_str());  //e.g. Ue_Foo
	fp->write(superblock->getName());
      }

    //output class template arguments type and name
    if(m_nodeParameterList->getNumberOfNodes() > 0)
      {
	SymbolClassNameTemplate * cnsym = NULL;
	assert(m_state.alreadyDefinedSymbolClassNameTemplate(m_state.getUlamKeyTypeSignatureByIndex(cuti).getUlamKeyTypeSignatureNameId(), cnsym));
	cnsym->printClassTemplateArgsForPostfix(fp);
	//m_nodeParameterList->print(fp);
      }

    if(isEmpty())
      {
	fp->write(" { /* empty class block */ }\n");
	return;
      }

    fp->write(" {");
    // has no m_node!
    // use Symbol Table of variables instead of parse tree;
    // only want the UEventWindow storage
    // since the two stack-type storage are all gone by now.
    NodeBlockFunctionDefinition * func = findTestFunctionNode();
    if(func)
      {
	ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(cuti)->getUlamClass(); //may not need classtype
	assert(classtype == UC_ELEMENT || classtype == UC_QUARK); //sanity check after eval (below)

	//simplifying assumption for testing purposes: center site
	Coord c0(0,0);
	s32 slot = c0.convertCoordToIndex();

	printPostfixDataMembersSymbols(fp, slot, ATOMFIRSTSTATEBITPOS, classtype);

	func->printPostfix(fp);
      }
    else
      {
	//has only initialized DM values
	printPostfixDataMembersParseTree(fp);
	fp->write(" <NOMAIN>"); //not an error
      }
    fp->write(" }");
    fp->write("\n");
  } //printPostfix

  void NodeBlockClass::printPostfixDataMembersParseTree(File * fp)
  {
    UTI cuti = getNodeType();
    if(cuti == Int) //kludge nodetype clobbered to test
      cuti = m_state.getCompileThisIdx();

    if(m_state.isClassASubclass(cuti))
      {
	NodeBlockClass * superblock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superblock);
	fp->write(" :<");
	superblock->printPostfixDataMembersParseTree(fp);
	fp->write(">");
      }

    if(m_nodeNext)
      m_nodeNext->printPostfix(fp); //datamember vardecls
  } //printPostfixDataMembersParseTree

  void NodeBlockClass::printPostfixDataMembersSymbols(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    UTI cuti = getNodeType();
    if(cuti == Int) //kludge nodetype clobbered to test
      cuti = m_state.getCompileThisIdx();

    if(m_state.isClassASubclass(cuti))
      {
	NodeBlockClass * superblock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superblock);
	fp->write(" :<");
	superblock->printPostfixDataMembersSymbols(fp, slot, startpos, classtype);
	fp->write(">");
      }

    m_ST.printPostfixValuesForTableOfVariableDataMembers(fp, slot, startpos, classtype);
  } //printPostfixDataMembersSymbols

  const char * NodeBlockClass::getName()
  {
    return m_state.getUlamKeyTypeSignatureByIndex(getNodeType()).getUlamKeyTypeSignatureName(&m_state).c_str();
  } //getName

  const std::string NodeBlockClass::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeBlockClass::checkAndLabelType()
  {
    // for debug purposes too
    bool isTemplate = m_state.isClassATemplate(m_state.getCompileThisIdx());

    // Inheritance checks
    UTI nuti = getNodeType();
    UTI superuti = m_state.isClassASubclass(nuti);

    if(superuti != Nav)
      {
	//this is a subclass.

	//neither the subclass nor the superclass may be a template too
	if(isTemplate)
	  {
	    std::ostringstream msg;
	    msg << "Template class as a subclass is not supported: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	  }

	if(m_state.isClassATemplate(superuti))
	  {
	    std::ostringstream msg;
	    msg << "Template class '";
	    msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str();
	    msg << "' as a superclass for '";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << "' is not supported";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	  }

	NodeBlockClass * superblock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superblock);
	ULAMCLASSTYPE superclasstype = m_state.getUlamTypeByIndex(superuti)->getUlamClass();
	ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(nuti)->getUlamClass();
	if(superclasstype != classtype)
	  {
	    std::ostringstream msg;
	    msg << "Subclass '";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << "' inherits from '";
	    msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str();
	    msg << "', a class of a different type";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	  }
      } //done with inheritance checks, continue.

    //do first, might be important!
    checkParameterNodeTypes();

    //side-effect DataMember VAR DECLS
    if(m_nodeNext)
      m_nodeNext->checkAndLabelType();

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
	    msg << "By convention, Function '" << funcNode->getName();
	    msg << "''s Return type must be 'Int', not ";
	    msg << m_state.getUlamTypeNameBriefByIndex(funcType).c_str();
	    MSG(funcNode->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    funcNode->setNodeType(Nav); //missing
	  }
      }
    // type already set during parsing
    return getNodeType();
  } //checkAndLabelType

  bool NodeBlockClass::checkParameterNodeTypes()
  {
    if(m_nodeParameterList)
      return m_nodeParameterList->checkAndLabelType();
    return true;
  }

  void NodeBlockClass::addParameterNode(Node * nodeArg)
  {
    assert(m_nodeParameterList); //must be a template
    m_nodeParameterList->addNodeToList(nodeArg);
  }

  Node * NodeBlockClass::getParameterNode(u32 n) const
  {
    assert(m_nodeParameterList); //must be a template
    return m_nodeParameterList->getNodePtr(n);
  }

  void NodeBlockClass::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt); //missing
    if(m_nodeNext) //may not have data members
      {
	NodeBlock::countNavNodes(cnt);

	if(cnt > 0)
	  {
	    std::ostringstream msg;
	    msg << cnt << " data members with unresolved types remain";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }

    cnt += m_functionST.countNavNodesAcrossTableOfFunctions();

    if(m_nodeParameterList)
      m_nodeParameterList->countNavNodes(cnt);
  } //countNavNodes

  void NodeBlockClass::checkDuplicateFunctions()
  {
    // check all the function names for duplicate definitions
    if(!isEmpty())
      m_functionST.checkTableOfFunctions(); //returns prob counts, outputs errs
  }

  void NodeBlockClass::calcMaxDepthOfFunctions()
  {
    // for all the function names, calculate their max depth
    if(!isEmpty())
      m_functionST.calcMaxDepthForTableOfFunctions(); //returns prob counts, outputs errs
  }

  bool NodeBlockClass::hasCustomArray()
  {
    bool hasCA = false;
    // custom array flag set at parse time
    UTI cuti = getNodeType();
    if(cuti == Int) //kludge nodetype clobbered to test
      cuti = m_state.getCompileThisIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    hasCA = ((UlamTypeClass *) cut)->isCustomArray();

    if(!hasCA)
      {
	if(m_state.isClassASubclass(cuti))
	  {
	    NodeBlockClass * superblock = (NodeBlockClass *) getPreviousBlockPointer();
	    assert(superblock);
	    hasCA = superblock->hasCustomArray();
	  }
      }
  return hasCA;
} //checkCustomArrayTypeFunctions

void NodeBlockClass::checkCustomArrayTypeFunctions()
  {
    if(isEmpty()) return;

    // custom array flag set at parse time
    UTI cuti = getNodeType();
    if(cuti == Int) //kludge nodetype clobbered to test
      cuti = m_state.getCompileThisIdx();

    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    if(((UlamTypeClass *) cut)->isCustomArray())
      m_functionST.checkCustomArrayTypeFuncs();

    if(m_state.isClassASubclass(cuti))
      {
	NodeBlockClass * superblock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superblock);
	superblock->checkCustomArrayTypeFunctions();
      }
  } //checkCustomArrayTypeFunctions

  UTI NodeBlockClass::getCustomArrayTypeFromGetFunction()
  {
    UTI catype = m_functionST.getCustomArrayReturnTypeGetFunction();

    if(catype == Nav)
      {
	UTI cuti = getNodeType();
	if(cuti == Int) //kludge nodetype clobbered to test
	  cuti = m_state.getCompileThisIdx();

	if(m_state.isClassASubclass(cuti))
	  {
	    NodeBlockClass * superblock = (NodeBlockClass *) getPreviousBlockPointer();
	    assert(superblock);
	    catype = superblock->getCustomArrayTypeFromGetFunction();
	  }
      }
    return catype;
  } //getCustomArrayTypeFromGetFunction

  u32 NodeBlockClass::getCustomArrayIndexTypeFromGetFunction(Node * rnode, UTI& idxuti, bool& hasHazyArgs)
  {
    UTI catype = m_functionST.getCustomArrayIndexTypeGetFunction(rnode, idxuti, hasHazyArgs);

    if(catype == Nav)
      {
	UTI cuti = getNodeType();
	if(cuti == Int) //kludge nodetype clobbered to test
	  cuti = m_state.getCompileThisIdx();

	if(m_state.isClassASubclass(cuti))
	  {
	    NodeBlockClass * superblock = (NodeBlockClass *) getPreviousBlockPointer();
	    assert(superblock);
	    catype = superblock->getCustomArrayIndexTypeFromGetFunction(rnode, idxuti, hasHazyArgs);
	  }
      }
    return catype;
  } //getCustomArrayIndexTypeFromGetFunction

  //starts here, called by SymbolClass
  bool NodeBlockClass::buildDefaultQuarkValue(u32& dqref)
  {
    bool aok = true;
    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superblock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superblock);
	aok = superblock->buildDefaultQuarkValue(dqref);
      }

    if(aok)
      if(m_nodeNext)
	aok = m_nodeNext->buildDefaultQuarkValue(dqref); //side-effect for dm vardecls
    return aok;
  } //buildDefaultQuarkValue

  EvalStatus NodeBlockClass::eval()
  {
    //    #define _DEBUG_SKIP_EVAL
#ifndef _DEBUG_SKIP_EVAL
    if(isEmpty())
#endif
      return NORMAL;

#if 0
    //determine size of stackframe (enable for test t3116)
    u32 stackframetotal = getSizeOfFuncSymbolsInTable();
    u32 numberoffuncs = getNumberOfFuncSymbolsInTable();
    {
      std::ostringstream msg;
      msg << stackframetotal << " is the total stackframe size required for ";
      msg << numberoffuncs << " functions";
      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
    }
#endif

    evalNodeProlog(0); //new current frame pointer for nodeeval stack

    EvalStatus evs = ERROR; //init

#if 1
    // NodeVarDecl's make UlamValue storage now, so don't want their
    // side-effects for the class definition, rather the instance.
    if(m_nodeNext)
      m_nodeNext->eval(); //side-effect for datamember vardecls
#endif

    // eval test method, if there's one:
    NodeBlockFunctionDefinition * funcNode = findTestFunctionNode();
    if(funcNode)
      {
	UTI saveClassType = getNodeType(); //temp!!
	setNodeType(Int); //for testing WHY? clobbers element/quark type
	//for Node::assignReturnValueToStack assertion (l334)
	UTI funcType = funcNode->getNodeType();

	makeRoomForNodeType(funcType); //Int return

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
  } //eval

  //override to check both variables and function names; not any super class.
  bool NodeBlockClass::isIdInScope(u32 id, Symbol * & symptrref)
  {
    return (m_ST.isInTable(id, symptrref) || isFuncIdInScope(id, symptrref));
  } //isIdInScope

  u32 NodeBlockClass::getNumberOfSymbolsInTable()
  {
    s32 supers = 0;
    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superClassBlock);
	supers = superClassBlock->getNumberOfSymbolsInTable();
      }

    return supers + m_ST.getTableSize();
  } //getNumberOfSymbolsInTable

  u32 NodeBlockClass::getSizeOfSymbolsInTable()
  {
    s32 supers = 0;
    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	supers = superClassBlock->getSizeOfSymbolsInTable();
      }
    return supers + m_ST.getTotalSymbolSize();
  } //getSizeOfSymbolsInTable

  s32 NodeBlockClass::getBitSizesOfVariableSymbolsInTable()
  {
    s32 superbs = 0;
    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superClassBlock);
	superbs = superClassBlock->getBitSizesOfVariableSymbolsInTable();
      }

    if(m_ST.getTableSize() == 0 && superbs == 0)
      return EMPTYSYMBOLTABLE; //should allow no variable data members

    if(superbs < 0)
      return superbs; //error

    s32 mybs = m_ST.getTotalVariableSymbolsBitSize();
    if(mybs < 0)
      return mybs; //error

    return superbs + mybs;
  } //getBitSizesOfVariableSymbolsInTable

  s32 NodeBlockClass::getMaxBitSizeOfVariableSymbolsInTable()
  {
    s32 superbs = 0;
    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superClassBlock);
	superbs = superClassBlock->getMaxBitSizeOfVariableSymbolsInTable();
      }
    if(m_ST.getTableSize() == 0 && superbs == 0)
      return EMPTYSYMBOLTABLE; //should allow no variable data members

    if(superbs < 0)
      return superbs; //error

    s32 mybs = m_ST.getMaxVariableSymbolsBitSize();
    if(mybs < 0)
      return mybs; //error

    return (superbs > mybs ? superbs : mybs); //return max
  } //getMaxBitSizeOfVariableSymbolsInTable

  s32 NodeBlockClass::findUlamTypeInTable(UTI utype)
  {
    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superClassBlock);
	s32 superpos = superClassBlock->findUlamTypeInTable(utype);
	if(superpos >= 0)
	  return superpos; //short-circuit
      }

    return m_ST.findPosOfUlamTypeInTable(utype);
  } //findUlamTypeInTable

  bool NodeBlockClass::isFuncIdInScope(u32 id, Symbol * & symptrref)
  {
    return m_functionST.isInTable(id, symptrref); // not including any superclass
  } //isFuncIdInScope

  void NodeBlockClass::addFuncIdToScope(u32 id, Symbol * symptr)
  {
    m_functionST.addToTable(id, symptr);
  }

  u32 NodeBlockClass::getNumberOfFuncSymbolsInTable()
  {
    s32 superfs = 0;
    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superClassBlock);
	superfs = superClassBlock->getNumberOfFuncSymbolsInTable();
      }
    return superfs + m_functionST.getTableSize();
  } //getNumberOfFuncSymbolsInTable

  u32 NodeBlockClass::getSizeOfFuncSymbolsInTable()
  {
    s32 superfs = 0;
    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	superfs = superClassBlock->getSizeOfFuncSymbolsInTable();
      }
    return superfs + m_functionST.getTotalSymbolSize();
  } //getSizeOfFuncSymbolsInTable

  void NodeBlockClass::updatePrevBlockPtrOfFuncSymbolsInTable()
  {
    m_functionST.updatePrevBlockPtrAcrossTableOfFunctions(this);
  }

  void NodeBlockClass::initElementDefaultsForEval(UlamValue& uv)
  {
    UTI buti = getNodeType();
    if(buti == Int) //kludge nodetype clobbered to test
      buti = m_state.getCompileThisIdx();
    if(m_state.isClassASubclass(buti))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superClassBlock);
	superClassBlock->initElementDefaultsForEval(uv);
      }
    return m_ST.initializeElementDefaultsForEval(uv);
  } //initElementDefaultsForEval

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
	if(((SymbolFunctionName *) fnSym)->findMatchingFunction(voidVector, funcSymbol) == 1)
	  {
	    func = funcSymbol->getFunctionNode();
	  }
      }
    return func;
  } //findTestFunctionNode()

  //don't set nextNode since it'll get deleted with program.
  NodeBlockFunctionDefinition * NodeBlockClass::findToIntFunctionNode()
  {
    Symbol * fnSym;
    NodeBlockFunctionDefinition * func = NULL;
    u32 tointid = m_state.m_pool.getIndexForDataString("toInt");
    if(isFuncIdInScope(tointid, fnSym))
      {
	SymbolFunction * funcSymbol = NULL;
	std::vector<UTI> voidVector;
	if(((SymbolFunctionName *) fnSym)->findMatchingFunction(voidVector, funcSymbol) == 1)
	  {
	    func = funcSymbol->getFunctionNode();
	  }
      }
    return func;
  } //findToIntFunctionNode()

  void NodeBlockClass::packBitsForVariableDataMembers()
  {
    if(m_ST.getTableSize() == 0) return;
    u32 offset = 0; //relative to ATOMFIRSTSTATEBITPOS

    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superblock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superblock);
	UTI superUTI = superblock->getNodeType();
	offset += m_state.getTotalBitSize(superUTI);
      }

    //m_ST.packBitsForTableOfVariableDataMembers(); //ST order not as declared
    if(m_nodeNext)
      m_nodeNext->packBitsInOrderOfDeclaration(offset);
  } //packBitsForVariableDataMembers

  u32 NodeBlockClass::countNativeFuncDecls()
  {
    if(isEmpty())
      return 0;
    return m_functionST.countNativeFuncDeclsForTableOfFunctions();
  }

  void NodeBlockClass::generateCodeForFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    // done earlier so that problems can be caught before continuing
    // check all the function names for duplicate definitions
    //m_functionST.checkTableOfFunctions(); //returns prob counts, outputs errs
    m_functionST.genCodeForTableOfFunctions(fp, declOnly, classtype);
  }

  //header .h file
  void NodeBlockClass::genCode(File * fp, UlamValue& uvpass)
  {
    //use the instance UTI instead of the node's original type
    UTI cuti = getNodeType(); //was m_state.getCompileThisIdx()
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
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
    else //element
      {
	genCodeHeaderElement(fp);
      }

    //gencode declarations only for all the function definitions
    bool declOnly = true;
    generateCodeForFunctions(fp, declOnly, classtype);

    generateCodeForBuiltInClassFunctions(fp, declOnly, classtype);

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("};\n"); //end of class/struct

    //declaration of THE_INSTANCE for ELEMENT
    if(classtype == UC_ELEMENT)
      {
	fp->write("\n");
	m_state.indent(fp);
	fp->write("template<class EC>\n");
	m_state.indent(fp);
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<EC> ");
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::THE_INSTANCE;\n\n");
      }

    //output any externs, outside of class decl
    genCodeExtern(fp, declOnly);

    m_state.m_currentIndentLevel = 0;
    fp->write("} //MFM\n\n");
    //leave class' endif to caller
  } //genCode

  void NodeBlockClass::genCodeExtern(File * fp, bool declOnly)
  {
    fp->write("\n");

    if(m_nodeNext)
      m_nodeNext->genCodeExtern(fp, declOnly);

    fp->write("\n");
  } //genCodeExtern

  void NodeBlockClass::genCodeHeaderQuark(File * fp)
  {
    //use the instance UTI instead of the node's original type
    UlamType * cut = m_state.getUlamTypeByIndex(m_state.getCompileThisIdx());

    m_state.indent(fp);
    fp->write("template <class EC, u32 POS>\n");

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write(" : public UlamClass");  //was tbd inheritance

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

    m_state.indent(fp);
    fp->write("typedef AtomicParameterType <EC, VD::BITS, QUARK_SIZE, POS> Up_Us; //entire quark\n");

    fp->write("\n");

    //DataMember VAR DECLS
    if(m_nodeNext)
      {
	UlamValue uvpass;
	m_nodeNext->genCode(fp, uvpass);  //output the BitField typedefs
	//NodeBlock::genCodeDeclsForVariableDataMembers(fp, classtype); //not in order declared
	fp->write("\n");
      }
    //default constructor/destructor
  } //genCodeHeaderQuark

  void NodeBlockClass::genCodeHeaderElement(File * fp)
  {
    //use the instance UTI instead of the node's original type
    UTI cuti = m_state.getCompileThisIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("class ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write(" : public UlamElement<EC>");  //was DefaultElement

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    genShortNameParameterTypesExtractedForHeaderFile(fp);

    fp->write("\n");
    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("public:\n\n");

    m_state.m_currentIndentLevel++;

    //default constructor/destructor; initializes UlamElement with MFM__UUID_FOR
    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("();\n");

    m_state.indent(fp);
    fp->write("~");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("();\n\n");

    m_state.indent(fp);
    fp->write("static ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write(" THE_INSTANCE;\n");

    //DataMember VAR DECLS
    if(m_nodeNext)
      {
	UlamValue uvpass;
	m_nodeNext->genCode(fp, uvpass);  //output the BitField typedefs
	fp->write("\n");
      }

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
    fp->write("typedef typename EC::ATOM_CONFIG AC;\n");
    m_state.indent(fp);
    fp->write("typedef typename AC::ATOM_TYPE T;\n");
    m_state.indent(fp);
    fp->write("enum { BPA = AC::BITS_PER_ATOM };\n");

    m_state.indent(fp);
    fp->write("typedef BitVector<BPA> BV;\n");

    fp->write("\n");
  } //genShortNameParameterTypesExtractedForHeaderFile

  //Body for This Class only; practically empty if quark (.tcc)
  void NodeBlockClass::genCodeBody(File * fp, UlamValue& uvpass)
  {
    //use the instance UTI instead of the node's original type
    UlamType * cut = m_state.getUlamTypeByIndex(m_state.getCompileThisIdx());
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
	fp->write("template<class EC>\n");

	m_state.indent(fp);
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<EC>");
	fp->write("::");
	fp->write(cut->getUlamTypeMangledName().c_str());

	std::string namestr = cut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state);
	std::string namestrlong = removePunct(cut->getUlamTypeMangledName());

	fp->write("() : UlamElement<EC>(MFM_UUID_FOR(\"");
	fp->write(namestrlong.c_str());
	fp->write("\", 0))\n");

	m_state.indent(fp);
	fp->write("{\n");

	m_state.m_currentIndentLevel++;

	m_state.indent(fp);
	fp->write("//XXXX  Element<EC>::SetAtomicSymbol(\"");
	fp->write(namestr.substr(0,1).c_str());
	fp->write("\");  // figure this out later\n");

	m_state.indent(fp);
	fp->write("Element<EC>::SetName(\"");
	fp->write(namestr.c_str());
	fp->write("\");\n");

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("}\n\n");

	//default destructor
	m_state.indent(fp);
	fp->write("template<class EC>\n");

	m_state.indent(fp);
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<EC>");
	fp->write("::~");
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("(){}\n\n");

	assert(m_state.getCompileThisId() == cut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId());
      }

    generateCodeForFunctions(fp, false, classtype);

    generateCodeForBuiltInClassFunctions(fp, false, classtype);

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //MFM\n\n");
  } //genCodeBody

  void NodeBlockClass::generateCodeForBuiltInClassFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    m_state.indent(fp);
    fp->write("//BUILT-IN FUNCTIONS:\n\n");

    // 'has' is for both class types
    genCodeBuiltInFunctionHas(fp, declOnly, classtype);
    genCodeBuiltInFunctionBuildDefaultAtom(fp, declOnly, classtype);

    //generate 3 UlamClass:: methods for smart ulam debugging
    u32 dmcount = 0; //pass ref
    generateUlamClassInfoFunction(fp, declOnly, dmcount);
    generateUlamClassInfoCount(fp, declOnly, dmcount); //after dmcount is updated by nodes
    generateUlamClassGetMangledName(fp, declOnly);

    // 'is' is only for element/classes
    if(classtype == UC_ELEMENT)
      generateInternalIsMethodForElement(fp, declOnly);
  } //generateCodeForBuiltInClassFunctions

  void NodeBlockClass::genCodeBuiltInFunctionHas(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    //'has' applies to both quarks and elements
    UTI cuti = m_state.getCompileThisIdx();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("//helper method not called directly\n");

	m_state.indent(fp);
	fp->write("s32 ");
	fp->write(m_state.getHasMangledFunctionName(cuti));
	fp->write("(const char * namearg) const;\n\n");
	return;
      }

    m_state.indent(fp);
    if(classtype == UC_ELEMENT)
      fp->write("template<class EC>\n");
    else if(classtype == UC_QUARK)
      fp->write("template<class EC, u32 POS>\n");
    else
      assert(0);

    m_state.indent(fp);
    fp->write("s32 "); //return pos offset, or -1 if not found

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    if(classtype == UC_ELEMENT)
      fp->write("<EC>");
    else if(classtype == UC_QUARK)
      fp->write("<EC, POS>");

    fp->write("::");
    fp->write(m_state.getHasMangledFunctionName(cuti));
    fp->write("(const char * namearg) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    genCodeBuiltInFunctionHasDataMembers(fp);

    fp->write("\n");
    m_state.indent(fp);
    fp->write("return ");
    fp->write("(-1);   //not found\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}  //has\n\n");
  } //genCodeBuiltInFunctionHas

  void NodeBlockClass::genCodeBuiltInFunctionHasDataMembers(File * fp)
  {
    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superClassBlock);
	superClassBlock->genCodeBuiltInFunctionHasDataMembers(fp);
      }
    m_ST.genCodeBuiltInFunctionHasOverTableOfVariableDataMember(fp);
  } //genCodeBuiltInFunctionHasDataMembers

  void NodeBlockClass::genCodeBuiltInFunctionBuildDefaultAtom(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    //'default atom' applies only to elements
    if(classtype == UC_QUARK)
      return genCodeBuiltInFunctionBuildDefaultQuark(fp, declOnly, classtype);;

    assert(classtype == UC_ELEMENT);

    UTI cuti = m_state.getCompileThisIdx();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual T ");
	fp->write(m_state.getBuildDefaultAtomFunctionName(cuti));
	fp->write("() const;\n\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    //fp->write("T "); //returns object of type T
    fp->write("typename EC::ATOM_CONFIG::ATOM_TYPE "); //returns object of type T

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    fp->write("<EC>");

    fp->write("::");
    fp->write(m_state.getBuildDefaultAtomFunctionName(cuti));
    fp->write("( ) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("T da = Element<EC>::BuildDefaultAtom();\n\n");

    m_state.indent(fp);
    fp->write("// Initialize any data members:\n");

    //get all initialized data members packed
    genCodeBuiltInFunctionBuildingDefaultDataMembers(fp);

    fp->write("\n");
    m_state.indent(fp);
    fp->write("return ");
    fp->write("(da);\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //BuildDefaultAtom\n\n");
  } //genCodeBuiltInFunctionBuildDefaultAtom

  void NodeBlockClass::genCodeBuiltInFunctionBuildingDefaultDataMembers(File * fp)
  {
    UTI nuti = getNodeType();
    if(m_state.isClassASubclass(nuti))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superClassBlock);
	superClassBlock->genCodeBuiltInFunctionBuildingDefaultDataMembers(fp);
      }

    m_ST.genCodeBuiltInFunctionBuildDefaultsOverTableOfVariableDataMember(fp, nuti);
  } //genCodeBuiltInFunctionBuildingDefaultDataMembers

  void NodeBlockClass::genCodeBuiltInFunctionBuildDefaultQuark(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    //return;
    assert(classtype == UC_QUARK);

    UTI cuti = m_state.getCompileThisIdx();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("static u32 ");
	fp->write(m_state.getBuildDefaultAtomFunctionName(cuti));
	fp->write("( );\n\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC, u32 POS>\n");

    m_state.indent(fp);
    fp->write("u32 ");

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    fp->write("<EC, POS>");

    fp->write("::");
    fp->write(m_state.getBuildDefaultAtomFunctionName(cuti));
    fp->write("( )\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //get all initialized data members in quark
    SymbolClass * csym = NULL;
    assert(m_state.alreadyDefinedSymbolClass(cuti, csym));
    u32 qval = 0;
    assert(csym->getDefaultQuark(qval));

    m_state.indent(fp);
    fp->write("return ");
    fp->write_decimal_unsigned(qval);
    fp->write(";\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //getDefaultQuark\n\n");
  } //genCodeBuiltInFunctionBuildDefaultQuark

  void NodeBlockClass::generateInternalIsMethodForElement(File * fp, bool declOnly)
  {
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
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("bool ");  //return pos offset, or -1 if not found

    UTI cuti = getNodeType();
    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());

    fp->write("<EC>::");
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
  } //generateInternalIsMethodForElement

  void NodeBlockClass::generateUlamClassInfoFunction(File * fp, bool declOnly, u32& dmcount)
  {
    UTI cuti = getNodeType();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    ULAMCLASSTYPE classtype = cut->getUlamClass();

    if(!declOnly)
      {
	m_state.indent(fp);
	if(classtype == UC_ELEMENT)
	  fp->write("template<class EC>\n");
	else if(classtype == UC_QUARK)
	  fp->write("template<class EC, u32 POS>\n");
	else
	  assert(0);
      }

    m_state.indent(fp);
    fp->write("const UlamClassDataMemberInfo & "); //return type

    if(!declOnly)
      {
	//include the mangled class::
	fp->write(cut->getUlamTypeMangledName().c_str());

	if(classtype == UC_QUARK)
	  fp->write("<EC, POS>");
	else
	  fp->write("<EC>");

	fp->write("::");
      }

    fp->write("GetDataMemberInfo(u32 dataMemberNumber) const"); //method name!!!

    if(!declOnly)
      {
	fp->write("\n");

	m_state.indent(fp);
	fp->write("{\n");

	m_state.m_currentIndentLevel++;
	m_state.indent(fp);
	fp->write("switch (dataMemberNumber)\n");
	m_state.indent(fp);
	fp->write("{\n");
	m_state.m_currentIndentLevel++;

	generateUlamClassInfo(fp, declOnly, dmcount);

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("} //end switch\n");

	m_state.indent(fp);
	fp->write("FAIL(ILLEGAL_ARGUMENT);\n");

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("} //GetDataMemberInfo\n\n"); //end of func
      }
    else
      {
	fp->write(";\n\n"); //end of declaration
      }
  } //generateUlamClassInfoFunction

  void NodeBlockClass::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superClassBlock);
	superClassBlock->generateUlamClassInfo(fp, declOnly, dmcount);
      }

    if(m_nodeNext)
      m_nodeNext->generateUlamClassInfo(fp, declOnly, dmcount);
  } //generateUlamClassInfo

  void NodeBlockClass::generateUlamClassInfoCount(File * fp, bool declOnly, u32 dmcount)
  {
    UTI cuti = getNodeType();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    ULAMCLASSTYPE classtype = cut->getUlamClass();

    if(!declOnly)
      {
	m_state.indent(fp);
	if(classtype == UC_ELEMENT)
	  fp->write("template<class EC>\n");
	else if(classtype == UC_QUARK)
	  fp->write("template<class EC, u32 POS>\n");
	else
	  assert(0);
      }

    m_state.indent(fp);
    fp->write("s32 "); //return type

    if(!declOnly)
      {
	//include the mangled class::
	fp->write(cut->getUlamTypeMangledName().c_str());

	if(classtype == UC_QUARK)
	  fp->write("<EC, POS>");
	else
	  fp->write("<EC>");

	fp->write("::");
      }

    fp->write("GetDataMemberCount() const"); //method name!!!

    if(!declOnly)
      {
	fp->write("\n");

	m_state.indent(fp);
	fp->write("{\n");

	m_state.m_currentIndentLevel++;
	m_state.indent(fp);

	fp->write("return ");
	fp->write_decimal(dmcount);
	fp->write(";\n");

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("} //GetDataMemberCount\n\n");
      }
    else
      {
	fp->write(";\n\n");
      }
  } //generateUlamClassInfoCount

  void NodeBlockClass::generateUlamClassGetMangledName(File * fp, bool declOnly)
  {
    UTI cuti = getNodeType();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    ULAMCLASSTYPE classtype = cut->getUlamClass();

    if(!declOnly)
      {
	m_state.indent(fp);
	if(classtype == UC_ELEMENT)
	  fp->write("template<class EC>\n");
	else if(classtype == UC_QUARK)
	  fp->write("template<class EC, u32 POS>\n");
	else
	  assert(0);
      }

    m_state.indent(fp);
    fp->write("const char * "); //return type

    if(!declOnly)
      {
	//include the mangled class::
	fp->write(cut->getUlamTypeMangledName().c_str());

	if(classtype == UC_QUARK)
	  fp->write("<EC, POS>");
	else
	  fp->write("<EC>");

	fp->write("::");
      }

    fp->write("GetMangledClassName() const"); //method name!!!

    if(!declOnly)
      {
	fp->write("\n");

	m_state.indent(fp);
	fp->write("{\n");

	m_state.m_currentIndentLevel++;
	m_state.indent(fp);

	fp->write("return \"");
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("\";\n");

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("} //GetMangledClassName\n\n");
      }
    else
      {
	fp->write(";\n\n");
      }
  } //generateUlamClassGetMangledName

  std::string NodeBlockClass::removePunct(std::string str)
  {
    std::string newstr("");
    for(u32 i = 0; i < str.length(); i++)
      {
	char c = str[i];
	if(!ispunct(c))
	  newstr.push_back(c);
      }
    return newstr;
  } //removePunct

  void NodeBlockClass::addClassMemberDescriptionsToInfoMap(ClassMemberMap& classmembers)
  {
    if(m_state.isClassASubclass(getNodeType()))
      {
	NodeBlockClass * superClassBlock = (NodeBlockClass *) getPreviousBlockPointer();
	assert(superClassBlock);
	superClassBlock->addClassMemberDescriptionsToInfoMap(classmembers);
      }

    NodeBlock::addClassMemberDescriptionsToInfoMap(classmembers); //Table of Classes request
    m_functionST.addClassMemberFunctionDescriptionsToMap(this->getNodeType(), classmembers); //Table of Classes request
  } //addClassMemberDescriptionsToInfoMap

} //end MFM

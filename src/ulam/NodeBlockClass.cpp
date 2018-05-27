#include <stdio.h>
#include "NodeBlockClass.h"
#include "NodeBlockFunctionDefinition.h"
#include "CompilerState.h"
#include "SymbolFunctionName.h"
#include "SymbolTypedef.h"

namespace MFM {

  NodeBlockClass::NodeBlockClass(NodeBlock * prevBlockNode, CompilerState & state) : NodeBlockContext(prevBlockNode, state), m_functionST(state), m_virtualmethodMaxIdx(UNKNOWNSIZE), m_superBlockNode(NULL), m_buildingDefaultValueInProgress(false), m_bitPackingInProgress(false), m_isEmpty(false), m_registeredForTestInstance(false)

  {
    m_nodeParameterList = new NodeList(state);
    assert(m_nodeParameterList);
    m_nodeArgumentList = new NodeList(state);
    assert(m_nodeArgumentList);
  }

  NodeBlockClass::NodeBlockClass(const NodeBlockClass& ref) : NodeBlockContext(ref), m_functionST(ref.m_functionST) /* deep copy */, m_virtualmethodMaxIdx(ref.m_virtualmethodMaxIdx), m_superBlockNode(NULL), m_buildingDefaultValueInProgress(false), m_bitPackingInProgress(false), m_isEmpty(ref.m_isEmpty), m_registeredForTestInstance(false), m_nodeParameterList(NULL), m_nodeArgumentList(NULL)
  {
    UTI cuti = m_state.getCompileThisIdx();
    m_state.pushClassContext(cuti, this, this, false, NULL); //t3895
    setNodeType(cuti); //t3895, after push
    m_nodeParameterList = new NodeList(m_state);
    assert(m_nodeParameterList);
    m_nodeArgumentList = new NodeList(m_state);
    assert(m_nodeArgumentList);
    m_state.popClassContext();
    //m_nodeParameterList = (NodeList *) ref.m_nodeParameterList->instantiate(); //instances don't need this; its got symbols
    //m_nodeArgumentList = (NodeList *) ref.m_nodeArgumentList->instantiate(); //instances only need this for constant array arguments; templates don't need this.
  }

  NodeBlockClass::~NodeBlockClass()
  {
    delete m_nodeParameterList;
    m_nodeParameterList = NULL;
    delete m_nodeArgumentList;
    m_nodeArgumentList = NULL;
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
    if(m_nodeArgumentList)
      m_nodeArgumentList->updateLineage(getNodeNo());
    m_functionST.linkToParentNodesAcrossTableOfFunctions(this); //all the function defs
  } //updateLineage

  bool NodeBlockClass::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeBlock::findNodeNo(n, foundNode))
      return true;

    if(m_nodeParameterList && m_nodeParameterList->findNodeNo(n, foundNode))
      return true;

    if(m_nodeArgumentList && m_nodeArgumentList->findNodeNo(n, foundNode))
      return true;

    if(m_functionST.findNodeNoAcrossTableOfFunctions(n, foundNode)) //all the function defs
      return true;

    UTI cuti = getNodeType();
    UTI superuti = m_state.isClassASubclass(cuti);
    if(m_state.okUTItoContinue(superuti) && !m_state.isUrSelf(superuti))
      {
	NodeBlockClass * superblock = getSuperBlockPointer();
	//e.g. not a stub, yet not complete because its superclass is a stub! (ish 06222016)
	// or is a stub! (t3887), or just incomplete (t41012)
	if(superblock && superblock->findNodeNo(n, foundNode))
	  return true;
      }
    return false;
  } //findNodeNo

  void NodeBlockClass::checkAbstractInstanceErrors()
  {
    NodeBlock::checkAbstractInstanceErrors();
    if(!isEmpty())
      m_functionST.checkAbstractInstanceErrorsAcrossTableOfFunctions();
  }

  void NodeBlockClass::setNodeLocation(Locator loc)
  {
    if(m_nodeParameterList)
      m_nodeParameterList->setNodeLocation(loc);
    if(m_nodeArgumentList)
      m_nodeArgumentList->setNodeLocation(loc);
    Node::setNodeLocation(loc);
  }

  void NodeBlockClass::resetNodeLocations(Locator loc)
  {
    if(m_nodeParameterList)
      m_nodeParameterList->resetNodeLocations(loc);
    if(m_nodeArgumentList)
      m_nodeArgumentList->resetNodeLocations(loc);
    Node::setNodeLocation(loc);
  }

  void NodeBlockClass::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myuti = getNodeType();

    char id[255];
    if((myuti == Nav) || (myuti == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else if(myuti == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n", prettyNodeName().c_str());
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
    UTI cuti = m_state.getCompileThisIdx(); //maybe be hzy template, getNodeType(); (t3565)
    assert(m_state.okUTItoContinue(cuti));

    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeUPrefix().c_str());  //e.g. Ue_Foo
    fp->write(getName());  //unmangled

    //output class template parameter type and name
    if(m_nodeParameterList->getNumberOfNodes() > 0)
      {
	SymbolClassNameTemplate * cnsym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameTemplate(m_state.getUlamTypeNameIdByIndex(cuti), cnsym);
	assert(isDefined);
	cnsym->printClassTemplateArgsForPostfix(fp); //m_nodeParameterList->print(fp);
      }

    UTI superuti = m_state.isClassASubclass(cuti);
    //skip UrSelf to avoid extensive changes to all test answers
    if(m_state.okUTItoContinue(superuti) && !m_state.isUrSelf(superuti))
      {
	fp->write(" : ");
	fp->write(m_state.getUlamTypeNameBriefByIndex(superuti).c_str());  //e.g. Foo(a), an instance of
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
	assert(m_state.okUTItoContinue(cuti));
	assert(m_state.isASeenClass(cuti)); //sanity check after eval (below)

	//simplifying assumption for testing purposes: center site
	Coord c0(0,0);
	s32 slot = c0.convertCoordToIndex();

	//has (most) current values impacted by test()
	printPostfixDataMembersSymbols(fp, slot, ATOMFIRSTSTATEBITPOS, cuti); //may not need classtype
	func->printPostfix(fp);
      }
    else
      {
	//has only initialized DM values, not current values
	printPostfixDataMembersParseTree(fp, cuti);
	fp->write(" <NOMAIN>"); //not an error
      }
    fp->write(" }");
    fp->write("\n");
  } //printPostfix

  void NodeBlockClass::printPostfixDataMembersParseTree(File * fp, UTI cuti)
  {
    //UTI cuti = getNodeType(); maybe Hzy, use arg instead
    if(m_state.isUrSelf(cuti)) return;

    UTI superuti = m_state.isClassASubclass(cuti);
    //skip UrSelf to avoid extensive changes to all test answers
    if(m_state.okUTItoContinue(superuti) && !m_state.isUrSelf(superuti))
      {
	NodeBlockClass * superblock = getSuperBlockPointer();
	if(!isSuperClassLinkReady(cuti))
	  {
	    //use SCN instead of SC in case of stub (use template's classblock)
	    SymbolClassName * supercnsym = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameByUTI(superuti, supercnsym);
	    assert(isDefined);
	    superblock = supercnsym->getClassBlockNode();
	    superuti = supercnsym->getUlamTypeIdx(); //in case of stub (t41007)
	  }
	assert(superblock);
	fp->write(" :<");
	superblock->printPostfixDataMembersParseTree(fp, superuti);
	fp->write(">");
      }

    if(m_nodeNext)
      m_nodeNext->printPostfix(fp); //datamember vardecls
  } //printPostfixDataMembersParseTree

  void NodeBlockClass::printPostfixDataMembersSymbols(File * fp, s32 slot, u32 startpos, UTI cuti)
  {
    //UTI cuti = getNodeType(); //maybe Hzy use arg instead
    UTI superuti = m_state.isClassASubclass(cuti);
    //skip UrSelf to avoid extensive changes to all test answers
    if(m_state.okUTItoContinue(superuti) && !m_state.isUrSelf(superuti))
      {
	NodeBlockClass * superblock = getSuperBlockPointer();
	assert(superblock && UlamType::compare(superblock->getNodeType(), superuti, m_state) == UTIC_SAME);
	fp->write(" :<");
	superblock->printPostfixDataMembersSymbols(fp, slot, startpos, superuti);
	fp->write(">");
      }
    m_ST.printPostfixValuesForTableOfVariableDataMembers(fp, slot, startpos, m_state.getUlamTypeByIndex(cuti)->getUlamClassType());
  } //printPostfixDataMembersSymbols

  void NodeBlockClass::noteClassTypeAndName(UTI nuti, s32 totalsize, u32& accumsize)
  {
    //called when superclass of an oversized class instance
    //UTI nuti = getNodeType(); //maybe Hzy, use arg instead

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nsize = nut->getTotalBitSize();

    std::ostringstream note;
    note << "(" << nsize << " of ";
    note << totalsize << " bits, at " << accumsize << ") ";
    note << "from superclass: " << nut->getUlamTypeClassNameBrief(nuti).c_str();
    MSG(getNodeLocationAsString().c_str(), note.str().c_str(), NOTE);

    accumsize += nsize;
  } //noteTypeAndName

  void NodeBlockClass::noteDataMembersParseTree(UTI cuti, s32 totalsize)
  {
    //UTI cuti = getNodeType(); maybe be Hzy, use callers arg
    if(m_state.isUrSelf(cuti)) return;

    u32 accumsize = 0;
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    std::ostringstream note;
    note << "Components of " << cut->getUlamTypeClassNameBrief(cuti).c_str() << " are."; //terminating double dot
    MSG(getNodeLocationAsString().c_str(), note.str().c_str(), NOTE);

    UTI superuti = m_state.isClassASubclass(cuti);
    //skip UrSelf to avoid extensive changes to all test answers
    if(m_state.okUTItoContinue(superuti) && !m_state.isUrSelf(superuti))
      {
	NodeBlockClass * superblock = getSuperBlockPointer();
	if(!isSuperClassLinkReady(cuti))
	  {
	    //use SCN instead of SC in case of stub (use template's classblock)
	    SymbolClassName * supercnsym = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameByUTI(superuti, supercnsym);
	    assert(isDefined);
	    superblock = supercnsym->getClassBlockNode();
	  }
	assert(superblock);
	superblock->noteClassTypeAndName(superuti, totalsize, accumsize); //no recursion
      }

    if(m_nodeNext)
      m_nodeNext->noteTypeAndName(totalsize, accumsize); //datamember vardecls
  } //noteDataMembersParseTree

  const char * NodeBlockClass::getName()
  {
    UTI cuti = getNodeType();
    if(!m_state.okUTItoContinue(cuti))
      cuti = m_state.getCompileThisIdx(); //maybe be hzy template, getNodeType(); (t3565)
    return m_state.getUlamKeyTypeSignatureByIndex(cuti).getUlamKeyTypeSignatureName(&m_state).c_str();
  }

  const std::string NodeBlockClass::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeBlockClass::getNodeType()
  {
    UTI cuti = Node::getNodeType();
    if(cuti == Int) //kludge nodetype clobbered to test eval
      cuti = m_state.getCompileThisIdx();
    return cuti;
  }

  bool NodeBlockClass::isAClassBlock()
  {
    return true; //used in searches for already defined symbol
  }

  NodeBlockClass * NodeBlockClass::getSuperBlockPointer()
  {
    return m_superBlockNode;
  }

  void NodeBlockClass::setSuperBlockPointer(NodeBlockClass * superblock)
  {
    m_superBlockNode = superblock;
  }

  bool NodeBlockClass::isSuperClassLinkReady(UTI cuti)
  {
    //call for known subclasses only
    UTI superuti = m_state.isClassASubclass(cuti);
    assert((superuti != Nouti) || !m_state.okUTItoContinue(cuti)); //t41013

    if(!m_state.okUTItoContinue(superuti))
      return false; //t41013

    //this is a subclass.
    NodeBlockClass * superblock = getSuperBlockPointer();
    if(superblock == NULL)
      return false;

    UTI sblockuti = superblock->getNodeType();
    return !(!m_state.okUTItoContinue(sblockuti) || (UlamType::compare(sblockuti, superuti, m_state) != UTIC_SAME));
  } //isSuperClassLinkReady

  bool NodeBlockClass::hasStringDataMembers()
  {
    bool hasStrings = NodeBlockContext::hasStringDataMembers(); //btw, does not check superclasses!!!
    if(!hasStrings)
      {
	UTI superuti = m_state.isClassASubclass(getNodeType());
	if((superuti != Nouti) && !m_state.isUrSelf(superuti))
	  {
	    NodeBlockClass * superblock = getSuperBlockPointer();
	    if(superblock != NULL)
	      hasStrings |= superblock->hasStringDataMembers();
	  }
      }
    return hasStrings;
  } //hasStringsDataMembers

  UTI NodeBlockClass::checkAndLabelType()
  {
    //do first, might be important!
    checkParameterNodeTypes();

    //Argument c&l handled during pending arg step of resolving loop
    //if(!checkArgumentNodeTypes())

    // Inheritance checks
    UTI nuti = m_state.getCompileThisIdx(); //may be Hzy getNodeType();
    UTI superuti = m_state.isClassASubclass(nuti);

    //skip the ancestor of a template
    if(m_state.okUTItoContinue(superuti))
      {
	if(m_state.isHolder(superuti) || !m_state.isComplete(superuti)) //t3874, t41010
	  {
	    UTI mappedUTI = superuti;
	    if(m_state.mappedIncompleteUTI(nuti, superuti, mappedUTI))
	      {
		//shouldn't happen, caught at parse time (t3900, t3901)
		assert(UlamType::compare(nuti, superuti, m_state) != UTIC_SAME);

		std::ostringstream msg;
		msg << "Substituting mapped UTI" << mappedUTI;
		msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
		msg << " for SUPERCLASS holder type: '";
		msg << m_state.getUlamTypeNameByIndex(superuti).c_str();
		msg << "' UTI" << superuti << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		//need to break the chain; e.g. don't want template symbol addresses used
		setSuperBlockPointer(NULL); //force to try again!! avoid inf loop
		superuti = mappedUTI;
		m_state.resetClassSuperclass(nuti, superuti);
		setNodeType(Hzy); //t41150
		m_state.setGoAgain();
		return Hzy; //short-circuit
	      }
	  }

	//this is a subclass.
	if(!isSuperClassLinkReady(nuti))
	  {
	    if(!m_state.isComplete(superuti))
	      {
		bool brtnhzy = true;
		std::ostringstream msg;
		msg << "Subclass '";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		msg << "' inherits from '";

		if(m_state.isClassAStub(superuti))
		  {
		    msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str();

		    msg << "', a class with pending arguments";
		  }
		else if(m_state.isHolder(superuti))
		  {
		    //O(n) search of localdefs for nicer error message (t3875, t41010)
		    u32 lostid = m_state.findTypedefNameIdInLocalsScopeByIndex(superuti);
		    if(lostid > 0)
		      {
			msg << m_state.m_pool.getDataAsString(lostid).c_str();
			msg << "', an unresolved local filescope typedef";
		      }
		    else
		      {
			msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str();
			msg << "', a holder class";
		      }
		  }
		else
		  {
		    msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str();
		    msg << "', an incomplete class";
		    brtnhzy = false; //t3889, t3831
		  }

		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		//need to break the chain; e.g. don't want template symbol addresses used
		setSuperBlockPointer(NULL); //force to try again!! avoid inf loop
		setNodeType(Hzy); //t41150
		m_state.setGoAgain();
		if(brtnhzy)
		  return Hzy; //short-circuit holders and stubs (e.g. t41010, t3831, t3889)
		//o.w. continue..
	      }

	    //look up this instance
	    SymbolClass * csym = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(superuti, csym);
	    assert(isDefined);
	    NodeBlockClass * superclassblock = csym->getClassBlockNode();
	    setSuperBlockPointer(superclassblock); //fixed

	    if(!isSuperClassLinkReady(nuti))
	      {
		setSuperBlockPointer(NULL); //force to try again!! avoid inf loop
		std::ostringstream msg;
		msg << "Subclass '";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		if(superclassblock->getNodeType() == Nav)
		  {
		    msg << "' inherits from illegal '";
		    msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str() << "'";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    setNodeType(Nav);
		    return Nav; //t41013, t3599
		  }
		else
		  {
		    msg << "' inherits from unready '";
		    msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str() << "'";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		    setNodeType(Hzy);
		    m_state.setGoAgain();
		    return Hzy;
		  }
	      }
	  } //super link ready or not

	setNodeType(nuti); //t41150

	assert(isSuperClassLinkReady(nuti));
	ULAMCLASSTYPE superclasstype = m_state.getUlamTypeByIndex(superuti)->getUlamClassType();
	ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(nuti)->getUlamClassType();
	if(classtype == UC_TRANSIENT)
	  {
	    //allow transients to inherit from either transients or quarks (t3723)
	    if((superclasstype != UC_TRANSIENT) && (superclasstype != UC_QUARK))
	      {
		std::ostringstream msg;
		msg << "Subclass '";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		msg << "' inherits from '";
		msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str();
		msg << "', a class that's neither a transient nor a quark"; //e.g. error/t3725
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav);
	      }
	  }
	else if(superclasstype != UC_QUARK)
	  {
	    //O(n) search of localdefs for nicer error message (t3875, t41010)
	    u32 lostid = m_state.findTypedefNameIdInLocalsScopeByIndex(superuti);
	    if(lostid > 0)
	      {
		std::ostringstream msg;
		msg << "Subclass '";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		msg << "' inherits from '";
		msg << m_state.m_pool.getDataAsString(lostid).c_str();
		msg << "', an unresolved local filescope typedef";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	    else
	      {
		//for all others (elements and quarks)
		//must be "seen" by now; e.g. typedef array of quarks (t3674), t3862, t41150
		std::ostringstream msg;
		msg << "Subclass '";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		msg << "' inherits from '";
		msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str();
		msg << "', a class that's not a quark";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	    setNodeType(Nav);
	  }
      } //done with inheritance checks, continue.

    //side-effect DataMember VAR DECLS
    if(m_nodeNext)
      m_nodeNext->checkAndLabelType();

    // label all the function definition bodies
    m_functionST.labelTableOfFunctions();

    // check that a 'test' function returns Int (ulam convention)
    checkTestFunctionReturnType();

    // check that a 'alengthof' function returns Unsigned (ulam convention)
    checkCustomArrayLengthofFunctionReturnType();

    // type already set during parsing
    return getNodeType();
  } //checkAndLabelType

  void NodeBlockClass::checkTestFunctionReturnType()
  {
    NodeBlockFunctionDefinition * funcNode = findTestFunctionNode();
    if(funcNode)
      {
	UTI funcType = funcNode->getNodeType();
	if(UlamType::compareForArgumentMatching(funcType, Int, m_state) == UTIC_NOTSAME)
	  {
	    std::ostringstream msg;
	    msg << "By convention, Function '" << funcNode->getName();
	    msg << "''s Return type must be 'Int', not ";
	    msg << m_state.getUlamTypeNameBriefByIndex(funcType).c_str();
	    MSG(funcNode->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    funcNode->setNodeType(Nav); //missing
	  }
      }
  } //checkTestFunctionReturnType

  void NodeBlockClass::checkCustomArrayLengthofFunctionReturnType()
  {
    NodeBlockFunctionDefinition * funcNode = findCustomArrayLengthofFunctionNode();
    if(funcNode)
      {
	UTI funcType = funcNode->getNodeType();
	if(UlamType::compareForArgumentMatching(funcType, Unsigned, m_state) == UTIC_NOTSAME)
	  {
	    std::ostringstream msg;
	    msg << "By convention, Function '" << funcNode->getName();
	    msg << "''s Return type must be 'Unsigned', not ";
	    msg << m_state.getUlamTypeNameBriefByIndex(funcType).c_str();
	    MSG(funcNode->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    funcNode->setNodeType(Nav); //missing
	  }
      }
  } //checkCustomArrayLengthofFunctionReturnType

  bool NodeBlockClass::checkParameterNodeTypes()
  {
    if(m_nodeParameterList)
      {
	UTI cuti = m_state.getCompileThisIdx(); //getNodeType() maybe Hzy
	u32 nparms = m_nodeParameterList->getNumberOfNodes();
	assert((nparms == 0) || m_state.isClassATemplate(cuti));
	m_nodeParameterList->checkAndLabelType();
	//delay template parameter type check for instance argument type check
	//since potential problems may still be Hazy. (t3894,5,8)
      }
    return true;
  }

  void NodeBlockClass::addParameterNode(Node * nodeArg)
  {
    assert(m_nodeParameterList); //must be a template
    assert(m_state.isClassATemplate(getNodeType()));
    m_nodeParameterList->addNodeToList(nodeArg);
  }

  Node * NodeBlockClass::getParameterNode(u32 n)
  {
    assert(m_nodeParameterList); //must be a template
    //assert(m_state.isClassATemplate(getNodeType()));
    return m_nodeParameterList->getNodePtr(n);
  }

  u32 NodeBlockClass::getNumberOfParameterNodes()
  {
    assert(m_nodeParameterList); //must be a template
    //assert(m_state.isClassATemplate(getNodeType()));
    return m_nodeParameterList->getNumberOfNodes();
  }

  bool NodeBlockClass::checkArgumentNodeTypes()
  {
    const s32 MAX_CLASS_PARAMETER_ARRAY_LENGTH = 16;
    //unlike Parameter Nodes, the argument Nodes are c&l during
    // the resolving loop (t3894, t3895, t3898).
    if(m_nodeArgumentList)
      {
	UTI nuti = getNodeType();
	u32 n = m_nodeArgumentList->getNumberOfNodes();
	assert((n == 0) || !m_state.isClassATemplate(nuti));

	for(u32 i = 0; i < n; i++)
	  {
	    UTI auti = m_nodeArgumentList->getNodeType(i);
	    if(m_state.okUTItoContinue(auti))
	      {
		UlamType * aut = m_state.getUlamTypeByIndex(auti);
		if(aut->getBitSize() > MAXBITSPERINT)
		  {
		    std::ostringstream msg;
		    msg << "Class argument types are limited to ";
		    msg << MAXBITSPERINT << " bits by MFM::UlamTypeInfo: ";
		    msg << aut->getUlamTypeName().c_str() << " (arg " << i+1 << ") in class ";
		    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    m_nodeArgumentList->setNodeType(Nav);
		  }
		if(!aut->isScalar())
		  {
		    if(UlamType::compareForString(auti, m_state) == UTIC_SAME)
		      {
			std::ostringstream msg;
			msg << "Class arguments of type String are limited to scalars ";
			msg << "by MFM::UlamTypeInfo: ";
			msg << aut->getUlamTypeName().c_str() << " (arg " << i+1 << ") in class ";
			msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			m_nodeArgumentList->setNodeType(Nav);
		      }

		    if(aut->getArraySize() > MAX_CLASS_PARAMETER_ARRAY_LENGTH)
		      {
			std::ostringstream msg;
			msg << "Class argument array types are limited to ";
			msg << MAX_CLASS_PARAMETER_ARRAY_LENGTH;
			msg << " items by MFM::UlamTypeInfo: ";
			msg << aut->getUlamTypeName().c_str() << " (arg " << i+1 << ") in class ";
			msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			m_nodeArgumentList->setNodeType(Nav);
		      }
		  }
	      }
	  }

	u32 navcnt, hzycnt, nocnt;
	navcnt = hzycnt = nocnt = 0;
	m_nodeArgumentList->countNavHzyNoutiNodes(navcnt, hzycnt, nocnt);
	return (navcnt + hzycnt == 0);
      }
    return true;
  }

  void NodeBlockClass::addArgumentNode(Node * nodeArg)
  {
    assert(m_nodeArgumentList); //must be a template class instance
    assert(!m_state.isClassATemplate(getNodeType()));
    m_nodeArgumentList->addNodeToList(nodeArg);
  }

  Node * NodeBlockClass::getArgumentNode(u32 n)
  {
    assert(m_nodeArgumentList); //must be a template class instance
    assert(!m_state.isClassATemplate(getNodeType()));
    return m_nodeArgumentList->getNodePtr(n);
  }

  u32 NodeBlockClass::getNumberOfArgumentNodes()
  {
    assert(m_nodeArgumentList); //must be a template class instance
    assert(!m_state.isClassATemplate(getNodeType()));
    return m_nodeArgumentList->getNumberOfNodes();
  }

  void NodeBlockClass::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt); //missing

    if(m_nodeNext) //may not have data members
      NodeBlock::countNavHzyNoutiNodes(ncnt, hcnt, nocnt); //traverses the tree

    m_functionST.countNavNodesAcrossTableOfFunctions(ncnt, hcnt, nocnt);

    if(m_nodeParameterList)
      m_nodeParameterList->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);

    if(m_nodeArgumentList)
      m_nodeArgumentList->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  u32 NodeBlockClass::checkDuplicateFunctions()
  {
    //starts here with fresh map!
    u32 probcount = 0;
    std::map<std::string, UTI> mangledFunctionMapWithReturnType;

    // check all the function names for duplicate definitions
    if(!isEmpty())
      {
	//first check own table for duplicate function definitions
	//populate map, updates probcount, output errors
	m_functionST.checkTableOfFunctions(mangledFunctionMapWithReturnType, probcount);

	//check all ancestors for matching function definitions
	UTI nuti = getNodeType();
	UTI superuti = m_state.isClassASubclass(nuti);
	if(superuti != Nouti)
	  {
	    NodeBlockClass * superClassBlock = getSuperBlockPointer();
	    assert(superClassBlock);
	    m_state.pushClassContext(superuti, superClassBlock, superClassBlock, false, NULL);
	    superClassBlock->checkMatchingFunctionsInAncestors(mangledFunctionMapWithReturnType, probcount);
	    m_state.popClassContext(); //restore
	  }
      }
    mangledFunctionMapWithReturnType.clear(); //strings only
    return probcount;
  } //checkDuplicateFunctions

  void NodeBlockClass::checkMatchingFunctionsInAncestors(std::map<std::string, UTI>& mangledFunctionMap, u32& probcount)
  {
    // check all the function names for matches, okay if return type the same (regardless if virtual or not)
    if(!isEmpty())
      {
	m_functionST.checkTableOfFunctionsInAncestor(mangledFunctionMap, probcount); //populate map, update probcount, output errors

	//check next ancestor in turn
	UTI nuti = getNodeType();
	UTI superuti = m_state.isClassASubclass(nuti);
	if(superuti != Nouti)
	  {
	    NodeBlockClass * superClassBlock = getSuperBlockPointer();
	    assert(superClassBlock);
	    m_state.pushClassContext(superuti, superClassBlock, superClassBlock, false, NULL);
	    superClassBlock->checkMatchingFunctionsInAncestors(mangledFunctionMap, probcount);
	    m_state.popClassContext(); //restore
	  }
      }
  } //checkMatchingFunctionsInAncestors

  void NodeBlockClass::calcMaxDepthOfFunctions()
  {
    // for all the function names, calculate their max depth
    if(!isEmpty())
      m_functionST.calcMaxDepthForTableOfFunctions(); //returns prob counts, outputs errs
  }

  void NodeBlockClass::calcMaxIndexOfVirtualFunctions()
  {
    UTI nuti = getNodeType();
    UTI superuti = m_state.isClassASubclass(nuti);
    s32 maxidx = getVirtualMethodMaxIdx();

    if(maxidx != UNKNOWNSIZE)
      return; //short-circuit, re-called if any subclass is waiting on a super

    assert(superuti != Hzy);
    if(superuti != Nouti)
      {
	NodeBlockClass * superblock = getSuperBlockPointer();
	if(!superblock)
	  return; //error after all the iterations

	maxidx = superblock->getVirtualMethodMaxIdx();
	if(maxidx < 0)
	  {
	    std::ostringstream msg;
	    msg << "Subclass '";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << "' inherits from '";
	    msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str();
	    msg << "', a class whose max index for virtual functions is still unknown";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    return;
	  }
      }

    // for all the virtual function names, calculate their index in the VM Table
    m_functionST.calcMaxIndexForVirtualTableOfFunctions(maxidx);
    setVirtualMethodMaxIdx(maxidx);
  } //calcMaxIndexOfVirtualFunctions

  s32 NodeBlockClass::getVirtualMethodMaxIdx()
  {
    return m_virtualmethodMaxIdx;
  }

  void NodeBlockClass::setVirtualMethodMaxIdx(s32 maxidx)
  {
    m_virtualmethodMaxIdx = maxidx;
  }

  u32 NodeBlockClass::getLocalsFilescopeType()
  {
    UTI locuti = Nouti;
    NodeBlockLocals * localsblock = m_state.getLocalsScopeBlock(getNodeLocation());
    if(localsblock)
      {
	locuti = localsblock->getNodeType();
	assert(m_state.okUTItoContinue(locuti));
      }
    return locuti;
  }

  bool NodeBlockClass::hasCustomArray()
  {
    bool hasCA = false;
    // custom array flag set at parse time
    UTI cuti = getNodeType();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    hasCA = ((UlamTypeClass *) cut)->isCustomArray();

    if(!hasCA)
      {
	UTI superuti = m_state.isClassASubclass(cuti);
	if(m_state.okUTItoContinue(superuti))
	  {
	    NodeBlockClass * superblock = getSuperBlockPointer();
	    if(!superblock) //might be during resolving loop, not set yet
	      {
		SymbolClass * supercsym = NULL;
		AssertBool isDefined = m_state.alreadyDefinedSymbolClass(superuti, supercsym);
		assert(isDefined);
		superblock = supercsym->getClassBlockNode();
		assert(superblock);
	      }
	    return superblock->hasCustomArray();
	  }
      }
    return hasCA;
  } //hasCustomArray

void NodeBlockClass::checkCustomArrayTypeFunctions()
  {
    if(isEmpty()) return;

    // custom array flag set at parse time
    UTI cuti = getNodeType();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    if(((UlamTypeClass *) cut)->isCustomArray())
      m_functionST.checkCustomArrayTypeFuncs();

    UTI superuti = m_state.isClassASubclass(cuti);
    assert(superuti != Hzy);
    if(superuti != Nouti)
      {
	NodeBlockClass * superblock = getSuperBlockPointer();
	if(!superblock)
	  {
	    std::ostringstream msg;
	    msg << "Subclass '";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << "' inherits from '";
	    msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str();
	    msg << "', an INCOMPLETE Super class; ";
	    msg << "No check of custom array type functions";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return;
	  }
	superblock->checkCustomArrayTypeFunctions();
      }
  } //checkCustomArrayTypeFunctions

  UTI NodeBlockClass::getCustomArrayTypeFromGetFunction()
  {
    UTI catype = m_functionST.getCustomArrayReturnTypeGetFunction();
    if(catype == Nouti)
      {
	UTI cuti = getNodeType();
	if(m_state.isClassAStub(cuti))
	  {
	    //search template
	    NodeBlockClass * prevblock = (NodeBlockClass *) getPreviousBlockPointer();
	    assert(prevblock);
	    catype = prevblock->getCustomArrayTypeFromGetFunction();
	  }

	if((catype == Nouti) && (m_state.isClassASubclass(cuti) != Nouti))
	  {
	    NodeBlockClass * superblock = getSuperBlockPointer();
	    assert(superblock);
	    catype = superblock->getCustomArrayTypeFromGetFunction();
	  }
      }
    return catype;
  } //getCustomArrayTypeFromGetFunction

  u32 NodeBlockClass::getCustomArrayIndexTypeFromGetFunction(Node * rnode, UTI& idxuti, bool& hasHazyArgs)
  {
    u32 camatches = m_functionST.getCustomArrayIndexTypeGetFunction(rnode, idxuti, hasHazyArgs);
    if(camatches == 0)
      {
	UTI cuti = getNodeType();
	if(m_state.isClassAStub(cuti))
	  {
	    //search template
	    NodeBlockClass * prevblock = (NodeBlockClass *) getPreviousBlockPointer();
	    assert(prevblock);
	    camatches = prevblock->getCustomArrayIndexTypeFromGetFunction(rnode, idxuti, hasHazyArgs);
	  }

	if((camatches == 0) && (m_state.isClassASubclass(cuti) != Nouti))
	  {
	    NodeBlockClass * superblock = getSuperBlockPointer();
	    assert(superblock);
	    camatches = superblock->getCustomArrayIndexTypeFromGetFunction(rnode, idxuti, hasHazyArgs);
	  }
      }
    //else if(catype == Hzy)
    //  hasHazyArgs = true;
    return camatches;
  } //getCustomArrayIndexTypeFromGetFunction

  bool NodeBlockClass::hasCustomArrayLengthofFunction()
  {
    bool camatch = m_functionST.hasCustomArrayLengthofFunction();
    if(!camatch)
      {
	UTI cuti = getNodeType();
	if(m_state.isClassAStub(cuti))
	  {
	    //search template
	    NodeBlockClass * prevblock = (NodeBlockClass *) getPreviousBlockPointer();
	    assert(prevblock);
	    camatch = prevblock->hasCustomArrayLengthofFunction();
	  }

	if((!camatch) && (m_state.isClassASubclass(cuti) != Nouti))
	  {
	    NodeBlockClass * superblock = getSuperBlockPointer();
	    assert(superblock);
	    camatch = superblock->hasCustomArrayLengthofFunction();
	  }
      }
    return camatch;
  } //hasCustomArrayLengthofFunction

  //starts here, called by SymbolClass
  bool NodeBlockClass::buildDefaultValue(u32 wlen, BV8K& dvref)
  {
    if(m_buildingDefaultValueInProgress)
      return false;

    m_buildingDefaultValueInProgress = true; //set

    bool aok = true;
    if((m_state.isClassASubclass(getNodeType()) != Nouti))
      {
	NodeBlockClass * superblock = getSuperBlockPointer();
	assert(superblock);
	aok = superblock->buildDefaultValue(wlen, dvref);
      }

    if(aok)
      if(m_nodeNext)
	aok = m_nodeNext->buildDefaultValue(wlen, dvref); //side-effect for dm vardecls

    m_buildingDefaultValueInProgress = false; //clear
    return aok;
  } //buildDefaultValue

  bool NodeBlockClass::buildDefaultValueForClassConstantDefs()
  {
    if(classConstantsReady())
      return true;

    bool aok = true;
    if((m_state.isClassASubclass(getNodeType()) != Nouti))
      {
	NodeBlockClass * superblock = getSuperBlockPointer();
	assert(superblock);
	aok = superblock->buildDefaultValueForClassConstantDefs();
      }

    if(aok)
      if(m_nodeNext)
	aok = m_nodeNext->buildDefaultValueForClassConstantDefs();

    if(aok)
      m_classConstantsReady = true;

    return aok;
  } //buildDefaultValueForClassConstantDefs

  void NodeBlockClass::genCodeDefaultValueOrTmpVarStringRegistrationNumber(File * fp, u32 startpos, const UVPass * const uvpassptr, const BV8K * const bv8kptr)
  {
    ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(getNodeType())->getUlamClassType();
    if(classtype == UC_ELEMENT)
      startpos += ATOMFIRSTSTATEBITPOS; //t3972

    if((m_state.isClassASubclass(getNodeType()) != Nouti))
      {
	NodeBlockClass * superblock = getSuperBlockPointer();
	assert(superblock);
	superblock->genCodeDefaultValueOrTmpVarStringRegistrationNumber(fp, startpos, uvpassptr, bv8kptr);
      }

    if(m_nodeNext)
      m_nodeNext->genCodeDefaultValueOrTmpVarStringRegistrationNumber(fp, startpos, uvpassptr, bv8kptr); //side-effect for dm vardecls
  }

  void NodeBlockClass::genCodeElementTypeIntoDataMemberDefaultValueOrTmpVar(File * fp, u32 startpos, const UVPass * const uvpassptr)
  {
    if((m_state.isClassASubclass(getNodeType()) != Nouti))
      {
	NodeBlockClass * superblock = getSuperBlockPointer();
	assert(superblock);
	superblock->genCodeElementTypeIntoDataMemberDefaultValueOrTmpVar(fp, startpos, uvpassptr);
      }

    if(m_nodeNext)
      m_nodeNext->genCodeElementTypeIntoDataMemberDefaultValueOrTmpVar(fp, startpos, uvpassptr); //side-effect for dm vardecls
  } //genCodeElementTypeIntoDataMemberDefaultValueOrTmpVar

  EvalStatus NodeBlockClass::eval()
  {
    //#define _DEBUG_SKIP_EVAL
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

    m_nodeArgumentList->eval();

    // NodeVarDecl's make UlamValue storage now, so don't want their
    // side-effects for the class definition, rather the instance.
    if(m_nodeNext)
      m_nodeNext->eval(); //side-effect for datamember vardecls

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
	if(evs != NORMAL) return evalStatusReturn(evs);

	UlamValue testUV = m_state.m_nodeEvalStack.popArg();
	Node::assignReturnValueToStack(testUV);
	setNodeType(saveClassType); //temp, restore
      }
    evalNodeEpilog();
    return NORMAL;
  } //eval

  //override to check both variables and function names; not any super class.
  bool NodeBlockClass::isIdInScope(u32 id, Symbol * & symptrref)
  {
    return (m_ST.isInTable(id, symptrref) || isFuncIdInScope(id, symptrref));
  }

  u32 NodeBlockClass::getNumberOfSymbolsInTable()
  {
    s32 supers = 0;
    if(m_state.isClassASubclass(getNodeType()) != Nouti)
      {
	NodeBlockClass * superClassBlock = getSuperBlockPointer();
	assert(superClassBlock);
	supers = superClassBlock->getNumberOfSymbolsInTable();
      }

    return supers + NodeBlock::getNumberOfSymbolsInTable();
  } //getNumberOfSymbolsInTable

  u32 NodeBlockClass::getNumberOfPotentialClassArgumentSymbols()
  {
    return getNumberOfArgumentNodes();
  }

  u32 NodeBlockClass::getSizeOfSymbolsInTable()
  {
    s32 supers = 0;
    if(m_state.isClassASubclass(getNodeType()) != Nouti)
      {
	NodeBlockClass * superClassBlock = getSuperBlockPointer();
	supers = superClassBlock->getSizeOfSymbolsInTable();
      }
    return supers + m_ST.getTotalSymbolSize();
  } //getSizeOfSymbolsInTable

  s32 NodeBlockClass::getBitSizesOfVariableSymbolsInTable()
  {
    UTI cuti = m_state.getCompileThisIdx(); //getNodeType() maybe Hzy
    s32 superbs = 0;
    UTI superuti = m_state.isClassASubclass(cuti);
    assert(superuti != Hzy);
    if(superuti != Nouti)
      superbs = m_state.getBitSize(superuti);

    if(superbs < 0)
      return superbs; //error

    if(m_ST.getTableSize() == 0 && superbs == 0)
      return EMPTYSYMBOLTABLE; //should allow no variable data members

    s32 mybs = m_ST.getTotalVariableSymbolsBitSize();
    if(mybs < 0)
      return mybs; //error

    return superbs + mybs;
  } //getBitSizesOfVariableSymbolsInTable

  s32 NodeBlockClass::getMaxBitSizeOfVariableSymbolsInTable()
  {
    UTI cuti = m_state.getCompileThisIdx(); //getNodeType() maybe Hzy
    s32 superbs = 0;
    UTI superuti = m_state.isClassASubclass(cuti);
    assert(superuti != Hzy);
    if(superuti != Nouti)
      {
	NodeBlockClass * superClassBlock = getSuperBlockPointer();
	assert(superClassBlock);
	m_state.pushClassContext(superuti, superClassBlock, superClassBlock, false, NULL);
	superbs = superClassBlock->getMaxBitSizeOfVariableSymbolsInTable();
	m_state.popClassContext(); //restore
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

  bool NodeBlockClass::isFuncIdInScope(u32 id, Symbol * & symptrref)
  {
    return m_functionST.isInTable(id, symptrref); //not including any superclass
  }

  void NodeBlockClass::addFuncIdToScope(u32 id, Symbol * symptr)
  {
    m_functionST.addToTable(id, symptr);
  }

  u32 NodeBlockClass::getNumberOfFuncSymbolsInTable()
  {
    s32 superfs = 0;
    if(m_state.isClassASubclass(getNodeType()) != Nouti)
      {
	NodeBlockClass * superClassBlock = getSuperBlockPointer();
	assert(superClassBlock);
	superfs = superClassBlock->getNumberOfFuncSymbolsInTable();
      }
    return superfs + m_functionST.getTableSize();
  } //getNumberOfFuncSymbolsInTable

  u32 NodeBlockClass::getSizeOfFuncSymbolsInTable()
  {
    s32 superfs = 0;
    if(m_state.isClassASubclass(getNodeType()) != Nouti)
      {
	NodeBlockClass * superClassBlock = getSuperBlockPointer();
	superfs = superClassBlock->getSizeOfFuncSymbolsInTable();
      }
    return superfs + m_functionST.getTotalSymbolSize();
  } //getSizeOfFuncSymbolsInTable

  void NodeBlockClass::updatePrevBlockPtrOfFuncSymbolsInTable()
  {
    m_functionST.updatePrevBlockPtrAcrossTableOfFunctions(this);
  }

  void NodeBlockClass::initElementDefaultsForEval(UlamValue& uv, UTI cuti)
  {
    UTI buti = getNodeType();

    UlamType * but = m_state.getUlamTypeByIndex(buti);
    if((but->getUlamClassType() == UC_TRANSIENT) && (but->getTotalBitSize() > MAXSTATEBITS))
      return; //cannot do eval on a huge transient

    UTI superuti = m_state.isClassASubclass(buti);
    assert(superuti != Hzy);
    if(superuti != Nouti)
      {
	NodeBlockClass * superClassBlock = getSuperBlockPointer();
	assert(superClassBlock);
	m_state.pushClassContext(superuti, superClassBlock, superClassBlock, false, NULL);
	superClassBlock->initElementDefaultsForEval(uv, cuti);
	m_state.popClassContext(); //restore
      }
    return m_ST.initializeElementDefaultsForEval(uv, cuti);
  } //initElementDefaultsForEval

  //don't set nextNode since it'll get deleted with program.
  NodeBlockFunctionDefinition * NodeBlockClass::findTestFunctionNode()
  {
    Symbol * fnSym = NULL;
    NodeBlockFunctionDefinition * func = NULL;
    u32 testid = m_state.m_pool.getIndexForDataString("test");
    if(isFuncIdInScope(testid, fnSym))
      {
	SymbolFunction * funcSymbol = NULL;
	if(((SymbolFunctionName *) fnSym)->findMatchingFunctionStrictlyVoid(funcSymbol) == 1)
	  func = funcSymbol->getFunctionNode();
      }
    return func;
  } //findTestFunctionNode

  //don't set nextNode since it'll get deleted with program.
  NodeBlockFunctionDefinition * NodeBlockClass::findCustomArrayLengthofFunctionNode()
  {
    Symbol * fnSym = NULL;
    NodeBlockFunctionDefinition * func = NULL;
    u32 lenid = m_state.getCustomArrayLengthofFunctionNameId();
    if(isFuncIdInScope(lenid, fnSym))
      {
	SymbolFunction * funcSymbol = NULL;
	if(((SymbolFunctionName *) fnSym)->findMatchingFunctionStrictlyVoid(funcSymbol) == 1)
	  func = funcSymbol->getFunctionNode();
      }
    return func;
  } //findCustomArrayLengthofFunctionNode

  //don't set nextNode since it'll get deleted with program.
  NodeBlockFunctionDefinition * NodeBlockClass::findToIntFunctionNode()
  {
    Symbol * fnSym = NULL;
    NodeBlockFunctionDefinition * func = NULL;
    u32 tointid = m_state.m_pool.getIndexForDataString("toInt");
    if(isFuncIdInScope(tointid, fnSym))
      {
	SymbolFunction * funcSymbol = NULL;
	if(((SymbolFunctionName *) fnSym)->findMatchingFunctionStrictlyVoid(funcSymbol) == 1)
	  func = funcSymbol->getFunctionNode();
      }
    return func;
  } //findToIntFunctionNode

  TBOOL NodeBlockClass::packBitsForVariableDataMembers()
  {
    if(m_ST.getTableSize() == 0) return TBOOL_TRUE;

    if(m_bitPackingInProgress)
      return TBOOL_HAZY; //or false?

    u32 reloffset = 0;

    UTI nuti = getNodeType();
    UTI superuti = m_state.isClassASubclass(nuti);
    //assert(superuti != Hzy);
    if(superuti == Hzy)
      {
	return TBOOL_HAZY;
      }

    if(superuti != Nouti)
      {
	NodeBlockClass * superblock = getSuperBlockPointer();
	if(!superblock)
	  {
	    std::ostringstream msg;
	    msg << "Subclass '";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << "' inherits from '";
	    msg << m_state.getUlamTypeNameBriefByIndex(superuti).c_str();
	    msg << "', an INCOMPLETE Super class; ";
	    msg << "No bit packing of variable data members";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    return TBOOL_HAZY;
	  }

	assert(UlamType::compare(superblock->getNodeType(), superuti, m_state) == UTIC_SAME);
	u32 superoffset = m_state.getTotalBitSize(superuti);
	if(superoffset < 0)
	  {
	    return TBOOL_HAZY;
	  }
	reloffset += superoffset;
      }

    TBOOL rtntb = TBOOL_TRUE;
    m_bitPackingInProgress = true;;

    //m_ST.packBitsForTableOfVariableDataMembers(); //ST order not as declared
    if(m_nodeNext)
      {
	TBOOL nodetb = m_nodeNext->packBitsInOrderOfDeclaration(reloffset);
	rtntb = Node::minTBOOL(rtntb, nodetb);
      }

    m_bitPackingInProgress = false;
    return rtntb;
  } //packBitsForVariableDataMembers

  void NodeBlockClass::printUnresolvedVariableDataMembers()
  {
    if(m_nodeNext)
      m_nodeNext->printUnresolvedVariableDataMembers();

    m_functionST.printUnresolvedLocalVariablesForTableOfFunctions();
  }

  void NodeBlockClass::printUnresolvedLocalVariables(u32 fid)
  {
    m_state.abortShouldntGetHere(); //override
  }

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
    m_functionST.genCodeForTableOfFunctions(fp, declOnly, classtype);
  }

  //header .h file
  void NodeBlockClass::genCode(File * fp, UVPass& uvpass)
  {
    //use the instance UTI instead of the node's original type
    UTI cuti = getNodeType(); //was m_state.getCompileThisIdx()
    assert(m_state.okUTItoContinue(cuti));

    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    ULAMCLASSTYPE classtype = cut->getUlamClassType();
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
    else if(classtype == UC_ELEMENT)
      {
	genCodeHeaderElement(fp);
      }
    else if(classtype == UC_TRANSIENT)
      {
	genCodeHeaderTransient(fp);
      }
    else if(classtype == UC_LOCALSFILESCOPE)
      {
	genCodeHeaderLocalsFilescope(fp);
      }
    else
      m_state.abortUndefinedUlamClassType();

    //gencode declarations only for all the function definitions
    bool declOnly = true;
    generateCodeForFunctions(fp, declOnly, classtype);

    generateCodeForBuiltInClassFunctions(fp, declOnly, classtype);

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("};\n"); //end of class/struct

    //declaration of THE_INSTANCE for ELEMENTs and Quarks (same)
    fp->write("\n");
    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC> ");
    fp->write(m_state.getTheInstanceMangledNameByIndex(cuti).c_str());
    fp->write(";\n\n");

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
    UTI cuti = m_state.getCompileThisIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    m_state.indent(fp);
    fp->write("template <class EC>"); GCNL;

    m_state.indent(fp);
    fp->write("struct ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write(" : public UlamQuark<EC>");

    genThisUlamSuperClassAsAHeaderComment(fp);

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
    fp->write("};"); GCNL;
    fp->write("\n");

    m_state.indent(fp);
    fp->write("typedef UlamRefFixed<EC, 0u, QUARK_SIZE> Up_Us; //entire quark"); GCNL; //left-just
    fp->write("\n");

    //default constructor/destructor; initializes UlamElement with MFM__UUID_FOR
    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("();"); GCNL;

    m_state.indent(fp);
    fp->write("~");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("();"); GCNL;
    fp->write("\n");

    m_state.indent(fp);
    fp->write("static ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write(" THE_INSTANCE;"); GCNL;

    // any constant array class arguments
    if(m_nodeArgumentList)
      {
	UVPass uvpass;
	m_nodeArgumentList->genCode(fp, uvpass);
      }

    //DataMember VAR DECLS DMs
    if(m_nodeNext)
      {
	UVPass uvpass;
	m_nodeNext->genCode(fp, uvpass); //output the BitField typedefs
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
    fp->write(" : public UlamElement<EC>"); //was DefaultElement

    genThisUlamSuperClassAsAHeaderComment(fp);

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
    fp->write("();"); GCNL;

    m_state.indent(fp);
    fp->write("~");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("();"); GCNL;
    fp->write("\n");

    m_state.indent(fp);
    fp->write("static ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write(" THE_INSTANCE;"); GCNL;

    // any constant array class arguments
    if(m_nodeArgumentList)
      {
	UVPass uvpass;
	m_nodeArgumentList->genCode(fp, uvpass);
      }

    //DataMember VAR DECLS DM
    if(m_nodeNext)
      {
	UVPass uvpass;
	m_nodeNext->genCode(fp, uvpass);  //output the BitField typedefs
	fp->write("\n");
      }

    // if this 'element' contains more than one template (quark) data members,
    // we need vector of offsets to generate a separate function decl/dfn for each one's POS
    // in case a function has one as a return value and/or parameter.
  } //genCodeHeaderElement

  void NodeBlockClass::genCodeHeaderTransient(File * fp)
  {
    //use the instance UTI instead of the node's original type
    UTI cuti = m_state.getCompileThisIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("class ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write(" : public UlamTransient<EC> ");

    genThisUlamSuperClassAsAHeaderComment(fp);

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    genShortNameParameterTypesExtractedForHeaderFile(fp);

    fp->write("\n");
    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("public:\n\n");

    m_state.m_currentIndentLevel++;

    //default constructor/destructor;
    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("();"); GCNL;

    m_state.indent(fp);
    fp->write("~");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("();"); GCNL;
    fp->write("\n");

    m_state.indent(fp);
    fp->write("static ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write(" THE_INSTANCE;"); GCNL;

    // any constant array class arguments
    if(m_nodeArgumentList)
      {
	UVPass uvpass;
	m_nodeArgumentList->genCode(fp, uvpass);
      }

    //DataMember VAR DECLS DM
    if(m_nodeNext)
      {
	UVPass uvpass;
	m_nodeNext->genCode(fp, uvpass);  //output the BitField typedefs
	fp->write("\n");
      }

    // if this 'element' contains more than one template (quark) data members,
    // we need vector of offsets to generate a separate function decl/dfn for each one's POS
    // in case a function has one as a return value and/or parameter.
  } //genCodeHeaderTransient

  void NodeBlockClass::genCodeHeaderLocalsFilescope(File * fp)
  {
    //use the instance UTI instead of the node's original type
    UTI cuti = m_state.getCompileThisIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("class ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write(" : public UlamClass<EC> ");

    genThisUlamSuperClassAsAHeaderComment(fp);

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    genShortNameParameterTypesExtractedForHeaderFile(fp);

    fp->write("\n");
    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("public:\n\n");

    m_state.m_currentIndentLevel++;

    //default constructor/destructor;
    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("();"); GCNL;

    m_state.indent(fp);
    fp->write("~");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("();"); GCNL;
    fp->write("\n");

    m_state.indent(fp);
    fp->write("static ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write(" THE_INSTANCE;"); GCNL;

    //DataMember VAR DECLS DM
    if(m_nodeNext)
      {
	UVPass uvpass;
	m_nodeNext->genCode(fp, uvpass);  //output the BitField typedefs
	fp->write("\n");
      }

    // if this 'element' contains more than one template (quark) data members,
    // we need vector of offsets to generate a separate function decl/dfn for each one's POS
    // in case a function has one as a return value and/or parameter.
  } //genCodeHeaderLocalsFilescope

  void NodeBlockClass::genThisUlamSuperClassAsAHeaderComment(File * fp)
  {
    UTI superuti = m_state.isClassASubclass(m_state.getCompileThisIdx());
    if(superuti != Nouti)
      {
	fp->write(" /*, ");
	fp->write(m_state.getUlamTypeByIndex(superuti)->getUlamTypeMangledName().c_str());
	fp->write(" */");
      }
  }

  void NodeBlockClass::genShortNameParameterTypesExtractedForHeaderFile(File * fp)
  {
    m_state.indent(fp);
    fp->write("// Extract short names for parameter types\n");

    //typedef atomic parameter type inside struct
    UlamType::genStandardConfigTypedefTypenames(fp, m_state);

    m_state.indent(fp);
    fp->write("typedef BitVector<BPA> BV;"); GCNL;

    m_state.indent(fp);
    fp->write("typedef BitField<BitVector<BPA>, VD::BITS, T::ATOM_FIRST_STATE_BIT, 0> BFTYP;"); GCNL; //used to read MFM Type.

    fp->write("\n");
  } //genShortNameParameterTypesExtractedForHeaderFile

  //Body for This Class only (.tcc)
  void NodeBlockClass::genCodeBody(File * fp, UVPass& uvpass)
  {
    //use the instance UTI instead of the node's original type
    UlamType * cut = m_state.getUlamTypeByIndex(m_state.getCompileThisIdx());
    ULAMCLASSTYPE classtype = cut->getUlamClassType();

    m_state.m_currentIndentLevel = 0;

    if(classtype != UC_LOCALSFILESCOPE)
      {
	//don't include own header file, since .tcc is included in the .h
	//generate include statements for all the other classes that have appeared
	m_state.m_programDefST.generateIncludesForTableOfClasses(fp);

	//include the locals filescope for this class' locator, if one exists
	UTI locuti = getLocalsFilescopeType();
	if(locuti != Nouti)
	  {
	    assert(m_state.okUTItoContinue(locuti));
	    u32 mangledclassid = m_state.getMangledClassNameIdForUlamLocalsFilescope(locuti);
	    m_state.indent(fp);
	    fp->write("#include \"");
	    fp->write(m_state.m_pool.getDataAsString(mangledclassid).c_str());
	    fp->write(".h\""); GCNL;
	  }
      }
    fp->write("\n");

    m_state.indent(fp);
    fp->write("namespace MFM{\n\n");

    m_state.m_currentIndentLevel++;

    if(classtype == UC_ELEMENT)
      {
	genCodeBodyElement(fp, uvpass);
      }
    else if(classtype == UC_QUARK)
      {
	genCodeBodyQuark(fp, uvpass);
      }
    else if(classtype == UC_TRANSIENT)
      {
	genCodeBodyTransient(fp, uvpass);
      }
    else if(classtype == UC_LOCALSFILESCOPE)
      {
	genCodeBodyLocalsFilescope(fp, uvpass);
      }
    else
      m_state.abortUndefinedUlamClassType();

    generateCodeForFunctions(fp, false, classtype);

    generateCodeForBuiltInClassFunctions(fp, false, classtype);

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //MFM\n\n");
  } //genCodeBody

  void NodeBlockClass::genCodeBodyElement(File * fp, UVPass& uvpass)
  {
    UlamType * cut = m_state.getUlamTypeByIndex(m_state.getCompileThisIdx());

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

    //genCodeConstantArrayInitialization(fp);

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
    fp->write("\");"); GCNL;

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
    fp->write("(){}"); GCNL;
    fp->write("\n");

    assert(m_state.getCompileThisId() == cut->getUlamTypeNameId());
  } //genCodeBodyElement

  void NodeBlockClass::genCodeBodyQuark(File * fp, UVPass& uvpass)
  {
    UlamType * cut = m_state.getUlamTypeByIndex(m_state.getCompileThisIdx());

    //default constructor for quark
    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");
    fp->write("::");
    fp->write(cut->getUlamTypeMangledName().c_str());

    std::string namestr = cut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state);
    std::string namestrlong = removePunct(cut->getUlamTypeMangledName());

    fp->write("() : UlamQuark<EC>(MFM_UUID_FOR(\"");
    fp->write(namestrlong.c_str());
    fp->write("\", 0))\n");

    //genCodeConstantArrayInitialization(fp); t41198 constants now static

    m_state.indent(fp);
    fp->write("{ }\n\n");

    //default destructor
    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");
    fp->write("::~");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("(){}"); GCNL;
    fp->write("\n");

    assert(m_state.getCompileThisId() == cut->getUlamTypeNameId());
  } //genCodeBodyQuark

  void NodeBlockClass::genCodeBodyTransient(File * fp, UVPass& uvpass)
  {
    UlamType * cut = m_state.getUlamTypeByIndex(m_state.getCompileThisIdx());

    //default constructor for transient
    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");
    fp->write("::");
    fp->write(cut->getUlamTypeMangledName().c_str());

    std::string namestr = cut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state);
    std::string namestrlong = removePunct(cut->getUlamTypeMangledName());

    fp->write("() : UlamTransient<EC");
    fp->write(">(MFM_UUID_FOR(\"");
    fp->write(namestrlong.c_str());
    fp->write("\", 0))\n");

    //genCodeConstantArrayInitialization(fp);

    m_state.indent(fp);
    fp->write("{ }\n\n");

    //default destructor
    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");
    fp->write("::~");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("(){}"); GCNL;
    fp->write("\n");

    assert(m_state.getCompileThisId() == cut->getUlamTypeNameId());
  } //genCodeBodyTransient

  void NodeBlockClass::genCodeBodyLocalsFilescope(File * fp, UVPass& uvpass)
  {
    UlamType * cut = m_state.getUlamTypeByIndex(m_state.getCompileThisIdx());

    //default constructor for LocalsFilescope
    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");
    fp->write("::");
    fp->write(cut->getUlamTypeMangledName().c_str());

    std::string namestr = cut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state);
    std::string namestrlong = removePunct(cut->getUlamTypeMangledName());

    fp->write("() : UlamClass<EC");
    fp->write(">()\n");

    //genCodeConstantArrayInitialization(fp); //t41238

    m_state.indent(fp);
    fp->write("{ }\n\n");

    //default destructor
    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");
    fp->write("::~");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("(){}"); GCNL;
    fp->write("\n");

    assert(m_state.getCompileThisId() == cut->getUlamTypeNameId());
  } //genCodeBodyLocalsFilescope

  void NodeBlockClass::generateCodeForBuiltInClassFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    //ALL builtin should be hident!!!! XXXXXX
    m_state.indent(fp);
    fp->write("//BUILT-IN FUNCTIONS:\n");
    fp->write("\n");

    //define built-in init method for any "data member" constant class or arrays:
    generateBuiltinConstantClassOrArrayInitializationFunction(fp, declOnly);

    //generate 3 UlamClass:: methods for smart ulam debugging
    u32 dmcount = 0; //pass ref
    generateUlamClassInfoFunction(fp, declOnly, dmcount);
    generateUlamClassInfoCount(fp, declOnly, dmcount); //after dmcount is updated by nodes
    generateUlamClassGetMangledName(fp, declOnly);

    genCodeBuiltInFunctionGetClassLength(fp, declOnly, classtype);

    genCodeBuiltInFunctionGetString(fp, declOnly);
    genCodeBuiltInFunctionGetStringLength(fp, declOnly);

    genCodeBuiltInFunctionBuildDefaultAtom(fp, declOnly, classtype);

    genCodeBuiltInVirtualTable(fp, declOnly, classtype);

    // 'is' quark related for both class types; overloads is-Method with THE_INSTANCE arg
    genCodeBuiltInFunctionIsMethodRelatedInstance(fp, declOnly, classtype);

    // 'is' is only for element/classes
    if(classtype == UC_ELEMENT)
      {
	generateInternalIsMethodForElement(fp, declOnly); //overload
	generateInternalTypeAccessorsForElement(fp, declOnly);
      }
    else if(classtype == UC_QUARK)
      generateGetPosForQuark(fp, declOnly);
    else if(classtype == UC_TRANSIENT)
      {
	//nothing to do
      }
    else if(classtype == UC_LOCALSFILESCOPE)
      {
	//nothing to do
      }
    else
      m_state.abortUndefinedUlamClassType(); //sanity

  } //generateCodeForBuiltInClassFunctions

  void NodeBlockClass::genCodeBuiltInFunctionIsMethodRelatedInstance(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    UTI cuti = m_state.getCompileThisIdx();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("//helper method not called directly\n");

	m_state.indent(fp);
	fp->write("bool ");
	fp->write(m_state.getIsMangledFunctionName(cuti));
	fp->write("(const UlamClass<EC> * cptrarg) const;"); GCNL; //overloade
	fp->write("\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("bool "); //return true if related

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write(m_state.getIsMangledFunctionName(cuti));
    fp->write("(const UlamClass<EC> * cptrarg) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    genCodeBuiltInFunctionIsRelatedInstance(fp);

    fp->write("\n");
    m_state.indent(fp);
    fp->write("return ");
    fp->write("(false); //not found"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //is-related\n\n");
  } //genCodeBuiltInFunctionIsMethodRelatedInstance

  void NodeBlockClass::genCodeBuiltInFunctionIsRelatedInstance(File * fp)
  {
    UTI nuti = getNodeType();
    UTI superuti = m_state.isClassASubclass(nuti);
    assert(superuti != Hzy);
    if(superuti != Nouti)
      {
	//then include any of its relatives:
	NodeBlockClass * superClassBlock = getSuperBlockPointer();
	assert(superClassBlock);
	superClassBlock->genCodeBuiltInFunctionIsRelatedInstance(fp);
      }
    m_state.indent(fp);
    fp->write("if(cptrarg == &");
    fp->write(m_state.getTheInstanceMangledNameByIndex(nuti).c_str());
    fp->write(") return(true); //inherited class, or self (last)"); GCNL;
  } //genCodeBuiltInFunctionIsRelatedInstance

  void NodeBlockClass::genCodeBuiltInFunctionGetClassLength(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    UTI cuti = m_state.getCompileThisIdx();
    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual u32 ");
	fp->write(m_state.getClassLengthFunctionName(cuti));
	fp->write("() const;"); GCNL;
	fp->write("\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("u32 "); //returns class length

    //include the mangled class::
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");

    fp->write("::");
    fp->write(m_state.getClassLengthFunctionName(cuti));
    fp->write("( ) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("return ");
    fp->write_decimal_unsigned(cut->getTotalBitSize()); //t3663
    fp->write(";"); GCNL;

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //getClassLength\n\n");
  } //genCodeBuiltInFunctionGetClassLength

  void NodeBlockClass::genCodeBuiltInFunctionBuildDefaultAtom(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    //'default atom' applies only to elements
    if(classtype == UC_QUARK)
      return genCodeBuiltInFunctionBuildDefaultQuark(fp, declOnly, classtype);
    else if(classtype == UC_TRANSIENT)
      return genCodeBuiltInFunctionBuildDefaultTransient(fp, declOnly, classtype);
    else if(classtype == UC_LOCALSFILESCOPE)
      return;

    assert(classtype == UC_ELEMENT);

    UTI cuti = m_state.getCompileThisIdx();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual T ");
	fp->write(m_state.getBuildDefaultAtomFunctionName(cuti));
	fp->write("() const;"); GCNL;
	fp->write("\n");
	return;
      }

    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("typename EC::ATOM_CONFIG::ATOM_TYPE "); //returns object of type T

    //include the mangled class::
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");

    fp->write("::");
    fp->write(m_state.getBuildDefaultAtomFunctionName(cuti));
    fp->write("( ) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("AtomBitStorage<EC> da(");
    fp->write("Element<EC>::BuildDefaultAtom());"); GCNL;
    fp->write("\n");

    if(cut->getBitSize() > 0)
      {
	m_state.indent(fp);
	fp->write("// Initialize any data members:\n");

	//get all initialized data members packed;
	// (including 25 zeros for Type) e.g. t3510
	//restore type.
	if(genCodeBuiltInFunctionBuildingDefaultDataMembers(fp))
	  {
	    m_state.indent(fp);
	    fp->write("initBV.Write(0u, T::ATOM_FIRST_STATE_BIT, ");
	    fp->write("da.Read(0u, T::ATOM_FIRST_STATE_BIT));"); GCNL; //can't use GetType
	    m_state.indent(fp);
	    fp->write("da.WriteBV(0u, initBV);"); GCNL;
	  }
      }

    m_state.indent(fp);
    fp->write("return ");
    fp->write("(da.ReadAtom());"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //BuildDefaultAtom\n\n");
  } //genCodeBuiltInFunctionBuildDefaultAtom

  //return false for zero default value (effects the needed gen code)
  bool NodeBlockClass::genCodeBuiltInFunctionBuildingDefaultDataMembers(File * fp)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    assert(nut->isScalar());

    u32 len = nut->getTotalBitSize();
    if(len == 0)
      return false;

    if(nut->getUlamClassType() == UC_ELEMENT)
      len = BITSPERATOM; //atom-based (not empty)

    BV8K dval;
    AssertBool isDefault = m_state.getDefaultClassValue(nuti, dval);
    assert(isDefault);

    return m_state.genCodeClassDefaultConstantArray(fp, len, dval); //non-zero default
  } //genCodeBuiltInFunctionBuildingDefaultDataMembers

  void NodeBlockClass::genCodeBuiltInFunctionBuildDefaultQuark(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    //return;
    assert(classtype == UC_QUARK);
    UTI cuti = m_state.getCompileThisIdx();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual u32 ");
	fp->write(m_state.getBuildDefaultAtomFunctionName(cuti));
	fp->write("( ) const;"); GCNL;
	fp->write("\n");
	return;
      }

    //get all initialized data members in quark
    u32 qval = 0;
    AssertBool isDefaultQuark = m_state.getDefaultQuark(cuti, qval);
    assert(isDefaultQuark);

    std::ostringstream qdhex;
    qdhex << "0x" << std::hex << qval;


    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("u32 ");

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    fp->write("<EC>");

    fp->write("::");
    fp->write(m_state.getBuildDefaultAtomFunctionName(cuti));
    fp->write("( ) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    if(hasStringDataMembers())
      {
	//must by the only data member then (max size Quark == size of String) t41167
	genCodeBuiltInFunctionBuildingDefaultDataMembers(fp);

	m_state.indent(fp);
	fp->write("return initBV.Read(0u, QUARK_SIZE);"); GCNL;
      }
    else
      {
	m_state.indent(fp);
	fp->write("return ");
	fp->write(qdhex.str().c_str());
	fp->write("; //=");
	fp->write_decimal_unsigned(qval); GCNL;
      }

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //getDefaultQuark\n\n");
  } //genCodeBuiltInFunctionBuildDefaultQuark

  void NodeBlockClass::genCodeBuiltInFunctionBuildDefaultTransient(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    assert(classtype == UC_TRANSIENT);

    UTI cuti = m_state.getCompileThisIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual void ");
	fp->write(m_state.getBuildDefaultAtomFunctionName(cuti));
	fp->write("(u32 pos, BitStorage<EC");
	fp->write(">& bvsref) const;"); GCNL;
	fp->write("\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("void ");

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    fp->write("<EC>");

    fp->write("::");
    fp->write(m_state.getBuildDefaultAtomFunctionName(cuti));
    fp->write("(u32 pos, BitStorage<EC");
    fp->write(">& bvsref) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    u32 len = cut->getTotalBitSize();
    if(len > 0)
      {
	m_state.indent(fp);
	fp->write("MFM_API_ASSERT_ARG((pos + bvsref.GetBitSize()) >= ");
	fp->write_decimal_unsigned(len);
	fp->write("u);"); GCNL;
	fp->write("\n");

	m_state.indent(fp);
	fp->write("// Initialize any data members:\n");

	//get all initialized data members packed into 'daref'
	// unlike element and quarks, data members can be elements, atoms and other transients
	//e.g. t3811, t3812
	genCodeBuiltInFunctionBuildingDefaultDataMembers(fp);
	genCodeElementTypeIntoDataMemberDefaultValueOrTmpVar(fp, 0, NULL); //startpos = 0

	m_state.indent(fp);
	fp->write("bvsref.WriteBV(pos, "); //first arg
	fp->write("initBV);"); GCNL;
      }
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //getDefaultTransient\n\n");
  } //genCodeBuiltInFunctionBuildDefaultTransient

  void NodeBlockClass::genCodeBuiltInVirtualTable(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    if(classtype == UC_LOCALSFILESCOPE)
      return;

    //VTable applies to both quarks and elements
    UTI cuti = m_state.getCompileThisIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    s32 maxidx = getVirtualMethodMaxIdx();
    assert(maxidx >= 0);

    if(maxidx == 0)
      return;

    if(declOnly)
      {
	//enum for method indexes; see UlamElement.h for first two.
	m_state.indent(fp);
	fp->write("enum VTABLE_IDX {\n");
	m_state.m_currentIndentLevel++;
	for(s32 i = 0; i < maxidx; i++)
	  {
	    m_state.indent(fp);
	    fp->write("VTABLE_IDX_");
	    fp->write(csym->getMangledFunctionNameWithTypesForVTableEntry(i).c_str());
	    if(i == 0)
	      fp->write(" = 0");
	    fp->write(",\n");
	  }
	m_state.indent(fp);
	fp->write("VTABLE_IDX_MAX\n");
	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("};"); GCNL;
	fp->write("\n");

	m_state.indent(fp);
	fp->write("static ");
	fp->write("VfuncPtr m_vtable[");
	fp->write_decimal_unsigned(maxidx);
	fp->write("];"); GCNL;

	//VTable accessor method
	m_state.indent(fp);
	fp->write("virtual VfuncPtr getVTableEntry(u32 idx) const;"); GCNL;
	fp->write("\n");
	return;
      } //done w h-file

    //The VTABLE Definition:
    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("VfuncPtr ");

    //include the mangled class::
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::"); //same for elements and quarks
    fp->write("m_vtable[");
    fp->write_decimal_unsigned(maxidx);
    fp->write("] = {\n");

    m_state.m_currentIndentLevel++;
    //generate each VT entry:
    for(s32 i = 0; i < maxidx; i++)
      {
	if(i > 0)
	  fp->write(",\n");

	if(csym->isPureVTableEntry(i))
	  {
	    m_state.indent(fp);
	    fp->write("&UlamClass<EC>::PureVirtualFunctionCalled");
	    continue;
	  }

	UTI veuti = csym->getClassForVTableEntry(i);
	assert(m_state.okUTItoContinue(veuti));
	UlamType * veut = m_state.getUlamTypeByIndex(veuti);
	m_state.indent(fp);
	fp->write("(VfuncPtr) "); //cast to void
	fp->write("((typename "); //cast to contextual type info
	fp->write(veut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::"); //same for elements and quarks
	fp->write(csym->getMangledFunctionNameWithTypesForVTableEntry(i).c_str());
	fp->write(") &");
	fp->write(veut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::"); //same for elements and quarks
	fp->write(csym->getMangledFunctionNameForVTableEntry(i).c_str());
	fp->write(")");
      } //next vt entry

    fp->write("\n");
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}; //VTABLE Definition"); GCNL;
    fp->write("\n");

    //VTable accessor method
    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("VfuncPtr ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::"); //same for elements and quarks
    fp->write("getVTableEntry(u32 idx) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("if(idx >= ");
    fp->write_decimal_unsigned(maxidx);
    fp->write(") FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);"); GCNL;

    m_state.indent(fp);
    fp->write("return m_vtable[idx];"); GCNL;
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n\n");
  }//genCodeBuiltInVirtualTable

  void NodeBlockClass::generateInternalIsMethodForElement(File * fp, bool declOnly)
  {
    UTI cuti = getNodeType();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("//helper method not called directly\n");

	m_state.indent(fp);
	fp->write("bool ");
	fp->write(m_state.getIsMangledFunctionName(cuti));
	fp->write("(const T& targ) const;"); GCNL;
	fp->write("\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("bool "); //return pos offset, or -1 if not found

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());

    fp->write("<EC>::");
    fp->write(m_state.getIsMangledFunctionName(cuti));
    fp->write("(const T& targ) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("return (THE_INSTANCE.GetType() == targ.GetType());"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //isMethod\n\n");
  } //generateInternalIsMethodForElement

  void NodeBlockClass::generateInternalTypeAccessorsForElement(File * fp, bool declOnly)
  {
    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("const u32 ReadTypeField(const BV bv);"); GCNL;
	fp->write("\n");
	m_state.indent(fp);
	fp->write("void WriteTypeField(BV& bv, const u32 v);"); GCNL;
	fp->write("\n");
	return;
      }

    UTI cuti = getNodeType();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("const u32 ");  //return NULL if not found

    //include the mangled class::
    fp->write(cut->getUlamTypeMangledName().c_str());

    fp->write("<EC>::");
    fp->write("ReadTypeField(const BV bv)\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("return BFTYP::Read(bv);"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //ReadTypeField\n\n");


    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("void ");  //return NULL if not found

    //include the mangled class::
    fp->write(cut->getUlamTypeMangledName().c_str());

    fp->write("<EC>::");
    fp->write("WriteTypeField(BV& bv, const u32 v)\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("BFTYP::Write(bv, v);"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //WriteTypeField\n\n");
  } //generateInternalTypeAccessorsForElement

  void NodeBlockClass::generateGetPosForQuark(File * fp, bool declOnly)
  {
    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("__inline__ const u32 GetPos() const { return 0u; }\n"); //?????
      }
  } //generateGetPosForQuark

  void NodeBlockClass::genCodeConstantArrayInitialization(File * fp)
  {
    if(m_nodeArgumentList)
      m_nodeArgumentList->genCodeConstantArrayInitialization(fp);

    if(m_nodeNext)
      m_nodeNext->genCodeConstantArrayInitialization(fp);
  }

  void NodeBlockClass::generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly)
  {
    if(m_nodeArgumentList) //t3894, t41209
      m_nodeArgumentList->generateBuiltinConstantClassOrArrayInitializationFunction(fp, declOnly);

    if(m_nodeNext)
      m_nodeNext->generateBuiltinConstantClassOrArrayInitializationFunction(fp, declOnly);
  }

  void NodeBlockClass::genCodeBuiltInFunctionGetString(File * fp, bool declOnly)
  {
    UTI cuti = m_state.getCompileThisIdx();

    //class specific UserStringPool
    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual const ");
	fp->write("unsigned char * ");
	fp->write(m_state.getClassGetStringFunctionName(cuti));
	fp->write("(u32 sidx) const;"); GCNL;
	fp->write("\n");
	return;
      }

    StringPoolUser& classupool = m_state.getUPoolRefForClass(cuti);

    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("const ");
    fp->write("unsigned char * ");

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write(m_state.getClassGetStringFunctionName(cuti));
    fp->write("(u32 sidx)  const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("const u32 ");
    fp->write(m_state.getDefineNameForUserStringPoolSize());
    fp->write(" = ");
    fp->write_decimal_unsigned(classupool.getUserStringPoolSize());
    fp->write(";"); GCNL;

    //the pool table definition
    classupool.generateUserStringPoolEntries(fp, &m_state);

    m_state.indent(fp);
    fp->write("if(sidx == 0)\n");
    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("FAIL(UNINITIALIZED_VALUE);"); GCNL;
    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("if(sidx >= ");
    fp->write(m_state.getDefineNameForUserStringPoolSize());
    fp->write(")\n");
    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);"); GCNL;
    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("return ");
    fp->write(m_state.getMangledNameForUserStringPool());
    fp->write(" + sidx + 1;"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //GetString\n\n");
  } //genCodeBuiltInFunctionGetString

  void NodeBlockClass::genCodeBuiltInFunctionGetStringLength(File * fp, bool declOnly)
  {
    UTI cuti = m_state.getCompileThisIdx();

    //class specific UserStringPool
    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual u32 ");
	fp->write(m_state.getClassGetStringFunctionName(cuti));
	fp->write("Length(u32 sidx) const;"); GCNL;
	fp->write("\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("u32 ");

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write(m_state.getClassGetStringFunctionName(cuti));
    fp->write("Length(u32 sidx)  const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("return *(");
    fp->write(m_state.getClassGetStringFunctionName(cuti));
    fp->write("(sidx) - 1);"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //GetStringLength\n\n");
  } //genCodeBuiltInFunctionGetStringLength

  void NodeBlockClass::generateUlamClassInfoFunction(File * fp, bool declOnly, u32& dmcount)
  {
    UTI cuti = getNodeType();
    assert(m_state.okUTItoContinue(cuti));
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    if(!declOnly)
      {
	m_state.indent(fp);
	fp->write("template<class EC>\n"); //same for elements and quarks
	m_state.indent(fp);
      }
    else m_state.indent(fp);

    fp->write("const UlamClassDataMemberInfo & "); //return type

    if(!declOnly)
      {
	//include the mangled class::
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::"); //same for elements and quarks
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
	fp->write("}; //end switch"); GCNL;

	m_state.indent(fp);
	fp->write("FAIL(ILLEGAL_ARGUMENT);"); GCNL;

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("} //GetDataMemberInfo\n\n"); //end of func
      }
    else
      {
	fp->write(";"); GCNL; //end of declaration
	fp->write("\n");
      }
  } //generateUlamClassInfoFunction

  void NodeBlockClass::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    if(m_state.isClassASubclass(getNodeType()) != Nouti)
      {
	NodeBlockClass * superClassBlock = getSuperBlockPointer();
	assert(superClassBlock);
	superClassBlock->generateUlamClassInfo(fp, declOnly, dmcount);
      }

    if(m_nodeNext)
      m_nodeNext->generateUlamClassInfo(fp, declOnly, dmcount);
  } //generateUlamClassInfo

  void NodeBlockClass::generateUlamClassInfoCount(File * fp, bool declOnly, u32 dmcount)
  {
    UTI cuti = getNodeType();
    assert(m_state.okUTItoContinue(cuti));
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    if(!declOnly)
      {
	m_state.indent(fp);
	fp->write("template<class EC>\n"); //same for elements and quarks
	m_state.indent(fp);
      }
    else
      m_state.indent(fp);

    fp->write("s32 "); //return type

    if(!declOnly)
      {
	//include the mangled class::
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::"); //same for elements and quarks
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
	fp->write(";"); GCNL;

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("} //GetDataMemberCount\n\n");
      }
    else
      {
	fp->write(";"); GCNL;
	fp->write("\n");
      }
  } //generateUlamClassInfoCount

  void NodeBlockClass::generateUlamClassGetMangledName(File * fp, bool declOnly)
  {
    UTI cuti = getNodeType();
    assert(m_state.okUTItoContinue(cuti));
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    if(!declOnly)
      {
	m_state.indent(fp);
	fp->write("template<class EC>\n"); //same for elements and quarks
	m_state.indent(fp);
      }
    else
      m_state.indent(fp);

    fp->write("const char * "); //return type

    if(!declOnly)
      {
	//include the mangled class::
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::"); //same for elements and quarks
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
	fp->write("\";"); GCNL;

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("} //GetMangledClassName\n\n");
      }
    else
      {
	fp->write(";"); GCNL;
	fp->write("\n");
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

  void NodeBlockClass::addTargetDescriptionToInfoMap(TargetMap& classtargets, u32 scid)
  {
    UTI cuti = getNodeType();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    std::string className = cut->getUlamTypeNameOnly();
    std::string mangledName = cut->getUlamTypeMangledName();
    struct TargetDesc desc;

    NodeBlockFunctionDefinition * func = findTestFunctionNode();
    desc.m_hasTest = (func != NULL);

    ULAMCLASSTYPE classtype = cut->getUlamClassType();
    desc.m_classType = classtype;

    desc.m_bitsize = cut->getTotalBitSize();
    desc.m_loc = getNodeLocation();
    desc.m_className = className;

    if(scid > 0)
      {
	std::ostringstream sc;
	sc << "/**";
	sc << m_state.m_pool.getDataAsString(scid).c_str();
	sc << "*/";
	desc.m_structuredComment = sc.str();
      }
    else
      desc.m_structuredComment = "NONE";

    //format Ulam Class Signature
    SymbolClassName * cnsym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameByUTI(cuti, cnsym);
    assert(isDefined);
    desc.m_classSignature = cnsym->generateUlamClassSignature();

    //format Ulam Class Signature of super class
    if(!m_state.isUrSelf(cuti))
      {
	UTI superuti = m_state.isClassASubclass(cuti);
	assert(m_state.okUTItoContinue(superuti));
	SymbolClassName * supercnsym = NULL;
	AssertBool isSuperDefined = m_state.alreadyDefinedSymbolClassNameByUTI(superuti, supercnsym);
	assert(isSuperDefined);

	desc.m_baseClassSignature = supercnsym->generateUlamClassSignature();
      }
    else
      desc.m_baseClassSignature = "nobase";

    classtargets.insert(std::pair<std::string, struct TargetDesc>(mangledName, desc));
  } //addTargetDescriptionToInfoMap

  void NodeBlockClass::addMemberDescriptionsToInfoMap(ClassMemberMap& classmembers)
  {
    NodeBlock::addMemberDescriptionsToInfoMap(classmembers); //Table of Variables request
    m_functionST.addClassMemberFunctionDescriptionsToMap(this->getNodeType(), classmembers); //Table of Classes request
  }

  void NodeBlockClass::generateTestInstance(File * fp, bool runtest)
  {
    if(runtest)
      return generateTestInstanceRun(fp); //refactor

    UTI suti = getNodeType();
    UlamType * sut = m_state.getUlamTypeByIndex(suti);
    if(!sut->isComplete()) return;

    // recursively register all classes, for testing only, o.w. ILLEGAL_STATE t3879. t3922, t3948, t3967, t3982, t41183
    //if((suti != m_state.getCompileThisIdx()) && !m_state.isOtherClassInThisContext(suti)) return; //e.g. t3373,5,6,7, t3923

    if(m_state.isClassASubclass(suti) != Nouti)
      {
	NodeBlockClass * superClassBlock = getSuperBlockPointer();
	assert(superClassBlock);
	superClassBlock->generateTestInstance(fp, runtest);
      }

    if(m_registeredForTestInstance)
      return; //once only in main (t41183)

    //need to insure all class data members are also registered before us (t41167)
    if(m_nodeNext)
      m_nodeNext->generateTestInstance(fp, runtest);

    if(sut->getUlamClassType() == UC_ELEMENT)
      {
	// output for each element before testing; a test may include
	// one or more of them!
	fp->write("\n");
	m_state.indent(fp);
	fp->write("{\n");

	m_state.m_currentIndentLevel++;

	m_state.indent(fp);
	fp->write("UlamElement<EC> & elt = ");
	fp->write(m_state.getTheInstanceMangledNameByIndex(suti).c_str());
	fp->write(";"); GCNL;

	//register before allocate type to avoid ILLEGAL_STATE (t3968, t41183)
	m_state.indent(fp);
	fp->write("tile.GetUlamClassRegistry().RegisterUlamClass(elt);"); GCNL;
	m_state.indent(fp);
	fp->write("elt.AllocateType(etnm); //Force element type allocation now"); GCNL;
	m_state.indent(fp);
	fp->write("tile.RegisterElement(elt);"); GCNL;

	m_state.m_currentIndentLevel--;

	m_state.indent(fp);
	fp->write("}\n");
      }
    else
      {
	//non-elements:
	fp->write("\n");
	m_state.indent(fp);
	fp->write("{\n");

	m_state.m_currentIndentLevel++;

	m_state.indent(fp);
	fp->write("UlamClass<EC> & clt = ");
	fp->write(m_state.getTheInstanceMangledNameByIndex(suti).c_str());
	fp->write(";"); GCNL;

	m_state.indent(fp);
	fp->write("tile.GetUlamClassRegistry().RegisterUlamClass(clt);"); GCNL;

	m_state.m_currentIndentLevel--;

	m_state.indent(fp);
	fp->write("}\n");
      }

    m_registeredForTestInstance = true;
    return;
  } //generateTestInstance

  void NodeBlockClass::generateTestInstanceRun(File * fp)
  {
    // called after all the generateTestInstance() are generated.
    UTI suti = getNodeType();
    UlamType * sut = m_state.getUlamTypeByIndex(suti);
    assert(sut->isComplete());

    if(suti != m_state.getCompileThisIdx())
      return;

    fp->write("\n");

    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("OurAtomAll atom = "); //OurAtomAll
    fp->write(m_state.getTheInstanceMangledNameByIndex(suti).c_str());
    fp->write(".GetDefaultAtom();"); GCNL;
    m_state.indent(fp);
    fp->write("tile.PlaceAtom(atom, center);"); GCNL;

    m_state.indent(fp);
    fp->write("AtomRefBitStorage<EC> atbs(atom);"); GCNL;

    m_state.indent(fp);
    fp->write("UlamRef<EC> ur(EC::ATOM_CONFIG::ATOM_TYPE::ATOM_FIRST_STATE_BIT, "); //e.g. t3255
    fp->write_decimal_unsigned(sut->getTotalBitSize()); //t3655
    fp->write("u, atbs, &");
    fp->write(m_state.getTheInstanceMangledNameByIndex(suti).c_str());
    fp->write(", UlamRef<EC>::ELEMENTAL, uc);"); GCNL;

    m_state.indent(fp);
    fp->write(m_state.getTheInstanceMangledNameByIndex(suti).c_str());

    // pass uc with effective self setup
    fp->write(".Uf_4test(");
    fp->write("uc, ur);"); GCNL;

    m_state.indent(fp);
    fp->write("//std::cerr << rtn.read() << std::endl;\n");//useful to return result of test?
    m_state.indent(fp);
    fp->write("//return rtn.read();\n"); //was useful to return result of test

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");
  } //generateTestInstanceRun

} //end MFM

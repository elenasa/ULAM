#include <stdio.h>
#include <iomanip>
#include "NodeBlockClass.h"
#include "NodeBlockFunctionDefinition.h"
#include "CompilerState.h"
#include "SymbolFunctionName.h"
#include "SymbolTypedef.h"

namespace MFM {

  NodeBlockClass::NodeBlockClass(NodeBlock * prevBlockNode, CompilerState & state) : NodeBlockContext(prevBlockNode, state), m_functionST(state), m_virtualmethodMaxIdx(UNKNOWNSIZE), m_buildingDefaultValueInProgress(false), m_bitPackingInProgress(false), m_isEmpty(false), m_registeredForTestInstance(false)

  {
    m_nodeParameterList = new NodeList(state);
    assert(m_nodeParameterList);
    m_nodeArgumentList = new NodeList(state);
    assert(m_nodeArgumentList);
    initBaseClassBlockList();
  }

  NodeBlockClass::NodeBlockClass(const NodeBlockClass& ref) : NodeBlockContext(ref), m_functionST(ref.m_functionST) /* deep copy */, m_virtualmethodMaxIdx(ref.m_virtualmethodMaxIdx), m_buildingDefaultValueInProgress(false), m_bitPackingInProgress(false), m_isEmpty(ref.m_isEmpty), m_registeredForTestInstance(false), m_nodeParameterList(NULL), m_nodeArgumentList(NULL)
  {
    UTI cuti = m_state.getCompileThisIdx();
    m_state.pushClassContext(cuti, this, this, false, NULL); //t3895
    setNodeType(cuti); //t3895, after push
    m_nodeParameterList = new NodeList(m_state);
    assert(m_nodeParameterList);
    m_nodeArgumentList = new NodeList(m_state);
    assert(m_nodeArgumentList);
    initBaseClassBlockList();
    m_state.popClassContext();
  }

  NodeBlockClass::~NodeBlockClass()
  {
    delete m_nodeParameterList;
    m_nodeParameterList = NULL;
    delete m_nodeArgumentList;
    m_nodeArgumentList = NULL;
    clearBaseClassBlockList();
    clearSharedBaseClassBlockList();
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

    //check all ancestors..
    bool rtnb = false;
    UTI cuti = getNodeType();
    SymbolClass * csym = NULL;
    if(m_state.alreadyDefinedSymbolClass(cuti, csym))
      {
	u32 basecount = csym->getBaseClassCount() + 1; //include super
	u32 i = 0;
	while(!rtnb && (i < basecount))
	  {
	    UTI baseuti = csym->getBaseClass(i);

	    SymbolClass * basecsym = NULL;
	    if(m_state.alreadyDefinedSymbolClass(baseuti, basecsym))
	      {
		NodeBlockClass * basecblock = basecsym->getClassBlockNode();
		//e.g. not a stub, yet not complete because its baseclass is a stub! (ish 06222016)
		// or is a stub! (t3887), or just incomplete (t41012)
		if(basecblock)
		  rtnb = basecblock->findNodeNo(n, foundNode); //recurse, depth-first
	      }
	    i++;
	  } //end while
      }
    return rtnb;
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

    //inheritance
    SymbolClass * csym = NULL;
    if(m_state.alreadyDefinedSymbolClass(cuti, csym))
      {
	UTI superuti = csym->getBaseClass(0);
	//skip UrSelf to avoid extensive changes to all test answers
	if(m_state.okUTItoContinue(superuti) && !m_state.isUrSelf(superuti))
	  {
	    fp->write(" : ");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(superuti).c_str());  //e.g. : Foo(a), an instance of
	  }
	//ulam-5 supports multiple base classes; superclass optional
	u32 basecount = csym->getBaseClassCount() + 1; //include super
	u32 i = 1;
	while(i < basecount)
	  {
	    UTI baseuti = csym->getBaseClass(i);
	    fp->write(" +");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(baseuti).c_str());  //e.g. +Foo(a), an instance of
	    i++;
	  } //end while
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

  //has only initialized DM values, not current values
  void NodeBlockClass::printPostfixDataMembersParseTree(File * fp, UTI cuti)
  {
    //UTI cuti = getNodeType(); maybe Hzy, use arg instead
    if(m_state.isUrSelf(cuti)) return;

    //ulam-5 data members precede base classes; all bases shared
    if(m_nodeNext)
      m_nodeNext->printPostfix(fp); //datamember vardecls

    //base classes stop with data members only
    if(UlamType::compare(cuti, m_state.getCompileThisIdx(), m_state) != UTIC_SAME)
      return;

    //inheritance
    SymbolClass * csym = NULL;
    if(m_state.alreadyDefinedSymbolClass(cuti, csym))
      {
	UTI superuti = csym->getBaseClass(0);
	//skip UrSelf to avoid extensive changes to all test answers
	if(m_state.okUTItoContinue(superuti) && !m_state.isUrSelf(superuti))
	  {
	    NodeBlockClass * superblock = getBaseClassBlockPointer(0);
	    if(!isBaseClassLinkReady(cuti,0))
	      {
		//use SCN instead of SC in case of stub (use template's classblock)
		SymbolClassName * supercnsym = NULL;
		AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameByUTI(superuti, supercnsym);
		assert(isDefined);
		superblock = supercnsym->getClassBlockNode();
		superuti = supercnsym->getUlamTypeIdx(); //in case of stub (t41007)
	      }
	    assert(superblock);
	    fp->write(" :");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(superuti).c_str()); //eg ^Foo(a), an instance of
	    fp->write("<");
	    superblock->printPostfixDataMembersParseTree(fp, superuti);
	    fp->write(">");
	    //else
	  }

	//ulam-5 supports multiple base classes; superclass optional
	u32 basecount = csym->getBaseClassCount() + 1; //include super
	u32 i = 1;
	while(i < basecount)
	  {
	    UTI baseuti = csym->getBaseClass(i);
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);

	    if(!isBaseClassLinkReady(cuti, i))
	      {
		//use SCN instead of SC in case of stub (use template's classblock)
		SymbolClassName * basecnsym = NULL;
		AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameByUTI(baseuti, basecnsym);
		assert(isDefined);
		basecblock = basecnsym->getClassBlockNode();
		baseuti = basecnsym->getUlamTypeIdx(); //in case of stub
	      }
	    assert(basecblock);
	    fp->write(" +");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(baseuti).c_str()); //eg ^Foo(a), an instance of
	    fp->write("<");
	    basecblock->printPostfixDataMembersParseTree(fp, baseuti);
	    fp->write(">");
	    i++;
	  } //end while

	//ulam-5 supports shared base classes (comes after datamembers)
	u32 shbasecount = csym->getSharedBaseClassCount();
	u32 j = 0;
	while(j < shbasecount)
	  {
	    UTI baseuti = csym->getSharedBaseClass(j);
	    if(!m_state.isUrSelf(baseuti))
	      {
		NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);

		if(!isSharedBaseClassLinkReady(cuti, j))
		  {
		    //use SCN instead of SC in case of stub (use template's classblock)
		    SymbolClassName * basecnsym = NULL;
		    AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameByUTI(baseuti, basecnsym);
		    assert(isDefined);
		    shbasecblock = basecnsym->getClassBlockNode();
		    baseuti = basecnsym->getUlamTypeIdx(); //in case of stub
		  }
		assert(shbasecblock);

		s32 bitem = csym->isABaseClassItem(baseuti);
		if(bitem < 0)
		  {
		    //not a direct shared base
		    fp->write(" ^");
		    fp->write(m_state.getUlamTypeNameBriefByIndex(baseuti).c_str()); //eg ^Foo(a), an instance of
		    fp->write("<");
		    shbasecblock->printPostfixDataMembersParseTree(fp, baseuti); //recurse here
		    fp->write(">");
		  }
		//else
	      }
	    j++;
	  } //end while
      }
  } //printPostfixDataMembersParseTree

  //ALL data members (incl ancestor's) w most current values impacted by test()
  void NodeBlockClass::printPostfixDataMembersSymbols(File * fp, s32 slot, u32 startpos, UTI cuti)
  {
    //ulam-5 data members precede base classes
    m_ST.printPostfixValuesForTableOfVariableDataMembers(fp, slot, startpos, m_state.getUlamTypeByIndex(cuti)->getUlamClassType());

    //Don't return, if(UlamType::compare(cuti, m_state.getCompileThisIdx(), m_state) != UTIC_SAME) (t3542)

    //ulam-5 output all base classes at the end of current class; all bases are shared;
    //inheritance
    SymbolClass * csym = NULL;
    if(m_state.alreadyDefinedSymbolClass(cuti, csym))
      {
	UTI superuti = csym->getBaseClass(0);
	//skip UrSelf to avoid extensive changes to all test answers
	if(m_state.okUTItoContinue(superuti) && !m_state.isUrSelf(superuti))
	  {
	    NodeBlockClass * superblock = getBaseClassBlockPointer(0);
	    assert(superblock && UlamType::compare(superblock->getNodeType(), superuti, m_state) == UTIC_SAME);
	    u32 relpos = csym->getBaseClassRelativePosition(0);
	    fp->write(" :");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(superuti).c_str()); //eg :Foo(a), an instance of
	    fp->write("<");
	    superblock->printPostfixDataMembersSymbols(fp, slot, startpos + relpos, superuti);
	    fp->write(">");
	  }

	//ulam-5 supports multiple base classes; superclass optional
	u32 basecount = csym->getBaseClassCount() + 1; //include super
	u32 i = 1;
	while(i < basecount)
	  {
	    UTI baseuti = csym->getBaseClass(i);
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    assert(basecblock && UlamType::compare(basecblock->getNodeType(), baseuti, m_state) == UTIC_SAME);
	    u32 relpos = csym->getBaseClassRelativePosition(i);
	    fp->write(" +");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(baseuti).c_str()); //eg +Foo(a), an instance of
	    fp->write("<");
	    basecblock->printPostfixDataMembersSymbols(fp, slot, startpos + relpos, baseuti);
	    fp->write(">");
	    i++;
	  } //end while

	//ulam-5 supports shared base classes
	u32 shbasecount = csym->getSharedBaseClassCount();
	u32 j = 0;
	while(j < shbasecount)
	  {
	    UTI baseuti = csym->getSharedBaseClass(j);
	    if(!m_state.isUrSelf(baseuti)) //t3102
	      {
		NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);
		assert(shbasecblock && UlamType::compare(shbasecblock->getNodeType(), baseuti, m_state) == UTIC_SAME);

		s32 bitem = csym->isABaseClassItem(baseuti);
		if(bitem < 0)
		  {
		    //not a direct shared base
		    u32 relpos = csym->getSharedBaseClassRelativePosition(j);
		    fp->write(" ^");
		    fp->write(m_state.getUlamTypeNameBriefByIndex(baseuti).c_str()); //eg ^Foo(a), an instance of
		    fp->write("<");
		    shbasecblock->printPostfixDataMembersSymbols(fp, slot, startpos + relpos, baseuti); //recurse
		    fp->write(">");
		  } //else
	      }
	    j++;
	  } //end while
      }
  } //printPostfixDataMembersSymbols

  void NodeBlockClass::noteBaseClassTypeAndName(UTI nuti, u32 baseitem, bool sharedbase, s32 totalsize, u32& accumsize)
  {
    //called when superclass of an oversized class instance
    //UTI nuti = getNodeType(); //maybe Hzy, use arg instead
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nsize = nut->getBitsizeAsBaseClass(); //never an element

    std::ostringstream note;
    note << "(" << nsize << " of ";
    note << totalsize << " bits, at " << accumsize << ") from "; //relpos unknown at this time
    if(baseitem == 0)
      note << "super ";

    if(sharedbase)
      note << "shared base: ";
    else
      note << "base class: ";

    note << nut->getUlamTypeClassNameBrief(nuti).c_str();
    MSG(getNodeLocationAsString().c_str(), note.str().c_str(), NOTE);

    accumsize += nsize;
  } //noteBaseClassTypeAndName

  void NodeBlockClass::noteDataMembersParseTree(UTI cuti, s32 totalsize)
  {
    //UTI cuti = getNodeType(); maybe be Hzy, use callers arg
    if(m_state.isUrSelf(cuti)) return;

    u32 accumsize = 0;
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    std::ostringstream note;
    note << "Components of " << cut->getUlamTypeClassNameBrief(cuti).c_str() << " are."; //terminating double dot
    MSG(getNodeLocationAsString().c_str(), note.str().c_str(), NOTE);

    //ulam-5 data members precede base classes; all bases are shared.
    if(m_nodeNext)
      m_nodeNext->noteTypeAndName(cuti, totalsize, accumsize); //datamember vardecls

    //inheritance
    SymbolClass * csym = NULL;
    if(m_state.alreadyDefinedSymbolClass(cuti, csym))
      {
	UTI superuti = csym->getBaseClass(0);
	//skip UrSelf to avoid extensive changes to all test answers
	if(m_state.okUTItoContinue(superuti) && !m_state.isUrSelf(superuti))
	  {
	    NodeBlockClass * superblock = getBaseClassBlockPointer(0);
	    if(!isBaseClassLinkReady(cuti, 0))
	      {
		//use SCN instead of SC in case of stub (use template's classblock)
		SymbolClassName * supercnsym = NULL;
		AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameByUTI(superuti, supercnsym);
		assert(isDefined);
		superblock = supercnsym->getClassBlockNode();
	      }
	    assert(superblock);
	    superblock->noteBaseClassTypeAndName(superuti, 0, (csym->getNumberSharingBase(0) > 1), totalsize, accumsize); //no recursion
	  }

	//ulam-5 supports multiple base classes; superclass optional
	u32 basecount = csym->getBaseClassCount() + 1; //include super
	u32 i = 1;
	while(i < basecount)
	  {
	    UTI baseuti = csym->getBaseClass(i);
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);

	    if(!isBaseClassLinkReady(cuti, i))
	      {
		//use SCN instead of SC in case of stub (use template's classblock)
		SymbolClassName * basecnsym = NULL;
		AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameByUTI(baseuti, basecnsym);
		assert(isDefined);
		basecblock = basecnsym->getClassBlockNode();
	      }
	    assert(basecblock);
	    basecblock->noteBaseClassTypeAndName(baseuti, i, (csym->getNumberSharingBase(1) > 1), totalsize, accumsize); //no recursion
	    i++;
	  } //end while

	//ulam-5 supports shared base classes;
	u32 sharedbasecount = csym->getSharedBaseClassCount();
	u32 j = 0;
	while(j < sharedbasecount)
	  {
	    UTI sbaseuti = csym->getSharedBaseClass(j);
	    NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);

	    if(!isSharedBaseClassLinkReady(cuti, j))
	      {
		//use SCN instead of SC in case of stub (use template's classblock)
		SymbolClassName * basecnsym = NULL;
		AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameByUTI(sbaseuti, basecnsym);
		assert(isDefined);
		shbasecblock = basecnsym->getClassBlockNode();
	      }
	    assert(shbasecblock);
	    s32 bitem = csym->isABaseClassItem(sbaseuti);
	    if(bitem < 0)
	      {
		//not a direct shared base
		if(!m_state.isUrSelf(sbaseuti))
		  shbasecblock->noteBaseClassTypeAndName(sbaseuti, i + j, true, totalsize, accumsize); //no recursion
	      }
	    j++;
	  } //end while
      }
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

  NodeBlockClass * NodeBlockClass::getBaseClassBlockPointer(u32 item)
  {
    if(item < m_nodeBaseClassBlockList.size())
      return m_nodeBaseClassBlockList[item];
    return NULL;
  }

  NodeBlockClass * NodeBlockClass::getSharedBaseClassBlockPointer(u32 item)
  {
    if(item < m_nodeSharedBaseClassBlockList.size())
      return m_nodeSharedBaseClassBlockList[item];
    return NULL;
  }

  void NodeBlockClass::setBaseClassBlockPointer(NodeBlockClass * classblock, u32 item)
  {
    if(m_nodeBaseClassBlockList.size() == item) //append
      m_nodeBaseClassBlockList.push_back(classblock);
    else
      m_nodeBaseClassBlockList[item] = classblock; //replace
  }

  void NodeBlockClass::setSharedBaseClassBlockPointer(NodeBlockClass * classblock, u32 item)
  {
    if(m_nodeSharedBaseClassBlockList.size() == item) //append
      m_nodeSharedBaseClassBlockList.push_back(classblock);
    else
      m_nodeSharedBaseClassBlockList[item] = classblock; //replace
  }

  void NodeBlockClass::clearBaseClassBlockList()
  {
    for(u32 i=0; i < m_nodeBaseClassBlockList.size(); i++)
      m_nodeBaseClassBlockList[i] = NULL; //we don't own the blocks, so don't destroy
    m_nodeBaseClassBlockList.clear();
  }

  void NodeBlockClass::clearSharedBaseClassBlockList()
  {
    for(u32 i=0; i < m_nodeSharedBaseClassBlockList.size(); i++)
      m_nodeSharedBaseClassBlockList[i] = NULL; //we don't own the blocks, so don't destroy
    m_nodeSharedBaseClassBlockList.clear();
  }

  void NodeBlockClass::initBaseClassBlockList()
  {
    clearBaseClassBlockList();
    setBaseClassBlockPointer(NULL, 0); //super
  }

  void NodeBlockClass::initSharedBaseClassBlockList()
  {
    clearSharedBaseClassBlockList();
  }

  bool NodeBlockClass::isBaseClassLinkReady(UTI cuti, u32 item)
  {
    //call for known subclasses only
    SymbolClass * csym = NULL;
    if(m_state.alreadyDefinedSymbolClass(cuti, csym))
      {
	assert(item < (csym->getBaseClassCount() + 1));
	UTI baseuti = csym->getBaseClass(item);
	if(!m_state.okUTItoContinue(baseuti))
	  return false; //t41013 (for super)

	//this is a subclass.
	NodeBlockClass * basecblock = getBaseClassBlockPointer(item);
	if(basecblock == NULL)
	  return false;

	UTI sblockuti = basecblock->getNodeType();
	return !(!m_state.okUTItoContinue(sblockuti) || (UlamType::compare(sblockuti, baseuti, m_state) != UTIC_SAME));
      }
    return false;
  } //isBaseClassLinkReady

  bool NodeBlockClass::isSharedBaseClassLinkReady(UTI cuti, u32 item)
  {
    //call for known subclasses only
    SymbolClass * csym = NULL;
    if(m_state.alreadyDefinedSymbolClass(cuti, csym))
      {
	assert(item < (csym->getSharedBaseClassCount()));
	UTI baseuti = csym->getSharedBaseClass(item);
	if(!m_state.okUTItoContinue(baseuti))
	  return false;

	//this is a subclass.
	NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(item);
	if(shbasecblock == NULL)
	  return false;

	UTI shblockuti = shbasecblock->getNodeType();
	return !(!m_state.okUTItoContinue(shblockuti) || (UlamType::compare(shblockuti, baseuti, m_state) != UTIC_SAME));
      }
    return false;
  } //isSharedBaseClassLinkReady

  bool NodeBlockClass::hasStringDataMembers()
  {
    bool hasStrings = NodeBlockContext::hasStringDataMembers(); //btw, does not check superclasses!!!
    if(!hasStrings)
      {
	//inheritance
	UTI cuti = getNodeType();
	SymbolClass * csym = NULL;
	if(m_state.alreadyDefinedSymbolClass(cuti, csym))
	  {
	    UTI superuti = csym->getBaseClass(0);
	    //skip UrSelf to avoid extensive changes to all test answers
	    if((superuti != Nouti) && !m_state.isUrSelf(superuti))
	      {
		NodeBlockClass * superblock = getBaseClassBlockPointer(0);
		if(superblock != NULL)
		  hasStrings |= superblock->hasStringDataMembers();
	      }

	    //ulam-5 supports multiple base classes; superclass optional
	    u32 basecount = csym->getBaseClassCount() + 1; //include super
	    u32 i = 1;
	    while(!hasStrings && (i < basecount))
	      {
		NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
		if(basecblock != NULL)
		  hasStrings |= basecblock->hasStringDataMembers();
		i++;
	      } //end while

	    //ulam-5 supports shared base classes;
	    u32 shbasecount = csym->getSharedBaseClassCount();
	    u32 j = 0;
	    while(!hasStrings && (j < shbasecount))
	      {
		NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);
		if(shbasecblock != NULL)
		  hasStrings |= shbasecblock->hasStringDataMembers();
		j++;
	      } //end while
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

    // Inheritance checks (ulam-5)
    checkMultipleInheritances();

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


UTI NodeBlockClass::checkMultipleInheritances()
{
  UTI nuti = m_state.getCompileThisIdx(); //may be Hzy getNodeType();

  //make progress even when not yet complete (t41009,t41010, t3874, t41184)
  //  if(!m_state.isComplete(nuti)) return Hzy;

  bool brtnhzy = false;

  SymbolClass * csym = NULL;
  AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
  assert(isDefined);

  //ulam-5 supports multiple base classes; superclass optional
  u32 basecount = csym->getBaseClassCount() + 1; //include super
  u32 i = 0;
  while(i < basecount)
    {
      UTI baseuti = csym->getBaseClass(i);
      //skip the ancestor of a template
      if(m_state.okUTItoContinue(baseuti))
	{
	  if(m_state.isHolder(baseuti) || !m_state.isComplete(baseuti)) //t3874, t41010 (super)
	    {
	      UTI mappedUTI = baseuti;
	      if(m_state.mappedIncompleteUTI(nuti, baseuti, mappedUTI))
		{
		  //shouldn't happen, caught at parse time (t3900, t3901)
		  assert(UlamType::compare(nuti, baseuti, m_state) != UTIC_SAME);

		  std::ostringstream msg;
		  msg << "Substituting mapped UTI" << mappedUTI;
		  msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
		  msg << " for BASE CLASS holder type: '";
		  msg << m_state.getUlamTypeNameByIndex(baseuti).c_str();
		  msg << "' UTI" << baseuti << " while labeling class: ";
		  msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		  //need to break the chain; e.g. don't want template symbol addresses used
		  setBaseClassBlockPointer(NULL, i); //force to try again!! avoid inf loop
		  m_state.resetABaseClassType(nuti, baseuti, mappedUTI);
		  baseuti = mappedUTI;
		  brtnhzy |= true;
		}
	    }

	  //this is a subclass.
	  if(!isBaseClassLinkReady(nuti, i))
	    {
	      if(!m_state.isComplete(baseuti))
		{
		  std::ostringstream msg;
		  msg << "Subclass '";
		  msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		  msg << "' inherits from '";

		  if(m_state.isClassAStub(baseuti))
		    {
		      msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();

		      msg << "', a class with pending arguments or ancestors";
		      brtnhzy |= true;
		    }
		  else if(m_state.isHolder(baseuti))
		    {
		      //O(n) search of localdefs for nicer error message (t3875, t41010)
		      u32 lostid = m_state.findTypedefNameIdInLocalsScopeByIndex(baseuti);
		      if(lostid > 0)
			{
			  msg << m_state.m_pool.getDataAsString(lostid).c_str();
			  msg << "', an unresolved local filescope typedef";
			}
		      else
			{
			  msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
			  msg << "', a holder class";
			}
		      brtnhzy |= true;
		    }
		  else
		    {
		      msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
		      msg << "', an incomplete class";
		      brtnhzy |= false; //t3889, t3831, t3674
		    }

		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		  //need to break the chain; e.g. don't want template symbol addresses used
		  setBaseClassBlockPointer(NULL, i); //force to try again!! avoid inf loop
		}
	    }
	}
      i++;
    } //end while

  //ulam-5 supports shared base classes;
  u32 shbasecount = csym->getSharedBaseClassCount();
  u32 j = 0;
  while(j < shbasecount)
    {
      UTI baseuti = csym->getSharedBaseClass(j);
      //skip the ancestor of a template
      if(m_state.okUTItoContinue(baseuti))
	{
	  if(m_state.isHolder(baseuti) || !m_state.isComplete(baseuti)) //t3874, t41010 (super)
	    {
	      UTI mappedUTI = baseuti;
	      if(m_state.mappedIncompleteUTI(nuti, baseuti, mappedUTI))
		{
		  //shouldn't happen, caught at parse time (t3900, t3901)
		  assert(UlamType::compare(nuti, baseuti, m_state) != UTIC_SAME);

		  std::ostringstream msg;
		  msg << "Substituting mapped UTI" << mappedUTI;
		  msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
		  msg << " for SHARED BASE CLASS holder type: '";
		  msg << m_state.getUlamTypeNameByIndex(baseuti).c_str();
		  msg << "' UTI" << baseuti << " while labeling class: ";
		  msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		  //need to break the chain; e.g. don't want template symbol addresses used
		  setSharedBaseClassBlockPointer(NULL, j); //force to try again!! avoid inf loop
		  m_state.resetABaseClassType(nuti, baseuti, mappedUTI);
		  baseuti = mappedUTI;
		  brtnhzy |= true;
		}
	    }

	  //this is a subclass.
	  if(!isSharedBaseClassLinkReady(nuti, j))
	    {
	      //not a direct shared base
	      if(!m_state.isComplete(baseuti))
		{
		  std::ostringstream msg;
		  msg << "Subclass '";
		  msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		  msg << "' inherits from shared base '";

		  if(m_state.isClassAStub(baseuti))
		    {
		      msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();

		      msg << "', a class with pending arguments or ancestors";
		      brtnhzy |= true;
		    }
		  else if(m_state.isHolder(baseuti))
		    {
		      //O(n) search of localdefs for nicer error message (t3875, t41010)
		      u32 lostid = m_state.findTypedefNameIdInLocalsScopeByIndex(baseuti);
		      if(lostid > 0)
			{
			  msg << m_state.m_pool.getDataAsString(lostid).c_str();
			  msg << "', an unresolved local filescope typedef";
			}
		      else
			{
			  msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
			  msg << "', a holder class";
			}
		      brtnhzy |= true;
		    }
		  else
		    {
		      msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
		      msg << "', an incomplete class";
		      brtnhzy |= false; //t3889, t3831, t3674
		    }

		  s32 bitem = csym->isABaseClassItem(baseuti);
		  if(bitem < 0)
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		  //else drop msg
		  //need to break the chain; e.g. don't want template symbol addresses used
		  setSharedBaseClassBlockPointer(NULL, j); //force to try again!! avoid inf loop
		}
	    }
	}
      j++;
    } //end while


  if(brtnhzy) //any base class is hazy..
    {
      setNodeType(Hzy); //t41150
      m_state.setGoAgain();
      return Hzy; //short-circuit holders and stubs (e.g. t41010, t3831, t3889)
    }

  //o.w. continue..

  bool errs = false;
  i = 0;
  while(i < basecount)
    {
      UTI baseuti = csym->getBaseClass(i);

      //look up this instance
      SymbolClass * basecsym = NULL;
      if(m_state.alreadyDefinedSymbolClass(baseuti, basecsym))
	{
	  NodeBlockClass * baseclassblock = basecsym->getClassBlockNode();
	  setBaseClassBlockPointer(baseclassblock, i); //fixed

	  if(!isBaseClassLinkReady(nuti, i))
	    {
	      setBaseClassBlockPointer(NULL, i); //force to try again!! avoid inf loop
	      std::ostringstream msg;
	      msg << "Subclass '";
	      msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	      if(baseclassblock->getNodeType() == Nav)
		{
		  msg << "' inherits from illegal '";
		  msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str() << "'";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		  errs = true;
		}
	      else
		{
		  msg << "' inherits from unready '";
		  msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str() << "'";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		  brtnhzy = true;
		}
	    }
	}
      i++;
    } //end while

  j = 0;
  while(j < shbasecount)
    {
      UTI baseuti = csym->getSharedBaseClass(j);

      //look up this instance
      SymbolClass * basecsym = NULL;
      if(m_state.alreadyDefinedSymbolClass(baseuti, basecsym))
	{
	  NodeBlockClass * baseclassblock = basecsym->getClassBlockNode();
	  setSharedBaseClassBlockPointer(baseclassblock, j); //fixed

	  if(!isSharedBaseClassLinkReady(nuti, j))
	    {
	      setSharedBaseClassBlockPointer(NULL, j); //force to try again!! avoid inf loop
	      std::ostringstream msg;
	      msg << "Subclass '";
	      msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	      if(baseclassblock->getNodeType() == Nav)
		{
		  msg << "' inherits from illegal shared base '";
		  msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str() << "'";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		  errs = true;
		}
	      else
		{
		  msg << "' inherits from unready shared base '";
		  msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str() << "'";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		  brtnhzy = true;
		}
	    }
	}
      j++;
    } //end while

  if(errs)
    {
      setNodeType(Nav);
      return Nav; //t41013, t3599 (super)
    }
  else if(brtnhzy)
    {
      setNodeType(Hzy);
      m_state.setGoAgain();
      return Hzy;
    }
  //else
  setNodeType(nuti); //t41150

  //super links ready or not
  i = 0;
  while(i < basecount)
    {
      UTI baseuti = csym->getBaseClass(i);
      if(m_state.okUTItoContinue(baseuti))
	{
	  ULAMCLASSTYPE baseclasstype = m_state.getUlamTypeByIndex(baseuti)->getUlamClassType();
	  ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(nuti)->getUlamClassType();
	  if(m_state.isClassAQuarkUnion(baseuti))
	    {
	      std::ostringstream msg;
	      msg << "Subclass '";
	      msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	      msg << "' inherits from '";
	      msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str() << "'";
	      msg << ", a currently unsupported base class type: quark-union";
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      errs = true;
	    }
	  else if(classtype == UC_TRANSIENT)
	    {
	      //allow transients to inherit from either transients or quarks (t3723,t3725);
	      // but not quark-union (t41352)
	      if((baseclasstype != UC_TRANSIENT) && (baseclasstype != UC_QUARK))
		{
		  std::ostringstream msg;
		  msg << "Subclass '";
		  msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		  msg << "' inherits from '";
		  msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
		  msg << "', a class that's neither a transient nor a quark";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		  errs = true;
		}
	    }
	  else if(baseclasstype != UC_QUARK)
	    {
	      //O(n) search of localdefs for nicer error message (t3875, t41010)
	      u32 lostid = m_state.findTypedefNameIdInLocalsScopeByIndex(baseuti);
	      if(lostid > 0)
		{
		  std::ostringstream msg;
		  msg << "Subclass '";
		  msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		  msg << "' inherits from '";
		  msg << m_state.m_pool.getDataAsString(lostid).c_str();
		  msg << "', an unresolved local filescope typedef";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		  errs = true;
		}
	      else
		{
		  //for all others (elements and quarks)
		  //must be "seen" by now; e.g.typedef array of quarks t3674,t3862,t41150
		  std::ostringstream msg;
		  msg << "Subclass '";
		  msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		  msg << "' inherits from '";
		  msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
		  msg << "', a class that's not a quark";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		  errs = true;
		}
	    } //else
	} //not ready
      i++;
    } //end while

  //super links ready or not
  j = 0;
  while(j < shbasecount)
    {
      UTI baseuti = csym->getSharedBaseClass(j);
      if(m_state.okUTItoContinue(baseuti))
	{
	  ULAMCLASSTYPE baseclasstype = m_state.getUlamTypeByIndex(baseuti)->getUlamClassType();
	  ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(nuti)->getUlamClassType();
	  if(m_state.isClassAQuarkUnion(baseuti))
	    {
	      std::ostringstream msg;
	      msg << "Subclass '";
	      msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	      msg << "' inherits from shared base '";
	      msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str() << "'";
	      msg << ", a currently unsupported base class type: quark-union";
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      errs = true;
	    }
	  else if(classtype == UC_TRANSIENT)
	    {
	      //allow transients to inherit from either transients or quarks (t3723,t3725)
	      if((baseclasstype != UC_TRANSIENT) && (baseclasstype != UC_QUARK))
		{
		  std::ostringstream msg;
		  msg << "Subclass '";
		  msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		  msg << "' inherits from shared base '";
		  msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
		  msg << "', a class that's neither a transient nor a quark";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		  errs = true;
		}
	    }
	  else if(baseclasstype != UC_QUARK)
	    {
	      //O(n) search of localdefs for nicer error message (t3875, t41010)
	      u32 lostid = m_state.findTypedefNameIdInLocalsScopeByIndex(baseuti);
	      if(lostid > 0)
		{
		  std::ostringstream msg;
		  msg << "Subclass '";
		  msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		  msg << "' inherits from shared base '";
		  msg << m_state.m_pool.getDataAsString(lostid).c_str();
		  msg << "', an unresolved local filescope typedef";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		  errs = true;
		}
	      else
		{
		  //for all others (elements and quarks)
		  //must be "seen" by now; e.g.typedef array of quarks t3674,t3862,t41150
		  std::ostringstream msg;
		  msg << "Subclass '";
		  msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		  msg << "' inherits from shared base '";
		  msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
		  msg << "', a class that's not a quark";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		  errs = true;
		}
	    } //else
	} //not ready
      j++;
    } //end while

  if(errs)
    setNodeType(Nav);

  //done with inheritance checks, return.
  return getNodeType();
} //checkMultipleInheritances

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
		    if(m_state.isAStringType(auti))
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

  void NodeBlockClass::checkDuplicateFunctionsInClassAndAncestors()
  {
    //starts here with fresh map!
    u32 probcount = 0;
    FSTable mangledFunctionMapWithReturnType;

    // check all functions with matching args for the same return type (regardless if virtual)
    if(!isEmpty())
      {
	UTI nuti = getNodeType();
	BaseclassWalker walker;
	walker.init(nuti);

	UTI baseuti = Nouti;
	while(walker.getNextBase(baseuti, m_state))
	  {
	    SymbolClass * basecsym = NULL;
	    if(m_state.alreadyDefinedSymbolClass(baseuti, basecsym))
	      {
		NodeBlockClass * basecblock = basecsym->getClassBlockNode();
		assert(basecblock);

		basecblock->checkMatchingFunctions(); //t3297

		basecblock->checkDuplicateFunctions(mangledFunctionMapWithReturnType, probcount);

		walker.addAncestorsOf(basecsym);
	      }
	  } //end while
      }
    mangledFunctionMapWithReturnType.clear();
  } //checkDuplicateFunctionsInClassAndAncestors

  u32 NodeBlockClass::checkDuplicateFunctions(FSTable& mangledFunctionMap, u32& probcount)
  {
    // check all the function names for duplicate definitions with different return types
    if(!isEmpty())
      {
	//first check own table for duplicate function definitions
	//populate map, updates probcount, output errors
	m_functionST.checkTableOfFunctionsSignatureReturnTypes(mangledFunctionMap, probcount);
      }
    return probcount;
  } //checkDuplicateFunctions

  void NodeBlockClass::checkMatchingFunctions()
  {
    // check all functions in this class with matching argument types; not ancestors.
    if(!isEmpty())
      {
	//starts here with fresh map!
	u32 probcount = 0;
	FSTable mangledFunctionMap;

	//checkTableOfFunctionsInAncestor is without error message
	//In this class, not ancestors..tricky as to what's allowed per c++.
	//populate map, update probcount, output errors
	m_functionST.checkTableOfFunctions(mangledFunctionMap, probcount);
	mangledFunctionMap.clear();
      }
  } //checkMatchingFunctions

  void NodeBlockClass::calcMaxDepthOfFunctions()
  {
    // for all the function names, calculate their max depth
    if(!isEmpty())
      m_functionST.calcMaxDepthForTableOfFunctions(); //returns prob counts, outputs errs
  }

  void NodeBlockClass::calcMaxIndexOfVirtualFunctions()
  {
    UTI nuti = getNodeType();
    s32 maxidx = getVirtualMethodMaxIdx();

    if(maxidx != UNKNOWNSIZE)
      return; //short-circuit, re-called if any subclass is waiting on a super

    u32 basesnotready = 0;
    // quick check that all first-level base classes have completed their own vtables before continuing;
    //ulam-5 supports multiple base classes; superclass optional
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
    assert(isDefined);

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	if(baseuti != Nouti)
	  {
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    assert(basecblock);

	    s32 basesmaxidx = basecblock->getVirtualMethodMaxIdx();
	    if(basesmaxidx < 0)
	      {
		std::ostringstream msg;
		msg << "Subclass '";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		msg << "' inherits from '";
		msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
		msg << "', a class whose max index for virtual functions is still unknown";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		basesnotready++;
		//return;
	      }
	  }
	i++;
      } //end while

    if(basesnotready == 0)
      {
	// for all the virtual function names, calculate their index in the VM Table
	// adds up each base classes vowned indexes, entire ancestory tree, for maxidx
	m_functionST.calcMaxIndexForVirtualTableOfFunctions(maxidx);

	//in order declared..
	s32 vownedcount = 0;
	if(m_nodeNext) m_nodeNext->calcMaxIndexOfVirtualFunctionInOrderOfDeclaration(csym, vownedcount);

	maxidx += vownedcount; //update ref arg

	setVirtualMethodMaxIdx(maxidx);
      }
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

void NodeBlockClass::checkCustomArrayTypeFunctions()
  {
    if(isEmpty()) return;

    // custom array flag set at parse time
    UTI cuti = getNodeType();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    if(((UlamTypeClass *) cut)->isCustomArray())
      m_functionST.checkCustomArrayTypeFuncs();

    //ulam-5 supports multiple base classes; superclass optional;
    //ulam-5 supports shared base classes;
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	assert(baseuti != Hzy);
	if(baseuti != Nouti)
	  {
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    if(!basecblock) //might be during resolving loop, not set yet
	      {
		std::ostringstream msg;
		msg << "Subclass '";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		msg << "' inherits from '";
		msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
		msg << "', an INCOMPLETE Base class; ";
		msg << "No check of custom array type functions";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	    basecblock->checkCustomArrayTypeFunctions();
	  }
	i++;
      } //end while
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

	if(catype == Nouti)
	  {
	    //ulam-5 supports multiple base classes; superclass optional
	    //ulam-5 supports shared base classes;
	    SymbolClass * csym = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
	    assert(isDefined);

	    u32 basecount = csym->getBaseClassCount() + 1; //include super
	    u32 i = 0;
	    while((catype == Nouti) && (i < basecount))
	      {
		UTI baseuti = csym->getBaseClass(i);
		if(baseuti != Nouti)
		  {
		    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
		    assert(basecblock);
		    catype = basecblock->getCustomArrayTypeFromGetFunction();
		  }
		i++;
	      } //end while
	  }
      }
    return catype; //from first base class?
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

	if(camatches == 0)
	  {
	    //ulam-5 supports multiple base classes; superclass optional
	    //ulam-5 supports shared base classes;
	    SymbolClass * csym = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
	    assert(isDefined);

	    u32 basecount = csym->getBaseClassCount() + 1; //include super
	    u32 i = 0;
	    while((i < basecount))
	      {
		UTI baseuti = csym->getBaseClass(i);
		if(baseuti != Nouti)
		  {
		    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
		    assert(basecblock);
		    bool tmphzyargs = false;
		    camatches += basecblock->getCustomArrayIndexTypeFromGetFunction(rnode, idxuti, tmphzyargs);
		    hasHazyArgs |= tmphzyargs;
		  }
		i++;
	      } //end while
	  }
      }
    return camatches; //search all base classes, just super, or first one with a match???
  } //getCustomArrayIndexTypeFromGetFunction

  bool NodeBlockClass::hasCustomArrayLengthofFunction()
  {
    UTI cuti = getNodeType();
    bool camatch = m_functionST.hasCustomArrayLengthofFunction();

    if(!camatch)
      {
	if(m_state.isClassAStub(cuti))
	  {
	    //search template
	    NodeBlockClass * prevblock = (NodeBlockClass *) getPreviousBlockPointer();
	    assert(prevblock);
	    camatch = prevblock->hasCustomArrayLengthofFunction();
	  }
      }

    if(!camatch)
      {
	//ulam-5 supports multiple base classes; superclass optional
	//ulam-5 supports shared base classes;
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
	assert(isDefined);

	u32 basecount = csym->getBaseClassCount() + 1; //include super
	u32 i = 0;
	while((i < basecount))
	  {
	    UTI baseuti = csym->getBaseClass(i);
	    if(baseuti != Nouti)
	      {
		NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
		assert(basecblock);
		camatch += basecblock->hasCustomArrayLengthofFunction();
	      }
	    i++;
	  } //end while
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
    UTI nuti = getNodeType();

    // ulam-5 data members precede base classes
    if(m_nodeNext)
      aok = m_nodeNext->buildDefaultValue(wlen, dvref); //side-effect for dm vardecls

    m_buildingDefaultValueInProgress = false; //clear

    //ulam-5 supports multiple base classes; superclass optional
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
    assert(isDefined);
    s32 pos = 0;
    if(m_state.getUlamTypeByIndex(nuti)->getUlamClassType() == UC_ELEMENT)
      pos += ATOMFIRSTSTATEBITPOS;

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	if((baseuti != Nouti))
	  {
	    //cleaner not circumventing SymbolClass (e.g. t41182,3, t3532)
	    BV8K bvbase;
	    if((m_state.getDefaultClassValue(baseuti, bvbase)))
	      {
		s32 relpos = csym->getBaseClassRelativePosition(i);
		assert(relpos >= 0);
		bvbase.CopyBV(0, (u32) pos + relpos, m_state.getBaseClassBitSize(baseuti), dvref); //only its data members (no bases)
	      }
	    else
	      aok = false;
	  }
	i++;
      } //end while

    //ulam-5 supports shared base classes; after data members
    u32 shbasecount = csym->getSharedBaseClassCount();
    u32 j = 0;
    while(j < shbasecount)
      {
	UTI baseuti = csym->getSharedBaseClass(j);
	s32 bitem = csym->isABaseClassItem(baseuti);
	if(bitem < 0)
	  {
	    //not a direct shared base
	    BV8K bvbase;
	    if((m_state.getDefaultClassValue(baseuti, bvbase)))
	      {
		s32 relpos = csym->getSharedBaseClassRelativePosition(j);
		assert(relpos >= 0);
		bvbase.CopyBV(0, (u32) pos + relpos, m_state.getBaseClassBitSize(baseuti), dvref); //only its data members
	      }
	    else
	      aok = false;
	  }
	j++;
      } //end while

    return aok;
  } //buildDefaultValue

  bool NodeBlockClass::buildDefaultValueForClassConstantDefs()
  {
    if(classConstantsReady())
      return true;

    bool aok = true;
    UTI nuti = getNodeType();

    //ulam-5 supports multiple base classes; superclass optional
    //ulam-5 supports shared base classes;
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
    assert(isDefined);

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	if(baseuti != Nouti)
	  {
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    assert(basecblock);
	    aok &= basecblock->buildDefaultValueForClassConstantDefs();
	  }
	i++;
      } //end while

    if(aok)
      if(m_nodeNext)
	aok = m_nodeNext->buildDefaultValueForClassConstantDefs();

    if(aok)
      m_classConstantsReady = true;

    return aok;
  } //buildDefaultValueForClassConstantDefs

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
    UTI nuti = getNodeType();

    s32 supers = 0;

    //ulam-5 supports multiple base classes; superclass optional
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
    assert(isDefined);

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	if((baseuti != Nouti) && !csym->isDirectSharedBase(i))
	  {
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    assert(basecblock);
	    supers += basecblock->getNumberOfSymbolsInTable();
	  }
	i++;
      } //end while

    //ulam-5 supports shared base classes;
    UTI cuti = m_state.getCompileThisIdx();
    if(nuti == cuti) //count shared symbols only once!
      {
	u32 shbasecount = csym->getSharedBaseClassCount();
	u32 j = 0;
	while(j < shbasecount)
	  {
	    UTI baseuti = csym->getSharedBaseClass(j);
	    if(baseuti != Nouti)
	      {
		NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);
		assert(shbasecblock);
		supers += shbasecblock->getNumberOfSymbolsInTable();
	      }
	    j++;
	  } //end while
      }
    return supers + NodeBlock::getNumberOfSymbolsInTable();
  } //getNumberOfSymbolsInTable

  u32 NodeBlockClass::getNumberOfPotentialClassArgumentSymbols()
  {
    return getNumberOfArgumentNodes();
  }

  u32 NodeBlockClass::getSizeOfSymbolsInTable()
  {
    UTI nuti = getNodeType();
    s32 supers = 0;

    //ulam-5 supports multiple base classes; superclass optional
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
    assert(isDefined);

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	if((baseuti != Nouti) && !csym->isDirectSharedBase(i))
	  {
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    assert(basecblock);
	    supers += basecblock->getSizeOfSymbolsInTable();
	  }
	i++;
      } //end while

    //ulam-5 supports shared base classes;
    UTI cuti = m_state.getCompileThisIdx();
    if(nuti == cuti) //count shared symbols only once!
      {
	u32 shbasecount = csym->getSharedBaseClassCount();
	u32 j = 0;
	while(j < shbasecount)
	  {
	    UTI baseuti = csym->getSharedBaseClass(i);
	    if(baseuti != Nouti)
	      {
		NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);
		assert(shbasecblock);
		supers += shbasecblock->getSizeOfSymbolsInTable();
	      }
	    j++;
	  } //end while
      }

    return supers + m_ST.getTotalSymbolSize();
  } //getSizeOfSymbolsInTable

  s32 NodeBlockClass::getBitSizesOfVariableSymbolsInTable()
  {
    UTI cuti = m_state.getCompileThisIdx(); //getNodeType() maybe Hzy
    s32 superbs = 0;

    //ulam-5 supports multiple base classes; superclass optional
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	assert(baseuti != Hzy);
	if(baseuti != Nouti)
	  {
	    s32 bs = m_state.getBitSize(baseuti); //may contain shared bits!
	    //s32 bs = m_state.getBaseClassBitSize(baseuti); //only dm bits!
	    if(bs < 0)
	      return bs; //error if any base class size is negative
	    else
	      superbs += bs;
	  }
	i++;
      } //end while

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

    //ulam-5 supports multiple base classes; superclass optional
    //ulam-5 supports shared base classes;
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	if((baseuti != Nouti))
	  {
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    assert(basecblock);
	    m_state.pushClassContext(baseuti, basecblock, basecblock, false, NULL); //use baseuti, not cuti (t3451)
	    s32 bs = basecblock->getMaxBitSizeOfVariableSymbolsInTable();
	    m_state.popClassContext(); //restore

	    if(bs < 0)
	      return bs; //error if any base class size is negative
	    else if(bs > superbs)
	      superbs = bs;  //max of all base class maxes
	  }
	i++;
      } //end while

    if(m_ST.getTableSize() == 0 && superbs == 0)
      return EMPTYSYMBOLTABLE; //should allow no variable data members

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
    UTI nuti = getNodeType();
    s32 superfs = 0;

    //ulam-5 supports multiple base classes; superclass optional
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
    assert(isDefined);

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	if((baseuti != Nouti) && !csym->isDirectSharedBase(i))
	  {
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    assert(basecblock);
	    superfs += basecblock->getNumberOfFuncSymbolsInTable();
	  }
	i++;
      } //end while

    //ulam-5 supports shared base classes;
    UTI cuti = m_state.getCompileThisIdx();
    if(nuti == cuti) //count shared func symbols once!
      {
	u32 shbasecount = csym->getSharedBaseClassCount();
	u32 j = 0;
	while(j < shbasecount)
	  {
	    UTI baseuti = csym->getSharedBaseClass(j);
	    if(baseuti != Nouti)
	      {
		NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(i);
		assert(shbasecblock);
		superfs += shbasecblock->getNumberOfFuncSymbolsInTable();
	      }
	    j++;
	  } //end while
      }

    return superfs + m_functionST.getTableSize();
  } //getNumberOfFuncSymbolsInTable

  u32 NodeBlockClass::getSizeOfFuncSymbolsInTable()
  {
    UTI nuti = getNodeType();
    s32 superfs = 0;

    //ulam-5 supports multiple base classes; superclass optional
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
    assert(isDefined);

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	if((baseuti != Nouti) && !csym->isDirectSharedBase(i))
	  {
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    assert(basecblock);
	    superfs += basecblock->getSizeOfFuncSymbolsInTable();
	  }
	i++;
      } //end while

    //ulam-5 supports shared base classes;
    UTI cuti = m_state.getCompileThisIdx();
    if(nuti == cuti) //count shared func sizes once
      {
	u32 shbasecount = csym->getSharedBaseClassCount();
	u32 j = 0;
	while(j < shbasecount)
	  {
	    UTI baseuti = csym->getSharedBaseClass(j);
	    if(baseuti != Nouti)
	      {
		NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);
		assert(shbasecblock);
		superfs += shbasecblock->getSizeOfFuncSymbolsInTable();
	      }
	    j++;
	  } //end while
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

    //ulam-5 data members precede bases; all bases are shared
    m_ST.initializeElementDefaultsForEval(uv, cuti);

    //ulam-5 shared bases come after all the base class data members
    if(UlamType::compare(cuti, buti, m_state) != UTIC_SAME)
      return;

    //ulam-5 supports multiple base classes; superclass optional
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(buti, csym);
    assert(isDefined);

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	assert(baseuti != Hzy);
	if((baseuti != Nouti)) // && !csym->isDirectSharedBase(i))
	  {
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    assert(basecblock);
	    m_state.pushClassContext(baseuti, basecblock, basecblock, false, NULL);
	    basecblock->initElementDefaultsForEval(uv, cuti);
	    m_state.popClassContext(); //restore
	  }
	i++;
      } //end while

    //ulam-5 supports shared base classes;
    u32 shbasecount = csym->getSharedBaseClassCount();
    u32 j = 0;
    while(j < shbasecount)
      {
	UTI baseuti = csym->getSharedBaseClass(j);
	assert(baseuti != Hzy);
	if(baseuti != Nouti)
	  {
	    s32 bitem = csym->isABaseClassItem(baseuti);
	    if(bitem < 0)
	      {
		//not a direct shared base
		NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);
		assert(shbasecblock);
		m_state.pushClassContext(baseuti, shbasecblock, shbasecblock, false, NULL);
		shbasecblock->initElementDefaultsForEval(uv, cuti);
		m_state.popClassContext(); //restore
	      }
	  }
	j++;
      } //end while
    return;
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

    //ulam-5 put data members first, then bases in declared order, and end w non-direct shared
    TBOOL rtntb = TBOOL_TRUE;
    m_bitPackingInProgress = true;;

    if(m_nodeNext)
      {
	TBOOL nodetb = m_nodeNext->packBitsInOrderOfDeclaration(reloffset);
	rtntb = Node::minTBOOL(rtntb, nodetb);
      }

    m_bitPackingInProgress = false;

    //ulam-5 supports multiple base classes; superclass optional
    UTI nuti = getNodeType();
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
    assert(isDefined);

    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	if(baseuti == Hzy)
	  {
	    return TBOOL_HAZY;
	  }

	if((baseuti != Nouti))// && !csym->isDirectSharedBase(i))
	  {
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    if(!basecblock)
	      {
		std::ostringstream msg;
		msg << "Subclass '";
		msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		msg << "' inherits from '";
		msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
		msg << "', an INCOMPLETE Base class; ";
		msg << "No bit packing of variable data members";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		return TBOOL_HAZY;
	      }

	    assert(UlamType::compare(basecblock->getNodeType(), baseuti, m_state) == UTIC_SAME);
	    if(!m_state.isComplete(baseuti))
	      {
		return TBOOL_HAZY;
	      }

	    s32 baseoffset = m_state.getBaseClassBitSize(baseuti);
	    assert(baseoffset >= 0); //t3318,t3755
	    csym->setBaseClassRelativePosition(i, reloffset); //t3102, t3536 before updating reloffset
	    reloffset += baseoffset;
	  }
	i++;
      } //end while

    //ulam-5 supports shared base classes:
    UTI cuti = m_state.getCompileThisIdx();
    if(nuti == cuti)
      {
	u32 shbasecount = csym->getSharedBaseClassCount();
	u32 j = 0;
	while(j < shbasecount)
	  {
	    UTI baseuti = csym->getSharedBaseClass(j);
	    if(baseuti == Hzy)
	      {
		return TBOOL_HAZY;
	      }

	    if(baseuti != Nouti)
	      {
		NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);
		if(!shbasecblock)
		  {
		    std::ostringstream msg;
		    msg << "Subclass '";
		    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
		    msg << "' inherits from '";
		    msg << m_state.getUlamTypeNameBriefByIndex(baseuti).c_str();
		    msg << "', an INCOMPLETE Shared Base class; ";
		    msg << "No bit packing of variable data members";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		    return TBOOL_HAZY;
		  }

		assert(UlamType::compare(shbasecblock->getNodeType(), baseuti, m_state) == UTIC_SAME);
		if(!m_state.isComplete(baseuti))
		  {
		    return TBOOL_HAZY;
		  }

		u32 baseoffset = m_state.getBaseClassBitSize(baseuti);
		s32 bitem = csym->isABaseClassItem(baseuti);
		if(bitem < 0)
		  {
		    //not a direct shared base
		    csym->setSharedBaseClassRelativePosition(j, reloffset); //directshared
		    reloffset += baseoffset;
		  }
		else
		  {
		    assert(csym->isDirectSharedBase(bitem));
		    u32 directsharedoffset = csym->getBaseClassRelativePosition(bitem);
		    csym->setSharedBaseClassRelativePosition(j, directsharedoffset);
		  }
	      }
	    j++;
	  } //end while
      }
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
    //in parse tree order, the order of appearance (ulam-5)
    if(m_nodeNext)
      m_nodeNext->generateFunctionInDeclarationOrder(fp, declOnly, classtype);
  }

  //header .h file
  void NodeBlockClass::genCode(File * fp, UVPass& uvpass)
  {
    //use the instance UTI instead of the node's original type (hmm?)
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

    //output a chart of data members with name, length, and position
    genCodeDataMemberChartAsComment(fp, cuti);

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

  void NodeBlockClass::genCodeDataMemberChartAsComment(File * fp, UTI cuti)
  {
    u32 accumsize = 0;
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    s32 totalsize = cut->getTotalBitSize(); //actual for elements(as in mangled name)
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    m_state.indent(fp);
    fp->write("/*__________________________________________________\n");
    m_state.indent(fp);
    fp->write("| COMPONENTS of ");
    fp->write(cut->getUlamTypeClassNameBrief(cuti).c_str());
    fp->write(" (");
    fp->write_decimal_unsigned(csym->getRegistryNumber());
    fp->write(")");
    fp->write(" (");
    fp->write_decimal(totalsize);
    fp->write(" bits total) are: \n");
    m_state.indent(fp);
    fp->write("| \n"); //blank

    m_state.indent(fp);
    fp->write("| Pos\t| Bits\t| Name\t| Type (classid)\n");

    //ulam-5 data members precede base classes
    if(m_nodeNext)
      m_nodeNext->genTypeAndNameEntryAsComment(fp, totalsize, accumsize); //dm vardecls

    //ulam-5 supports multiple base classes; superclass optional
    if(csym->isQuarkUnion())
      accumsize = totalsize; //i.e. max dm bitsize, not cummulative

    //ulam-5 supports multiple base classes; superclass optional.
    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	if(m_state.okUTItoContinue(baseuti)) //skips implicit/default super here
	  {
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);

	    if(!isBaseClassLinkReady(cuti, i))
	      {
		//use SCN instead of SC in case of stub (use template's classblock)
		SymbolClassName * basecnsym = NULL;
		AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameByUTI(baseuti, basecnsym);
		assert(isDefined);
		basecblock = basecnsym->getClassBlockNode();
	      }
	    assert(basecblock);
	    s32 pos = csym->getBaseClassRelativePosition(i);
	    basecblock->genBaseClassTypeAndNameEntryAsComment(fp, baseuti, pos, accumsize, i); //no recursion
	  }
	i++;
      } //end while

    //ulam-5 supports shared base classes;
    u32 shbasecount = csym->getSharedBaseClassCount();
    u32 j = 0;
    while(j < shbasecount)
      {
	UTI baseuti = csym->getSharedBaseClass(j);
	NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);
	if(!isSharedBaseClassLinkReady(cuti, j))
	  {
	    //use SCN instead of SC in case of stub (use template's classblock)
	    SymbolClassName * basecnsym = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClassNameByUTI(baseuti, basecnsym);
	    assert(isDefined);
	    shbasecblock = basecnsym->getClassBlockNode();
	  }
	assert(shbasecblock);

	s32 bitem = csym->isABaseClassItem(baseuti);
	if(bitem < 0) //not a direct shared base
	  {
	    s32 pos = csym->getSharedBaseClassRelativePosition(j);
	    shbasecblock->genBaseClassTypeAndNameEntryAsComment(fp, baseuti, pos, accumsize, j+1000); //no recursion
	  }
	j++;
      } //end while

    m_state.indent(fp);
    fp->write("|___________________________________________________\n");
    m_state.indent(fp);
    fp->write("*/"); //end of comment
    GCNL;
  } //genClassDataMemberChartAsComment

  void NodeBlockClass::genBaseClassTypeAndNameEntryAsComment(File * fp, UTI nuti, s32 atpos, u32& accumsize, u32 baseitem)
  {
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nsize = nut->getBitsizeAsBaseClass();

    // "// | Position\t| Bitsize\t| Name\t| Type (classid)"
    m_state.indent(fp);
    fp->write("| ");
    fp->write_decimal_unsigned(atpos); //at
    fp->write("\t| ");
    fp->write_decimal(nsize); // of totalsize
    if(baseitem == 0)
      fp->write("\t| super\t| "); //name
    else
      fp->write("\t| base\t| "); //name
    fp->write(nut->getUlamTypeClassNameBrief(nuti).c_str());
    SymbolClass * csym = NULL;
    if(m_state.alreadyDefinedSymbolClass(nuti, csym))
      {
	fp->write(" (");
	fp->write_decimal_unsigned(csym->getRegistryNumber());
	fp->write(")");
      }
    fp->write("\n");

    //quarkunions don't accumulate sizes of dm, they use max dm size;
    //quarkunions cannot be base classes, or have any (except UrSelf); (t3209, t41145)
    assert((atpos == (s32) accumsize) || m_state.isClassAQuarkUnion(m_state.getCompileThisIdx()));
    accumsize += nsize;
  } //genBaseClassTypeAndNameEntryAsComment

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

    genThisUlamBaseClassAsAHeaderComment(fp);

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
    fp->write(" /* Requiring quarks to advertise their size in a std way.*/\n");
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

    genThisUlamBaseClassAsAHeaderComment(fp);

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
    fp->write("\n");

    // output public element type enum (t3173)
    u16 elementType = m_state.getAClassElementType(cuti);
    m_state.indent(fp);
    fp->write("enum { \n");
    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("ELEMENT_TYPE = ");
    fp->write_hexadecimal(elementType);
    fp->write(" /* Requiring elements to advertise their type in a std way.*/\n");
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("};"); GCNL;

    if(m_state.isEmptyElement(cuti))
      {
	fp->write("\n");
	m_state.indent(fp);
	fp->write("virtual bool IsTheEmptyClass() const { return true; }");
	GCNL;
      }

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

    genThisUlamBaseClassAsAHeaderComment(fp);

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

    //genThisUlamBaseClassAsAHeaderComment(fp);

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

  void NodeBlockClass::genThisUlamBaseClassAsAHeaderComment(File * fp)
  {
    UTI cuti = m_state.getCompileThisIdx(); //getContextBlock?
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    //ulam-5 supports multiple base classes; superclass optional
    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;

    fp->write(" /*");

    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	//skip the ancestor of a template
	if(baseuti != Nouti)
	  {
	    if(i==0)
	      fp->write(" :");
	    else
	      fp->write(" +");
	    fp->write(m_state.getUlamTypeByIndex(baseuti)->getUlamTypeMangledName().c_str());
	  }
	i++;
      } //end while
    fp->write(" */\n");
  } //genThisUlamBaseClassAsAHeaderComment

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
    fp->write("() : UlamQuark<EC>()\n"); //no UUID for quarks
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
    fp->write("() : UlamTransient<EC>()\n"); //no UUID for transients
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
    fp->write("\n\n");
    m_state.indent(fp);
    fp->write("//BUILT-IN FUNCTIONS:\n\n");

    //define built-in init method for any "data member" constant class or arrays:
    generateBuiltinConstantClassOrArrayInitializationFunction(fp, declOnly);

    //generate 3 UlamClass:: methods for smart ulam debugging
    u32 dmcount = 0; //pass ref
    generateUlamClassInfoFunction(fp, declOnly, dmcount);
    generateUlamClassInfoCount(fp, declOnly, dmcount); //after dmcount is updated by nodes
    generateUlamClassGetMangledName(fp, declOnly);
    generateUlamClassGetMangledNameAsStringIndex(fp, declOnly);
    generateUlamClassGetNameAsStringIndex(fp, declOnly);

    genCodeBuiltInFunctionGetClassLength(fp, declOnly, classtype);

    genCodeBuiltInFunctionGetClassRegistrationNumber(fp, declOnly, classtype); //ulam-4

    genCodeBuiltInFunctionBuildDefaultAtom(fp, declOnly, classtype);

    genCodeBuiltInVirtualTableStartOffsetHelper(fp, declOnly, classtype);

    genCodeBuiltInVirtualTable(fp, declOnly, classtype);

    // 'is' quark related for both class types; overloads is-Method with THE_INSTANCE arg
    genCodeBuiltInFunctionIsMethodRelatedInstance(fp, declOnly, classtype);
    genCodeBuiltInIsMethodByRegistrationNumber(fp, declOnly, classtype);

    // returns relative position of baseclass or self; -1 if unrelated.
    genCodeBuiltInFunctionGetRelPosMethodRelatedInstance(fp, declOnly, classtype);
    genCodeBuiltInFunctionGetRelPosMethodRelatedInstanceByRegistrationNumber(fp, declOnly, classtype);

    genCodeBuiltInFunctionNumberOfBases(fp, declOnly, classtype);
    genCodeBuiltInFunctionNumberOfDirectBases(fp, declOnly, classtype);
    genCodeBuiltInFunctionGetOrderedBaseClass(fp, declOnly, classtype);
    genCodeBuiltInFunctionIsDirectBaseClass(fp, declOnly, classtype);


    // 'is' is only for element/classes
    if(classtype == UC_ELEMENT)
      {
	genCodeBuiltInFunctionGetElementType(fp, declOnly, classtype); //ulam-4
	generateInternalIsMethodForElement(fp, declOnly); //overload
	generateInternalTypeAccessorsForElement(fp, declOnly);
      }
    else if(classtype == UC_QUARK)
      {
	generateGetPosForQuark(fp, declOnly);
      }
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
    if(classtype == UC_LOCALSFILESCOPE) return;

    UTI cuti = m_state.getCompileThisIdx();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("//helper method\n");

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

    m_state.indent(fp);
    fp->write("return (");
    fp->write(m_state.getGetRelPosMangledFunctionName(cuti));
    fp->write("(cptrarg->");
    fp->write(m_state.getClassRegistrationNumberFunctionName(cuti));
    fp->write("()) >= 0);"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //is-related\n\n");
  } //genCodeBuiltInFunctionIsMethodRelatedInstance

  void NodeBlockClass::genCodeBuiltInIsMethodByRegistrationNumber(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    if(classtype == UC_LOCALSFILESCOPE)
      return;

    UTI cuti = m_state.getCompileThisIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    if(declOnly)
      {
	//VTable accessor method
	m_state.indent(fp);
	fp->write("virtual bool ");
	fp->write(m_state.getIsMangledFunctionName(cuti));
	fp->write("(u32 rn) const;"); GCNL;
	fp->write("\n");
	return;
      } //done w h-file

    //isbyregnum BV accessor method
    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("bool ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::"); //same for elements and quarks
    fp->write(m_state.getIsMangledFunctionName(cuti));
    fp->write("(u32 rn) const"); GCNL;
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("return (");
    fp->write(m_state.getGetRelPosMangledFunctionName(cuti));
    fp->write("(rn) >= 0);"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //is-related\n\n");
  }//genCodeBuiltInIsMethodByRegistrationNumber

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
    fp->write("return (ELEMENT_TYPE == targ.GetType());"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //isMethod\n\n");
  } //generateInternalIsMethodForElement

  void NodeBlockClass::genCodeBuiltInFunctionGetRelPosMethodRelatedInstance(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    if(classtype == UC_LOCALSFILESCOPE) return;

    UTI cuti = m_state.getCompileThisIdx();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("//helper method, not called directly\n");

	m_state.indent(fp);
	fp->write("s32 ");
	fp->write(m_state.getGetRelPosMangledFunctionName(cuti));
	fp->write("(const UlamClass<EC> * cptrarg) const;"); GCNL; //overloade
	fp->write("\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("s32 "); //return relpos >=0 if related

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write(m_state.getGetRelPosMangledFunctionName(cuti));
    fp->write("(const UlamClass<EC> * cptrarg) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("return ");
    fp->write(m_state.getGetRelPosMangledFunctionName(cuti));
    fp->write("(cptrarg->");
    fp->write(m_state.getClassRegistrationNumberFunctionName(cuti));
    fp->write("());"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //relpos\n\n");
  } //genCodeBuiltInFunctionGetRelPosMethodRelatedInstance

  void NodeBlockClass::genCodeBuiltInFunctionGetRelPosMethodRelatedInstanceByRegistrationNumber(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    if(classtype == UC_LOCALSFILESCOPE) return;

    UTI cuti = m_state.getCompileThisIdx();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("//helper method, not called directly\n");

	m_state.indent(fp);
	fp->write("s32 ");
	fp->write(m_state.getGetRelPosMangledFunctionName(cuti));
	fp->write("(const u32 regid) const;"); GCNL; //overloade
	fp->write("\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("s32 "); //return relpos >=0 if related

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write(m_state.getGetRelPosMangledFunctionName(cuti));
    fp->write("(const u32 regid) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("switch(regid)\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //cases:
    genCodeBuiltInFunctionGetRelPosRelatedInstanceByRegistrationNumber(fp);

    m_state.indent(fp);
    fp->write("default: ;\n");

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("};\n");

    m_state.indent(fp);
    fp->write("return ");
    fp->write("(-1); //not found"); GCNL; //for compiler

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //relpos\n\n");
  } //genCodeBuiltInFunctionGetRelPosMethodRelatedInstanceByRegistrationNumber

  void NodeBlockClass::genCodeBuiltInFunctionGetRelPosRelatedInstanceByRegistrationNumber(File * fp)
  {
    UTI nuti = getNodeType();

    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
    assert(isDefined);

    //build temporary map to output switch in regnum order
    std::map<u32, u32> tmpmapbyrn;

    u32 myregnum = m_state.getAClassRegistrationNumber(nuti);
    tmpmapbyrn.insert(std::pair<u32, u32>(myregnum, 0));

    u32 basecount = csym->getSharedBaseClassCount();
    u32 j = 0;
    while(j < basecount)
      {
	UTI baseuti = csym->getSharedBaseClass(j); //unordered
	//skip the ancestor of a template
	if((baseuti != Nouti))
	  {
	    u32 regnum = m_state.getAClassRegistrationNumber(baseuti);
	    tmpmapbyrn.insert(std::pair<u32, u32>(regnum,j + 1));
	  }
	j++;
      } //end while

    //ulam-5 supports multiple base classes; superclass optional
    //ulam-5 supports shared base classes; all bases are shared!
    std::map<u32, u32>::iterator it;
    for(it = tmpmapbyrn.begin(); it != tmpmapbyrn.end(); it++)
      {
	u32 regnum = it->first;
	u32 k = it->second;
	if(k == 0)
	  {
	    m_state.indent(fp);
	    fp->write("case ");
	    fp->write_decimal_unsigned(myregnum);
	    fp->write(": return (0); //position of ");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(nuti).c_str());
	    fp->write(" (self) ");
	    GCNL;
	  }
	else
	  {
	    k--;
	    UTI baseuti = csym->getSharedBaseClass(k); //unordered
	    u32 relpos = csym->getSharedBaseClassRelativePosition(k);
	    m_state.indent(fp);
	    fp->write("case ");
	    fp->write_decimal_unsigned(regnum);
	    fp->write(": return (");
	    fp->write_decimal_unsigned(relpos);
	    fp->write("); //position of ");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(baseuti).c_str());
	    fp->write("\n");
	  }
      } //forloop
  } //genCodeBuiltInFunctionGetRelPosRelatedInstanceByRegistrationNumber

  void NodeBlockClass::genCodeBuiltInFunctionNumberOfBases(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    UTI cuti = m_state.getCompileThisIdx();
    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual u32 ");
	fp->write(m_state.getNumberOfBasesFunctionName(cuti));
	fp->write("() const;"); GCNL;
	fp->write("\n");
	return;
      }

    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("u32 "); //returns number of (shared) bases

    //include the mangled class::
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");

    fp->write("::");
    fp->write(m_state.getNumberOfBasesFunctionName(cuti));
    fp->write("( ) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("return ");
    fp->write_decimal_unsigned(csym->getSharedBaseClassCount() + 1); //add self
    fp->write("; //including self"); GCNL;

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //");
    fp->write(m_state.getNumberOfBasesFunctionName(cuti));
    fp->write("\n\n");
  } //genCodeBuiltInFunctionNumberOfBases

  void NodeBlockClass::genCodeBuiltInFunctionNumberOfDirectBases(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    UTI cuti = m_state.getCompileThisIdx();
    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual u32 ");
	fp->write(m_state.getNumberOfDirectBasesFunctionName(cuti));
	fp->write("() const;"); GCNL;
	fp->write("\n");
	return;
      }

    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("u32 "); //returns number of direct bases

    //include the mangled class::
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");

    fp->write("::");
    fp->write(m_state.getNumberOfDirectBasesFunctionName(cuti));
    fp->write("( ) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("return ");
    fp->write_decimal_unsigned(csym->countDirectSharedBases() + 1); //add self
    fp->write("; //including self"); GCNL;

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //");
    fp->write(m_state.getNumberOfDirectBasesFunctionName(cuti));
    fp->write("\n\n");
  } //genCodeBuiltInFunctionNumberOfDirectBases

  void NodeBlockClass::genCodeBuiltInFunctionGetOrderedBaseClass(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    UTI cuti = m_state.getCompileThisIdx();
    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual UlamClass<EC> * ");
	fp->write(m_state.getOrderedBaseClassFunctionName(cuti));
	fp->write("(u32 ith) const;"); GCNL;
	fp->write("\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("UlamClass<EC> * "); //returns the instance of ith baseclass

    //include the mangled class::
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");

    fp->write("::");
    fp->write(m_state.getOrderedBaseClassFunctionName(cuti));
    fp->write("(u32 ith) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("switch (ith)\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //cases:
    generateBuiltInFunctionGetOrderedBaseClassEntry(fp);

    m_state.indent(fp);
    fp->write("default: ;\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}; //end switch"); GCNL;

    m_state.indent(fp);
    fp->write("return (NULL); //not found"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //");
    fp->write(m_state.getOrderedBaseClassFunctionName(cuti));
    fp->write("\n\n");
  } //genCodeBuiltInFunctionGetOrderedBaseClass

  void NodeBlockClass::generateBuiltInFunctionGetOrderedBaseClassEntry(File * fp)
  {
    UTI cuti = m_state.getCompileThisIdx();
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    // zero-th entry is thyself
    m_state.indent(fp);
    fp->write("case 0: return (& ");
    fp->write(m_state.getTheInstanceMangledNameByIndex(cuti).c_str());
    fp->write("); //self\n");

    //ulam-5 supports multiple base classes; superclass optional, default UrSelf.
    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    u32 next = 1; //includes self

    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	if(m_state.okUTItoContinue(baseuti)) //skips default super
	  {
	    m_state.indent(fp);
	    fp->write("case ");
	    fp->write_decimal_unsigned(next++);
	    fp->write(": return (& ");
	    fp->write(m_state.getTheInstanceMangledNameByIndex(baseuti).c_str());
	    fp->write(");\n");
	  }
	i++;
      } //end while

    //ulam-5 supports shared base classes;
    u32 shbasecount = csym->getSharedBaseClassCount();
    u32 j = 0;
    while(j < shbasecount)
      {
	UTI baseuti = csym->getSharedBaseClass(j);
	s32 bitem = csym->isABaseClassItem(baseuti);
	if(bitem < 0) //not a direct shared base
	  {
	    m_state.indent(fp);
	    fp->write("case ");
	    fp->write_decimal_unsigned(next++);
	    fp->write(": return (& ");
	    fp->write(m_state.getTheInstanceMangledNameByIndex(baseuti).c_str());
	    fp->write(");\n");
	  } //else skip
	j++;
      } //end while
    return;
  } //generateBuiltInFunctionGetOrderedBaseClassEntry (helper)

  void NodeBlockClass::genCodeBuiltInFunctionIsDirectBaseClass(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    if(classtype == UC_LOCALSFILESCOPE) return;

    UTI cuti = m_state.getCompileThisIdx();

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("bool ");
	fp->write(m_state.getIsDirectBaseClassFunctionName(cuti));
	fp->write("(const u32 regid) const;"); GCNL; //overloade
	fp->write("\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("bool ");

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write(m_state.getIsDirectBaseClassFunctionName(cuti));
    fp->write("(const u32 regid) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("switch(regid)\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //cases:
    genCodeBuiltInFunctionIsDirectBaseClassByRegistrationNumber(fp);

    m_state.indent(fp);
    fp->write("default: ;\n");

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("}; //end switch"); GCNL;
    fp->write("\n");

    m_state.indent(fp);
    fp->write("return ");
    fp->write("(false); //unrelated"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //");
    fp->write(m_state.getIsDirectBaseClassFunctionName(cuti)); //IsDirectBaseClass
    fp->write("\n\n");
  } //genCodeBuiltInFunctionIsDirectBaseClass

  void NodeBlockClass::genCodeBuiltInFunctionIsDirectBaseClassByRegistrationNumber(File * fp)
  {
    UTI nuti = getNodeType();

    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
    assert(isDefined);

    //build temporary map to output switch in regnum order
    std::map<u32, u32> tmpmapbyrn;

    u32 myregnum = m_state.getAClassRegistrationNumber(nuti);
    tmpmapbyrn.insert(std::pair<u32, u32>(myregnum, 0));

    u32 basecount = csym->getSharedBaseClassCount();
    u32 j = 0;
    while(j < basecount)
      {
	UTI baseuti = csym->getSharedBaseClass(j); //unordered
	//skip the ancestor of a template
	if((baseuti != Nouti))
	  {
	    u32 regnum = m_state.getAClassRegistrationNumber(baseuti);
	    tmpmapbyrn.insert(std::pair<u32, u32>(regnum,j + 1));
	  }
	j++;
      } //end while

    //ulam-5 supports multiple base classes; superclass optional
    std::map<u32, u32>::iterator it;
    for(it = tmpmapbyrn.begin(); it != tmpmapbyrn.end(); it++)
      {
	u32 regnum = it->first;
	u32 k = it->second;
	if(k == 0)
	  {
	    m_state.indent(fp);
	    fp->write("case ");
	    fp->write_decimal_unsigned(myregnum);
	    fp->write(": return (false); //");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(nuti).c_str());
	    fp->write(" (self) ");
	    GCNL;
	  }
	else
	  {
	    k--;
	    UTI baseuti = csym->getSharedBaseClass(k); //unordered
	    s32 bitem = csym->isABaseClassItem(baseuti);
	    bool isdirectbase = (bitem >= 0);
	    m_state.indent(fp);
	    fp->write("case ");
	    fp->write_decimal_unsigned(regnum);
	    if(isdirectbase)
	      fp->write(": return (true); //direct,");
	    else
	      fp->write(": return (false); //shared,");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(baseuti).c_str());
	    fp->write("\n");
	  }
      } //forloop
  } //genCodeBuiltInFunctionIsDirectBaseClassByRegistrationNumber (helper)

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

	if(classtype != UC_ELEMENT)
	  {
	    m_state.indent(fp);
	    fp->write("virtual u32 ");
	    fp->write(m_state.getBaseClassLengthFunctionName(cuti));
	    fp->write("() const;"); GCNL;
	    fp->write("\n");
	  }
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
    fp->write("} //");
    fp->write(m_state.getClassLengthFunctionName(cuti));
    fp->write("\n\n");

    //elements are never a baseclass, fail.
    if(classtype == UC_ELEMENT) return;

    //next, returns base class size:
    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("u32 ");

    //include the mangled class::
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");

    fp->write("::");
    fp->write(m_state.getBaseClassLengthFunctionName(cuti));
    fp->write("( ) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("return ");
    fp->write_decimal_unsigned(cut->getBitsizeAsBaseClass());
    fp->write(";"); GCNL;

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //");
    fp->write(m_state.getBaseClassLengthFunctionName(cuti));
    fp->write("\n\n");
  } //genCodeBuiltInFunctionGetClassLength

  void NodeBlockClass::genCodeBuiltInFunctionGetClassRegistrationNumber(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    //returns class registry number(replaces UlamClass.tcc in MFM virtually)
    UTI cuti = m_state.getCompileThisIdx();
    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual u32 ");
	fp->write(m_state.getClassRegistrationNumberFunctionName(cuti));
	fp->write("() const;"); GCNL;
	fp->write("\n");
	return;
      }

    u32 regnum = m_state.getAClassRegistrationNumber(cuti);

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("u32 ");

    //include the mangled class::
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>");

    fp->write("::");
    fp->write(m_state.getClassRegistrationNumberFunctionName(cuti));
    fp->write("( ) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("return ");

    fp->write_hexadecimal(regnum);
    fp->write(";"); GCNL;

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //GetRegistrationNumber\n\n");
  } //genCodeBuiltInFunctionGetClassRegistrationNumber

  void NodeBlockClass::genCodeBuiltInFunctionGetElementType(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    assert(classtype == UC_ELEMENT);

    UTI cuti = m_state.getCompileThisIdx();
    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("virtual u32 ");
	fp->write(m_state.getElementTypeFunctionName(cuti));
	fp->write("() const;"); GCNL;
	fp->write("\n");
	return;
      }

    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("u32 "); //returns Element Type (replaces Element.h in MFM virtually)

    //include the mangled class::
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write(m_state.getElementTypeFunctionName(cuti));
    fp->write("( ) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("return ELEMENT_TYPE;"); GCNL;

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //");
    fp->write(m_state.getElementTypeFunctionName(cuti));
    fp->write("\n\n");
  } //genCodeBuiltInFunctionGetElementType

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
	// (including 25 zeros for Type) e.g. t3510, t3173.
	if(genCodeBuiltInFunctionBuildingDefaultDataMembers(fp))
	  {
	    fp->write("\n");
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
    AssertBool isDefault = m_state.getDefaultClassValue(nuti, dval); //ulam-4 includes element type
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
	fp->write_hexadecimal(qval);
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
	// ulam-4 element type already into default value
	genCodeBuiltInFunctionBuildingDefaultDataMembers(fp);

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
	u32 vowned = csym->getOrigVTableSize();
	u32 vownstart = csym->getVTstartoffsetOfRelatedOriginatingClass(cuti);
	//enum for method indexes; see UlamElement.h for first two.
	m_state.indent(fp);
	fp->write("enum VOWNED_IDX {\n");
	m_state.m_currentIndentLevel++;
	for(u32 i = 0; i < vowned; i++)
	  {
	    m_state.indent(fp);
	    fp->write("VOWNED_IDX_");
	    fp->write(csym->getMangledFunctionNameWithTypesForVTableEntry(i + vownstart).c_str());
	    if(i == 0)
	      fp->write(" = 0");
	    fp->write(",\n");
	  }
	m_state.indent(fp);
	fp->write("VOWNED_IDX_COUNT\n");

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("};"); GCNL;
	fp->write("\n");

	m_state.indent(fp);
	fp->write("enum { VTABLE_IDX_MAX = ");
	fp->write_decimal_unsigned(maxidx);
	fp->write(" };\n");

	m_state.indent(fp);
	fp->write("static ");
	fp->write("struct VTentry<EC> m_vtable[");
	fp->write_decimal_unsigned(maxidx);
	fp->write("];"); GCNL;

	//VTable accessor methods
	m_state.indent(fp);
	fp->write("virtual VfuncPtr ");
	fp->write(m_state.getVTableEntryFunctionName(cuti));
	fp->write("(u32 idx) const;"); GCNL;
	fp->write("\n");

	m_state.indent(fp);
	fp->write("virtual const UlamClass<EC> * ");
	fp->write(m_state.getVTableEntryClassPtrFunctionName(cuti));
	fp->write("(u32 idx) const;"); GCNL;
	fp->write("\n");
	return;
      } //done w h-file

    //The VTABLE Definition:
    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("struct VTentry<EC> ");

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
	UTI origclass = csym->getOriginatingClassForVTableEntry(i);

	if(i > 0)
	  fp->write(",\n");

	if(csym->isPureVTableEntry(i))
	  {
	    m_state.indent(fp);
	    fp->write(" { "); //VTentry struct
	    fp->write("&UlamClass<EC>::PureVirtualFunctionCalled");
	    fp->write(", & ");
	    UTI emptyinstead = m_state.getEmptyElementUTI();
	    fp->write(m_state.getUlamTypeByIndex(emptyinstead)->getUlamTypeMangledName().c_str());
	    fp->write("<EC>::THE_INSTANCE"); //instead of NULL  (e,g, t41301)
	    fp->write(" } /* ");
	    fp->write(m_state.m_pool.getDataAsString(csym->getVFuncNameSignatureIdForVTableEntry(i)).c_str()); //helpful..
	    fp->write(" (vfidx:");
	    fp->write_decimal_unsigned(csym->getVFuncIndexForVTableEntry(i));
	    fp->write(", origclass:");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(origclass).c_str());
	    fp->write(", override:");
	    fp->write(m_state.getUlamTypeNameBriefByIndex(emptyinstead).c_str());
	    fp->write(") */");
	    continue;
	  }

	UTI veuti = csym->getClassForVTableEntry(i);
	assert(m_state.okUTItoContinue(veuti));
	UlamType * veut = m_state.getUlamTypeByIndex(veuti);

	u32 veclassrelpos;
	AssertBool gotPos = m_state.getABaseClassRelativePositionInAClass(cuti, veuti, veclassrelpos);
	assert(gotPos);

	m_state.indent(fp);
	fp->write(" { "); //VTentry struct
	fp->write("(VfuncPtr) "); //cast to void
	fp->write("((typename "); //cast to contextual type info
	fp->write(veut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::"); //same for elements and quarks
	fp->write(csym->getMangledFunctionNameWithTypesForVTableEntry(i).c_str());
	fp->write(") & ");
	fp->write(veut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::"); //same for elements and quarks
	fp->write(csym->getMangledFunctionNameForVTableEntry(i).c_str());
	fp->write("), & ");
	fp->write(veut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::THE_INSTANCE"); //same for elements and quarks
	fp->write(" } /* ");
	fp->write(m_state.m_pool.getDataAsString(csym->getVFuncNameSignatureIdForVTableEntry(i)).c_str()); //helpful..
	fp->write(" (vfidx:");
	fp->write_decimal_unsigned(csym->getVFuncIndexForVTableEntry(i));
	fp->write(", origclass:");
	fp->write(m_state.getUlamTypeNameBriefByIndex(origclass).c_str());
	fp->write(", override:");
	fp->write(m_state.getUlamTypeNameBriefByIndex(veuti).c_str());
	fp->write(") */");
      } //next vt entry

    fp->write("\n");
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}; //VTABLE Definition"); GCNL;
    fp->write("\n");

    //VTable accessor methods
    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("VfuncPtr ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::"); //same for elements and quarks
    fp->write(m_state.getVTableEntryFunctionName(cuti));
    fp->write("(u32 idx) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("if(idx >= ");
    fp->write_decimal_unsigned(maxidx);
    fp->write(") FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);"); GCNL;

    m_state.indent(fp);
    fp->write("return (VfuncPtr) m_vtable[idx].vfptr;"); GCNL;
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n\n");

    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("const UlamClass<EC> * ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::"); //same for elements and quarks
    fp->write(m_state.getVTableEntryClassPtrFunctionName(cuti));
    fp->write("(u32 idx) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("if(idx >= ");
    fp->write_decimal_unsigned(maxidx);
    fp->write(") FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);"); GCNL;

    m_state.indent(fp);
    fp->write("return m_vtable[idx].oclassptr;"); GCNL;
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n\n");

    return;
  }//genCodeBuiltInVirtualTable

  void NodeBlockClass::genCodeBuiltInVirtualTableStartOffsetHelper(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    if(classtype == UC_LOCALSFILESCOPE)
      return;

    //VTable applies to all classes; TODO: binary search of regnum->offset pairs.
    UTI cuti = m_state.getCompileThisIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    u32 maxregistry = m_state.getMaxNumberOfRegisteredUlamClasses();
    assert(maxregistry < MAX_REGISTRY_NUMBER);  //UlamClassRegistry<EC>::TABLE_SIZE
    s32 maxidx = getVirtualMethodMaxIdx();
    assert(maxidx >= 0);

    if(maxidx == 0)
      return;

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("static ");
	fp->write("u16 m_vtablestartoffsets[");
	fp->write_decimal_unsigned(maxregistry);
	fp->write("];"); GCNL;

	//VTable accessor method
	m_state.indent(fp);
	fp->write("virtual u32 ");
	fp->write(m_state.getVTableEntryStartOffsetForClassFunctionName(cuti));
	fp->write("(u32 rn) const;"); GCNL;
	fp->write("\n");
	return;
      } //done w h-file

    std::map<u32, u32> mapbyregnum;
    u32 mapsize = csym->convertVTstartoffsetmap(mapbyregnum);
    assert(mapsize > 0);

    //The VT Start Index table Definition:
    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("u16 ");

    //include the mangled class::
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::"); //same for elements and quarks
    fp->write("m_vtablestartoffsets[");
    fp->write_decimal_unsigned(maxregistry);
    fp->write("] = {\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    //generate each VT entry:
    for(u32 i = 0; i < maxregistry; i++)
      {
	if(i > 0)
	  fp->write(", ");

	std::map<u32,u32>::iterator it = mapbyregnum.find(i);
	if(it != mapbyregnum.end())
	  fp->write_decimal_unsigned(it->second);
	else
	  fp->write_decimal_unsigned(0);
      } //next vt start idx entry

    fp->write("\n");
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}; //VTABLE Start Offsets by Class RegNum Definition"); GCNL;
    fp->write("\n");

    //VTable accessor method
    m_state.indent(fp);
    fp->write("template<class EC>\n"); //same for elements and quarks

    m_state.indent(fp);
    fp->write("u32 ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::"); //same for elements and quarks
    fp->write(m_state.getVTableEntryStartOffsetForClassFunctionName(cuti));
    fp->write("(u32 rn) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("if(rn >= ");
    fp->write_decimal_unsigned(maxregistry);
    fp->write(") FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);"); GCNL;

    m_state.indent(fp);
    fp->write("return m_vtablestartoffsets[rn];"); GCNL;
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n\n");
  }//genCodeBuiltInVirtualTableStartOffsetHelper

  void NodeBlockClass::generateInternalTypeAccessorsForElement(File * fp, bool declOnly)
  {
    UTI cuti = getNodeType();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("const u32 ");
	fp->write(m_state.getReadTypeFieldFunctionName(cuti));
	fp->write("(const BV bv);"); GCNL;
	fp->write("\n");
	m_state.indent(fp);
	fp->write("void ");
	fp->write(m_state.getWriteTypeFieldFunctionName(cuti));
	fp->write("(BV& bv, const u32 v);"); GCNL;
	fp->write("\n");
	return;
      }


    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("const u32 ");  //return NULL if not found

    //include the mangled class::
    fp->write(cut->getUlamTypeMangledName().c_str());

    fp->write("<EC>::");
    fp->write(m_state.getReadTypeFieldFunctionName(cuti));
    fp->write("(const BV bv)\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("return BFTYP::Read(bv);"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //");
    fp->write(m_state.getReadTypeFieldFunctionName(cuti));
    fp->write("\n\n");


    m_state.indent(fp);
    fp->write("template<class EC>\n");
    m_state.indent(fp);
    fp->write("void ");  //return NULL if not found

    //include the mangled class::
    fp->write(cut->getUlamTypeMangledName().c_str());

    fp->write("<EC>::");
    fp->write(m_state.getWriteTypeFieldFunctionName(cuti));
    fp->write("(BV& bv, const u32 v)\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_state.indent(fp);
    fp->write("BFTYP::Write(bv, v);"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("} //");
    fp->write(m_state.getWriteTypeFieldFunctionName(cuti));
    fp->write("\n\n");
  } //generateInternalTypeAccessorsForElement

  void NodeBlockClass::generateGetPosForQuark(File * fp, bool declOnly)
  {
    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("__inline__ const u32 GetPos() const { return 0u; }\n"); //?
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

    fp->write(m_state.getDataMemberInfoFunctionName(cuti));
    fp->write("(u32 dataMemberNumber) const"); //method name!!!

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

	m_state.indent(fp);
	fp->write("default: ;\n");

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("}; //end switch"); GCNL;

	m_state.indent(fp);
	fp->write("FAIL(ILLEGAL_ARGUMENT);"); GCNL;

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("} //"); //end of func
	fp->write(m_state.getDataMemberInfoFunctionName(cuti));
	fp->write("\n\n");
      }
    else
      {
	fp->write(";"); GCNL; //end of declaration
	fp->write("\n");
      }
  } //generateUlamClassInfoFunction

  void NodeBlockClass::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    UTI nuti = getNodeType();

    if(m_nodeNext)
      m_nodeNext->generateUlamClassInfo(fp, declOnly, dmcount);

    //base classes stop with data members only
    if(UlamType::compare(nuti, m_state.getCompileThisIdx(), m_state) != UTIC_SAME)
      return;

    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
    assert(isDefined);

    //ulam-5 supports multiple base classes; superclass optional
    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	//skip the ancestor of a template
	if(baseuti != Nouti)
	  {
	    //then include any of its relatives:
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    assert(basecblock);
	    basecblock->generateUlamClassInfo(fp, declOnly, dmcount);
	  }
	i++;
      } //end while

    //ulam-5 supports shared base classes;
    u32 shbasecount = csym->getSharedBaseClassCount();
    u32 j = 0;
    while(j < shbasecount)
      {
	UTI baseuti = csym->getSharedBaseClass(j);
	//skip the ancestor of a template
	if(baseuti != Nouti)
	  {
	    //not a direct shared base
	    //then include any of its relatives not yet seen
	    s32 bitem = csym->isABaseClassItem(baseuti);
	    if(bitem < 0)
	      {
		NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);
		assert(shbasecblock);
		shbasecblock->generateUlamClassInfo(fp, declOnly, dmcount);
	      }
	  }
	j++;
      } //end while
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

    fp->write(m_state.getDataMemberCountFunctionName(cuti));  //method name!!!
    fp->write("() const");

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
	fp->write("} //");
	fp->write(m_state.getDataMemberCountFunctionName(cuti));  //method name!!!
	fp->write("\n\n");
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

    fp->write(m_state.getClassMangledNameFunctionName(cuti));
    fp->write("() const"); //method name!!!

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
	fp->write("} //");
	fp->write(m_state.getClassMangledNameFunctionName(cuti));
	fp->write("\n\n");
      }
    else
      {
	fp->write(";"); GCNL;
	fp->write("\n");
      }
  } //generateUlamClassGetMangledName

  void NodeBlockClass::generateUlamClassGetMangledNameAsStringIndex(File * fp, bool declOnly)
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

    fp->write("u32 "); //return type

    if(!declOnly)
      {
	//include the mangled class::
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::"); //same for elements and quarks
      }

    fp->write(m_state.getClassMangledNameAsStringIndexFunctionName(cuti));
    fp->write("() const"); //method name!!!

    if(!declOnly)
      {
	fp->write("\n");

	m_state.indent(fp);
	fp->write("{\n");

	m_state.m_currentIndentLevel++;
	m_state.indent(fp);

	fp->write("return ");
	std::string mangled = cut->getUlamTypeMangledName();
	fp->write_decimal_unsigned(m_state.formatAndGetIndexForDataUserString(mangled));
	fp->write("u; //");
	fp->write(mangled.c_str());
	GCNL;

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("} //");
	fp->write(m_state.getClassMangledNameAsStringIndexFunctionName(cuti));
	fp->write("\n\n");
      }
    else
      {
	fp->write(";"); GCNL;
	fp->write("\n");
      }
  } //generateUlamClassGetMangledNameAsStringIndex

  void NodeBlockClass::generateUlamClassGetNameAsStringIndex(File * fp, bool declOnly)
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

    fp->write("u32 "); //return type

    if(!declOnly)
      {
	//include the mangled class::
	fp->write(cut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::"); //same for elements and quarks
      }

    fp->write(m_state.getClassNameAsStringIndexFunctionName(cuti));
    fp->write("(bool templateParameters, bool templateValues) const"); //method name!!!

    if(!declOnly)
      {
	u32 id = cut->getUlamTypeNameId();
	SymbolClassName * cnsym = (SymbolClassName *) m_state.m_programDefST.getSymbolPtr(id);
	assert(cnsym);
	bool isTemplateClass = cnsym->isClassTemplate();

	fp->write("\n");

	m_state.indent(fp);
	fp->write("{\n");

	m_state.m_currentIndentLevel++;

	if(isTemplateClass)
	  {
	    m_state.indent(fp);
	    fp->write("if(templateParameters && templateValues)\n");

	    m_state.m_currentIndentLevel++;
	    m_state.indent(fp);
	    fp->write("return ");
	    std::string pretty = cnsym->generatePrettyNameOrSignature(cuti,true,true);
	    fp->write_decimal_unsigned(m_state.formatAndGetIndexForDataUserString(pretty));
	    fp->write("u; //");
	    fp->write(pretty.c_str()); GCNL;
	    m_state.m_currentIndentLevel--;

	    m_state.indent(fp);
	    fp->write("if(templateParameters && !templateValues)\n");

	    m_state.m_currentIndentLevel++;
	    m_state.indent(fp);
	    fp->write("return ");
	    std::string sig = cnsym->generatePrettyNameOrSignature(cuti,true,false);
	    fp->write_decimal_unsigned(m_state.formatAndGetIndexForDataUserString(sig));
	    fp->write("u; //");
	    fp->write(sig.c_str()); GCNL;
	    m_state.m_currentIndentLevel--;


	    m_state.indent(fp);
	    fp->write("if(!templateParameters && templateValues)\n");

	    m_state.m_currentIndentLevel++;
	    m_state.indent(fp);
	    fp->write("return ");
	    std::string simple = cnsym->generatePrettyNameOrSignature(cuti,false,true);
	    fp->write_decimal_unsigned(m_state.formatAndGetIndexForDataUserString(simple));
	    fp->write("u; //");
	    fp->write(simple.c_str()); GCNL;
	    m_state.m_currentIndentLevel--;

	    m_state.indent(fp);
	    fp->write("//if(!templateParameters && !templateValues)\n");
	  }

	m_state.indent(fp);
	fp->write("return ");
	std::string plain = cnsym->generatePrettyNameOrSignature(cuti,false,false);
	fp->write_decimal_unsigned(m_state.formatAndGetIndexForDataUserString(plain));
	fp->write("u; //");
	fp->write(plain.c_str()); GCNL;

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("} //");
	fp->write(m_state.getClassNameAsStringIndexFunctionName(cuti));
	fp->write("\n\n");
      }
    else
      {
	fp->write(";"); GCNL;
	fp->write("\n");
      }
  } //generateUlamClassGetClassNameAsStringIndex

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
    desc.m_classSignature = cnsym->generatePrettyNameOrSignature(cuti,true,false); //unfancy

    //format Ulam Class Signature of super class (ONLY???)
    if(!m_state.isUrSelf(cuti))
      {
	UTI superuti = cnsym->getBaseClassForClassInstance(cuti, 0);
	assert(m_state.okUTItoContinue(superuti));
	SymbolClassName * supercnsym = NULL;
	AssertBool isSuperDefined = m_state.alreadyDefinedSymbolClassNameByUTI(superuti, supercnsym);
	assert(isSuperDefined);
	desc.m_baseClassSignature = supercnsym->generatePrettyNameOrSignature(superuti,true,false); //unfancy
      }
    else
      desc.m_baseClassSignature = "nobase";

    classtargets.insert(std::pair<std::string, struct TargetDesc>(mangledName, desc));
  } //addTargetDescriptionToInfoMap

  void NodeBlockClass::addMemberDescriptionsToInfoMap(ClassMemberMap& classmembers)
  {
    NodeBlock::addMemberDescriptionsToInfoMap(classmembers); //Table of Variables request

    //work done by NodeFuncDecl (ulam-5)
    //m_functionST.addClassMemberFunctionDescriptionsToMap(this->getNodeType(), classmembers); //Table of Classes request
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

    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(suti, csym);
    assert(isDefined);

    //ulam-5 supports multiple base classes; superclass optional
    u32 basecount = csym->getBaseClassCount() + 1; //include super
    u32 i = 0;
    while(i < basecount)
      {
	UTI baseuti = csym->getBaseClass(i);
	//skip the ancestor of a template
	if(baseuti != Nouti)
	  {
	    //then include any of its relatives:
	    NodeBlockClass * basecblock = getBaseClassBlockPointer(i);
	    assert(basecblock);
	    basecblock->generateTestInstance(fp, runtest);
	  }
	i++;
      } //end while

    //ulam-5 supports shared base classes;
    u32 shbasecount = csym->getSharedBaseClassCount();
    u32 j = 0;
    while(j < shbasecount)
      {
	UTI baseuti = csym->getSharedBaseClass(j);
	//skip the ancestor of a template
	if(baseuti != Nouti)
	  {
	    //then include any of its relatives:
	    NodeBlockClass * shbasecblock = getSharedBaseClassBlockPointer(j);
	    assert(shbasecblock);
	    shbasecblock->generateTestInstance(fp, runtest);
	  }
	j++;
      } //end while

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
    fp->write("u, atbs, & ");
    fp->write(m_state.getTheInstanceMangledNameByIndex(suti).c_str());
    fp->write(", UlamRef<EC>::ELEMENTAL, uc);"); GCNL;

    m_state.indent(fp);
    fp->write(m_state.getTheInstanceMangledNameByIndex(suti).c_str());

    // pass uc with effective self setup
    fp->write(".Uf_4test(");
    fp->write("uc, ur);"); GCNL;

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");
  } //generateTestInstanceRun

} //end MFM

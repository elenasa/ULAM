#include "SymbolClassName.h"
#include "Resolver.h"
#include "CompilerState.h"

namespace MFM {

  SymbolClassName::SymbolClassName(Token id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClass(id, utype, classblock, NULL/* parent template */, state)
  {
    unsetStub(); //regular class; classblock may be null if utype is UC_UNSEEN class type.
  }

  SymbolClassName::~SymbolClassName(){}

  void SymbolClassName::resetUnseenClassLocation(Token identTok)
  {
    //during parsing
    Symbol::resetIdToken(identTok);
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    classNode->setNodeLocation(identTok.m_locator);
  } //resetUnseenClassLocation

  void SymbolClassName::setStructuredComment()
  {
    Token scTok;
    if(m_state.getStructuredCommentToken(scTok)) //and clears it
      {
	m_structuredCommentToken = scTok;
	m_gotStructuredCommentToken = true;
      }
  } //setStructuredComment

  void SymbolClassName::getTargetDescriptorsForClassInstances(TargetMap& classtargets)
  {
    u32 scid = 0;
    Token scTok;
    if(getStructuredComment(scTok))
      scid = scTok.m_dataindex;

    SymbolClass::addTargetDescriptionMapEntry(classtargets, scid);
  } //getTargetDescriptorsForClassInstances

  void SymbolClassName::getClassMemberDescriptionsForClassInstances(ClassMemberMap& classmembers)
  {
    SymbolClass::addClassMemberDescriptionsMapEntry(classmembers);
  } //getClassMemberDescriptionsForClassInstances

  bool SymbolClassName::isClassTemplate()
  {
    return false;
  } //isClassTemplate

  void SymbolClassName::setSuperClassForClassInstance(UTI superclass, UTI instance)
  {
    assert(instance == getUlamTypeIdx());
    SymbolClass::setSuperClass(superclass);
  } //setSuperClassForClassInstance

  UTI SymbolClassName::getSuperClassForClassInstance(UTI instance)
  {
    return SymbolClass::getSuperClass(); //Nouti is none, not a subclass.
  } //getSuperClassForClassInstance

  Node * SymbolClassName::findNodeNoInAClassInstance(UTI instance, NNO n)
  {
    assert(getUlamTypeIdx() == instance);

    Node * foundNode = NULL;
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->findNodeNo(n, foundNode);

    //if not in the tree, ask the resolver
    if(!foundNode)
      {
	SymbolClass::findNodeNoInResolver(n, foundNode);
      }

    m_state.popClassContext(); //restore
    return foundNode;
  } //findNodeNoInAClassInstance

  std::string SymbolClassName::formatAnInstancesArgValuesAsAString(UTI instance)
  {
    assert((instance == getUlamTypeIdx()) || (m_state.isARefTypeOfUlamType(instance, getUlamTypeIdx()) == UTIC_SAME)); //or could be a reference
    return "10"; //zero args
  }

  void SymbolClassName::updateLineageOfClass()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    if(!classNode)
      {
	std::ostringstream msg;
	msg << "LineageUpdate skipped for a class '";
	msg << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	msg << "' without a definition (maybe not a class at all)";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return;
      }

    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    UTI superuti = getSuperClassForClassInstance(getUlamTypeIdx());
    if(superuti == Nouti)
      classNode->updateLineage(0);
    //else if(!m_state.isClassAStub(superuti) && (superuti != Hzy))
    else if((superuti != Hzy))
      {
	//for inheritance, get the node no of superblock
	u32 sid = m_state.getUlamKeyTypeSignatureByIndex(superuti).getUlamKeyTypeSignatureNameId();
	SymbolClassName * cnsym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClassName(sid, cnsym);
	assert(isDefined);
	NodeBlockClass * superblock = cnsym->getClassBlockNode();
	assert(superblock);

	classNode->updateLineage(superblock->getNodeNo());
      }
    m_state.popClassContext(); //restore
  } //updateLineageOfClass

  void SymbolClassName::checkCustomArraysOfClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->checkCustomArrayTypeFunctions();
    m_state.popClassContext(); //restore
  } //checkCustomArraysOfClassInstances()

  void SymbolClassName::checkDuplicateFunctionsForClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->checkDuplicateFunctions();
    m_state.popClassContext(); //restore
  } //checkDuplicateFunctionsForClassInstances

  void SymbolClassName::calcMaxDepthOfFunctionsForClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->calcMaxDepthOfFunctions();
    m_state.popClassContext(); //restore
  } //calcMaxDepthOfFunctionsForClassInstances

  bool SymbolClassName::calcMaxIndexOfVirtualFunctionsForClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->calcMaxIndexOfVirtualFunctions();
    m_state.popClassContext(); //restore
    return (classNode->getVirtualMethodMaxIdx() != UNKNOWNSIZE);
  } //calcMaxIndexOfVirtualFunctionsForClassInstances

  void SymbolClassName::checkAbstractInstanceErrorsForClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->checkAbstractInstanceErrors();
    m_state.popClassContext(); //restore
    return;
  } //checkAbstractInstanceErrorsForClassInstances

  void SymbolClassName::checkAndLabelClassFirst()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    if(!classNode)
      {
	std::ostringstream msg;
	msg << "Check and Label skipped for a class '";
	msg << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	msg << "' without a definition (maybe not a class at all)";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return;
      }

    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->checkAndLabelType();
    m_state.popClassContext(); //restore
  } //checkAndLabelClassFirst()

  void SymbolClassName::checkAndLabelClassInstances()
  {
    checkAndLabelClassFirst();
  } //checkAndLabelClassInstances

  void SymbolClassName::countNavNodesInClassInstances(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    assert(!isClassTemplate());
    u32 navCounter = ncnt;
    u32 hzyCounter = hcnt;
    u32 unsetCounter = nocnt;

    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    if((ncnt - navCounter) > 0)
      {
	std::ostringstream msg;
	msg << navCounter << " data member nodes with erroneous types remain in class '";
	msg << m_state.getUlamTypeNameBriefByIndex(getUlamTypeIdx()).c_str();
	msg << "'";
	MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }

    if((hcnt - hzyCounter) > 0)
      {
	std::ostringstream msg;
	msg << hzyCounter << " data member nodes with unresolved types remain in class '";
	msg << m_state.getUlamTypeNameBriefByIndex(getUlamTypeIdx()).c_str();
	msg << "'";
	MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }

    if((nocnt - unsetCounter) > 0)
      {
	std::ostringstream msg;
	msg << unsetCounter << " data member nodes with unset types remain in class '";
	msg << m_state.getUlamTypeNameBriefByIndex(getUlamTypeIdx()).c_str();
	msg << "'";
	MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }

    SymbolClass::countNavNodesInClassResolver(ncnt, hcnt, nocnt);

    m_state.popClassContext(); //restore

    return;
  } //countNavNodesInClassInstances

  bool SymbolClassName::setBitSizeOfClassInstances()
  {
    bool aok = true;
    assert(!isClassTemplate());

    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode); //infinite loop "Incomplete Class <> was never defined, fails sizing"
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    s32 totalbits = 0;
    aok = SymbolClass::trySetBitsizeWithUTIValues(totalbits);
    if(aok)
      {
	UTI cuti = getUlamTypeIdx();
	m_state.setBitSize(cuti, totalbits); //"scalar" Class bitsize  KEY ADJUSTED
	if(m_state.getBitSize(cuti) != totalbits)
	  {
	    std::ostringstream msg;
	    msg << "CLASS (regular) '" << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << "' SIZED " << totalbits << " FAILED";
	    MSG(Symbol::getTokPtr(), msg.str().c_str(),ERR);
	    classNode->setNodeType(Nav); //avoid assert in resolving loop
	    aok = false; //missing?
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "CLASS (regular) '" << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << "' SIZED: " << totalbits;
	    MSG(Symbol::getTokPtr(), msg.str().c_str(),DEBUG);
	  }
      }
    m_state.popClassContext(); //restore
    return aok;
  } //setBitSizeOfClassInstances

  // separate pass...after labeling all classes is completed;
  void SymbolClassName::printBitSizeOfClassInstances()
  {
    SymbolClass::printBitSizeOfClass();
  } //printBitSizeOfClassInstances

  void SymbolClassName::packBitsForClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->packBitsForVariableDataMembers();
    m_state.popClassContext(); //restore
  } //packBitsForClassInstances

  void SymbolClassName::buildDefaultQuarkForClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    u32 dqval = 0;
    SymbolClass::getDefaultQuark(dqval); //this instance
    m_state.popClassContext(); //restore
  } //buildDefaultQuarkForClassInstances

  void SymbolClassName::testForClassInstances(File * fp)
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    SymbolClass::testThisClass(fp);
    m_state.popClassContext(); //restore
  } //testForClassInstances

  void SymbolClassName::generateCodeForClassInstances(FileManager * fm)
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    SymbolClass::generateCode(fm);
    m_state.popClassContext(); //restore
  } //generateCodeForClassInstances

  void SymbolClassName::generateIncludesForClassInstances(File * fp)
  {
    return SymbolClass::generateAsOtherInclude(fp);
  } //generateIncludesForClassInstances

  void SymbolClassName::generateForwardDefsForClassInstances(File * fp)
  {
    return SymbolClass::generateAsOtherForwardDef(fp);
  } //generateForwardDefsForClassInstances

  void SymbolClassName::generateTestInstanceForClassInstances(File * fp, bool runtest)
  {
    SymbolClass::generateTestInstance(fp, runtest);
  } //generateTestInstanceForClassInstances

} //end MFM

#include "SymbolClassName.h"
#include "Resolver.h"
#include "CompilerState.h"

namespace MFM {

  SymbolClassName::SymbolClassName(const Token& id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClass(id, utype, classblock, NULL/* parent template */, state)
  {
    unsetStub(); //regular class; classblock may be null if utype is UC_UNSEEN class type.
  }

  SymbolClassName::~SymbolClassName(){}

  void SymbolClassName::resetUnseenClassLocation(const Token& identTok)
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
  }

  bool SymbolClassName::isClassTemplate()
  {
    return false;
  }

  void SymbolClassName::setSuperClassForClassInstance(UTI superclass, UTI instance)
  {
    assert(instance == getUlamTypeIdx());
    assert(instance != superclass);
    SymbolClass::setBaseClass(superclass, 0);
  }

  UTI SymbolClassName::getSuperClassForClassInstance(UTI instance)
  {
    return SymbolClass::getBaseClass(0); //Nouti is none, Hzy a stub.
  }

  void SymbolClassName::appendBaseClassForClassInstance(UTI baseclass, UTI instance, bool sharedbase)
  {
    assert(instance == getUlamTypeIdx());
    assert(instance != baseclass);
    SymbolClass::appendBaseClass(baseclass, sharedbase);
  }

  u32 SymbolClassName::getBaseClassCountForClassInstance(UTI instance)
  {
    return SymbolClass::getBaseClassCount();
  }

  UTI SymbolClassName::getBaseClassForClassInstance(UTI instance, u32 item)
  {
    return SymbolClass::getBaseClass(item);
  }

  bool SymbolClassName::updateBaseClassforClassInstance(UTI instance, UTI oldbase, UTI newbaseuti)
  {
    bool aok = false;
    s32 item = isABaseClassItem(oldbase);
    if(item > 0) //excludes super
      {
	SymbolClass::updateBaseClass(oldbase, item, newbaseuti);
	aok = true;
      }
    return aok;
  }

  Node * SymbolClassName::findNodeNoInAClassInstance(UTI instance, NNO n)
  {
    assert(getUlamTypeIdx() == instance);

    Node * foundNode = NULL;
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->findNodeNo(n, foundNode);
    m_state.popClassContext(); //restore
    return foundNode;
  } //findNodeNoInAClassInstance

  std::string SymbolClassName::formatAnInstancesArgValuesAsAString(UTI instance, bool dereftypes)
  {
    UTI basicuti = m_state.getUlamTypeAsDeref(m_state.getUlamTypeAsScalar(instance));
    UTI rootbasicuti = basicuti;
    m_state.findaUTIAlias(basicuti, rootbasicuti); //in case alias Mon Jun 20 14:39:07 2016
    assert((rootbasicuti == getUlamTypeIdx()) || (m_state.isARefTypeOfUlamType(rootbasicuti, getUlamTypeIdx()) == UTIC_SAME)); //or could be a reference
    return "10"; //zero args
  } //formatAnInstancesArgValuesAsAString

  void SymbolClassName::generatePrettyNameAndSignatureOfClassInstancesAsUserStrings()
  {
    std::string signame = generatePrettyNameOrSignature(getUlamTypeIdx(), false, false);
    m_state.formatAndGetIndexForDataUserString(signame);
    return;
  }

  std::string SymbolClassName::generatePrettyNameOrSignature(UTI instance, bool signa, bool argvals)
  {
    //same fancy, signature, or simple
    std::ostringstream sig;
    sig << m_state.m_pool.getDataAsString(getId()).c_str(); //class name
    return sig.str();
  }

  bool SymbolClassName::hasInstanceMappedUTI(UTI instance, UTI auti, UTI& mappedUTI)
  {
    assert(instance == getUlamTypeIdx());
    return SymbolClass::hasMappedUTI(auti, mappedUTI);
  }

  bool SymbolClassName::mapInstanceUTI(UTI instance, UTI auti, UTI mappeduti)
  {
    assert(instance == getUlamTypeIdx());
    return SymbolClass::mapUTItoUTI(auti, mappeduti);
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

    //skip super classes here.
    classNode->updateLineage(0);
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

    classNode->checkDuplicateFunctionsInClassAndAncestors();
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
    //Regular Elements can't be Abstract, since NO subclass can
    //possibly handle PURE Virtual methods later (t41296); Templates
    //not checked, just their instances (t41297);
    SymbolClass::checkAbstractClassError(); //outputs error

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
  } //checkAndLabelClassFirst

  void SymbolClassName::checkAndLabelClassInstances()
  {
    checkAndLabelClassFirst();
  }

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
    m_state.popClassContext(); //restore
    return;
  } //countNavNodesInClassInstances

  bool SymbolClassName::statusUnknownTypeInClassInstances(UTI huti)
  {
    bool aok = true;
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    aok = SymbolClass::statusUnknownTypeInClass(huti);

    m_state.popClassContext(); //restore
    return aok;
  } //statusUnknownTypeInClassInstances

  bool SymbolClassName::statusUnknownTypeNamesInClassInstances()
  {
    bool aok = true;
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    aok = SymbolClass::statusUnknownTypeNamesInClass();

    m_state.popClassContext(); //restore
    return aok;
  } //statusUnknownTypeNamesInClassInstances

  u32 SymbolClassName::reportUnknownTypeNamesInClassInstances()
  {
    return SymbolClass::reportUnknownTypeNamesInClass();
  }

  u32 SymbolClassName::reportClassInstanceNamesThatAreTooLong()
  {
    if(SymbolClass::reportLongClassName())
      return 1;
    return 0;
  } //reportClassInstanceNamesThatAreTooLong

  bool SymbolClassName::setBitSizeOfClassInstances()
  {
    bool aok = true;
    assert(!isClassTemplate());
    UTI cuti = getUlamTypeIdx();

    if(m_state.isComplete(cuti))
      return true;

    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode); //infinite loop "Incomplete Class <> was never defined, fails sizing"
    m_state.pushClassContext(cuti, classNode, classNode, false, NULL);

    s32 totalbits = 0;
    s32 sharedbits = UNKNOWNSIZE;
    aok = SymbolClass::trySetBitsizeWithUTIValues(totalbits);
    if(aok)
      {
	s32 sharedbitssaved = UNKNOWNSIZE;
	aok = SymbolClass::determineSharedBasesAndTotalBitsize(sharedbitssaved, sharedbits);
	if(aok) //3755 QBase not ready
	  {
	    assert(sharedbits >= 0);
	    assert(sharedbits <= totalbits);
	    assert(sharedbitssaved >= sharedbits);
	    totalbits = (totalbits - sharedbitssaved + sharedbits); //updates total here!!
	  }
      }

    if(aok)
      {
	m_state.setBitSize(cuti, totalbits); //"scalar" Class bitsize  KEY ADJUSTED
	if(m_state.getBitSize(cuti) != totalbits)
	  {
	    std::ostringstream msg;
	    msg << "CLASS (regular) '" << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    msg << "' SIZED " << totalbits << " FAILED";
	    MSG(Symbol::getTokPtr(), msg.str().c_str(),ERR);
	    classNode->setNodeType(Nav); //avoid assert in resolving loop
	    aok = false; //t3155
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "CLASS (regular) '" << m_state.getUlamTypeNameByIndex(cuti).c_str();
	    msg << "' SIZED: " << totalbits;
	    MSG(Symbol::getTokPtr(), msg.str().c_str(),DEBUG);
	    //after setBitSize so not to clobber it.
	    m_state.setBaseClassBitSize(cuti, totalbits - sharedbits); //noop for elements
	  }
      }
    m_state.popClassContext(); //restore
    return aok;
  } //setBitSizeOfClassInstances

  // separate pass...after labeling all classes is completed;
  void SymbolClassName::printBitSizeOfClassInstances()
  {
    SymbolClass::printBitSizeOfClass();
  }

  void SymbolClassName::packBitsForClassInstances()
  {
    SymbolClass::packBitsForClassVariableDataMembers(); //return TBOOL, if anybody cares
  } //packBitsForClassInstances

  void SymbolClassName::printUnresolvedVariablesForClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->printUnresolvedVariableDataMembers();
    m_state.popClassContext(); //restore
  } //printUnresolvedVariablesForClassInstances

  void SymbolClassName::buildDefaultValueForClassInstances()
  {
    BV8K dval;
    SymbolClass::getDefaultValue(dval); //this instance
  } //buildDefaultValueForClassInstances

  void SymbolClassName::testForClassInstances(File * fp)
  {
    SymbolClass::testThisClass(fp);
  } //testForClassInstances

  void SymbolClassName::assignRegistrationNumberForClassInstances()
  {
    u32 n = SymbolClass::getRegistryNumber();
    assert(n != UNINITTED_REGISTRY_NUMBER); //sanity
  } //assignRegistrationNumberForClassInstances

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
  }

  void SymbolClassName::generateAllIncludesForTestMainForClassInstances(File * fp)
  {
    return SymbolClass::generateAllIncludesForTestMain(fp);
  }

  void SymbolClassName::generateForwardDefsForClassInstances(File * fp)
  {
    return SymbolClass::generateAsOtherForwardDef(fp);
  }

  void SymbolClassName::generateTestInstanceForClassInstances(File * fp, bool runtest)
  {
    SymbolClass::generateTestInstance(fp, runtest);
  }

} //end MFM

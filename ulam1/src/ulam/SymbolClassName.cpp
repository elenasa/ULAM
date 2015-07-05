#include "SymbolClassName.h"
#include "Resolver.h"
#include "CompilerState.h"

namespace MFM {

  SymbolClassName::SymbolClassName(Token id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClass(id, utype, classblock, NULL/* parent template */, state), m_gotStructuredCommentToken(false)
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

  bool SymbolClassName::getStructuredComment(Token& scTok)
  {
    if(m_gotStructuredCommentToken)
      {
	assert(m_structuredCommentToken.m_type == TOK_STRUCTURED_COMMENT);
	scTok = m_structuredCommentToken;
	return true;
      }
    return false;
  } //getStructuredComment

  void SymbolClassName::getTargetDescriptorsForClassInstances(TargetMap& classtargets)
  {
    u32 scid = 0;
    Token scTok;
    if(getStructuredComment(scTok))
      scid = scTok.m_dataindex;

    SymbolClass::addTargetDescriptionMapEntry(classtargets, scid);
  } //getTargetDescriptorsForClassInstances

  void SymbolClassName::getModelParameterDescriptionsForClassInstances(ParameterMap& classmodelparameters)
  {
    SymbolClass::addModelParameterDescriptionsMapEntry(classmodelparameters);
  } //getModelParameterDescriptionsForClassInstances

  bool SymbolClassName::isClassTemplate()
  {
    return false;
  } //isClassTemplate

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
    assert(instance == getUlamTypeIdx());
    return "10"; //zero args
  }

  void SymbolClassName::updateLineageOfClass()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    if(!classNode)
      {
	std::ostringstream msg;
	msg << "LineageUpdate skipped for a class <";
	msg << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	msg << "> without a definition, maybe not a class at all";
	MSG(Symbol::getTokPtr(), msg.str().c_str(), ERR);
	return;
      }

    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

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

  void SymbolClassName::checkAndLabelClassFirst()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    if(!classNode)
      {
	std::ostringstream msg;
	msg << "Check and Label skipped for a class <";
	msg << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	msg << "> without a definition, maybe not a class at all";
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

  u32 SymbolClassName::countNavNodesInClassInstances()
  {
    assert(!isClassTemplate());
    u32 navCounter = 0;
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->countNavNodes(navCounter);
    if(navCounter > 0)
      {
	std::ostringstream msg;
	msg << navCounter << " data member nodes with unresolved types remain in class <";
	msg << m_state.getUlamTypeNameBriefByIndex(getUlamTypeIdx()).c_str();
	msg << ">";
	MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
      }
    m_state.popClassContext(); //restore
    return navCounter;
  } //countNavNodesInClassInstances

  bool SymbolClassName::setBitSizeOfClassInstances()
  {
    bool aok = true;
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
	    msg << "CLASS (regular): " << m_state.getUlamTypeNameByIndex(cuti).c_str();
	    msg << " SIZED FAILED: " << totalbits;
	    MSG(Symbol::getTokPtr(), msg.str().c_str(),ERR);
	    classNode->setNodeType(Nav); //avoid assert in resolving loop
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "CLASS (regular): " << m_state.getUlamTypeNameByIndex(cuti).c_str();
	    msg << " SIZED: " << totalbits;
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

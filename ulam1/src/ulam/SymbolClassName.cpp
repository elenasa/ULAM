#include "SymbolClassName.h"
#include "Resolver.h"
#include "CompilerState.h"

namespace MFM {

  SymbolClassName::SymbolClassName(u32 id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClass(id, utype, classblock, NULL/* parent template */, state)
  {
    unsetStub(); //regular class; classblock may be null if utype is UC_UNSEEN class type.
  }

  SymbolClassName::~SymbolClassName(){}

  void SymbolClassName::getTargetDescriptorsForClassInstances(TargetMap& classtargets)
  {
    SymbolClass::addTargetDescriptionMapEntry(classtargets);
  } //getTargetDescriptorsForClassInstances

  bool SymbolClassName::isClassTemplate()
  {
    return false;
  } //isClassTemplate

  void SymbolClassName::linkUnknownBitsizeConstantExpression(UTI auti, NodeTypeBitsize * ceNode)
  {
    SymbolClass::linkConstantExpression(auti, ceNode);
  }

  void SymbolClassName::linkUnknownBitsizeConstantExpression(UTI fromtype, UTI totype)
  {
    SymbolClass::cloneAndLinkConstantExpression(fromtype, totype);
  }

  void SymbolClassName::linkUnknownArraysizeConstantExpression(UTI auti, NodeSquareBracket * ceNode)
  {
    SymbolClass::linkConstantExpression(auti, ceNode);
  }

  void SymbolClassName::linkUnknownNamedConstantExpression(NodeConstantDef * ceNode)
  {
    SymbolClass::linkConstantExpression(ceNode);
  }

  bool SymbolClassName::statusUnknownConstantExpressionsInClassInstances()
  {
    bool aok = true; //all done
    NodeBlockClass * classNode = getClassBlockNode();
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);
    aok = SymbolClass::statusUnknownConstantExpressions();
    m_state.popClassContext(); //restore
    return aok;
  } //statusUnknownConstantExpressionsInClassInstances

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

  void SymbolClassName::constantFoldIncompleteUTIOfClassInstance(UTI instance, UTI auti)
  {
    assert(instance == getUlamTypeIdx());
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    SymbolClass::constantFoldIncompleteUTI(auti);
    m_state.popClassContext(); //restore
  } //constantFoldIncompleteUTIOfClassInstance

  std::string SymbolClassName::formatAnInstancesArgValuesAsAString(UTI instance)
  {
    assert(instance == getUlamTypeIdx());
    return "0";
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
	MSG("", msg.str().c_str(), ERR);
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

  void SymbolClassName::checkAndLabelClassInstances()
  {
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->checkAndLabelType();
    m_state.popClassContext(); //restore
  } //checkAndLabelClassInstances

  u32 SymbolClassName::countNavNodesInClassInstances()
  {
    u32 navCounter = 0;
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.pushClassContext(getUlamTypeIdx(), classNode, classNode, false, NULL);

    classNode->countNavNodes(navCounter);
    if(navCounter > 0)
      {
	std::ostringstream msg;
	msg << navCounter << " data member nodes with illegal 'Nav' types remain in class <";
	msg << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
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
	m_state.setBitSize(cuti, totalbits);  //"scalar" Class bitsize  KEY ADJUSTED
	std::ostringstream msg;
	msg << "CLASS (regular): " << m_state.getUlamTypeNameByIndex(cuti).c_str() << " SIZED: " << totalbits;
	MSG("", msg.str().c_str(),DEBUG);
      }
    m_state.popClassContext(); //restore
    return aok;
  } //setBitSizeOfClassInstances()

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

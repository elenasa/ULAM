#include "SymbolClassName.h"
#include "Resolver.h"
#include "CompilerState.h"

namespace MFM {

  SymbolClassName::SymbolClassName(u32 id, UTI utype, NodeBlockClass * classblock, CompilerState& state) : SymbolClass(id, utype, classblock, NULL, state)
  {
    //    setDeep();
  }

  SymbolClassName::~SymbolClassName(){}

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
    NodeBlockClass * saveClassNode = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;
    NodeBlockClass * classNode = getClassBlockNode();
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(getUlamTypeIdx());
    aok = SymbolClass::statusUnknownConstantExpressions();
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
    return aok;
  } //statusUnknownConstantExpressionsInClassInstances

  Node * SymbolClassName::findNodeNoInAClassInstance(UTI instance, NNO n)
  {
    assert(getUlamTypeIdx() == instance);

    Node * foundNode = NULL;
    NodeBlockClass * saveclassblock = m_state.m_classBlock;
    NodeBlock * savecurrentblock = m_state.m_currentBlock;
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;
    classNode->findNodeNo(n, foundNode);
    m_state.m_classBlock = saveclassblock; //restore
    m_state.m_currentBlock = savecurrentblock;
    return foundNode;
  } //findNodeNoInAClassInstance

  void SymbolClassName::constantFoldIncompleteUTIOfClassInstance(UTI instance, UTI auti)
  {
    assert(instance == getUlamTypeIdx());
    NodeBlockClass * saveclassnode = m_state.m_classBlock;
    NodeBlock * saveblocknode = m_state.m_currentBlock;
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;
    SymbolClass::constantFoldIncompleteUTI(auti);
    m_state.m_classBlock = saveclassnode; //restore
    m_state.m_currentBlock = saveblocknode;
  } //constantFoldIncompleteUTIOfClassInstance

  std::string SymbolClassName::formatAnInstancesArgValuesAsAString(UTI instance)
  {
    assert(instance == getUlamTypeIdx());
    return "0";
  }

  void SymbolClassName::updateLineageOfClassInstanceUTI(UTI instance)
  {
    assert(getUlamTypeIdx() == instance);
    NodeBlockClass * saveclassnode = m_state.m_classBlock;
    NodeBlock * saveblocknode = m_state.m_currentBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(getUlamTypeIdx());
    classNode->updateLineage(NULL);
    m_state.m_classBlock = saveclassnode; //restore
    m_state.m_currentBlock = saveblocknode;
    m_state.setCompileThisIdx(savecompilethisidx);
  } //updateLineageOfClassInstanceUTI

  void SymbolClassName::checkCustomArraysOfClassInstances()
  {
    NodeBlockClass * saveClassNode = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;
    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(getUlamTypeIdx());
    classNode->checkCustomArrayTypeFunctions();
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
  } //checkCustomArraysOfClassInstances()


  void SymbolClassName::checkAndLabelClassInstances()
  {
    NodeBlockClass * saveClassNode = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(getUlamTypeIdx());
    classNode->checkAndLabelType();
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
  } //checkAndLabelClassInstances

  u32 SymbolClassName::countNavNodesInClassInstances()
  {
    u32 navCounter = 0;
    NodeBlockClass * saveClassNode = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(getUlamTypeIdx());

    classNode->countNavNodes(navCounter);
    if(navCounter > 0)
      {
	std::ostringstream msg;
	msg << navCounter << " data member nodes with illegal 'Nav' types remain in class <";
	msg << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str();
	msg << ">";
	MSG(classNode->getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
      }
    m_state.m_classBlock = saveClassNode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
    return navCounter;
  } //countNavNodesInClassInstances

  bool SymbolClassName::setBitSizeOfClassInstances()
  {
    bool aok = true;
    NodeBlockClass * saveclassnode = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode); //infinite loop "Incomplete Class <> was never defined, fails sizing"
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(getUlamTypeIdx());
    s32 totalbits = 0;
    aok = SymbolClass::trySetBitsizeWithUTIValues(totalbits);
    if(aok)
      {
	UTI cuti = getUlamTypeIdx();
	m_state.setBitSize(cuti, totalbits);  //"scalar" Class bitsize  KEY ADJUSTED
	std::ostringstream msg;
	msg << "CLASS (without instances): " << m_state.getUlamTypeNameByIndex(cuti).c_str() << " SIZED: " << totalbits;
	MSG("", msg.str().c_str(),DEBUG);
      }
    m_state.m_classBlock = saveclassnode; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
    return aok;
  } //setBitSizeOfClassInstances()

  // separate pass...after labeling all classes is completed;
  void SymbolClassName::printBitSizeOfClassInstances()
  {
    SymbolClass::printBitSizeOfClass();
  } //printBitSizeOfClassInstances

  void SymbolClassName::packBitsForClassInstances()
  {
    NodeBlockClass * saveclassBlock = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.m_classBlock = getClassBlockNode();
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(getUlamTypeIdx());

    classNode->packBitsForVariableDataMembers();
    m_state.m_classBlock = saveclassBlock; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
  } //packBitsForClassInstances

  void SymbolClassName::testForClassInstances(File * fp)
  {
    NodeBlockClass * saveclassBlock = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    m_state.m_classBlock = getClassBlockNode();
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(getUlamTypeIdx());

    SymbolClass::testThisClass(fp);
    m_state.m_classBlock = saveclassBlock; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
  } //testForClassInstances

  void SymbolClassName::generateCodeForClassInstances(FileManager * fm)
  {
    NodeBlockClass * saveclassBlock = m_state.m_classBlock;
    UTI savecompilethisidx = m_state.m_compileThisIdx;

    NodeBlockClass * classNode = getClassBlockNode();
    assert(classNode);
    m_state.m_classBlock = classNode;
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(getUlamTypeIdx());

    SymbolClass::generateCode(fm);
    m_state.m_classBlock = saveclassBlock; //restore
    m_state.m_currentBlock = m_state.m_classBlock;
    m_state.setCompileThisIdx(savecompilethisidx);
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

#include <stdio.h>
#include "NodeBlock.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlock::NodeBlock(NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s) : NodeStatements(NULL, state), m_ST(state), m_prevBlockNode(prevBlockNode)
  {
    setNextNode(s);
  }

  NodeBlock::~NodeBlock()
  {
    //do not delete prevBlockNode
  }


  void NodeBlock::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    m_nextNode->print(fp);

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }


  void NodeBlock::printPostfix(File * fp)
  {
    fp->write(" {");
    // has no m_node!
    if(m_nextNode)
      m_nextNode->printPostfix(fp);
    else
      fp->write(" <EMPTYSTMT>");  //not an error

    fp->write(" }");
  }


  const char * NodeBlock::getName()
  {
    return "{}";
  }


  const std::string NodeBlock::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeBlock::checkAndLabelType()
  {
    assert(m_nextNode);

    m_state.m_currentBlock = this;

    m_nextNode->checkAndLabelType();

    m_state.m_currentBlock = m_prevBlockNode;  //missing?

    //blocks don't have types
    setNodeType(Void);
    return getNodeType();
  } //checkAndLabelType


  void NodeBlock::countNavNodes(u32& cnt)
  {
      m_nextNode->countNavNodes(cnt);
  }


  EvalStatus NodeBlock::eval()
  {
    assert(m_nextNode);
    //evalNodeProlog(0);

    //makeRoomForNodeType(m_nextNode->getNodeType());
    EvalStatus evs = m_nextNode->eval();    //no return value

    //evalNodeEpilog();
    return evs;
  }


  bool NodeBlock::isIdInScope(u32 id, Symbol * & symptrref)
  {
    return m_ST.isInTable(id, symptrref);
  }


  void NodeBlock::addIdToScope(u32 id, Symbol * symptr)
  {
    m_ST.addToTable(id, symptr);
  }

  NodeBlock * NodeBlock::getPreviousBlockPointer()
  {
    return m_prevBlockNode;
  }

  u32 NodeBlock::getNumberOfSymbolsInTable()
  {
    return m_ST.getTableSize();
  }

  u32 NodeBlock::getSizeOfSymbolsInTable()
  {
    return m_ST.getTotalSymbolSize();
  }


  s32 NodeBlock::getBitSizesOfVariableSymbolsInTable()
  {
    if(m_ST.getTableSize() == 0)
      return EMPTYSYMBOLTABLE;  //should allow no variable data members

    return m_ST.getTotalVariableSymbolsBitSize();
  }


  s32 NodeBlock::getMaxBitSizeOfVariableSymbolsInTable()
  {
    if(m_ST.getTableSize() == 0)
      return EMPTYSYMBOLTABLE;  //should allow no variable data members

    return m_ST.getMaxVariableSymbolsBitSize();
  }


  s32 NodeBlock::findUlamTypeInTable(UTI utype)
  {
    return m_ST.findPosOfUlamTypeInTable(utype);
  }


  SymbolTable * NodeBlock::getSymbolTablePtr()
  {
    return &m_ST;
  }


  void NodeBlock::genCodeDeclsForVariableDataMembers(File * fp, ULAMCLASSTYPE classtype)
  {
    assert(0); //using the NodeVarDecl:genCode approach instead.
    m_ST.genCodeForTableOfVariableDataMembers(fp, classtype);
  }


  void NodeBlock::generateCodeForBuiltInClassFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    m_ST.genCodeBuiltInFunctionsOverTableOfVariableDataMember(fp, declOnly, classtype);
  }


  void NodeBlock::genCode(File * fp, UlamValue& uvpass)
  {
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_nextNode->genCode(fp, uvpass);
    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("}\n");
  }

} //end MFM

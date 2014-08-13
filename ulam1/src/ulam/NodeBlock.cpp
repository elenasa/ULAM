#include <stdio.h>
#include "NodeBlock.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlock::NodeBlock(NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s) : NodeStatements(NULL, state), m_prevBlockNode(prevBlockNode) 
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
    UlamType * myut = getNodeType();
    char id[255];
    if(!myut)    
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), myut->getUlamTypeName().c_str());
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


  UlamType * NodeBlock::checkAndLabelType()
  { 
    assert(m_nextNode);
    setNodeType(m_nextNode->checkAndLabelType()); //missing Fri Aug  8 09:10:27 2014 
    return getNodeType();
  }


  void NodeBlock::eval()
  {
    assert(m_nextNode);
    evalNodeProlog(0);

    makeRoomForNodeType(getNodeType());
    m_nextNode->eval();    //no return

    evalNodeEpilog();
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

} //end MFM

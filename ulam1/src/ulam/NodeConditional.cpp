#include <stdio.h>
#include "NodeConditional.h"
#include "CompilerState.h"
#include "UlamTypeBool.h"

namespace MFM {

  NodeConditional::NodeConditional(Node * leftNode, UTI classInstanceIdx, CompilerState & state): Node(state), m_nodeLeft(leftNode), m_utypeRight(classInstanceIdx) {}

  NodeConditional::~NodeConditional()
  {
    delete m_nodeLeft;
    m_nodeLeft = NULL;
  }


  void NodeConditional::updateLineage(Node * p)
  {
    setYourParent(p);
    m_nodeLeft->updateLineage(this);
  }


  void NodeConditional::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)
      sprintf(id,"%s<NOTYPE>\n",prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    fp->write("conditional:\n");
    assert(m_nodeLeft);
    m_nodeLeft->print(fp);

    sprintf(id," %s ",getName());
    fp->write(id);

    //fp->write(m_typeTok.getTokenString());
    fp->write(m_state.getUlamTypeByIndex(m_utypeRight)->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state).c_str());
    fp->write("\n");
  } //print


  void NodeConditional::printPostfix(File * fp)
  {
    assert(m_nodeLeft);
    m_nodeLeft->printPostfix(fp);

    fp->write(" ");
    //fp->write(m_typeTok.getTokenString());
    fp->write(m_state.getUlamTypeByIndex(m_utypeRight)->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state).c_str());

    printOp(fp);  //operators last
  }


  void NodeConditional::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  }


  void NodeConditional::countNavNodes(u32& cnt)
  {
    m_nodeLeft->countNavNodes(cnt);
  }

  UTI NodeConditional::getRightType()
  {
    return m_utypeRight;
  }

} //end MFM

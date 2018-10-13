#include <stdio.h>
#include "NodeConditional.h"
#include "CompilerState.h"
#include "UlamTypePrimitiveBool.h"

namespace MFM {

  NodeConditional::NodeConditional(Node * leftNode, NodeTypeDescriptor * classType, CompilerState & state): Node(state), m_nodeLeft(leftNode), m_nodeTypeDesc(classType)
  {
    assert(m_nodeLeft);
    assert(m_nodeTypeDesc);
  }

  NodeConditional::NodeConditional(const NodeConditional& ref) : Node(ref)
  {
    m_nodeLeft = ref.m_nodeLeft->instantiate();
    m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeConditional::~NodeConditional()
  {
    delete m_nodeLeft;
    m_nodeLeft = NULL;

    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
  }

  void NodeConditional::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    m_nodeLeft->updateLineage(getNodeNo());
    m_nodeTypeDesc->updateLineage(getNodeNo());
  }

  bool NodeConditional::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_nodeLeft == oldnptr)
      {
	m_nodeLeft = newnptr;
	return true;
      }
    return false;
  }

  bool NodeConditional::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeLeft->findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  }

  void NodeConditional::checkAbstractInstanceErrors()
  {
    if(m_nodeLeft)
      m_nodeLeft->checkAbstractInstanceErrors();
  }

  void NodeConditional::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    UTI ruti = getRightType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n",prettyNodeName().c_str());
    else if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n",prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    fp->write("conditional:\n");
    assert(m_nodeLeft);
    m_nodeLeft->print(fp);

    sprintf(id," %s ", getName());
    fp->write(id);

    fp->write(m_state.getUlamKeyTypeSignatureByIndex(ruti).getUlamKeyTypeSignatureName(&m_state).c_str());
    fp->write("\n");
  } //print

  void NodeConditional::printPostfix(File * fp)
  {
    UTI ruti = getRightType();

    assert(m_nodeLeft);
    m_nodeLeft->printPostfix(fp);

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(ruti).c_str());

    printOp(fp); //operators last
  } //printPostfix

  void NodeConditional::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  }

  FORECAST NodeConditional::safeToCastTo(UTI newType)
  {
    //ulamtype checks for complete, non array, and type specific rules
    return m_state.getUlamTypeByIndex(newType)->safeCast(getNodeType());
  }

  void NodeConditional::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    m_nodeLeft->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    m_nodeTypeDesc->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  }

  UTI NodeConditional::getRightType()
  {
    return m_nodeTypeDesc->givenUTI();
  }

} //end MFM

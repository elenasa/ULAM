#include <assert.h>
#include "ParameterListOfNodes.h"
#include "CompilerState.h"
#include "Constants.h"

namespace MFM{

  ParameterListOfNodes::ParameterListOfNodes() {}

  ParameterListOfNodes::ParameterListOfNodes(const ParameterListOfNodes & pref)
  {
    u32 numparams = pref.getNumberOfParameterNodes();
    for(u32 i = 0; i < numparams; i++)
      {
	Node * pnode = pref.getParameterNodePtr(i);
	Node * pclone = pnode->instantiate();
	addParameterNode(pclone);
      }
  }

  ParameterListOfNodes::~ParameterListOfNodes()
  {
    for(u32 i = 0; i < m_parameterNodes.size(); i++)
      {
	delete m_parameterNodes[i];
	m_parameterNodes[i] = NULL;
      }
    m_parameterNodes.clear();
  }

  ParameterListOfNodes * ParameterListOfNodes::clone()
  {
    return new ParameterListOfNodes(*this);
  }

  void ParameterListOfNodes::addParameterNode(Node * argNode)
  {
    m_parameterNodes.push_back(argNode);
  }

  u32 ParameterListOfNodes::getNumberOfParameterNodes() const
  {
    return m_parameterNodes.size();
  }

  Node * ParameterListOfNodes::getParameterNodePtr(u32 n) const
  {
    assert(n < m_parameterNodes.size());
    return m_parameterNodes[n];
  }

  void ParameterListOfNodes::updateParameterLineage(NNO pno)
  {
    for(u32 i = 0; i < m_parameterNodes.size(); i++)
      {
	m_parameterNodes[i]->updateLineage(pno);
      }
  } //updateParameterLineage

  bool ParameterListOfNodes::findParameterNodeNo(NNO n, Node *& foundNode)
  {
    bool rtnb = false;
    for(u32 i = 0; i < m_parameterNodes.size(); i++)
      {
	if(m_parameterNodes[i]->findNodeNo(n, foundNode))
	  {
	    rtnb = true;
	    break;
	  }
      }
    return rtnb;
  } //findParameterNodeNo

  bool ParameterListOfNodes::checkAndLabelTypesOfParameterNodes(CompilerState & state)
  {
    bool rtnb = true;
    for(u32 i = 0; i < m_parameterNodes.size(); i++)
      {
	UTI puti = m_parameterNodes[i]->checkAndLabelType();
	rtnb &= state.isComplete(puti);
      }
    return rtnb;
  } //checkAndLabelTypesOfParameters

  UTI ParameterListOfNodes::getParameterNodeType(u32 n)
  {
    assert(n < m_parameterNodes.size());
    return m_parameterNodes[n]->getNodeType();
  } //getParameterNodeType

  void ParameterListOfNodes::countNavNodeTypes(u32& cnt)
  {
    for(u32 i = 0; i < m_parameterNodes.size(); i++)
      {
	m_parameterNodes[i]->countNavNodes(cnt);
      }
  } //countNavNodeTypes

} //MFM

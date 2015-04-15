#include <stdlib.h>
#include "NodeTypeArray.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeArray::NodeTypeArray(Token typetoken, UTI auti, NodeType * scalarnode, CompilerState & state) : NodeType(typetoken, auti, state), m_nodeScalar(scalarnode), m_unknownArraysizeSubtree(NULL)
  {
    m_nodeScalar->updateLineage(getNodeNo()); //for unknown subtrees
  }

  NodeTypeArray::NodeTypeArray(const NodeTypeArray& ref) : NodeType(ref), m_nodeScalar(NULL), m_unknownArraysizeSubtree(NULL)
  {
    if(m_nodeScalar)
      m_nodeScalar = (NodeType *) ref.m_nodeScalar->instantiate();

    if(ref.m_unknownArraysizeSubtree)
      m_unknownArraysizeSubtree = new NodeSquareBracket(*ref.m_unknownArraysizeSubtree); //mappedUTI?
  }

  NodeTypeArray::~NodeTypeArray()
  {
    delete m_nodeScalar;
    m_nodeScalar = NULL;

    delete m_unknownArraysizeSubtree;
    m_unknownArraysizeSubtree = NULL;
  } //destructor

  Node * NodeTypeArray::instantiate()
  {
    return new NodeTypeArray(*this);
  }

  void NodeTypeArray::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    m_nodeScalar->updateLineage(getNodeNo());
    m_unknownArraysizeSubtree->updateLineage(getNodeNo());
  }//updateLineage

  bool NodeTypeArray::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeScalar->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTypeArray::printPostfix(File * fp)
  {
    fp->write(getName());
  }

  const char * NodeTypeArray::getName()
  {
    std::ostringstream nstr;
    nstr << m_nodeScalar->getName();
    nstr << "[]";

    return nstr.str().c_str();
  } //getName

  const std::string NodeTypeArray::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeTypeArray::linkConstantExpressionArraysize(NodeSquareBracket * ceForArraySize)
  {
    //tfr owner, or deletes if dup or anothertd ???
    assert(!m_unknownArraysizeSubtree);
    m_unknownArraysizeSubtree = ceForArraySize;
  } //linkConstantExpressionArraysize

  UTI NodeTypeArray::checkAndLabelType()
  {
    if(isReadyType())
      return getNodeType();

    UTI it = Nav;
    if(resolveType(it))
      {
	m_ready = true; // set here
      }

    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  bool NodeTypeArray::resolveType(UTI& rtnuti)
  {
    bool rtnb = false;
    if(isReadyType())
      {
	rtnuti = getNodeType();
	return true;
      }

    // scalar type
    assert(m_nodeScalar);

    UTI scuti;
    if(m_nodeScalar->resolveType(scuti))
      {
	// not node select, we are the leaf Type: a typedef, class or primitive scalar.
	UTI nuti = getNodeType();
	UTI mappedUTI = nuti;
	UTI cuti = m_state.getCompileThisIdx();

	// the symbol associated with this type, was mapped during instantiation
	// since we're call AFTER that (not during), we can look up our
	// new UTI and pass that on up the line of NodeType Selects, if any.
	if(m_state.mappedIncompleteUTI(cuti, nuti, mappedUTI))
	  nuti = mappedUTI;

	if(resolveTypeArraysize(nuti))
	  {
	    rtnb = true;
	    rtnuti = nuti;
	  }
      } //else select not ready, so neither are we!!
    return rtnb;
  } //resolveType

  bool NodeTypeArray::resolveTypeArraysize(UTI auti)
  {
    bool rtnb = true;
    if(m_unknownArraysizeSubtree)
      {
	s32 as = UNKNOWNSIZE;
	//array of primitives or classes
	rtnb = m_unknownArraysizeSubtree->getArraysizeInBracket(as); //eval
	if(rtnb && as != UNKNOWNSIZE)
	  {
	    delete m_unknownArraysizeSubtree;
	    m_unknownArraysizeSubtree = NULL;
	    m_state.setUTISizes(auti, m_state.getBitSize(auti), as); //update UlamType
	  }
      }
    return rtnb;
  } //resolveTypeArraysize

  void NodeTypeArray::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    m_nodeScalar->countNavNodes(cnt);
  }


} //end MFM

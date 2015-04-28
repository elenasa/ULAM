#include <stdlib.h>
#include "NodeTypeDescriptorArray.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeDescriptorArray::NodeTypeDescriptorArray(Token typetoken, UTI auti, NodeTypeDescriptor * scalarnode, CompilerState & state) : NodeTypeDescriptor(typetoken, auti, state), m_nodeScalar(scalarnode), m_unknownArraysizeSubtree(NULL)
  {
    m_nodeScalar->updateLineage(getNodeNo()); //for unknown subtrees
  }

  NodeTypeDescriptorArray::NodeTypeDescriptorArray(const NodeTypeDescriptorArray& ref) : NodeTypeDescriptor(ref), m_nodeScalar(NULL), m_unknownArraysizeSubtree(NULL)
  {
    if(ref.m_nodeScalar)
      m_nodeScalar = (NodeTypeDescriptor *) ref.m_nodeScalar->instantiate();

    if(ref.m_unknownArraysizeSubtree)
      m_unknownArraysizeSubtree = new NodeSquareBracket(*ref.m_unknownArraysizeSubtree); //mappedUTI?
  }

  NodeTypeDescriptorArray::~NodeTypeDescriptorArray()
  {
    delete m_nodeScalar;
    m_nodeScalar = NULL;

    delete m_unknownArraysizeSubtree;
    m_unknownArraysizeSubtree = NULL;
  } //destructor

  Node * NodeTypeDescriptorArray::instantiate()
  {
    return new NodeTypeDescriptorArray(*this);
  }

  void NodeTypeDescriptorArray::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    assert(m_nodeScalar);
    assert(m_unknownArraysizeSubtree);
    m_nodeScalar->updateLineage(getNodeNo());
    m_unknownArraysizeSubtree->updateLineage(getNodeNo());
  }//updateLineage

  bool NodeTypeDescriptorArray::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeScalar->findNodeNo(n, foundNode))
      return true;
    if(m_unknownArraysizeSubtree->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTypeDescriptorArray::printPostfix(File * fp)
  {
    fp->write(getName());
  }

  const char * NodeTypeDescriptorArray::getName()
  {
    std::ostringstream nstr;
    nstr << m_nodeScalar->getName();
    nstr << "[]";

    return nstr.str().c_str();
  } //getName

  const std::string NodeTypeDescriptorArray::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeTypeDescriptorArray::linkConstantExpressionArraysize(NodeSquareBracket * ceForArraySize)
  {
    //tfr owner, or deletes if dup or anothertd ???
    assert(!m_unknownArraysizeSubtree);
    m_unknownArraysizeSubtree = ceForArraySize;
  } //linkConstantExpressionArraysize

  UTI NodeTypeDescriptorArray::checkAndLabelType()
  {
    UTI it = getNodeType();

    if(isReadyType())
      {
	assert(m_state.isComplete(it)); //could it be???
	return it;
      }

    if(resolveType(it))
      {
	m_ready = true; // set here
      }

    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  bool NodeTypeDescriptorArray::resolveType(UTI& rtnuti)
  {
    bool rtnb = false;
    if(isReadyType())
      {
	rtnuti = getNodeType();
	return true;
      }

    // scalar type
    assert(m_nodeScalar);

    UTI scuti = m_nodeScalar->checkAndLabelType();
    if(m_nodeScalar->isReadyType())
      {
	// not node select, we are the array on top of the scalar leaf
	UTI nuti = givenUTI();
	UTI mappedUTI = nuti;
	UTI cuti = m_state.getCompileThisIdx();

	// the symbol associated with this type, was mapped during instantiation
	// since we're call AFTER that (not during), we can look up our
	// new UTI and pass that on up the line of NodeType Selects, if any.
	if(m_state.mappedIncompleteUTI(cuti, nuti, mappedUTI))
	  {
	    std::ostringstream msg;
	    msg << "Substituting Mapped UTI" << mappedUTI;
	    msg << ", " << m_state.getUlamTypeNameByIndex(mappedUTI).c_str();
	    msg << " for incomplete descriptor array type: ";
	    msg << m_state.getUlamTypeNameByIndex(nuti).c_str();
	    msg << "' UTI" << nuti << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    nuti = mappedUTI;
	  }

	// of course, their keys' nameids should be the same (~ enum)!!
	// unless one is a "holder"
	assert((m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum() == m_state.getUlamTypeByIndex(scuti)->getUlamTypeEnum()) || m_state.isHolder(nuti));

	if(resolveTypeArraysize(nuti, scuti))
	  {
	    rtnb = true;
	    rtnuti = nuti;
	  }
      } //else select not ready, so neither are we!!
    return rtnb;
  } //resolveType

  bool NodeTypeDescriptorArray::resolveTypeArraysize(UTI auti, UTI scuti)
  {
    assert(m_unknownArraysizeSubtree);
    s32 as = UNKNOWNSIZE;

    //array of primitives or classes
    bool rtnb = m_unknownArraysizeSubtree->getArraysizeInBracket(as); //eval
    if(rtnb && as != UNKNOWNSIZE)
      {
	// keep in case a template
	//delete m_unknownArraysizeSubtree;
	    //m_unknownArraysizeSubtree = NULL;
	m_state.setUTISizes(auti, m_state.getBitSize(scuti), as); //update UlamType
      }

    attemptToResolveHolderArrayType(auti, scuti);
    //return (m_state.getArraySize(auti) != UNKNOWNSIZE);
    return (m_state.isComplete(auti)); //repeat if holder or bitsize is still unknown
  } //resolveTypeArraysize

  bool NodeTypeDescriptorArray::attemptToResolveHolderArrayType(UTI auti, UTI buti)
  {
    UlamType *aut = m_state.getUlamTypeByIndex(auti);
    UlamType *but = m_state.getUlamTypeByIndex(buti);
    bool aholder = aut->isHolder();
    bool bholder = but->isHolder();

    bool rtnstat = false; //e.g. both are holders

    if(aholder ^ bholder)
      {
	//if auti or buti is a holder, but not both, update a key
	UlamKeyTypeSignature akey = aut->getUlamKeyTypeSignature();
	UlamKeyTypeSignature bkey = but->getUlamKeyTypeSignature();
	if(aholder)
	  {
	    UlamKeyTypeSignature newkey(bkey.getUlamKeyTypeSignatureNameId(), bkey.getUlamKeyTypeSignatureBitSize(), akey.getUlamKeyTypeSignatureArraySize(), akey.getUlamKeyTypeSignatureClassInstanceIdx());
	    m_state.makeUlamTypeFromHolder(akey, newkey, but->getUlamTypeEnum(), auti);
	  }
	else
	  {
	    UlamKeyTypeSignature newkey(akey.getUlamKeyTypeSignatureNameId(), akey.getUlamKeyTypeSignatureBitSize(), bkey.getUlamKeyTypeSignatureArraySize(), bkey.getUlamKeyTypeSignatureClassInstanceIdx());
	    m_state.makeUlamTypeFromHolder(bkey, newkey, aut->getUlamTypeEnum(), buti);
	  }
	rtnstat = true;
      }
    else if(!aholder && !bholder)
      rtnstat = true; //neither are holders

    return rtnstat;
  } //attemptToResolveHolderArrayType

  void NodeTypeDescriptorArray::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    m_nodeScalar->countNavNodes(cnt);
  }

} //end MFM

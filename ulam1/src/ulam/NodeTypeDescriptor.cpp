#include <stdlib.h>
#include "NodeTypeDescriptor.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeDescriptor::NodeTypeDescriptor(Token typetoken, UTI auti, CompilerState & state) : Node(state),  m_typeTok(typetoken), m_uti(auti), m_ready(false), m_unknownBitsizeSubtree(NULL)
  {
    //    setNodeType(auti); //not necessarily "ready"
    setNodeLocation(typetoken.m_locator);
  }

  NodeTypeDescriptor::NodeTypeDescriptor(const NodeTypeDescriptor& ref) : Node(ref), m_typeTok(ref.m_typeTok), m_uti(ref.m_uti), m_ready(false), m_unknownBitsizeSubtree(NULL)
  {
    if(ref.m_unknownBitsizeSubtree)
      m_unknownBitsizeSubtree = new NodeTypeBitsize(*ref.m_unknownBitsizeSubtree); //mapped UTI???
  }

  NodeTypeDescriptor::~NodeTypeDescriptor()
  {
    delete m_unknownBitsizeSubtree;
    m_unknownBitsizeSubtree = NULL;
  } //destructor

  Node * NodeTypeDescriptor::instantiate()
  {
    return new NodeTypeDescriptor(*this);
  }

  void NodeTypeDescriptor::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    if(m_unknownBitsizeSubtree)
      m_unknownBitsizeSubtree->updateLineage(getNodeNo());
  }//updateLineage

  bool NodeTypeDescriptor::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_unknownBitsizeSubtree && m_unknownBitsizeSubtree->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTypeDescriptor::printPostfix(File * fp)
  {
    fp->write(getName());
  }

  const char * NodeTypeDescriptor::getName()
  {
    return m_state.getTokenDataAsString(&m_typeTok).c_str();
  } //getName

  const std::string NodeTypeDescriptor::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeTypeDescriptor::linkConstantExpressionBitsize(NodeTypeBitsize * ceForBitSize)
  {
    assert(!m_unknownBitsizeSubtree);
    m_unknownBitsizeSubtree = ceForBitSize; //tfr owner
  } //linkConstantExpressionBitsize

  bool NodeTypeDescriptor::isReadyType()
  {
    return m_ready;
  }

  UTI NodeTypeDescriptor::givenUTI()
  {
    return m_uti;
  }

  UTI NodeTypeDescriptor::checkAndLabelType()
  {
    if(isReadyType())
      return getNodeType();

    UTI it = givenUTI();
    if(resolveType(it))
      {
	setNodeType(it);
	m_ready = true; // set here!!!
      }

    return getNodeType();
  } //checkAndLabelType

  bool NodeTypeDescriptor::resolveType(UTI& rtnuti)
  {
    bool rtnb = false;
    if(isReadyType())
      {
	rtnuti = getNodeType();
	return true;
      }

    // not node select, we are the leaf Type: a typedef, class or primitive scalar.
    UTI nuti = givenUTI(); //getNodeType();

    // if Nav, use token
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
	msg << " for incomplete descriptor type: ";
	msg << m_state.getUlamTypeNameByIndex(nuti).c_str();
	msg << "' UTI" << nuti << " while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	nuti = mappedUTI;
      }

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE etype = nut->getUlamTypeEnum();
    if(etype == Class)
      {
	if(nut->isComplete())
	  {
	    rtnuti = nuti;
	    rtnb = true;
	  } //else we're not ready!!
      }
    else
      {
	//primitive, or array typedef
	if(!m_unknownBitsizeSubtree)
	  {
	    if(nuti == Nav)
	      //use default primitive bitsize
	      rtnuti = m_state.makeUlamType(m_typeTok, ULAMTYPE_DEFAULTBITSIZE[etype], NONARRAYSIZE, Nav);
	    else
	      rtnuti = nuti;
	    rtnb = true;
	  }
	else
	  {
	    //primitive with possible unknown bit size
	    if(resolveTypeBitsize(nuti))
	      {
		rtnb = true;
		rtnuti = nuti;
	      }
	  }
      }
    return rtnb;
  } //resolveType

  bool NodeTypeDescriptor::resolveTypeBitsize(UTI auti)
  {
    UlamType * ut = m_state.getUlamTypeByIndex(auti);
    ULAMTYPE etype = ut->getUlamTypeEnum();
    if(m_unknownBitsizeSubtree)
      {
	s32 bs = UNKNOWNSIZE;
	//primitive with possible unknown bit sizes.
	bool rtnb = m_unknownBitsizeSubtree->getTypeBitSizeInParen(bs, etype); //eval
	if(rtnb)
	  {
	    // keep in case of template
	    // delete m_unknownBitsizeSubtree;
	    // m_unknownBitsizeSubtree = NULL;
	    m_state.setUTISizes(auti, bs, ut->getArraySize()); //update UlamType
	  }
      }
    return (m_state.isComplete(auti)); //repeat if bitsize is still unknown
  } //resolveTypeBitsize

  void NodeTypeDescriptor::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
  }

  bool NodeTypeDescriptor::assignClassArgValueInStubCopy()
  {
    return true;
  }

  EvalStatus NodeTypeDescriptor::eval()
  {
    assert(0);  //not in parse tree; part of Node's type
    return NORMAL;
  }

} //end MFM

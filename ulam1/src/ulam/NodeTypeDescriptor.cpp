#include <stdlib.h>
#include "NodeTypeDescriptor.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeDescriptor::NodeTypeDescriptor(Token tokarg, UTI auti, CompilerState & state) : Node(state), m_typeTok(tokarg), m_uti(auti), m_ready(false), m_unknownBitsizeSubtree(NULL), m_refType(ALT_NOT)
  {
    setNodeLocation(m_typeTok.m_locator);
  }

  NodeTypeDescriptor::NodeTypeDescriptor(Token tokarg, UTI auti, CompilerState & state, ALT refarg) : Node(state), m_typeTok(tokarg), m_uti(auti), m_ready(false), m_unknownBitsizeSubtree(NULL), m_refType(refarg)
  {
    setNodeLocation(m_typeTok.m_locator);
    //m_uti = m_state.getUlamTypeAsRef(auti, refarg);
  }

  //since there's no assoc symbol, we map the m_uti here (e.g. S(x,y).sizeof nodeterminalproxy)
  NodeTypeDescriptor::NodeTypeDescriptor(const NodeTypeDescriptor& ref) : Node(ref), m_typeTok(ref.m_typeTok), m_uti(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_uti)), m_ready(false), m_unknownBitsizeSubtree(NULL), m_refType(ref.m_refType)
  {
    if(ref.m_unknownBitsizeSubtree)
      m_unknownBitsizeSubtree = new NodeTypeBitsize(*ref.m_unknownBitsizeSubtree); //mapped UTI?
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
  } //updateLineage

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

  ALT NodeTypeDescriptor::getReferenceType()
  {
    return m_refType;
  }

  void NodeTypeDescriptor::setReferenceType(ALT refarg)
  {
    m_refType = refarg;
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
    else
      {
	setNodeType(Hzy);
	m_state.setGoAgain();
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

    if(m_refType != ALT_NOT)
      {
	nuti = m_state.getUlamTypeAsRef(nuti, m_refType);

	//if reference is not complete, but its deref is, use its sizes to complete us.
	if(!m_state.isComplete(nuti))
	  {
	    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
	    UTI ciuti = nut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureClassInstanceIdx();
	    UlamType * ciut = m_state.getUlamTypeByIndex(ciuti);
	    if(ciut->isComplete())
	      m_state.setUTISizes(nuti, ciut->getBitSize(), ciut->getArraySize());
	  }
      }

    if(!m_state.isComplete(nuti))
      {
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
	    msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
	    msg << " for incomplete descriptor type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << "' UTI" << nuti << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    nuti = mappedUTI;
	  }

	if(!m_state.isComplete(nuti)) //reloads to recheck for debug message
	  {
	    std::ostringstream msg;
	    msg << "Incomplete descriptor for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << " UTI" << nuti << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
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
	else
	  rtnuti = Hzy;
      }
    else
      {
	//primitive, or array typedef
	if(!m_unknownBitsizeSubtree)
	  {
	    if(nuti == Nav)
	      {
	      //use default primitive bitsize
	      rtnuti = m_state.makeUlamType(m_typeTok, ULAMTYPE_DEFAULTBITSIZE[etype], NONARRAYSIZE, Nouti);
	      rtnb = true;
	      }
	    else if(m_state.isComplete(nuti))
	      {
		rtnuti = nuti;
		rtnb = true;
	      }
	    else
	      rtnuti = Hzy;
	    //else mapped?
	  }
	else
	  {
	    //primitive with possible unknown bit size
	    rtnb = resolveTypeBitsize(nuti);
	    rtnuti = nuti;
	  }
      }
    return rtnb;
  } //resolveType

  bool NodeTypeDescriptor::resolveTypeBitsize(UTI& rtnuti)
  {
    UTI auti = rtnuti;
    UlamType * ut = m_state.getUlamTypeByIndex(auti);
    ULAMTYPE etype = ut->getUlamTypeEnum();
    if(m_unknownBitsizeSubtree)
      {
	s32 bs = UNKNOWNSIZE;
	//primitive with possible unknown bit sizes.
	bool rtnb = m_unknownBitsizeSubtree->getTypeBitSizeInParen(bs, etype); //eval
	if(rtnb)
	  {
	    if(bs < 0)
	      {
		std::ostringstream msg;
		msg << "Type Bitsize specifier for " << UlamType::getUlamTypeEnumAsString(etype);
		msg << " type, within (), is a negative numeric constant expression: ";
		msg << bs;
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		rtnuti = Nav;
		return false;
	      }

	    // keep m_unknownBitsizeSubtree in case of template (don't delete)
	    m_state.setUTISizes(auti, bs, ut->getArraySize()); //update UlamType, outputs errors
	  }
      }
    assert(auti == rtnuti);
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
  } //eval

} //end MFM

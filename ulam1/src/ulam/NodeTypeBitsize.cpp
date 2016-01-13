#include <stdlib.h>
#include "NodeTypeBitsize.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeBitsize::NodeTypeBitsize(Node * node, CompilerState & state) : Node(state), m_node(node)
  {
    m_node->updateLineage(getNodeNo()); //for unknown subtrees
  }

  NodeTypeBitsize::NodeTypeBitsize(const NodeTypeBitsize& ref) : Node(ref)
  {
    m_node = ref.m_node->instantiate();
  }

  NodeTypeBitsize::~NodeTypeBitsize()
  {
    delete m_node;
    m_node = NULL;
  }

  Node * NodeTypeBitsize::instantiate()
  {
    return new NodeTypeBitsize(*this);
  }

  void NodeTypeBitsize::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    m_node->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeTypeBitsize::exchangeKids(Node * oldnptr, Node * newnptr)
  {
    if(m_node == oldnptr)
      {
	m_node = newnptr;
	return true;
      }
    return false;
  } //exhangeKids

  bool NodeTypeBitsize::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_node && m_node->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTypeBitsize::printPostfix(File * fp)
  {
    m_node->printPostfix(fp);
  }

  const char * NodeTypeBitsize::getName()
  {
    return "(bitsize)";
  }

  const std::string NodeTypeBitsize::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeTypeBitsize::checkAndLabelType()
  {
    UTI it = Nav;
    it = m_node->checkAndLabelType();
    if(!m_state.isComplete(it))
      {
	std::ostringstream msg;
	msg << "Type Bitsize specifier: " << m_state.getUlamTypeNameBriefByIndex(it);
	msg << ", within (), is not ready";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	m_state.setGoAgain(); //since not error
	//it = Hzy; //???
      }

    // expects a constant numeric type
    if( !m_node->isAConstant() || (!(m_state.getUlamTypeByIndex(it)->isNumericType()) && m_node->isReadyConstant()))
      {
	std::ostringstream msg;
	msg << "Type Bitsize specifier: " << m_state.getUlamTypeNameBriefByIndex(it);
	msg << ", within (), is not a numeric constant expression";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	it = Nav;
      }

    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  void NodeTypeBitsize::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    m_node->countNavNodes(cnt);
  }

  bool NodeTypeBitsize::assignClassArgValueInStubCopy()
  {
    //return m_node->assignClassArgValueInStubCopy();
    return true;
  }

  EvalStatus NodeTypeBitsize::eval()
  {
    assert(0);  //not in parse tree; part of symbol's type
    return NORMAL;
  }

  // called during parsing Type of variable, typedef, func return val;
  // Requires a constant expression for the bitsize, else error;
  // guards against even Bool's.
  bool NodeTypeBitsize::getTypeBitSizeInParen(s32& rtnBitSize, ULAMTYPE BUT)
  {
    s32 newbitsize = UNKNOWNSIZE;
    UTI sizetype = checkAndLabelType();
    if(sizetype != Nav) //could be hzy
      {
	evalNodeProlog(0); //new current frame pointer
	makeRoomForNodeType(getNodeType()); //offset a constant expression
	if(m_node->eval() == NORMAL)
	  {
	    UlamValue bitUV = m_state.m_nodeEvalStack.popArg();
	    UTI bituti = bitUV.getUlamValueTypeIdx();
	    if(bituti == Nav)
	      newbitsize = UNKNOWNSIZE;
	    else
	      {
		if(m_state.getBitSize(bituti) == UNKNOWNSIZE)
		  newbitsize = bitUV.getImmediateData(MAXBITSPERINT, m_state); //use default
		else
		  newbitsize = bitUV.getImmediateData(m_state);

		//prepare bitsize into C-format:
		newbitsize = m_state.getUlamTypeByIndex(bituti)->getDataAsCs32(newbitsize);
	      }
	  }

	evalNodeEpilog();

	if(newbitsize == UNKNOWNSIZE)
	  {
	    std::ostringstream msg;
	    msg << "Type Bitsize specifier for base type: ";
	    msg << UlamType::getUlamTypeEnumAsString(BUT) << "(UTI" << sizetype;
	    msg << "), is not yet a \"known\" constant expression for class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    return false;
	  }

	if(newbitsize > MAXBITSPERLONG)
	  {
	    std::ostringstream msg;
	    msg << "Type Bitsize specifier for base type: ";
	    msg << UlamType::getUlamTypeEnumAsString(BUT);
	    msg << " has a constant value of " << newbitsize;
	    msg << " that exceeds the maximum bitsize " << MAXBITSPERLONG;
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return false;
	  }

	// warn against even Bool bits, and reduce by 1.
	if(BUT == Bool  && ((newbitsize % 2) == 0) )
	  {
	    newbitsize--;
	    std::ostringstream msg;
	    msg << "Bool Type with EVEN number of bits is internally inconsistent;";
	    msg << "  Reduced by one to " << newbitsize << " bits" ;
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	  }
      }
    else
      {
	// it's a Nav, perhaps not ready.
	if(!m_node->isAConstant())
	  {
	    std::ostringstream msg;
	    msg << "Type Bitsize specifier for base type: ";
	    msg << UlamType::getUlamTypeEnumAsString(BUT);
	    msg << " is not a constant expression";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return false;
	  }
	if(!m_node->isReadyConstant())
	  {
	    std::ostringstream msg;
	    msg << "Type Bitsize specifier for base type: ";
	    msg << UlamType::getUlamTypeEnumAsString(BUT);
	    msg << " is not a ready constant expression";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    return false;
	  }
      }
    rtnBitSize = newbitsize;
    return true;
  } //getTypeBitSizeInParen

} //end MFM

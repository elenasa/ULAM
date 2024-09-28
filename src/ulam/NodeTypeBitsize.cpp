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

  UTI NodeTypeBitsize::checkAndLabelType(Node * thisparentnode)
  {
    UTI it = getNodeType();

    if(it != Nav)
      {
	bool savCnstInitFlag = m_state.m_initSubtreeSymbolsWithConstantsOnly; //t3219
	m_state.m_initSubtreeSymbolsWithConstantsOnly = true;

	it = m_node->checkAndLabelType(this); //previous time through

	m_state.m_initSubtreeSymbolsWithConstantsOnly = savCnstInitFlag; //restore
      }

    if(!m_state.okUTItoContinue(it) || !m_state.isComplete(it))
      {
	std::ostringstream msg;
	msg << "Type Bitsize specifier, within (), is not ready";
	if(m_state.okUTItoContinue(it) || m_state.isStillHazy(it))
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //t3787
	    it = Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    it = Nav;
	  }
      }
    else
      {
	// expects a constant numeric type (t3526)
	TBOOL iscnst = m_node->isAConstant();
	if(iscnst != TBOOL_TRUE)
	  {
	    std::ostringstream msg;
	    msg << "Type Bitsize specifier: " << m_state.getUlamTypeNameBriefByIndex(it);
	    msg << ", within (), is not a numeric constant expression";
	    if(iscnst == TBOOL_HAZY)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		it = Hzy;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		it = Nav;
	      }
	  }
	else if(m_state.getUlamTypeByIndex(it)->isNumericType() && !m_node->isReadyConstant()) //t41636
	  {
	    std::ostringstream msg;
	    msg << "Type Bitsize specifier: " << m_state.getUlamTypeNameBriefByIndex(it);
	    msg << ", within (), is not a ready numeric constant expression";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    it = Hzy;
	  }
      }
    setNodeType(it);
    if(it == Hzy) m_state.setGoAgain(); //since not error
    return getNodeType();
  } //checkAndLabelType

  void NodeTypeBitsize::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    m_node->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  }

  EvalStatus NodeTypeBitsize::eval()
  {
    m_state.abortShouldntGetHere();  //not in parse tree; part of symbol's type
    return NORMAL;
  }

  // called during parsing Type of variable, typedef, func return val;
  // Requires a constant expression for the bitsize, else error;
  // guards against even Bool's.
  bool NodeTypeBitsize::getTypeBitSizeInParen(s32& rtnBitSize, ULAMTYPE BUT, UTI& sizetype)
  {
    s32 newbitsize = UNKNOWNSIZE;
    sizetype = checkAndLabelType(this);
    if(sizetype == Nav) //could be hzy
      {
	return false; //no rtnBitSize
      }
    else if(sizetype == Hzy)
      {
	return false; //no rtnBitSize (t3526)
      }
    else
      {
	TBOOL iscnst = m_node->isAConstant();
	// perhaps not ready.
	if(iscnst != TBOOL_TRUE)
	  {
	    std::ostringstream msg;
	    msg << "Type Bitsize specifier for base type: ";
	    msg << UlamType::getUlamTypeEnumAsString(BUT);
	    msg << ", is not a constant expression";
	    if(iscnst == TBOOL_HAZY)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		sizetype = Hzy;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		sizetype = Nav;
	      }
	    return false; //no rtnBitSize
	  }
	if(!m_node->isReadyConstant())
	  {
	    std::ostringstream msg;
	    msg << "Type Bitsize specifier for base type: ";
	    msg << UlamType::getUlamTypeEnumAsString(BUT);
	    msg << ", is not a ready constant expression";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //t3787
	    sizetype = Hzy;
	    return false; //no rtnBitSize
	  }
      }

    //eval for bit size constant:
    bool evalrtn = false;
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
	    UlamType * bitut = m_state.getUlamTypeByIndex(bituti);
	    if(bitut->getBitSize() == UNKNOWNSIZE)
	      newbitsize = bitUV.getImmediateData(MAXBITSPERINT, m_state); //use default
	    else
	      {
		u32 wordsize = bitut->getTotalWordSize();
		if(wordsize <= MAXBITSPERINT)
		  newbitsize = bitUV.getImmediateData(m_state); //u32 into s32? t41408
		else if(wordsize <= MAXBITSPERLONG)
		  newbitsize = (s32) bitUV.getImmediateDataLong(m_state);
		else
		  m_state.abortGreaterThanMaxBitsPerLong();
	      }
	    //prepare bitsize into C-format:
	    newbitsize = bitut->getDataAsCs32(newbitsize); //arg expects u32
	    evalrtn = true; //t41408 compensates for overloaded constant
	  }
      }

    evalNodeEpilog();

    if(!evalrtn && (newbitsize == UNKNOWNSIZE)) //overloaded constant (-2)
      {
	std::ostringstream msg;
	msg << "Type Bitsize specifier for base type: ";
	msg << UlamType::getUlamTypeEnumAsString(BUT) << "(UTI" << sizetype;
	msg << "), is not yet a \"known\" constant expression for class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	sizetype = Hzy;
	return false;
      }

    if(BUT == Bits)
      {
	if(newbitsize > MAXBITSPERTRANSIENT)
	  {
	    std::ostringstream msg;
	    msg << "Type Bitsize specifier for base type: ";
	    msg << UlamType::getUlamTypeEnumAsString(BUT); //Bits
	    msg << ", has a constant value of " << newbitsize;
	    msg << " that exceeds the maximum bitsize " << MAXBITSPERTRANSIENT;
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    sizetype = Nav;
	    return false; //t41558
	  }
      }
    else if(newbitsize > MAXBITSPERLONG)
      {
	std::ostringstream msg;
	msg << "Type Bitsize specifier for base type: ";
	msg << UlamType::getUlamTypeEnumAsString(BUT);
	msg << ", has a constant value of " << newbitsize;
	msg << " that exceeds the maximum bitsize " << MAXBITSPERLONG;
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	sizetype = Nav;
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
    rtnBitSize = newbitsize;
    return true;
  } //getTypeBitSizeInParen

} //end MFM

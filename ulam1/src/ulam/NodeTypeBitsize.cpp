#include <stdlib.h>
#include "NodeTypeBitsize.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeBitsize::NodeTypeBitsize(Node * node, CompilerState & state) : Node(state), m_node(node), m_currBlock(m_state.m_currentBlock), m_currBlockNo(m_state.getCurrentBlockNo()) {}
  NodeTypeBitsize::NodeTypeBitsize(const NodeTypeBitsize& ref) : Node(ref), m_currBlock(NULL), m_currBlockNo(ref.m_currBlockNo)
  {
    m_node = ref.m_node->clone();
  }

  NodeTypeBitsize::~NodeTypeBitsize()
  {
    delete m_node;
    m_node = NULL;
  }

  Node * NodeTypeBitsize::clone()
  {
    return new NodeTypeBitsize(*this);
  }

  void NodeTypeBitsize::updateLineage(Node * p)
  {
    setYourParent(p);
    m_currBlock = m_state.m_currentBlock; //do it now
    assert(m_state.getCurrentBlockNo() == m_currBlockNo);
    m_node->updateLineage(this);
  }//updateLineage

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
    if(!(it == m_state.getUlamTypeOfConstant(Int) || it == m_state.getUlamTypeOfConstant(Unsigned)))
      {
	std::ostringstream msg;
	msg << "Type Bitsize specifier: " << m_state.getUlamTypeNameByIndex(it) << ", inside (), is not a valid constant expression";
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

  NNO NodeTypeBitsize::getBlockNo()
  {
    return m_currBlockNo;
  }

  void NodeTypeBitsize::setBlock()
  {
    m_currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(m_currBlock);
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
    NodeBlock * savecurrentblock = m_state.m_currentBlock; //**********

    s32 newbitsize = UNKNOWNSIZE; //was ANYBITSIZECONSTANT;
    UTI sizetype = checkAndLabelType();
    if((sizetype == m_state.getUlamTypeOfConstant(Int) || sizetype == m_state.getUlamTypeOfConstant(Unsigned)))
      {
	m_state.m_currentBlock = m_currBlock;
	evalNodeProlog(0); //new current frame pointer
	makeRoomForNodeType(getNodeType()); //offset a constant expression
	m_node->eval();
	UlamValue bitUV = m_state.m_nodeEvalStack.popArg();
	evalNodeEpilog();
	m_state.m_currentBlock = savecurrentblock; //restore

	if(bitUV.getUlamValueTypeIdx() == Nav)
	  newbitsize = UNKNOWNSIZE;
	else
	  newbitsize = bitUV.getImmediateData(m_state);

	if(newbitsize == UNKNOWNSIZE)
	  {
	    std::ostringstream msg;
	    msg << "Type Bitsize specifier for base type: " << UlamType::getUlamTypeEnumAsString(BUT) << "(), is not yet a \"known\" constant expression for class: " << m_state.getUlamTypeNameByIndex(m_state.m_compileThisIdx).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	    return false;
	  }

	// warn against even Bool bits, and reduce by 1.
	if(BUT == Bool  && ((newbitsize % 2) == 0) )
	  {
	    newbitsize--;
	    std::ostringstream msg;
	    msg << "Bool Type with EVEN number of bits is internally inconsistent; Reduced by one to " << newbitsize << " bits" ;
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Type Bitsize specifier for base type: " << UlamType::getUlamTypeEnumAsString(BUT) << "() is not a constant expression for class: " << m_state.getUlamTypeNameByIndex(m_state.m_compileThisIdx).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }
    rtnBitSize = newbitsize;
    return true;
  } //getTypeBitSizeInParen

} //end MFM



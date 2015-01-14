#include <stdlib.h>
#include "NodeTypeBitsize.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeBitsize::NodeTypeBitsize(Node * node, CompilerState & state) : Node(state), m_node(node) {}

  NodeTypeBitsize::~NodeTypeBitsize()
  {
    delete m_node;
    m_node = NULL;
  }


  void NodeTypeBitsize::updateLineage(Node * p)
  {
    setYourParent(p);
    m_node->updateLineage(this);
  }


  void NodeTypeBitsize::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    m_node->countNavNodes(cnt);
  }


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
    s32 newbitsize = UNKNOWNSIZE; //was ANYBITSIZECONSTANT;
    UTI sizetype = checkAndLabelType();
    if((sizetype == m_state.getUlamTypeOfConstant(Int) || sizetype == m_state.getUlamTypeOfConstant(Unsigned)))
      {
	evalNodeProlog(0); //new current frame pointer
	makeRoomForNodeType(getNodeType()); //offset a constant expression
	m_node->eval();
	UlamValue bitUV = m_state.m_nodeEvalStack.popArg();
	evalNodeEpilog();

	if(bitUV.getUlamValueTypeIdx() == Nav)
	  newbitsize = UNKNOWNSIZE;
	else
	  newbitsize = bitUV.getImmediateData(m_state);
	if(newbitsize == UNKNOWNSIZE)
	  {
	    std::ostringstream msg;
	    msg << "Type Bitsize specifier for base type: " << UlamType::getUlamTypeEnumAsString(BUT) << "(), is not yet a \"known\" constant expression";
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
	msg << "Type Bitsize specifier for base type: " << UlamType::getUlamTypeEnumAsString(BUT) << "() is not a constant expression";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }

    rtnBitSize = newbitsize;
    return true;
  } //getTypeBitSizeInParen

} //end MFM



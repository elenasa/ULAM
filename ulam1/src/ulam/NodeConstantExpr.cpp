#include <stdlib.h>
#include "NodeConstantExpr.h"
#include "CompilerState.h"


namespace MFM {

  NodeConstantExpr::NodeConstantExpr(Node * node, CompilerState & state) : Node(state), m_node(node) {}

  NodeConstantExpr::~NodeConstantExpr()
  {
    delete m_node;
    m_node = NULL;
  }


  void NodeConstantExpr::updateLineage(Node * p)
  {
    setYourParent(p);
    m_node->updateLineage(this);
  }


  void NodeConstantExpr::printPostfix(File * fp)
  {
    m_node->printPostfix(fp);
  }


  const char * NodeConstantExpr::getName()
  {
    return "const";
  }


  const std::string NodeConstantExpr::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeConstantExpr::checkAndLabelType()
  {
    UTI it = Nav;
    it = m_node->checkAndLabelType();
    if(!(it == m_state.getUlamTypeOfConstant(Int) || it == m_state.getUlamTypeOfConstant(Unsigned)))
      {
	std::ostringstream msg;
	msg << "Named Constant Expr specifier: " << m_state.getUlamTypeNameByIndex(it) << ", is not a valid constant expression";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	it = Nav;
      }
    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType


  void NodeConstantExpr::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    m_node->countNavNodes(cnt);
  }


  EvalStatus NodeConstantExpr::eval()
  {
    assert(0);  //not in parse tree; part of 'nonready' symbol
    return NORMAL;
  }


  // called during parsing rhs of named constant
  // Requires a constant expression, else error;
  bool NodeConstantExpr::foldConstantExpr(s32& rtnBitSize, ULAMTYPE BUT)
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



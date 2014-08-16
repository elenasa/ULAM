#include <stdio.h>
#include "NodeBinaryOp.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOp::NodeBinaryOp(Node * left, Node * right, CompilerState & state) : Node(state), m_nodeLeft(left), m_nodeRight(right) {}

  NodeBinaryOp::~NodeBinaryOp()
  {
    delete m_nodeLeft;
    m_nodeLeft = NULL;
    delete m_nodeRight;
    m_nodeRight = NULL;
  }


  void NodeBinaryOp::print(File * fp)
  {
    printNodeLocation(fp);
    UlamType * myut = getNodeType();
    char id[255];
    if(!myut)
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), myut->getUlamTypeName().c_str());
    fp->write(id);

    if(m_nodeLeft)
      m_nodeLeft->print(fp);
    else
      fp->write("<NULLLEFT>\n");

    if(m_nodeRight)
      m_nodeRight->print(fp);
    else
      fp->write("<NULLRIGHT>\n");

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }



  void NodeBinaryOp::printPostfix(File * fp)
  {
    if(m_nodeLeft) 
      m_nodeLeft->printPostfix(fp);
    else  
      fp->write("<NULLLEFT>");

    if(m_nodeRight)
      m_nodeRight->printPostfix(fp);
    else  
      fp->write("<NULLRIGHT>");

    printOp(fp); //operators last
  }


  void NodeBinaryOp::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  }


  EvalStatus NodeBinaryOp::eval()
  {
    assert(m_nodeLeft && m_nodeRight);

    evalNodeProlog(0); //new current frame pointer

    u32 slot = makeRoomForNodeType(getNodeType());
    EvalStatus evs = m_nodeLeft->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    makeRoomForNodeType(getNodeType());
    evs = m_nodeRight->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //copies return UV to stack, -1 relative to current frame pointer
    doBinaryOperation(1, 1+slot, slot);

    evalNodeEpilog();
    return NORMAL;
  }



  UlamType * NodeBinaryOp::calcNodeType(UlamType * lt, UlamType * rt)
  {
    UlamType * newType = m_state.getUlamTypeByIndex(Nav);  //Nav

    if(!(lt->isScalar()) || !(rt->isScalar()))
      {
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) types, LHS: <" << lt->getUlamTypeName().c_str() << ">, RHS: <" << rt->getUlamTypeName().c_str() << "> for binary operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return newType;
      }
    
    UlamType * utInt = m_state.getUlamTypeByIndex(Int);
    UlamType * utFloat = m_state.getUlamTypeByIndex(Float);
    UlamType * utBool = m_state.getUlamTypeByIndex(Bool);

    if(lt == utFloat || rt == utFloat)
      {
	newType = utFloat;
      }
    else if(lt == utInt && rt == utInt)
      {
	newType = utInt;
      }
    else if(lt == utBool || rt == utBool)
      {
	newType = utInt;
      }
    else
      {
	  std::ostringstream msg;
	  msg << "Undefined type to use for " << lt->getUlamTypeName() << " op " << rt->getUlamTypeName();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }

    return newType;
  }


  UlamType * NodeBinaryOp::calcNodeTypeBitwise(UlamType * lt, UlamType * rt)
  {
    UlamType * newType = m_state.getUlamTypeByIndex(Nav);  //Nav

    if(!(lt->isScalar()) || !(rt->isScalar()))
      {
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) types, LHS: <" << lt->getUlamTypeName().c_str() << ">, RHS: <" << rt->getUlamTypeName().c_str() << "> for binary bitwise operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return newType;
      }


    UlamType * utFloat = m_state.getUlamTypeByIndex(Float);

    if(rt == utFloat || lt == utFloat)
      {
	std::ostringstream msg;
	msg << "Invalid Operands of Types: " << lt->getUlamTypeNameBrief() << " and " << rt->getUlamTypeNameBrief()  << " to binary operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	//error
      }
    else
      {    
	if(rt == lt)
	  {
	    //maintain type
	    newType = rt;
	  }
	else
	  {
	    //default is Int
	    newType = m_state.getUlamTypeByIndex(Int);
	  }
      }

    return newType;
  }


  void NodeBinaryOp::genCode(File * fp)
  {
    assert(m_nodeLeft && m_nodeRight);
    m_nodeLeft->genCode(fp);
    fp->write(" ");
    fp->write(getName());
    fp->write(" ");
    m_nodeRight->genCode(fp);
  }

} //end MFM

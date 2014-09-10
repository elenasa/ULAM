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
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
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

    u32 slot2 = makeRoomForNodeType(getNodeType());
    evs = m_nodeRight->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //copies return UV to stack, -1 relative to current frame pointer
    doBinaryOperation(1, 1+slot, slot2);

    evalNodeEpilog();
    return NORMAL;
  }


  void NodeBinaryOp::doBinaryOperationImmediate(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots == 1);
    UTI nuti = getNodeType();
    u32 arraysize = m_state.getArraySize(nuti);
    u32 bitsize   = m_state.getBitSize(nuti);
    u32 len = bitsize * (arraysize > 0 ? arraysize : 1);

    UlamValue luv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(lslot); //immediate value                  
    UlamValue ruv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(rslot); //immediate value                  

    u32 ldata = luv.getImmediateData(len);
    u32 rdata = ruv.getImmediateData(len);
    UlamValue rtnUV = makeImmediateBinaryOp(nuti, ldata, rdata, len);
    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1);
  } //end dobinaryopImmediate


  void NodeBinaryOp::doBinaryOperationArray(s32 lslot, s32 rslot, u32 slots)
  { 
    UlamValue rtnUV;

    UTI nuti = getNodeType();
    u32 arraysize = m_state.getArraySize(nuti);
    u32 bitsize   = m_state.getBitSize(nuti);
   
    UTI scalartypidx = m_state.getUlamTypeAsScalar(nuti);
    bool packRtn = m_state.determinePackable(nuti);

    if(packRtn)
      {
	// pack result too. (slot size known ahead of time)
	rtnUV = UlamValue::makeImmediate(nuti, 0, bitsize * arraysize); //accumulate result here
      }

    // point to base array slots, packedness determines its 'pos'
    UlamValue lArrayPtr = UlamValue::makePtr(lslot, EVALRETURN, nuti, packRtn, m_state);
    UlamValue rArrayPtr = UlamValue::makePtr(rslot, EVALRETURN, nuti, packRtn, m_state);

    // to use incrementPtr(), 'pos' depends on packedness
    UlamValue lp = UlamValue::makeScalarPtr(lArrayPtr, m_state);
    UlamValue rp = UlamValue::makeScalarPtr(rArrayPtr, m_state);

    //make immediate result for each element inside loop
    for(u32 i = 0; i < arraysize; i++)
      {
	UlamValue luv = m_state.getPtrTarget(lp);
	UlamValue ruv = m_state.getPtrTarget(rp);

	u32 ldata = luv.getData(lp.getPtrPos(), bitsize); //'pos' doesn't vary for unpacked
	u32 rdata = ruv.getData(rp.getPtrPos(), bitsize); //'pos' doesn't vary for unpacked
		
	if(packRtn)
	  // use calc position where base [0] is furthest from the end.
	  appendBinaryOp(rtnUV, ldata, rdata, (BITSPERATOM-(bitsize * (arraysize - i))), bitsize);
	else
	  {
	    rtnUV = makeImmediateBinaryOp(scalartypidx, ldata, rdata, bitsize);
	    
	    //copy result UV to stack, -1 (first array element deepest) relative to current frame pointer
	    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -slots + i); 
	  }
	
	lp.incrementPtr(m_state); 
	rp.incrementPtr(m_state);
      } //forloop
    
    if(packRtn)
      m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1);  //store accumulated packed result
    
  } //end dobinaryoparray


  UTI NodeBinaryOp::calcNodeType(UTI lt, UTI rt)
  {
    UTI newType = Nav; //init

    if( m_state.isScalar(lt) && m_state.isScalar(rt))
      {
	if(lt == Int && rt == Int)
	  {
	    newType = Int;
	  }
	else if(lt == Bool || rt == Bool)
	  {
	    newType = Int;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Undefined type to use for " << m_state.getUlamTypeNameByIndex(lt) << " op " << m_state.getUlamTypeNameByIndex(rt);
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
      }
    else
      {
	if(lt == rt)
	  {
	    newType = lt;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Incompatible (nonscalar) types, LHS: <" << m_state.getUlamTypeNameByIndex(lt).c_str() << ">, RHS: <" << m_state.getUlamTypeNameByIndex(rt).c_str() << "> for binary operator" << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }    
    return newType;
  }


  UTI NodeBinaryOp::calcNodeTypeBitwise(UTI lt, UTI rt)
  {
    UTI newType = Nav;  //init

    if(rt == lt)
      {
	//maintain type
	newType = rt;
      }
    else
      {
	if(m_state.isScalar(lt) && m_state.isScalar(rt))
	  {
	    newType = Int;  //default is Int
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Incompatible (nonscalar) types, LHS: <" << m_state.getUlamTypeNameByIndex(lt).c_str() << ">, RHS: <" << m_state.getUlamTypeNameByIndex(rt).c_str() << "> for binary bitwise operator" << getName();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
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

#include <stdio.h>
#include "NodeUnaryOp.h"
#include "CompilerState.h"

namespace MFM {

  NodeUnaryOp::NodeUnaryOp(Node * n, CompilerState & state): Node(state), m_node(n) {}

  NodeUnaryOp::~NodeUnaryOp()
  { 
    delete m_node;
    m_node = NULL;
  }


  void NodeUnaryOp::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)    
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n",prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);
    
    if(m_node)
      m_node->print(fp);
    else 
      fp->write("<NULL>\n");
    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }
  
  
  void NodeUnaryOp::printPostfix(File * fp)
  {
    if(m_node)
      m_node->printPostfix(fp);
    else 
      fp->write("<NULL>");
    
    printOp(fp);  //operators last
  }
  

  void NodeUnaryOp::printOp(File * fp)
  {
    char myname[16];
    sprintf(myname," %s", getName());
    fp->write(myname);
  }


  const std::string NodeUnaryOp::methodNameForCodeGen()
  {
    assert(0);
    return "_UNARY_NOOP";
  }


  UTI NodeUnaryOp::checkAndLabelType()
  { 
    assert(0);  //see unary operators..
    assert(m_node);
    UTI ut = m_node->checkAndLabelType();
    UTI newType = ut;         // init to stay the same
    
    if(!m_state.isScalar(ut)) //array unsupported at this time
      {
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) type: <" << m_state.getUlamTypeNameByIndex(ut).c_str() << "> for unary operator" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);	
	newType = Nav;
      }
    else
      {
	ULAMTYPE eut = m_state.getUlamTypeByIndex(ut)->getUlamTypeEnum();
	if(eut == Bool)
	  {
	    newType = Int;
	    m_node = makeCastingNode(m_node, newType);  //insert node/s
	  }
      }

    setNodeType(newType);

    setStoreIntoAble(false);

    return newType; 
  } //checkAndLabelType
  

  EvalStatus NodeUnaryOp::eval()
  {
    assert(m_node);

    UTI nut = getNodeType();
    evalNodeProlog(0); //new current frame pointer
    u32 slots = makeRoomForNodeType(nut);
    EvalStatus evs = m_node->eval();
    
    if(evs == NORMAL)
      doUnaryOperation(1,slots);

    evalNodeEpilog();
    return evs;
  }


  void NodeUnaryOp::doUnaryOperation(s32 slot, u32 nslots)
  {
    if(m_state.isScalar(getNodeType()))  //not an array
      {
	doUnaryOperationImmediate(slot, nslots);
      }
    else
      { //arrays not supported at this time
	assert(0); 
      }
  } //end dobinaryop


  void NodeUnaryOp::doUnaryOperationImmediate(s32 slot, u32 nslots)
  {
    assert(nslots == 1);
    UTI nuti = getNodeType();
    u32 len = m_state.getTotalBitSize(nuti);

    UlamValue uv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(slot); //immediate value                  
    u32 data = uv.getImmediateData(len);
    UlamValue rtnUV = makeImmediateUnaryOp(nuti, data, len);
    m_state.m_nodeEvalStack.storeUlamValueInSlot(rtnUV, -1);
  } //end dounaryopImmediate


  void NodeUnaryOp::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_node);
    m_node->genCode(fp, uvpass);

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString(&m_state).c_str()); //e.g. u32, s32, u64..
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(getNodeType(),tmpVarNum).c_str());
    fp->write(" = ");

    fp->write(methodNameForCodeGen().c_str());
    fp->write("(");

    UTI uti = uvpass.getUlamValueTypeIdx();
    assert(uti == Ptr);
      
    fp->write(m_state.getTmpVarAsString(uvpass.getPtrTargetType(), uvpass.getPtrSlotIndex()).c_str());

    fp->write(", ");
    fp->write_decimal(nut->getBitSize());

    fp->write(");\n");
  
    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0);  //POS 0 rightjustified.
  } //genCode


  void NodeUnaryOp::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    genCode(fp,uvpass);
  }

} //end MFM

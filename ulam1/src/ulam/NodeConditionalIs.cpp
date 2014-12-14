#include <stdio.h>
#include "NodeConditionalIs.h"
#include "CompilerState.h"

namespace MFM {

  NodeConditionalIs::NodeConditionalIs(Node * leftNode, Token typeTok, CompilerState & state): NodeConditional(leftNode, typeTok, state) {}

  NodeConditionalIs::~NodeConditionalIs()
  {}


  UTI NodeConditionalIs::checkAndLabelType()
  {
    assert(m_nodeLeft);
    UTI newType = Bool;  //except for 'Has'

    UTI luti = m_nodeLeft->checkAndLabelType();  //side-effect
    assert(m_state.isScalar(luti));

    ULAMCLASSTYPE lclasstype = m_state.getUlamTypeByIndex(luti)->getUlamClass();
    if(!(luti == UAtom || lclasstype == UC_ELEMENT))
      {
	std::ostringstream msg;
	msg << "Invalid type for LHS of conditional operator '" << getName() << "'; must be an atom or an element, not <" << m_state.getUlamTypeNameByIndex(luti).c_str() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    UTI ruti = m_state.getUlamTypeFromToken(m_typeTok, 0);

    ULAMCLASSTYPE rclasstype = m_state.getUlamTypeByIndex(ruti)->getUlamClass();
    if(rclasstype != UC_ELEMENT)
      {
	std::ostringstream msg;
	msg << "Invalid type for RHS of conditional operator '" << getName() << "'; must be an element name, not <" << m_state.getUlamTypeNameByIndex(ruti).c_str() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    m_utypeRight = ruti;
    setNodeType(newType);

    setStoreIntoAble(false);
    return getNodeType();
  } //checkAndLabelType


  const char * NodeConditionalIs::getName()
  {
    return "is";
  }


  const std::string NodeConditionalIs::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeConditionalIs::methodNameForCodeGen()
  {
    return  std::string(m_state.getIsMangledFunctionName());
  }


  EvalStatus  NodeConditionalIs::eval()
  {
    assert(m_nodeLeft);
    evalNodeProlog(0);   //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);

    // DO 'IS':
    UTI luti = pluv.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = pluv.getPtrTargetType();

    // inclusive result for eval purposes (atoms and element types are orthogonal)
    bool isit = (luti == UAtom || luti == m_utypeRight);
    UlamValue rtnuv = UlamValue::makeImmediate(getNodeType(), (u32) isit, m_state);

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(rtnuv);

    evalNodeEpilog();
    return evs;
  } //eval


  void NodeConditionalIs::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft);
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    UlamValue luvpass;
    m_nodeLeft->genCode(fp, luvpass);  //loads lhs into tmp (T)
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = luvpass.getPtrTargetType();  //replace

    UlamType * rut = m_state.getUlamTypeByIndex(m_utypeRight);
    ULAMCLASSTYPE rclasstype = rut->getUlamClass();

    s32 tmpVarNum = luvpass.getPtrSlotIndex();
    s32 tmpVarIs = m_state.getNextTmpVarNumber();

     m_state.indent(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString(&m_state).c_str()); //e.g. u32
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarIs).c_str());
    fp->write(" = ");

    fp->write(rut->getUlamTypeMangledName(&m_state).c_str());
    if(rclasstype == UC_ELEMENT)
      fp->write("<CC>::");
    else if(rclasstype == UC_QUARK)
      fp->write("<CC,POS>::");
    else
      assert(0);

    fp->write(methodNameForCodeGen().c_str());  //mangled
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(luti, tmpVarNum).c_str());
    fp->write(");\n");

    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarIs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0);  //POS 0 rightjustified (atom-based).
  } //genCode

} //end MFM

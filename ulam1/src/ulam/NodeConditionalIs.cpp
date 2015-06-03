#include <stdio.h>
#include "NodeConditionalIs.h"
#include "CompilerState.h"

namespace MFM {

  NodeConditionalIs::NodeConditionalIs(Node * leftNode, NodeTypeDescriptor * classType, CompilerState & state): NodeConditional(leftNode, classType, state) {}

  NodeConditionalIs::NodeConditionalIs(const NodeConditionalIs& ref) : NodeConditional(ref) {}

  NodeConditionalIs::~NodeConditionalIs() {}

  Node * NodeConditionalIs::instantiate()
  {
    return new NodeConditionalIs(*this);
  }

  UTI NodeConditionalIs::checkAndLabelType()
  {
    assert(m_nodeLeft);
    UTI newType = Bool;  //except for 'Has'

    UTI luti = m_nodeLeft->checkAndLabelType();  //side-effect
    //assert(m_state.isScalar(luti));

    ULAMCLASSTYPE lclasstype = m_state.getUlamTypeByIndex(luti)->getUlamClass();
    if(!(luti == UAtom || lclasstype == UC_ELEMENT || m_state.isScalar(luti)))
      {
	std::ostringstream msg;
	msg << "Invalid type for LHS of conditional operator '" << getName();
	msg << "'; must be an atom or an element, not type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(luti).c_str();
	msg << " (UTI" << luti << ")";
	if(lclasstype == UC_UNSEEN)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    assert(m_nodeTypeDesc);
    UTI ruti = m_nodeTypeDesc->checkAndLabelType();

    ULAMCLASSTYPE rclasstype = m_state.getUlamTypeByIndex(ruti)->getUlamClass();
    if(!(rclasstype == UC_ELEMENT || m_state.isScalar(ruti)))
      {
	std::ostringstream msg;
	msg << "Invalid type for RHS of conditional operator '" << getName();
	msg << "'; must be an element name, not type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(ruti).c_str();
	msg << " (UTI" << ruti << ")";
	if(rclasstype == UC_UNSEEN || ruti == Nav)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    if(!m_state.getUlamTypeByIndex(ruti)->isComplete())
      {
	std::ostringstream msg;
	msg << "RHS of conditional operator '" << getName() << "' type: ";
	msg << m_state.getUlamTypeNameByIndex(ruti).c_str();
	msg << "; is still incomplete while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	newType = Nav;
      }
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

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

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
    UTI ruti = getRightType();

    // inclusive result for eval purposes (atoms and element types are orthogonal)
    bool isit = (luti == UAtom || UlamType::compare(luti,ruti,m_state) == UTIC_SAME);
    UlamValue rtnuv = UlamValue::makeImmediate(nuti, (u32) isit, m_state);

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
    m_nodeLeft->genCode(fp, luvpass); //loads lhs into tmp (T)
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = luvpass.getPtrTargetType(); //replace

    UTI ruti = getRightType();
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    ULAMCLASSTYPE rclasstype = rut->getUlamClass();

    s32 tmpVarNum = luvpass.getPtrSlotIndex();
    s32 tmpVarIs = m_state.getNextTmpVarNumber();

     m_state.indent(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarIs).c_str());
    fp->write(" = ");

    fp->write(rut->getUlamTypeMangledName().c_str());
    if(rclasstype == UC_ELEMENT)
      fp->write("<EC>::THE_INSTANCE.");
    else if(rclasstype == UC_QUARK)
      fp->write("<EC,POS>::");
    else
      assert(0);

    fp->write(methodNameForCodeGen().c_str()); //mangled
    fp->write("(");
    fp->write(m_state.getTmpVarAsString(luti, tmpVarNum).c_str());
    fp->write(");\n");

    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarIs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0); //POS 0 rightjustified (atom-based).
  } //genCode

} //end MFM

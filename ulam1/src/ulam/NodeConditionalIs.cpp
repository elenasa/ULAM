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
    if(luti == Nav)
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '" << getName();
	msg << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav; //short-circuit
      }

    if(luti == Hzy)
      {
	std::ostringstream msg;
	msg << "Incomplete lefthand type of conditional operator '" << getName();
	msg << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	setNodeType(Hzy);
	m_state.setGoAgain();
	return Hzy; //short-circuit
      }

    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE lclasstype = lut->getUlamClass();
    if(!((lut->getUlamTypeEnum() == UAtom || lclasstype == UC_ELEMENT) && lut->isScalar()))
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '" << getName();
	msg << "'; must be an atom or an element, not type: ";
	msg << lut->getUlamTypeNameBrief().c_str();
	if(lclasstype == UC_UNSEEN || luti == Hzy)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    newType = Hzy;
	    m_state.setGoAgain();
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
      }

    assert(m_nodeTypeDesc);
    UTI ruti = m_nodeTypeDesc->checkAndLabelType();
    if(m_state.okUTItoContinue(ruti))
      {
	UlamType * rut = m_state.getUlamTypeByIndex(ruti);
	//rhs is allowed to be a quark due to inheritance.
	ULAMCLASSTYPE rclasstype = rut->getUlamClass();
	if(!((rclasstype == UC_QUARK || rclasstype == UC_ELEMENT) && rut->isScalar()))
	  {
	    std::ostringstream msg;
	    msg << "Invalid righthand type of conditional operator '" << getName();
	    msg << "'; must be an element name, not type: ";
	    msg << rut->getUlamTypeNameBrief().c_str();
	    if(rclasstype == UC_UNSEEN)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //goagain set
		newType = Hzy;
		m_state.setGoAgain();
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		newType = Nav;
	      }
	  }
      }

    if(!m_state.isComplete(ruti))
      {
	std::ostringstream msg;
	msg << "Righthand type of conditional operator '" << getName() << "' ";
	msg << m_state.getUlamTypeNameBriefByIndex(ruti).c_str();
	msg << " is still incomplete while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	newType = Hzy; //goagain set by nodetypedesc
	m_state.setGoAgain();
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
    assert(m_nodeLeft);
    return  std::string(m_state.getIsMangledFunctionName(m_nodeLeft->getNodeType()));
  }

  EvalStatus  NodeConditionalIs::eval()
  {
    assert(m_nodeLeft);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    evalNodeProlog(0);   //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(1);

    // DO 'IS':
    UTI luti = pluv.getUlamValueTypeIdx();
    assert(luti == Ptr);
    luti = pluv.getPtrTargetType();
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    UTI ruti = getRightType();

    // inclusive result for eval purposes (atoms and element types are orthogonal)
    bool isit = (lut->getUlamTypeEnum() == UAtom || UlamType::compare(luti,ruti,m_state) == UTIC_SAME || m_state.isClassASuperclassOf(luti, ruti));

    UlamValue rtnuv = UlamValue::makeImmediate(nuti, (u32) isit, m_state);

    //also copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(rtnuv);

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

    if(rclasstype == UC_ELEMENT)
      {
	fp->write(rut->getUlamTypeMangledName().c_str());
	fp->write("<EC>::THE_INSTANCE.");
	fp->write(m_state.getIsMangledFunctionName(ruti));
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(luti, tmpVarNum, luvpass.getPtrStorage()).c_str());
	fp->write(");\n");
      }
    else if(rclasstype == UC_QUARK)
      {
	UlamType * lut = m_state.getUlamTypeByIndex(luti);
	ULAMCLASSTYPE lclasstype = lut->getUlamClass();
	if(lclasstype == UC_ELEMENT)
	  {
	    fp->write(lut->getUlamTypeMangledName().c_str());
	    fp->write("<EC>::THE_INSTANCE.");
	    fp->write(m_state.getIsMangledFunctionName(luti)); //UlamElement IsMethod
	    fp->write("("); //one arg
	  }
	else
	  {
	    //then left must be an atom
	    fp->write(m_state.getIsMangledFunctionName(luti)); //UlamElement IsMethod
	    fp->write("(uc, ");
	    fp->write(m_state.getTmpVarAsString(luti, tmpVarNum, luvpass.getPtrStorage()).c_str());
	    fp->write(".GetType(), "); //from tmpvar T
	  }
	fp->write("\"");
	fp->write(rut->getUlamTypeMangledName().c_str());
	fp->write("\");\n");
      }
    else
      assert(0);

    //update uvpass
    uvpass = UlamValue::makePtr(tmpVarIs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0); //POS 0 rightjustified (atom-based).
  } //genCode

} //end MFM

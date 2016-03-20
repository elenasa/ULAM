#include <stdio.h>
#include "NodeConditionalAs.h"
#include "CompilerState.h"

namespace MFM {

  NodeConditionalAs::NodeConditionalAs(Node * leftNode, NodeTypeDescriptor * classType, CompilerState & state): NodeConditional(leftNode, classType, state) {}

  NodeConditionalAs::NodeConditionalAs(const NodeConditionalAs& ref) : NodeConditional(ref) {}

  NodeConditionalAs::~NodeConditionalAs() {}

  Node * NodeConditionalAs::instantiate()
  {
    return new NodeConditionalAs(*this);
  }

  const char * NodeConditionalAs::getName()
  {
    return "as";
  }

  const std::string NodeConditionalAs::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  const std::string NodeConditionalAs::methodNameForCodeGen()
  {
    return  std::string(m_state.getAsMangledFunctionName(m_nodeLeft->getNodeType(), getRightType()));
  }

  UTI NodeConditionalAs::checkAndLabelType()
  {
    assert(m_nodeLeft);
    UTI newType = Bool;

    UTI luti = m_nodeLeft->checkAndLabelType(); //side-effect
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

    assert(m_state.okUTItoContinue(luti));
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE lclasstype = lut->getUlamClass();
    if(!(m_state.isAtom(luti) || (lclasstype == UC_ELEMENT)) && lut->isScalar())
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '" << getName();
	msg << "'; must be an atom or element, not ";
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

    if(!strcmp(m_nodeLeft->getName(), "self")) //was "self"
      {
	std::ostringstream msg;
	msg << "Invalid lefthand identifier of conditional operator '" << getName();
	msg << "'; Suggest using a variable of type Atom as 'self'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }

    assert(m_nodeTypeDesc);
    UTI ruti = m_nodeTypeDesc->checkAndLabelType();
    if(m_state.okUTItoContinue(ruti))
      {
	UlamType * rut = m_state.getUlamTypeByIndex(ruti);
	ULAMCLASSTYPE rclasstype = rut->getUlamClass();
	if(!((rclasstype == UC_QUARK || rclasstype == UC_ELEMENT) && rut->isScalar()))
	  {
	    std::ostringstream msg;
	    msg << "Invalid righthand type of conditional operator '" << getName();
	    msg << "'; must be a quark or element name, not ";
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
	msg << m_state.getUlamTypeNameByIndex(ruti).c_str();
	msg << " is still incomplete while labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	newType = Hzy; //goagain set by nodetypedesc
	m_state.setGoAgain();
      }
    else
      {
	//a place to breakpoint for debugging
	std::ostringstream msg;
	msg << "Ready righthand type of conditional operator '" << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }

    setNodeType(newType);
    Node::setStoreIntoAble(TBOOL_FALSE);
    return getNodeType();
  } //checkAndLabelType

  EvalStatus  NodeConditionalAs::eval()
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

    // DO 'AS': rtype quark 'is' related (was 'has'); rtype element 'is'
    UTI luti = pluv.getUlamValueTypeIdx();
    assert(m_state.isPtr(luti));
    luti = pluv.getPtrTargetType();
    assert(m_state.okUTItoContinue(luti));
    UlamType * lut = m_state.getUlamTypeByIndex(luti);

    if(m_state.isAtom(luti))
      {
	//an atom can be element or quark in eval-land, so let's get specific!
	UlamValue luv = m_state.getPtrTarget(pluv);
	luti = luv.getUlamValueTypeIdx();
	lut = m_state.getUlamTypeByIndex(luti);
      }

    bool asit = false;
    UTI ruti = getRightType();
    assert(m_state.okUTItoContinue(ruti));
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    ULAMCLASSTYPE rclasstype = rut->getUlamClass();
    if(rclasstype == UC_QUARK)
      {
	if(m_state.isClassASubclassOf(luti, ruti))
	  {
	    asit = true;
	  }
	else
	  {
	    //atom's don't work in eval, only genCode, let pass as not found.
	    if(!m_state.isAtom(pluv.getPtrTargetType()))
	      {
		std::ostringstream msg;
		msg << "Invalid lefthand type of conditional operator '" << getName();
		msg << "'; Class '";
		msg << lut->getUlamTypeNameBrief().c_str();
		msg << "' Not Found during eval";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      }
	    else
	      {
		//lhs is an atom!
		std::ostringstream msg;
		msg << "Unsupported lefthand type of conditional operator '" << getName();
		msg <<  "', "  << lut->getUlamTypeNameBrief().c_str();
		msg << "; Passing through as UNFOUND for eval";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      }
	    evalNodeEpilog();
	    return UNEVALUABLE;
	  }
      }
    else if(rclasstype == UC_ELEMENT)
      {
	// like 'is'
	// was inclusive result for eval purposes (atoms and element types are orthogonal)
	// now optional for debugging
#define _LET_ATOM_AS_ELEMENT
#ifndef _LET_ATOM_AS_ELEMENT
	if(m_state.isAtom(luti))
	  {
	    evalNodeEpilog();
	    return UNEVALUABLE;
	  }
	asit = (UlamType::compare(luti, ruti, m_state) == UTIC_SAME);
#else
	asit = m_state.isAtom(luti) || (UlamType::compare(luti, ruti, m_state) == UTIC_SAME);
#endif
      }

    if(asit)
      {
	UTI asuti = ruti; //as deref'd type
	UlamValue ptr = UlamValue::makePtr(pluv.getPtrSlotIndex(), pluv.getPtrStorage(), asuti, m_state.determinePackable(asuti), m_state, pluv.getPtrPos() + 0, pluv.getPtrNameId());
	m_state.m_currentAutoObjPtr = ptr;
	m_state.m_currentAutoStorageType = luti;
      }
    else
      {
      m_state.m_currentAutoObjPtr = UlamValue(); //wipeout
      m_state.m_currentAutoStorageType = Nav; //clear
      }

    UlamValue rtnuv = UlamValue::makeImmediate(nuti, (u32) asit, m_state);
    //also copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(rtnuv);

    evalNodeEpilog();
    return evs;
  } //eval

  void NodeConditionalAs::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_nodeLeft);
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    UTI ruti = getRightType();
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    ULAMCLASSTYPE rclasstype = rut->getUlamClass();
    assert(!rut->isReference());

    UlamValue luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //needs to load lhs into tmp (T); symbol's in COS vector
    UTI luti = luvpass.getUlamValueTypeIdx();
    assert(m_state.isPtr(luti));
    luti = luvpass.getPtrTargetType(); //replaces
    assert(m_state.okUTItoContinue(luti));
    UlamType * lut = m_state.getUlamTypeByIndex(luti);

    //wiped out by reading lhs; needed later by auto NodeVarDecl
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    m_nodeLeft->genCodeReadIntoATmpVar(fp, luvpass);
    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore COS after read.

    s32 tmpVarNum = luvpass.getPtrSlotIndex();
    s32 tmpVarIs = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64, etc.
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarIs).c_str());
    fp->write(" = ");

    if(rclasstype == UC_ELEMENT)
      {
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(ruti).c_str());
	fp->write(".");
	fp->write(m_state.getAsMangledFunctionName(luti, ruti));
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(luti, tmpVarNum, luvpass.getPtrStorage()).c_str());
	fp->write(");\n");
      }
    // not possible!! we already know rhs is an element
    else if(rclasstype == UC_QUARK)
      {
	ULAMCLASSTYPE lclasstype = lut->getUlamClass();
	if(lclasstype == UC_ELEMENT)
	  {
	    if(lut->getReferenceType() == ALT_AS) //e.g. nested-as
	      {
		fp->write(lut->getLocalStorageTypeAsString().c_str());
		fp->write("::Us::THE_INSTANCE.");
	      }
	    else
	      {
		fp->write(lut->getUlamTypeMangledName().c_str());
		fp->write("<EC>::THE_INSTANCE.");
	      }
	    // uses UlamElement isMethod to de-reference a lhs atom
	    fp->write(m_state.getAsMangledFunctionName(luti, ruti));
	    fp->write("("); //one arg
	  }
	else
	  {
	    //then left must be an atom
	    fp->write(m_state.getAsMangledFunctionName(luti, ruti)); //UlamElement IsMethod
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

    //update uvpass, include lhs name id
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    u32 lid = m_state.m_currentObjSymbolsForCodeGen.back()->getId();

    uvpass = UlamValue::makePtr(tmpVarIs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, lid);
  } //genCode

} //end MFM

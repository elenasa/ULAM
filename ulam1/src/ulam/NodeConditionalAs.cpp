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
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	setNodeType(Hzy);
	m_state.setGoAgain();
	return Hzy; //short-circuit
      }

    assert(m_state.okUTItoContinue(luti));
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMTYPE letyp = lut->getUlamTypeEnum();
    ULAMCLASSTYPE lclasstype = lut->getUlamClassType();
    //    if(!(m_state.isAtom(luti) || (lclasstype == UC_ELEMENT)) && lut->isScalar())
    //if(!(m_state.isAtom(luti) || (lclasstype == UC_ELEMENT) || (lclasstype == UC_TRANSIENT)))
    if(!(m_state.isAtom(luti) || (letyp == Class)))
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '" << getName();
	//msg << "'; must be an atom, element or transient, not ";
	msg << "'; must be an atom or class, not ";
	msg << lut->getUlamTypeNameBrief().c_str();
	if(lclasstype == UC_UNSEEN || luti == Hzy)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    newType = Hzy;
	    m_state.setGoAgain();
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	    setNodeType(Nav);
	    return Nav;
	  }
      }

    if(!lut->isScalar())
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '" << getName();
	msg << "'; must be a scalar, not ";
	msg << lut->getUlamTypeNameBrief().c_str();
	if(lclasstype == UC_UNSEEN || luti == Hzy)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    newType = Hzy;
	    m_state.setGoAgain();
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	    setNodeType(Nav);
	    return Nav;
	  }
      }

#if 0
    if(!strcmp(m_nodeLeft->getName(), "self")) //was "self"
      {
	std::ostringstream msg;
	msg << "Invalid lefthand identifier of conditional operator '" << getName();
	msg << "'; Suggest using a variable of type Atom as 'self'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
      }
#endif

    assert(m_nodeTypeDesc);
    UTI ruti = m_nodeTypeDesc->checkAndLabelType();
    if(m_state.okUTItoContinue(ruti))
      {
	UlamType * rut = m_state.getUlamTypeByIndex(ruti);
	//rhs cannot be a ref type
	if(rut->isReference())
	  {
	    std::ostringstream msg;
	    msg << "Invalid righthand type of conditional operator '" << getName();
	    msg << "'; must be a class type, not a reference: ";
	    msg << rut->getUlamTypeNameBrief().c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
	else
	  {
	    ULAMCLASSTYPE rclasstype = rut->getUlamClassType();
	    if(m_state.isAtom(luti) || (lclasstype == UC_ELEMENT))
	      {
		if(!((rclasstype == UC_QUARK || rclasstype == UC_ELEMENT) && rut->isScalar()))
		  {
		    std::ostringstream msg;
		    msg << "Invalid righthand type of conditional operator '" << getName();
		    msg << "'; must be a quark or element name, not ";
		    msg << rut->getUlamTypeNameBrief().c_str();
		    if(rclasstype == UC_UNSEEN)
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
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
	    else if(lclasstype == UC_TRANSIENT)
	      {
		if(!((rclasstype == UC_QUARK || rclasstype == UC_TRANSIENT) && rut->isScalar()))
		  {
		    std::ostringstream msg;
		    msg << "Invalid righthand type of conditional operator '" << getName();
		    msg << "'; must be a quark or transient name, not ";
		    msg << rut->getUlamTypeNameBrief().c_str();
		    if(rclasstype == UC_UNSEEN)
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
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
	    else if(lclasstype == UC_QUARK)
	      {
		ULAMTYPE retyp = rut->getUlamTypeEnum();
		//any class may inherit from a quark
		//if(!((rclasstype == UC_QUARK) && rut->isScalar()))
		if(!((retyp == Class) && rut->isScalar()))
		  {
		    std::ostringstream msg;
		    msg << "Invalid righthand type of conditional operator '" << getName();
		    msg << "'; must be a class name, not ";
		    msg << rut->getUlamTypeNameBrief().c_str();
		    if(rclasstype == UC_UNSEEN)
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
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
	    else
	      assert(0);
	  }
      }

    if(m_state.okUTItoContinue(newType) && !m_state.isComplete(ruti))
      {
	std::ostringstream msg;
	msg << "Righthand type of conditional operator '" << getName() << "': ";
	msg << m_state.getUlamTypeNameByIndex(ruti).c_str();
	msg << ", is still incomplete";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	newType = Hzy; //goagain set by nodetypedesc
	m_state.setGoAgain();
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
    ULAMCLASSTYPE rclasstype = rut->getUlamClassType();
    if(rclasstype == UC_QUARK)
      {
	if(m_state.isClassASubclassOf(luti, ruti))
	  {
	    asit = true;
	  }
	else
	  {
	    if(lut->getUlamClassType() == UC_TRANSIENT)
	      {
		if(lut->getTotalBitSize() > MAXSTATEBITS)
		  {
		    evalNodeEpilog();
		    return UNEVALUABLE;
		  }
	      }
	    else
	      {
		//atom's don't work in eval, only genCode, let pass as not found.
		if(!m_state.isAtom(luti))
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
    else if(rclasstype == UC_TRANSIENT)
      {
	if(m_state.isClassASubclassOf(luti, ruti))
	  {
	    asit = true;
	  }
      }
    else
      assert(0); //honorable death

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

  void NodeConditionalAs::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft);
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    UTI ruti = getRightType();
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    ULAMCLASSTYPE rclasstype = rut->getUlamClassType();
    assert(!rut->isReference());

    UVPass luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //needs to load lhs into tmp (T); symbol's in COS vector
    UTI luti = luvpass.getPassTargetType(); //replaces
    assert(m_state.okUTItoContinue(luti));
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    //ULAMCLASSTYPE lclasstype = lut->getUlamClassType();

    //if(lclasstype != UC_TRANSIENT)
    //if((lclasstype != UC_TRANSIENT) && (lclasstype != UC_QUARK))
    if(m_state.isAtom(luti))
      {
	//wiped out by reading lhs; needed later by auto NodeVarDecl
	std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
	m_nodeLeft->genCodeReadIntoATmpVar(fp, luvpass);
	m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore COS after read.
      }

    s32 tmpVarIs = m_state.getNextTmpVarNumber();

    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //e.g. u32, s32, u64, etc.
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarIs, TMPREGISTER).c_str());
    fp->write(" = ");

    if(rclasstype == UC_ELEMENT)
      {
	// lhs either atom or element, or super quark (self)
	if(m_state.isAtom(luti))
	  {
	    fp->write(m_state.getEffectiveSelfMangledNameByIndex(ruti).c_str());
	    fp->write(".");
	    fp->write(m_state.getAsMangledFunctionName(luti, ruti));
	    fp->write("(");
	    fp->write(luvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(");"); GCNL;
	  }
	else
	  {
	    //can't be nested since elements are not inheritable from;
	    // lhs could be SUPER quark ref (runtime)
	    fp->write(m_state.getEffectiveSelfMangledNameByIndex(ruti).c_str());
	    fp->write(".");  //t3831
	    fp->write(m_state.getAsMangledFunctionName(luti, ruti));
	    fp->write("(&");
	    fp->write(m_state.getEffectiveSelfMangledNameByIndex(luti).c_str());
	    fp->write(");"); GCNL;
	  }
      }
    else if(rclasstype == UC_QUARK)
      {
	//if(lclasstype == UC_ELEMENT)
	if(lut->getUlamTypeEnum() == Class) //either element, quark, or transient (t3823)
	  {
	    if(lut->getReferenceType() == ALT_AS) //e.g. nested-as
	      {
		fp->write(lut->getLocalStorageTypeAsString().c_str());
		fp->write("::Us::THE_INSTANCE.");
	      }
	    else
	      {
		fp->write(m_state.getEffectiveSelfMangledNameByIndex(luti).c_str());
		fp->write(".");
	      }
	    fp->write(m_state.getAsMangledFunctionName(luti, ruti));
	    fp->write("(&"); //one arg
	  }
	else
	  {
	    //then left must be an atom
	    // uses UlamElement isMethod to de-reference a lhs atom
	    fp->write(m_state.getAsMangledFunctionName(luti, ruti)); //UlamElement IsMethod
	    fp->write("(uc, ");

	    fp->write(luvpass.getTmpVarAsString(m_state).c_str());
	    fp->write(".GetType(), &"); //from tmpvar T
	  }
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(ruti).c_str());
	fp->write(");"); GCNL;
      }
    else if(rclasstype == UC_TRANSIENT)
      {
	//lhs must also be a transient, or super quark?
	if(lut->getReferenceType() == ALT_AS) //e.g. nested-as
	  {
	    fp->write(lut->getLocalStorageTypeAsString().c_str());
	    fp->write("::Us::THE_INSTANCE."); //t3826
	  }
	else
	  {
	    fp->write(m_state.getEffectiveSelfMangledNameByIndex(luti).c_str());
	    fp->write(".");  //t3822
	  }
	fp->write(m_state.getAsMangledFunctionName(luti, ruti));
	fp->write("(&");
	fp->write(m_state.getEffectiveSelfMangledNameByIndex(ruti).c_str());
	fp->write(");"); GCNL;
      }
    else
      assert(0); // error/t3827

    //update uvpass, include lhs name id
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    u32 lid = m_state.m_currentObjSymbolsForCodeGen.back()->getId();

    uvpass = UVPass::makePass(tmpVarIs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, lid);

    //NO m_state.clearCurrentObjSymbolsForCodeGen()
  } //genCode

} //end MFM

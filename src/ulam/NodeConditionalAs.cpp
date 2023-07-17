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

  bool NodeConditionalAs::asConditionalNode()
  {
    return true;
  }

  UTI NodeConditionalAs::checkAndLabelType(Node * thisparentnode)
  {
    assert(m_nodeLeft);
    UTI newType = Bool;

    UTI luti = m_nodeLeft->checkAndLabelType(this); //side-effect
    if(luti == Nav)
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '";
	msg  << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav; //short-circuit
      }

    if(luti == Hzy)
      {
	std::ostringstream msg;
	msg << "Incomplete lefthand type of conditional operator '";
	msg  << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	setNodeType(Hzy);
	m_state.setGoAgain();
	return Hzy; //short-circuit
      }

    assert(m_state.okUTItoContinue(luti));
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMTYPE letyp = lut->getUlamTypeEnum();
    ULAMCLASSTYPE lclasstype = lut->getUlamClassType();

    if(!lut->isScalar())
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '";
	msg  << getName() << "'; must be a scalar";
	if(!m_state.isHolder(luti))
	  msg << ", not " << lut->getUlamTypeNameBrief().c_str() << " array";
	if((lclasstype == UC_UNSEEN) || m_state.isStillHazy(luti))
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    newType = Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	    setNodeType(Nav);
	    return Nav;
	  }
      }

    if(!(m_state.isAtom(luti) || (letyp == Class)))
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '";
	msg  << getName() << "'; must be an atom or class";
	if(!m_state.isHolder(luti))
	  msg << ", not " << lut->getUlamTypeNameBrief().c_str();
	if((lclasstype == UC_UNSEEN) || m_state.isStillHazy(luti))
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    newType = Hzy;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	    setNodeType(Nav);
	    return Nav;
	  }
      }

    if(m_nodeLeft->isArrayItem() || m_nodeLeft->isFunctionCall())
      {
	std::ostringstream msg;
	msg << "Invalid lefthand type of conditional operator '";
	msg  << getName() << "'; suggest a reference variable";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
	setNodeType(Nav);
	return Nav;
      }

    if(m_nodeLeft->hasASymbolSuper())
      {
	std::ostringstream msg;
	msg << "Shorthand 'super' is too mind-blowing as the lefthand name ";
	msg << "of conditional operator '" << getName();
	msg << "'; consider using virtual functions instead";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	newType = Nav;
	setNodeType(Nav);
	return Nav;
      }

    assert(m_nodeTypeDesc);
    UTI ruti = m_nodeTypeDesc->checkAndLabelType(this);
    if(m_state.okUTItoContinue(ruti))
      {
	UlamType * rut = m_state.getUlamTypeByIndex(ruti);
	//rhs cannot be a ref type
	if(rut->isAltRefType())
	  {
	    std::ostringstream msg;
	    msg << "Invalid righthand type of conditional operator '" << getName();
	    msg << "'; must be a class type, not a reference: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(ruti).c_str();
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
		    msg << m_state.getUlamTypeNameBriefByIndex(ruti).c_str();
		    if(rclasstype == UC_UNSEEN)
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
			newType = Hzy;
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
		    msg << m_state.getUlamTypeNameBriefByIndex(ruti).c_str();
		    if(rclasstype == UC_UNSEEN)
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
			newType = Hzy;
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
		      }
		    else
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			newType = Nav;
		      }
		  }
		else if(!m_state.isAltRefType(luti) && (rclasstype != UC_QUARK) && (m_state.getReferenceType(luti) != ALT_AS))
		  {
		    //quarks can only inherit from other quarks (t41326);
		    //quark refs cannot be checked at compile time.
		    // nested-as has ALT_AS reference type (t41585)
		    std::ostringstream msg;
		    msg << "Invalid righthand type of conditional operator '" << getName();
		    msg << "'; must be a quark name, not ";
		    msg << rut->getUlamTypeNameBrief().c_str();
		    if(rclasstype == UC_UNSEEN)
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
			newType = Hzy;
		      }
		    else
		      {
			MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
			newType = Nav;
		      }
		  }
		//else
	      }
	    else if(lclasstype == UC_UNSEEN)
	      {
		std::ostringstream msg;
		msg << "Lefthand type of conditional operator '" << getName();
		msg << "'; not a seen class ";
		msg << lut->getUlamTypeNameBrief().c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		newType = Hzy;
	      }
	    else
	      m_state.abortUndefinedUlamClassType();
	  }
      }
    else
      {
	std::ostringstream msg;
	if(ruti == Nav)
	  msg << "Invalid ";
	else
	  msg << "Incomplete ";
	msg << "righthand type of conditional operator '" << getName();
	msg << "' " << m_state.m_pool.getDataAsString(m_nodeTypeDesc->getTypeNameId()).c_str();
	if(ruti == Nav)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    newType = Nav;
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);//t3336,t3741,t41315,t41518
	    newType = Hzy;
	  }
      }

    if(m_state.okUTItoContinue(newType) && !m_state.isComplete(ruti))
      {
	std::ostringstream msg;
	msg << "Righthand type of conditional operator '" << getName() << "' ";
	msg << "is still incomplete";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	newType = Hzy; //goagain set by nodetypedesc (t41518)
      }

    setNodeType(newType);
    if(newType == Hzy) m_state.setGoAgain();
    Node::setStoreIntoAble(TBOOL_FALSE);
    return getNodeType();
  } //checkAndLabelType

  EvalStatus  NodeConditionalAs::eval()
  {
    assert(m_nodeLeft);

    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    evalNodeProlog(0);   //new current frame pointer

    makeRoomForSlots(1); //always 1 slot for ptr
    EvalStatus evs = m_nodeLeft->evalToStoreInto();
    if(evs != NORMAL) return evalStatusReturn(evs);

    UlamValue pluv = m_state.m_nodeEvalStack.loadUlamValuePtrFromSlot(1);

    // DO 'AS': rtype quark 'is' related (was 'has'); rtype element 'is'
    UTI luti = pluv.getUlamValueTypeIdx();
    assert(m_state.isPtr(luti));
    luti = pluv.getPtrTargetType();
    assert(m_state.okUTItoContinue(luti));
    UTI leffself = pluv.getPtrTargetEffSelfType();

    UlamValue luv = m_state.getPtrTarget(pluv); //stg
    UTI lstgeffself = luv.getUlamValueEffSelfTypeIdx(); //t41539
    UTI lstgtype = luv.getUlamValueTypeIdx();

    bool leftisatom = m_state.isAtom(luti) || m_state.isAtom(lstgtype); //t3754,t3837
    if(leftisatom)
      {
	//an atom can be element or quark in eval-land, so let's get specific!
	if(leffself != Nouti)
	  {
	    luti = leffself;
	  }
	else if(lstgeffself != Nouti)
	  {
	    luti = lstgeffself;
	    leffself = lstgeffself; //t3747
	  }
	else if((lstgtype != Nouti) && !m_state.isAtom(lstgtype))
	  {
	    luti = lstgtype; //t41506?
	    leffself = lstgtype;
	  }
	//else stays an atom
      }

    UlamType * lut = m_state.getUlamTypeByIndex(luti);

    bool asit = false;
    UTI ruti = getRightType();
    assert(m_state.okUTItoContinue(ruti));
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    ULAMCLASSTYPE rclasstype = rut->getUlamClassType();
    if(rclasstype == UC_QUARK)
      {
	if((leffself != Nouti) && (UlamType::compareForUlamValueAssignment(leffself, ruti, m_state) == UTIC_SAME))
	  {
	    asit = true; //t41625
	  }
	else if(m_state.isClassASubclassOf(luti, ruti))
	  {
	    asit = true;
	  }
	else if((leffself != Nouti) && m_state.isClassASubclassOf(leffself, ruti))
	  {
	    asit = true; //we must have a quark ref (t41539)
	  }
	else
	  {
	    if(lut->getUlamClassType() == UC_TRANSIENT)
	      {
		if(lut->getTotalBitSize() > MAXSTATEBITS)
		  return evalStatusReturn(UNEVALUABLE);
		//else ???
	      }
	    else
	      {
		//atom's don't work in eval, only genCode, let pass as not found.
		if(!m_state.isAtom(luti))
		  {
		    assert(!asit); //q not a base of effself (t3601)
		  }
		else
		  {
		    //lhs is an atom!
		    std::ostringstream msg;
		    msg << "Unsupported lefthand type of conditional operator '" << getName();
		    msg <<  "', "  << lut->getUlamTypeNameBrief().c_str();
		    msg << "; Passing through as UNFOUND for eval";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		    return evalStatusReturn(UNEVALUABLE);
		  }
	      }
	  }
      }
    else if(rclasstype == UC_ELEMENT)
      {
	if(leffself == Nouti) leffself = luti; //t3249
	// like 'is'
	// was inclusive result for eval purposes (atoms and element types are orthogonal)
	// now optional for debugging; was compared to luti, now effself (t41344)
	//#define _LET_ATOM_AS_ELEMENT
#ifndef _LET_ATOM_AS_ELEMENT
	if(m_state.isAtom(luti))
	  return evalStatusReturn(UNEVALUABLE);
	asit = (UlamType::compare(leffself, ruti, m_state) == UTIC_SAME);
#else
	asit = m_state.isAtom(luti) || (UlamType::compare(leffself, ruti, m_state) == UTIC_SAME);
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
      m_state.abortUndefinedUlamClassType(); //honorable death

    if(asit)
      {
	UTI asuti = ruti; //as deref'd type
	u32 relpos = 0u;
	u32 adjust = 0u;
	s32 invert = 1;
	if(rclasstype != UC_ELEMENT)
	  {
	    if(leffself == Nouti)
	      {
		leffself = luti;
		assert(leffself != Nouti);
	      }

	    if(m_state.isClassASubclassOf(leffself,ruti))
	      {
		AssertBool gotrelpos = m_state.getABaseClassRelativePositionInAClass(leffself, ruti, relpos);
		assert(gotrelpos); //t3589 (was luti)
	      }

	    if(m_state.isClassASubclassOf(leffself,luti))
	      {
		u32 relpos2;
		AssertBool gotrelpos = m_state.getABaseClassRelativePositionInAClass(leffself, luti, relpos2);
		assert(gotrelpos);
		relpos -= relpos2;
	      }
	  }

	if((leftisatom || m_state.isAtom(luti)) && (pluv.getPtrPos() == 0))
	  adjust = ATOMFIRSTSTATEBITPOS; //t3563, t3637

	UlamValue ptr = UlamValue::makePtr(pluv.getPtrSlotIndex(), pluv.getPtrStorage(), asuti, m_state.determinePackable(asuti), m_state, pluv.getPtrPos() + (relpos * invert) + adjust, pluv.getPtrNameId());
	ptr.setPtrTargetEffSelfType(leffself, m_state); //t41318, t41384 (was luti)
	ptr.checkForAbsolutePtr(pluv);

	m_state.m_currentAutoObjPtr = ptr;
	m_state.m_currentAutoStorageType = luti;
      }
    else
      {
	m_state.m_currentAutoObjPtr = UlamValue(); //wipeout
	m_state.m_currentAutoStorageType = Nouti; //clear (was Nav)
      }

    UlamValue rtnuv = UlamValue::makeImmediate(nuti, (u32) asit, m_state);
    //also copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValueToStack(rtnuv);

    if(evs != NORMAL) return evalStatusReturn(evs);

    evalNodeEpilog();
    return NORMAL;
  } //eval

  void NodeConditionalAs::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_nodeLeft);
    UTI lnuti = m_nodeLeft->getNodeType();
    if(m_state.isAtom(lnuti))
      return genCodeAtomAs(fp, uvpass); //reads into tmpvar
    else if(m_state.isReference(lnuti)) //not isAltRefType, consistent w NodeConditionalIs
      return genCodeReferenceAs(fp, uvpass); //doesn't read into tmpvar
    //else ClassAs.. reads into tmp var.

    UTI nuti = getNodeType(); //Bool
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    UVPass luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //no need to load lhs into tmp (including array item)
    UTI luti = luvpass.getPassTargetType();

    UTI ruti = getRightType();
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    assert(!rut->isAltRefType());

    s32 tmpVarIs = m_state.getNextTmpVarNumber();

    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //bool
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarIs, TMPREGISTER).c_str());
    fp->write(" = (");

    //is a class t3582,3,6,9 (reversed luti,ruti order to 'is')
    fp->write(m_state.getTheInstanceMangledNameByIndex(luti).c_str());
    fp->write(".");
    fp->write(m_state.getAsMangledFunctionName(luti, ruti)); //UlamElement IsMethod
    fp->write("(&"); //one arg
    fp->write(m_state.getTheInstanceMangledNameByIndex(ruti).c_str());
    fp->write("));"); GCNL;

    //update uvpass, include lhs name id
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    u32 lid = m_state.m_currentObjSymbolsForCodeGen.back()->getId();

    //luti and ruti can be the same class (e.g. t3754)
    //u32 relpos = UNRELIABLEPOS;
    //AssertBool gotPos = m_state.getABaseClassRelativePositionInAClass(luti, ruti, relpos);
    //assert(gotPos);

    uvpass = UVPass::makePass(tmpVarIs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, lid);
    //NO m_state.clearCurrentObjSymbolsForCodeGen()
  } //genCode

  void NodeConditionalAs::genCodeAtomAs(File * fp, UVPass & uvpass)
  {
    assert(m_nodeLeft);
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    UVPass luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //loads lhs into tmp (T)

    UTI luti = luvpass.getPassTargetType();
    assert(m_state.isAtom(luti)); //or Atomref

    //wiped out by reading lhs; needed later by auto NodeVarDecl
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;
    m_nodeLeft->genCodeReadIntoATmpVar(fp, luvpass);
    m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //restore COS after read.

    UTI ruti = getRightType();
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    assert(!rut->isAltRefType());

    s32 tmpVarIs = m_state.getNextTmpVarNumber();

    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //bool
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarIs, TMPREGISTER).c_str());
    fp->write(" = ");

    if(rut->getUlamClassType() == UC_ELEMENT)
      {
	//reversed call to rhs' overloaded c-implemented 'Is' method; rtn bool;
	// using lhs' T as argument; required for EMPTY-ELEMENT special case
	fp->write(m_state.getTheInstanceMangledNameByIndex(ruti).c_str());
	fp->write(".");
	fp->write(m_state.getAsMangledFunctionName(luti, ruti)); //UlamElement IsMethod
	fp->write("(");
	fp->write(luvpass.getTmpVarAsString(m_state).c_str()); //from tmpvar T or ABS
	if(m_state.isAtomRef(luti) && (luvpass.getPassStorage() == TMPBITVAL))
	  fp->write(".read()"); //t3920, not for t3921 (is tests)
	fp->write(");"); GCNL;
      }
    else
      {
	fp->write(m_state.getAsMangledFunctionName(luti, ruti)); //UlamClass IsMethod
	fp->write("(uc, ");
	fp->write(luvpass.getTmpVarAsString(m_state).c_str());
	fp->write(".GetType(), &"); //from tmpvar T or ABS
	fp->write(m_state.getTheInstanceMangledNameByIndex(ruti).c_str());
	fp->write(");"); GCNL;
      }
    //update uvpass, include lhs name id
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    u32 lid = m_state.m_currentObjSymbolsForCodeGen.back()->getId();

    uvpass = UVPass::makePass(tmpVarIs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, lid); //relpos?
    //NO m_state.clearCurrentObjSymbolsForCodeGen()
  } //genCodeAtomAs

  void NodeConditionalAs::genCodeReferenceAs(File * fp, UVPass & uvpass)
  {
    assert(m_nodeLeft);
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    UVPass luvpass;
    m_nodeLeft->genCodeToStoreInto(fp, luvpass); //loads lhs into tmp (T)
    UTI luti = luvpass.getPassTargetType(); //replace
    assert(m_state.isReference(luti));

    Symbol * stgcos = NULL;
    if(m_state.m_currentObjSymbolsForCodeGen.empty())
      stgcos = m_state.getCurrentSelfSymbolForCodeGen();
    else
      stgcos = m_state.m_currentObjSymbolsForCodeGen.back();

    UTI ruti = getRightType();
    UlamType * rut = m_state.getUlamTypeByIndex(ruti);
    assert(!rut->isAltRefType());

    s32 tmpVarIs = m_state.getNextTmpVarNumber();

    m_state.indentUlamCode(fp);
    fp->write("const ");
    fp->write(nut->getTmpStorageTypeAsString().c_str()); //bool
    fp->write(" ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpVarIs, TMPREGISTER).c_str());
    fp->write(" = (");

    //if array, error in c&l
    fp->write(stgcos->getMangledName().c_str());
    fp->write(".GetEffectiveSelf()->");
    fp->write(m_state.getAsMangledFunctionName(luti, ruti)); //UlamClass IsMethod
    fp->write("(&");
    fp->write(m_state.getTheInstanceMangledNameByIndex(ruti).c_str());
    fp->write("));"); GCNL;  //t3655

    //update uvpass, include lhs name id
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    u32 lid = m_state.m_currentObjSymbolsForCodeGen.back()->getId();

    uvpass = UVPass::makePass(tmpVarIs, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, lid); //t3754
    //NO m_state.clearCurrentObjSymbolsForCodeGen()
  } //genCodeReferenceAs


} //end MFM

#include <stdlib.h>
#include "NodeVarRefAs.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"

namespace MFM {

  NodeVarRefAs::NodeVarRefAs(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeVarRef(sym, nodetype, state) { }

  NodeVarRefAs::NodeVarRefAs(const NodeVarRefAs& ref) : NodeVarRef(ref) { }

  NodeVarRefAs::~NodeVarRefAs() { }

  Node * NodeVarRefAs::instantiate()
  {
    return new NodeVarRefAs(*this);
  }

  void NodeVarRefAs::updateLineage(NNO pno)
  {
    NodeVarRef::updateLineage(pno);
  } //updateLineage

  bool NodeVarRefAs::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeVarRef::findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  //see also SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarRefAs::printPostfix(File * fp)
  {
    NodeVarRef::printPostfix(fp);
  } //printPostfix

  const char * NodeVarRefAs::getName()
  {
    return NodeVarRef::getName(); //& ??
  }

  const std::string NodeVarRefAs::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeVarRefAs::safeToCastTo(UTI newType)
  {
    return CAST_CLEAR; //created automatically when 'as' is true
  }

  bool NodeVarRefAs::checkReferenceCompatibility(UTI uti)
  {
    assert(m_state.okUTItoContinue(uti));
    assert(m_state.getUlamTypeByIndex(uti)->isReference());
    return true; //ok
  } //checkReferenceCompatibility

  UTI NodeVarRefAs::checkAndLabelType()
  {
    UTI it = NodeVarRef::checkAndLabelType();
    setNodeType(it);
    makeSuperSymbolForAsBlock(); //only when lhs is 'self'
    return getNodeType();
  } //checkAndLabelType

  TBOOL NodeVarRefAs::packBitsInOrderOfDeclaration(u32& offset)
  {
    m_state.abortShouldntGetHere(); //refs can't be data members
    return TBOOL_FALSE;
  } //packBitsInOrderOfDeclaration

  void NodeVarRefAs::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    assert(m_varSymbol);
    return NodeVarRef::calcMaxDepth(depth, maxdepth, base);
  } //calcaMaxDepth

  void NodeVarRefAs::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    NodeVarRef::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  EvalStatus NodeVarRefAs::eval()
  {
    assert(m_varSymbol);

    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    assert(m_varSymbol->getUlamTypeIdx() == nuti);
    assert(!m_state.isAtom(nuti)); //rhs type of conditional as/has can't be an atom

    UlamValue pluv = m_state.m_currentAutoObjPtr;
    ((SymbolVariableStack *) m_varSymbol)->setAutoPtrForEval(pluv); //for future ident eval uses

    UTI luti = pluv.getPtrTargetType();
    assert(m_state.okUTItoContinue(luti));
    UlamType * lut = m_state.getUlamTypeByIndex(luti);
    ULAMCLASSTYPE lclasstype = lut->getUlamClassType();
    UTI autostgtype = m_state.m_currentAutoStorageType;
    if((UlamType::compare(autostgtype, UAtom, m_state) == UTIC_SAME) && (lclasstype == UC_ELEMENT))
       autostgtype = luti; //e.g. funccall expects a class, not an atom (t3636)

    ((SymbolVariableStack *) m_varSymbol)->setAutoStorageTypeForEval(autostgtype); //for future virtual function call eval uses

    //m_state.m_funcCallStack.storeUlamValueInSlot(pluv, ((SymbolVariableStack *) m_varSymbol)->getStackFrameSlotIndex()); //doesn't seem to matter..
    m_state.m_currentAutoObjPtr = UlamValue(); //wipeout
    m_state.m_currentAutoStorageType = Nouti; //clear (was Nav)

    return NORMAL;
  } //eval

  EvalStatus  NodeVarRefAs::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  }

  // this is the auto local variable's node, created at parse time,
  // for Conditional-As case.
  void NodeVarRefAs::genCode(File * fp, UVPass & uvpass)
  {
    assert(!m_state.m_currentObjSymbolsForCodeGen.empty());
    // the uvpass comes from NodeControl, and still has the POS obtained
    // during the condition statement for As..unorthodox, but necessary.

    // before shadowing the lhs of the h/as-conditional variable with its auto,
    // let's load its storage from the currentSelfSymbol:
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];
    UTI stgcosuti = stgcos->getUlamTypeIdx();
    UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
    ULAMTYPE stgetyp = stgcosut->getUlamTypeEnum();
    ULAMCLASSTYPE stgclasstype = stgcosut->getUlamClassType();

    assert((stgetyp == UAtom) || (stgetyp == Class)); //lhs

    if(stgcos->isSelf())
      return genCodeRefAsSelf(fp, uvpass);

    s32 tmpVarStg = m_state.getNextTmpVarNumber();

    // can't let Node::genCodeReadIntoTmpVar do this for us: need a ref.
    assert(m_state.m_currentObjSymbolsForCodeGen.size() == 1);
    m_state.indentUlamCode(fp);
    fp->write(stgcosut->getUlamTypeImmediateMangledName().c_str());
    fp->write("<EC> & "); //here it is!! brilliant
    fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str());
    fp->write(" = ");
    fp->write(stgcos->getMangledName().c_str());
    fp->write("; //c++ reference to immediate"); GCNL;

    // now we have our pos in tmpVarPos, and our T in tmpVarStg
    // time to shadow 'self' with auto local variable:
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);
    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();

    if(stgetyp == UAtom)
      {
	// get Type in tmpVarType at runtime
	s32 tmpVarType = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());;
	fp->write(" = ");
	fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str());
	fp->write(".GetType();"); GCNL;

	//get Pos in tmpVarPos at runtime; must be good pos since is-method passed
	s32 tmpVarPos = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
	fp->write(" = ");
	fp->write(m_state.getGetRelPosMangledFunctionName(stgcosuti)); //UlamClass Method
	fp->write("(uc, ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str());
	fp->write(", &");
	fp->write(m_state.getTheInstanceMangledNameByIndex(vuti).c_str());
	fp->write("); //relpos"); GCNL;

	m_state.indentUlamCode(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_varSymbol->getMangledName().c_str());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str());
	fp->write(", ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
	fp->write(" + T::ATOM_FIRST_STATE_BIT, "); //position as super quark (e.g. t3639, t3709, t3675, t3408, t3336); as element t3249, t3255, t3637; as atom ref t3908

	if((vclasstype == UC_QUARK) && !(stgcosut->isReference()))
	  {
	    fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str()); //new UlamRef arg: efftoself
	    fp->write(", ");
	  } //else (element, t3637) (atomref, t3639)

	//note: needs eff self of the atom, not simply the RHS type (t3835)
	fp->write(m_state.getHiddenContextArgName());
	fp->write(".LookupUlamElementTypeFromContext(");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarType, TMPREGISTER).c_str()); //3636
	fp->write(")");
	if(!stgcosut->isReference()) //not isAltRefType
	  fp->write(", uc"); //t3249

	fp->write("); //shadows lhs of 'as'"); GCNL;
      }
    else if(stgcosut->isReference()) //not isAltRefType,
      {
	//for refs (t41011,t41012) use the effself to get relpos at runtime;
	//must be good pos since is-method passed
	s32 tmpVarPos = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
	fp->write(" = (");

	fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str()); //3826
	fp->write(".GetEffectiveSelf()->");
	fp->write(m_state.getGetRelPosMangledFunctionName(stgcosuti)); //UlamClass Method
	fp->write("(");
	fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(vuti)); //efficiency
	fp->write("u");
	fp->write(")); //relpos of ");
	fp->write(m_state.getUlamTypeNameBriefByIndex(vuti).c_str()); GCNL;

	s32 tmpVarPosToEff = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write("const s32 ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarPosToEff, TMPREGISTER).c_str());
	fp->write(" = ");
	fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str());
	fp->write(".GetPosToEffectiveSelf();"); GCNL;

	m_state.indentUlamCode(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");
	fp->write(m_varSymbol->getMangledName().c_str());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str());
	fp->write(", ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarPos, TMPREGISTER).c_str());
	fp->write(" - ");
	fp->write(m_state.getTmpVarAsString(Int, tmpVarPosToEff, TMPREGISTER).c_str()); //t41325 'checkas'
	fp->write(", ");
	//note: needs effective self of the atom, not simply the RHS type.
	fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str());
	fp->write(".GetEffectiveSelf()"); //t3835,6,t3754, CLASSIC
	fp->write("); //shadows lhs of 'as'"); GCNL;
      }
    else
      {
	// here, stgcos not a reference
	// get relative position at compile time, both types are known.
	u32 relpos = UNRELIABLEPOS;
	AssertBool gotPos = m_state.getABaseClassRelativePositionInAClass(stgcosuti, vuti, relpos);
	assert(gotPos);

	m_state.indentUlamCode(fp);
	fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
	fp->write(" ");

	fp->write(m_varSymbol->getMangledName().c_str());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(stgcosuti, tmpVarStg, TMPBITVAL).c_str());

	if((stgclasstype == UC_ELEMENT))
	  {
	    fp->write(", ");
	    fp->write_decimal_unsigned(relpos);
	    fp->write("u + T::ATOM_FIRST_STATE_BIT, ");
	    if(vclasstype == UC_QUARK)
	      {
		fp->write_decimal_unsigned(relpos); //new UlamRef arg (t3589)
		fp->write("u, "); //t3586, t3589, t3637
	      } //else (element, t3754)
	    fp->write("&"); //t3586, t3589, t3637

	    //must be same as look up for elements only Sat Jun 18 17:30:17 2016
	    fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());
	    fp->write(", uc"); //t3249
	    fp->write("); //shadows lhs of 'as'"); GCNL;
	  }
	else if((stgclasstype == UC_TRANSIENT))
	  {
	    // transient can be another transient or a quark, not an element
	    fp->write(", ");
	    fp->write_decimal_unsigned(relpos);
	    fp->write("u, ");

	    if(vclasstype == UC_QUARK)
	      {
		fp->write_decimal_unsigned(relpos); //new Ulamref arg (t3827)
		fp->write("u, ");
	      }

	    fp->write("&"); //t3822
	    fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());
	    fp->write(", uc"); //t3249
	    fp->write("); //shadows lhs of 'as'"); GCNL;
	  }
	else if((stgclasstype == UC_QUARK))
	  {
	    // quark can be another quark, not an element, nor transient
	    fp->write(", ");
	    fp->write_decimal_unsigned(relpos);
	    fp->write(", ");
	    fp->write_decimal_unsigned(relpos); //new Ulamref arg (t3830)
	    fp->write("u, &");
	    fp->write(m_state.getTheInstanceMangledNameByIndex(stgcosuti).c_str());

	    assert(vclasstype == UC_QUARK);
	    fp->write(", uc"); //t3249
	    fp->write("); //shadows lhs of 'as'"); GCNL;
	  }
	else
	  m_state.abortUndefinedUlamClassType(); //WHAT THEN???
      } //end else

    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs ?
  } //genCode

  void NodeVarRefAs::genCodeRefAsSelf(File * fp, UVPass& uvpass)
  {
    //no tmpref needed since 'self' (i.e. ur) is already a C++ reference
    //t3821, t3815 (transient), t3828 (quark), t3831
    Symbol * stgcos = m_state.m_currentObjSymbolsForCodeGen[0];

    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    m_state.indentUlamCode(fp);
    fp->write(vut->getLocalStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");
    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write("(");
    fp->write(stgcos->getMangledName().c_str()); //ur
    fp->write(", ");
    fp->write(stgcos->getMangledName().c_str()); //ur
    fp->write(".GetEffectiveSelf()->");
    fp->write(m_state.getGetRelPosMangledFunctionName(vuti)); //nonatom
    fp->write("(");
    fp->write_decimal_unsigned(m_state.getAClassRegistrationNumber(vuti)); //efficiency
    fp->write("u ");
    fp->write("/* ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(vuti).c_str());
    fp->write(" */");
    fp->write(") - ");
    fp->write(stgcos->getMangledName().c_str()); //ur
    fp->write(".GetPosToEffectiveSelf()");

    fp->write(", ");
    fp->write(stgcos->getMangledName().c_str()); //stg
    fp->write(".GetEffectiveSelf()");
    fp->write("); //shadows lhs of 'as'"); GCNL;

    m_state.indentUlamCode(fp);
    fp->write("UlamRef<EC>& ur = ");
    fp->write(m_varSymbol->getMangledName().c_str());
    fp->write("; //shadows self"); GCNL;

    m_varSymbol->setIsSelf(); //tricky
    m_state.m_currentSelfSymbolForCodeGen = m_varSymbol;

    //what about 'super' in this context? tmp super symbol to the rescue! see c&l
    m_state.clearCurrentObjSymbolsForCodeGen(); //clear remnant of lhs
  } //genCodeRefAsSelf

  void NodeVarRefAs::makeSuperSymbolForAsBlock()
  {
    assert(m_varSymbol);
    u32 selfid = m_state.m_pool.getIndexForDataString("self");
    if(m_varSymbol->getId() != selfid)
      return; //nothing to do

    //similar to makeSuperSymbol in NodeBlockFunctionDefinition
    //important to make 'super' before its uses c&l, even if incomplete;
    UTI cuti = m_varSymbol->getUlamTypeIdx(); //the new self, getNodeType() might be Hzy
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    UTI superuti = csym->getBaseClass(0);
    if(superuti == Nouti)
      superuti = m_state.m_urSelfUTI;

    Symbol * supersym = NULL;
    u32 superid = m_state.m_pool.getIndexForDataString("super");
    NodeBlock * curblock = m_state.getCurrentBlock();
    assert(curblock);
    if(!curblock->isIdInScope(superid, supersym))
      {
	assert(m_state.okUTItoContinue(superuti));
	s32 slot = -(m_state.slotsNeeded(m_state.m_currentFunctionReturnType) + 1);

	Token superTok(TOK_KW_SUPER, getNodeLocation(), 0);
	supersym = new SymbolVariableStack(superTok, m_state.getUlamTypeAsRef(superuti, ALT_AS), slot, m_state);
	assert(supersym);
	supersym->setAutoLocalType(ALT_AS);
	supersym->setIsSuper();
	m_state.addSymbolToCurrentScope(supersym); //ownership goes to the block
      }
    else if(!m_state.isComplete(supersym->getUlamTypeIdx()))
      {
	//update superuti when complete
	if(m_state.isComplete(superuti))
	  supersym->resetUlamType(m_state.getUlamTypeAsRef(superuti, ALT_AS));
      }
    else
      assert(UlamType::compare(superuti, m_state.getUlamTypeAsDeref(supersym->getUlamTypeIdx()), m_state) == UTIC_SAME); //sanity check, already done.
  } //makeSuperSymbolForAsBlock

} //end MFM

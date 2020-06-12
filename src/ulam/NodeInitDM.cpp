#include <stdlib.h>
#include "NodeInitDM.h"
#include "NodeListArrayInitialization.h"
#include "NodeListClassInit.h"
#include "SymbolVariableDataMember.h"
#include "CompilerState.h"

namespace MFM {

  NodeInitDM::NodeInitDM(u32 dmid, Node * assignNode, UTI ofclass, CompilerState & state) : NodeConstantDef(NULL, NULL, state), m_posOfDM(UNRELIABLEPOS)
  {
    assert(assignNode);
    setConstantExpr(assignNode);
    m_cid = dmid;
    m_ofClassUTI = ofclass;
  }

  NodeInitDM::NodeInitDM(const NodeInitDM& ref) : NodeConstantDef(ref), m_ofClassUTI(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_ofClassUTI, ref.getNodeLocation())), m_posOfDM(ref.m_posOfDM) { }

  NodeInitDM::~NodeInitDM()
  {
    delete m_constSymbol; //owner, unlike NodeConstDef
    m_constSymbol = NULL;
  }

  Node * NodeInitDM::instantiate()
  {
    return new NodeInitDM(*this);
  }

  void NodeInitDM::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    if(m_nodeExpr)
      m_nodeExpr->updateLineage(getNodeNo());
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  } //updateLineage

  void NodeInitDM::printPostfix(File * fp)
  {
    fp->write(".");
    fp->write(m_state.m_pool.getDataAsString(m_cid).c_str());

    fp->write(" =");
    if(m_nodeExpr)
      m_nodeExpr->printPostfix(fp);
    else
      fp->write(" { }"); //null array init values
  } //printPostfix

  const char * NodeInitDM::getName()
  {
    return m_state.m_pool.getDataAsString(m_cid).c_str();
  }

  const std::string NodeInitDM::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeInitDM::setSymbolPtr(SymbolWithValue * cvsymptr)
  {
    m_state.abortShouldntGetHere();
  }

  void NodeInitDM::resetOfClassType(UTI cuti)
  {
    assert(m_state.okUTItoContinue(cuti)); //maybe not complete (t41169)
    m_ofClassUTI = cuti;
  }

  bool NodeInitDM::isDataMemberInit()
  {
    return true;
  }

  void NodeInitDM::setNodeType(UTI uti)
  {
    Node::setNodeType(uti);
    if(m_state.okUTItoContinue(uti) && m_state.isAClass(uti))
      if(m_nodeExpr) //t41180
	m_nodeExpr->setClassType(uti); //when another class
  }

  UTI NodeInitDM::checkAndLabelType()
  {
    UTI it = Nouti; //expression type

    // instantiate, look up in its class block
    if(m_constSymbol == NULL)
      checkForSymbol(); //for type and default value

    //short circuit, avoid assert
    if(m_constSymbol == NULL)
      {
	if((m_ofClassUTI == Hzy) || m_state.isStillHazy(m_ofClassUTI))
	  {
	    setNodeType(Hzy);
	    std::ostringstream msg;
	    msg << "Class type used with symbol name '" << getName() << "'";
	    msg << " is not ready to init data member";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    return Hzy;
	  }

	setNodeType(Nav);
	std::ostringstream msg;
	msg << "Invalid Class type used with symbol name '" << getName() << "'";
	msg << "; cannot init data member in ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_ofClassUTI).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	return Nav;
      }

    UTI suti = m_constSymbol->getUlamTypeIdx();
    UTI cuti = m_state.getCompileThisIdx();

    //move before m_nodeExpr "Void" check (t3883)
    if(!m_state.okUTItoContinue(suti) || !m_state.isComplete(suti))
      {
	std::ostringstream msg;
	msg << "Incomplete " << prettyNodeName().c_str() << " for type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	msg << ", used with symbol name '" << getName() << "'";
	if(m_state.okUTItoContinue(suti) || m_state.isStillHazy(suti))
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    suti = Hzy; //since not error; wait to goagain until not Nav
	  }
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }

    ULAMTYPE etyp = m_state.getUlamTypeByIndex(suti)->getUlamTypeEnum();
    if(etyp == Void)
      {
	//void only valid use is as a func return type
	std::ostringstream msg;
	msg << "Invalid use of type ";
	msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	msg << " with symbol name '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //could be clobbered by Hazy node expr
	return Nav;
      }

    setNodeType(suti); //t41169 pass along to class init expression node (scalar)

    if(m_nodeExpr)
      {
	it = m_nodeExpr->checkAndLabelType();
	if(it == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Initialization value expression for";
	    msg << " class data member: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is invalid";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	if(m_state.isStillHazy(it))
	  {
	    UTI cuti = m_state.getCompileThisIdx();
	    std::ostringstream msg;
	    msg << "Initialization value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is not ready, still hazy while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    setNodeType(Hzy);
	    m_state.setGoAgain();
	    return Hzy; //short-circuit
	  }

	assert(it != Nouti); //?

	//note: Void is flag that it's a list of constant initializers;
	// code lifted from NodeVarDecl.cpp c&l.
	if(it == Void)
	  {
	    //only possible if array type with initializers; (t41180,1)
	    if(m_state.isScalar(suti))
	      {
		std::ostringstream msg;
		msg << "Invalid initialization of scalar type ";
		msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
		msg << " with symbol name '" << getName() << "'";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav);
		return Nav;
	      }

	    if(m_state.okUTItoContinue(suti) && m_state.isComplete(suti))
	      {
		//arraysize specified, may have fewer initializers
		s32 arraysize = m_state.getArraySize(suti);
		assert(arraysize >= 0); //t3847
		u32 n = ((NodeList *) m_nodeExpr)->getNumberOfNodes();
		if((n > (u32) arraysize) && (arraysize > 0)) //not an error: t3847
		  {
		    std::ostringstream msg;
		    msg << "Too many initializers (" << n << ") specified for array '";
		    msg << getName() << "', size " << arraysize;
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		    setNodeType(Nav);
		    it = Nav;
		  }
		else if(n == 0)
		  {
		    delete m_nodeExpr;
		    m_nodeExpr = NULL; //t41206
		    it = suti; //ok to fall thru
		  }
		else
		  {
		    m_nodeExpr->setNodeType(suti);
		    it = suti;
		  }
	      }
	    else
	      {
		assert(suti != Nav);
		it = Hzy; //flag
	      }

	    if(m_state.isComplete(suti)) //reloads
	      {
		ULAMTYPE eit = m_state.getUlamTypeByIndex(it)->getUlamTypeEnum();
		ULAMTYPE esuti = m_state.getUlamTypeByIndex(suti)->getUlamTypeEnum();
		if(m_state.okUTItoContinue(it) && (eit != esuti))
		  {
		    std::ostringstream msg;
		    msg << prettyNodeName().c_str() << " '" << getName();
		    msg << "' type <" << m_state.getUlamTypeByIndex(suti)->getUlamTypeNameOnly().c_str();
		    msg << "> does not match its value type <";
		    msg << m_state.getUlamTypeByIndex(it)->getUlamTypeNameOnly().c_str() << ">";
		    msg << " while labeling class: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		    msg << " UTI" << cuti;
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		  }
	      }
	  } //end array initializers (eit == Void)

	if(m_nodeExpr && !m_nodeExpr->isAConstant())
	  {
	    std::ostringstream msg;
	    msg << "Initialization value expression for";
	    msg << " class data member: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is not a constant";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit (error) after a possible empty array init is deleted t41206
	  }

	if(m_state.okUTItoContinue(it) && m_state.okUTItoContinue(suti) && (m_state.isScalar(it) ^ m_state.isScalar(suti)))
	  {
	    std::ostringstream msg;
	    msg << "Initialization value expression for";
	    msg << " class data member: " << getName();
	    msg << ", array/scalar mismatch";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit, outside array initializers (t41181)
	  }
      } //end m_nodeExpr
    else
      {
	//only parseable as an empty list (like an array withOUT any initializers)
	// which will be removed accordingly; subsequent times around c&l should bypass
	// for arrays and classes.
	if(m_state.isScalar(suti) && !m_state.isAClass(suti))
	  {
	    std::ostringstream msg;
	    msg << "Invalid initialization of scalar type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << " with symbol name '" << getName() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav;
	  }
      }

    if(m_state.isAClass(suti) && m_state.isScalar(suti))
      {
	//test after any m_nodeExpr surgery during c&l
	if((m_nodeExpr == NULL) || !(m_nodeExpr->isClassInit() || m_nodeExpr->isAConstantClass()))
	  {
	    std::ostringstream msg;
	    msg << "Invalid initialization of class type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << " with symbol name '" << getName() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav); //t41180, t41232
	    return Nav;
	  }
      }

    if(!m_constSymbol->isInitValueReady() && m_nodeExpr)
      {
	UTI foldrtn = foldConstantExpression();
	if(foldrtn == Nav)
	  setNodeType(Nav);
	else if(foldrtn == Hzy)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete " << prettyNodeName().c_str() << " for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << ", used with symbol name '" << getName() << "', after folding";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    setNodeType(Hzy);
	  }
	else
	  {
	    if(!(m_constSymbol->isInitValueReady()) && !m_state.isAClass(foldrtn))
	      {
		std::ostringstream msg;
		msg << "Constant symbol '" << getName() << "' is not ready";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		setNodeType(Hzy);
	      }
	  }
      }
    if(getNodeType() == Hzy) m_state.setGoAgain();
    return getNodeType();
  } //checkAndLabelType

  void NodeInitDM::checkForSymbol()
  {
    assert(!m_constSymbol);

    if(!m_state.okUTItoContinue(m_ofClassUTI))
      return; //can't find DM without class UTI, wait

    if(m_state.isAnonymousClass(m_ofClassUTI))
      {
	std::ostringstream msg;
	msg << "Invalid Anonymous Class Type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_ofClassUTI).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return;
      }

    if(!m_state.isASeenClass(m_ofClassUTI))
      {
	std::ostringstream msg;
	msg << "Invalid UNSEEN Class Type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_ofClassUTI).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return;
      }

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbolByAClassOrAncestor(m_ofClassUTI, m_cid, asymptr, hazyKin)) //(e.g. t41182)
      {
	assert(asymptr);
	UTI auti = asymptr->getUlamTypeIdx();
	Token cTok(TOK_IDENTIFIER, getNodeLocation(), m_cid);
	m_constSymbol = new SymbolConstantValue(cTok, auti, m_state); //t41232
	assert(m_constSymbol);
	m_constSymbol->setHasInitValue();
	assert(!hazyKin);
      }
    else
      {
	std::ostringstream msg;
	msg << "Data Member '" << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << "' is not defined, and cannot be initialized for this instance of class ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_ofClassUTI).c_str();
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
  } //checkForSymbols

  UTI NodeInitDM::foldConstantExpression()
  {
    UTI uti = getNodeType();

    if(uti == Nav)
      return Nav;

    if(!m_state.isComplete(uti)) //not complete includes Hzy
      return Hzy; //e.g. not a constant; total word size (below) requires completeness

    assert(m_constSymbol);
    if(m_constSymbol->isInitValueReady())
      return uti;

    if(!m_state.isAClass(uti))
      return NodeConstantDef::foldConstantExpression();

    if(!m_state.isScalar(uti)) //array of classes (t41170)
      {
	if(!m_nodeExpr)
	  return uti;
	else if(((NodeListArrayInitialization *) m_nodeExpr)->foldArrayInitExpression()) //returns bool
	  return uti;
	else
	  return Nav;
      }

    //scalar classes wait until after c&l to build default value;
    //but pieces can be folded in advance
    assert(m_nodeExpr);
    if(m_nodeExpr->isClassInit())
      return ((NodeListClassInit *) m_nodeExpr)->foldConstantExpression();
    return m_nodeExpr->getNodeType(); //could be a name constant class! (t41232)
  } //foldConstantExpression

  UTI NodeInitDM::constantFold()
  {
    m_state.abortShouldntGetHere();
    return foldConstantExpression(); //t41170
  }

  bool NodeInitDM::initDataMemberConstantValue(BV8K& bvref, BV8K& bvmask)
  {
    bool rtnok = false;

    assert(m_constSymbol);

    //need updated POS for genCode after c&l
    Symbol * symptr = NULL;
    bool hazyKin = false;
    AssertBool gotIt = m_state.alreadyDefinedSymbolByAClassOrAncestor(m_ofClassUTI, m_cid, symptr, hazyKin);
    assert(gotIt);

    if(!symptr->isPosOffsetReliable())
      return false;

    u32 pos = symptr->getPosOffset();

    // check if by ancestor for base relative position (ulam-5)
    UTI dmclass = symptr->getDataMemberClass();
    u32 baserelpos = 0;
    if(m_state.getABaseClassRelativePositionInAClass(m_ofClassUTI, dmclass, baserelpos))
      pos += baserelpos; //t41183 m_str

    m_posOfDM = pos; //don't include the element adjustment (t41176)

    if(m_state.getUlamTypeByIndex(m_ofClassUTI)->getUlamClassType() == UC_ELEMENT)
      pos += ATOMFIRSTSTATEBITPOS; //t41230

    UTI nuti = m_constSymbol->getUlamTypeIdx();
    assert(UlamType::compare(nuti, getNodeType(), m_state) == UTIC_SAME);

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    u32 len = nut->getSizeofUlamType(); //t41168, t41232

    if(!SymbolWithValue::isValueAllZeros(pos, len, bvmask))
      {
	std::ostringstream msg;
	msg << "Data member '" << getName() << "'";
	msg << " initialization attempt clobbers a previous initialization value";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //compiler counts
	return false; //t3451
      }

    if(m_state.isAClass(nuti) && !m_constSymbol->isInitValueReady())
      {
	BV8K bvclass;
	bvref.CopyBV(pos, 0, len, bvclass); //uses any pre-initialized value

	//like NodeVarDeclDM buildDefaultValue for a class
	if(!m_nodeExpr)
	  {
	    rtnok = true;
	  }
	else if(!m_state.isScalar(nuti))
	  {
	    rtnok = (((NodeListArrayInitialization *) m_nodeExpr)->buildClassArrayValueInitialization(bvclass)); //at pos 0 (t41170), BUT adjusted for elements (t41263), uses and pre-init(t41179)
	  }
	else if(m_nodeExpr->isAConstantClass())
	  {
	    rtnok = m_nodeExpr->getConstantValue(bvclass); //t41234
	  }
	else
	  {
	    BV8K bvtmpmask;
	    rtnok = ((NodeListClassInit *) m_nodeExpr)->initDataMembersConstantValue(bvclass, bvtmpmask); //at pos 0, adjusted for elements! uses any pre-initialization (t41176,7,8)
	  }
	if(!rtnok)
	  return false;

	m_constSymbol->setInitValue(bvclass); //for consistency
      } //class, fall thru..

    if(!m_constSymbol->isInitValueReady())
      {
	BV8K bvtmp;
	bvref.CopyBV(pos, 0, len, bvtmp); //uses any pre-initialized value
	m_constSymbol->setInitValue(bvtmp); //for consistency (t41206)
      }

    if(len <= MAXBITSPERINT)
      {
	u32 value = 0;
	AssertBool gotVal = m_constSymbol->getInitValue(value);
	assert(gotVal);
	bvref.Write(pos, len, value);
      }
    else if(len <= MAXBITSPERLONG)
      {
	u64 value = 0;
	AssertBool gotVal = m_constSymbol->getInitValue(value);
	assert(gotVal);
	bvref.WriteLong(pos, len, value);
      }
    else
      {
	BV8K val8k;
	AssertBool gotVal = m_constSymbol->getInitValue(val8k);
	assert(gotVal);
	val8k.CopyBV(0, pos, len, bvref);
      }

    bvmask.SetBits(pos, len); //t3451, t41232
    return true; //pass on
  } //buildDataMemberConstantValue

  void NodeInitDM::fixPendingArgumentNode()
  {
    m_state.abortShouldntGetHere();
  }

  bool NodeInitDM::assignClassArgValueInStubCopy()
  {
    m_state.abortShouldntGetHere(); //??
    return false;
  }

  EvalStatus NodeInitDM::eval()
  {
    assert(m_constSymbol);
    if(m_constSymbol->isInitValueReady())
      return NORMAL;
    return evalErrorReturn();
  }

  TBOOL NodeInitDM::packBitsInOrderOfDeclaration(u32& offset)
  {
    m_state.abortNotImplementedYet();
    return TBOOL_FALSE;
  }

  void NodeInitDM::printUnresolvedVariableDataMembers()
  {
    assert(m_constSymbol);
    UTI it = m_constSymbol->getUlamTypeIdx();
    if(!m_state.isComplete(it))
      {
	std::ostringstream msg;
	msg << "Unresolved type <";
	msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	msg << "> used with init data member symbol name '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //compiler counts
      } //not complete
  } //printUnresolvedVariableDataMembers

  void NodeInitDM::printUnresolvedLocalVariables(u32 fid)
  {
    assert(m_constSymbol);
    UTI it = m_constSymbol->getUlamTypeIdx();
    if(!m_state.isComplete(it))
      {
	// e.g. error/t3298 Int(Fu.sizeof)
	std::ostringstream msg;
	msg << "Unresolved type <";
	msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	msg << "> used with local variable init data member symbol name '" << getName() << "'";
	msg << " in function: " << m_state.m_pool.getDataAsString(fid);
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //compiler counts
      } //not complete
  } //printUnresolvedLocalVariables

  bool NodeInitDM::isAConstant()
  {
    if(m_nodeExpr)
      return m_nodeExpr->isAConstant(); //t41169 (not m_constSymbol check)
    return true; //i.e. array without initial values
  }

  void NodeInitDM::genCode(File * fp, UVPass& uvpass)
  {
    UTI nuti = getNodeType();
    assert(m_constSymbol);
    assert(m_state.isComplete(nuti));
    assert(UlamType::compare(m_constSymbol->getUlamTypeIdx(), nuti, m_state) == UTIC_SAME); //sanity check
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(!nut->isScalar() && m_nodeExpr==NULL)
      return; //t41206 no change to array init values

    assert(m_nodeExpr);

    ULAMTYPE etyp = nut->getUlamTypeEnum();
    u32 len = nut->getTotalBitSize();
    TMPSTORAGE cstor = nut->getTmpStorageTypeForTmpVar();
    u32 pos = UNRELIABLEPOS; //m_constSymbol->getPosOffset();

    const bool useLocalVar = (uvpass.getPassVarNum() == 0); //use variable name on stack t41171
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    assert(!useLocalVar || cosSize==1);

    if(cosSize > 0)
      {
	//refresh 'pos' when a local variable (t41172)
	Symbol * asymptr = NULL;
	bool hazyKin = false;
	AssertBool isDef = m_state.alreadyDefinedSymbolByAClassOrAncestor(m_ofClassUTI, m_cid, asymptr, hazyKin);
	assert(isDef);
	pos = asymptr->getPosOffset();
	m_posOfDM = pos; //no adjust for elements here (t41230, t41184)
      }
    else
      {
	assert(m_posOfDM != UNRELIABLEPOS); //reliable (t41185)
	pos = m_posOfDM;
      }

    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    s32 rtnstgidx = -1;
    if(cosSize > 0)
      {
	//avoid when empty since no "self" defined within initialization scope. t41170
	rtnstgidx = loadStorageAndCurrentObjectSymbols(stgcos, cos);
	assert(stgcos && cos);
	assert(cosSize == m_state.m_currentObjSymbolsForCodeGen.size()); //??
      }
    assert(rtnstgidx <= 0 || useLocalVar);

    //when writing into an element, compensate for ATOMFIRSTSTATEBITS
    bool isVarElement = false;
    if(useLocalVar)
      {
	UTI stgcosuti = stgcos->getUlamTypeIdx();
	UlamType * stgcosut = m_state.getUlamTypeByIndex(stgcosuti);
	ULAMCLASSTYPE stgclasstype = stgcosut->getUlamClassType();
	if(stgclasstype == UC_ELEMENT)
	  isVarElement = true;
      }
    else
      {
	UTI vuti = uvpass.getPassTargetType();
	UlamType * vut = m_state.getUlamTypeByIndex(vuti);
	ULAMCLASSTYPE vclasstype = vut->getUlamClassType();
	if(vclasstype == UC_ELEMENT)
	  isVarElement = true;
      }

    if( (etyp == Class))
      {
	ULAMCLASSTYPE classtype = nut->getUlamClassType();

	//if we're building a class dm that might also have been initialized
	// read its value within its uvpass or useLocalVar (t41176)
	s32 tmpVarNum4 = m_state.getNextTmpVarNumber();

	m_state.indentUlamCode(fp);
	fp->write("const ");
	fp->write(nut->getTmpStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum4, cstor).c_str());
	fp->write(" = ");
	if(useLocalVar)
	  fp->write(cos->getMangledName().c_str()); //t41171
	else
	  fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //tmp class storage
	fp->write(".");
	fp->write(nut->readMethodForCodeGen().c_str()); //e.g. Read, ReadLong, etc
	fp->write("(");
	if(isVarElement)
	  fp->write("T::ATOM_FIRST_STATE_BIT + "); //t41175
	fp->write_decimal_unsigned(pos);
	fp->write("u, ");
	fp->write_decimal_unsigned(len);
	fp->write("u); // ");
	fp->write(m_constSymbol->getMangledName().c_str()); //comment
	GCNL;

	//recurse to its NodeListClassInit with an immediate tmp var w default value(s)
	// (like NodeVarDeclDM) -- in case of Strings.
	s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
	m_state.indentUlamCode(fp);
	fp->write(nut->getLocalStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, cstor).c_str());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum4, cstor).c_str());
	fp->write(");"); GCNL;

	UVPass uvpass2 = UVPass::makePass(tmpVarNum2, TMPBITVAL, nuti, m_state.determinePackable(nuti), m_state, 0, 0); //default class data member as immediate

	assert(m_nodeExpr);
	if(nut->isScalar())
	  {
	    m_nodeExpr->genCode(fp, uvpass2);  //updates initialized values before read (t41167)
	  }
	else
	  {
	    // we use a special genCode, here, that can handle case of String data members
	    // inefficent, each array item in turn.. (t41170)
	    ((NodeListArrayInitialization *) m_nodeExpr)->genCodeClassInitArray(fp, uvpass2);
	  }

	m_state.indentUlamCode(fp);
	if(useLocalVar)
	  fp->write(cos->getMangledName().c_str()); //t41171
	else
	  fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //tmp class storage
	fp->write(".");
	fp->write(nut->writeMethodForCodeGen().c_str()); //e.g. Write, WriteLong, etc (t41176)
	fp->write("(");
	if(isVarElement)
	  fp->write("T::ATOM_FIRST_STATE_BIT + "); //t41175
	fp->write_decimal_unsigned(pos);
	fp->write("u, ");
	fp->write_decimal_unsigned(len);
	fp->write("u, ");
	fp->write(uvpass2.getTmpVarAsString(m_state).c_str());
	if((classtype == UC_ELEMENT))// && nut->isScalar()) yep.
	  fp->write(".GetBits()"); //T into BV
	else
	  fp->write(".read()");
	fp->write("); // ");
	fp->write(m_constSymbol->getMangledName().c_str()); //comment
	GCNL;
      }
    else if(etyp == UAtom)
      {
	//i.e. we're initializing a transient; NodeVarDeclDM says we don't want to use WriteAtom.
	// and yet how is an atom ever a constant?
	m_state.abortShouldntGetHere();
      }
    else
      {
	//scalar, constant terminal, non-class primitive (32 or 64 bits), including String and arrays
	UVPass uvpass3;
	assert(m_nodeExpr); //t41206
	m_nodeExpr->genCode(fp, uvpass3);

	m_state.indentUlamCode(fp);
	if(useLocalVar)
	  fp->write(cos->getMangledName().c_str()); //t41171
	else
	  fp->write(uvpass.getTmpVarAsString(m_state).c_str()); //tmp class storage
	fp->write(".");
	fp->write(nut->writeMethodForCodeGen().c_str()); //e.g. Write, WriteLong, etc
	fp->write("(");
	if(isVarElement)
	  fp->write("T::ATOM_FIRST_STATE_BIT + "); //t41175
	fp->write_decimal_unsigned(pos);
	fp->write("u, ");
	fp->write_decimal_unsigned(len);
	fp->write("u, ");
	fp->write(uvpass3.getTmpVarAsString(m_state).c_str());
	fp->write("); // init for ");
	fp->write(m_constSymbol->getMangledName().c_str()); //comment
	GCNL;
      }
    return; //done
  } //genCode

  void NodeInitDM::genCodeConstantArrayInitialization(File * fp)
  {
    m_state.abortShouldntGetHere(); //the question!?
  } //genCodeConstantArrayInitialization

  void NodeInitDM::generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly)
  {
    m_state.abortShouldntGetHere(); //the question!?
  } //generateBuiltinConstantClassOrArrayInitializationFunction

  void NodeInitDM::cloneAndAppendNode(std::vector<Node *> & cloneVec)
  {
    //include scalars for generated comments; arrays for constructor initialization
    NodeInitDM * cloneofme = (NodeInitDM *) this->instantiate();
    assert(cloneofme);
    SymbolConstantValue * csymptr = NULL;
    AssertBool isSym = this->getSymbolPtr((Symbol *&) csymptr);
    assert(isSym);
    ((NodeInitDM *) cloneofme)->setSymbolPtr(csymptr); //another ptr to same symbol
    cloneVec.push_back(cloneofme);
  }

} //end MFM

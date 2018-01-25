#include <stdlib.h>
#include "NodeInitDM.h"
#include "NodeListArrayInitialization.h"
#include "NodeListClassInit.h"
#include "SymbolVariableDataMember.h"
#include "CompilerState.h"

namespace MFM {

  NodeInitDM::NodeInitDM(u32 dmid, Node * assignNode, UTI ofclass, CompilerState & state) : NodeConstantDef(NULL, NULL, state)
  {
    assert(assignNode);
    setConstantExpr(assignNode);
    m_cid = dmid;
    m_ofClassUTI = ofclass;
  }

NodeInitDM::NodeInitDM(const NodeInitDM& ref) : NodeConstantDef(ref), m_ofClassUTI(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_ofClassUTI)) { }

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

    if(m_nodeExpr)
      {
	fp->write(" =");
	m_nodeExpr->printPostfix(fp);
      }
    fp->write(", ");
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
    assert(m_state.okUTItoContinue(cuti));
    assert(m_state.isComplete(cuti));
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
    assert(m_nodeExpr); // REQUIRED (e.g. for class dm init)

    UTI it = Nouti; //expression type

    // instantiate, look up in its class block
    if(m_constSymbol == NULL)
      checkForSymbol(); //for type and default value

    //short circuit, avoid assert
    if(m_constSymbol == NULL)
      {
	if(m_ofClassUTI == Hzy)
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
	if(m_state.okUTItoContinue(suti) || (suti == Hzy))
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

    if(m_state.isAClass(suti) && m_state.isScalar(suti))
      {
	if(!m_nodeExpr->isClassInit())
	  {
	    std::ostringstream msg;
	    msg << "Invalid initialization of class type ";
	    msg << m_state.getUlamTypeNameBriefByIndex(suti).c_str();
	    msg << " with symbol name '" << getName() << "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav); //t41180
	    return Nav;
	  }
      }

    setNodeType(suti); //t41169 pass along to class init expression node (scalar)

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

    if(it == Hzy)
      {
	UTI cuti = m_state.getCompileThisIdx();
	std::ostringstream msg;
	msg << "Initialization value expression for: ";
	msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << ", is not ready, still hazy while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	m_state.setGoAgain();
	setNodeType(Hzy);
	return Hzy; //short-circuit
      }

    assert(it != Nouti); //?

    if(!m_nodeExpr->isAConstant())
      {
	std::ostringstream msg;
	msg << "Initialization value expression for";
	msg << " class data member: ";
	msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << ", is not a constant";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav; //short-circuit (error)
      }

    //note: Void is flag that it's a list of constant initializers;
    // code lifted from NodeVarDecl.cpp c&l.
    if(it == Void)
      {
	//only possible if array type with initializers;
	//assert(!m_state.okUTItoContinue(suti) || !m_state.isScalar(suti));
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
	    else
	      {
		m_nodeExpr->setNodeType(suti);
		it = suti;
	      }
	  }
	else
	  {
	    assert(suti != Nav);
	    it = Hzy;
	    m_state.setGoAgain();
	  }
      } //end array initializers (eit == Void)

    if(m_state.okUTItoContinue(it) && m_state.okUTItoContinue(suti) && (m_state.isScalar(it) ^ m_state.isScalar(suti)))
      {
	std::ostringstream msg;
	msg << "Initialization value expression for";
	msg << " class data member: " << getName();
	msg << ", array/scalar mismatch";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav; //short-circuit
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

	//note: Void is flag that it's a list of constant initializers.
	if((eit == Void))
	  {
	    //only possible if array type with initializers
	    //arraysize specified, may have fewer initializers
	    assert(m_state.okUTItoContinue(suti));
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
		return Nav;
	      }
	    m_nodeExpr->setNodeType(suti);
	  }
      } //end array initializers (eit == Void)

    setNodeType(suti);

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
	    m_state.setGoAgain();
	  }
	else
	  {
	    if(!(m_constSymbol->isInitValueReady()) && !m_state.isAClass(foldrtn))
	      {
		std::ostringstream msg;
		msg << "Constant symbol '" << getName() << "' is not ready";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);

		setNodeType(Hzy);
		m_state.setGoAgain();
	      }
	  }
      }
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
    if(m_state.findSymbolInAClass(m_cid, m_ofClassUTI, asymptr, hazyKin))
      {
	assert(asymptr);
	//make a clone for this dm initialization
	m_constSymbol = (SymbolVariableDataMember *) new SymbolVariableDataMember(* ((SymbolVariableDataMember *) asymptr), true); //keep type (best guess)!
	assert(m_constSymbol);
	m_constSymbol->setHasInitValue();
	assert(!hazyKin);
      }
    else
      {
	std::ostringstream msg;
	msg << "Data Member <" << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << "> is not defined, and cannot be initialized for this instance of class ";
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

    if(!m_nodeExpr)
      return Nav;

    if(!m_state.isAClass(uti))
      return NodeConstantDef::foldConstantExpression();

    if(!m_state.isScalar(uti)) //array of classes (t41170)
      {
	if(((NodeListArrayInitialization *) m_nodeExpr)->foldArrayInitExpression()) //returns bool
	  return uti;
	else
	  return Nav;
      }

    //scalar classes wait until after c&l to build default value; but pieces can be folded in advance
    return ((NodeListClassInit *) m_nodeExpr)->foldConstantExpression();
  } //foldConstantExpression

  UTI NodeInitDM::constantFold()
  {
    m_state.abortShouldntGetHere();
    return foldConstantExpression(); //t41170
  }

  bool NodeInitDM::buildDefaultValue(u32 wlen, BV8K& bvref)
  {
    bool rtnok = false;

    assert(m_constSymbol);

    //need updated POS for genCode after c&l
    Symbol * symptr = NULL;
    bool hazyKin = false;
    AssertBool gotIt = m_state.findSymbolInAClass(m_cid, m_ofClassUTI, symptr, hazyKin);
    assert(gotIt);
    u32 pos = symptr->getPosOffset();
    ((SymbolVariableDataMember *) m_constSymbol)->setPosOffset(pos); //update for consistency

    UTI nuti = m_constSymbol->getUlamTypeIdx();
    assert(UlamType::compare(nuti, getNodeType(), m_state) == UTIC_SAME);

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    u32 bitsize = nut->getTotalBitSize(); //t41168

    if(m_state.isAClass(nuti) && !m_constSymbol->isInitValueReady())
      {
	BV8K bvclass;
	//like NodeVarDeclDM buildDefaultValue for a class
	if(!m_state.isScalar(nuti))
	  {
	    rtnok = (((NodeListArrayInitialization *) m_nodeExpr)->buildClassArrayValueInitialization(bvclass)); //at pos 0 (t41170)
	  }
	else
	  {
	    if(m_state.getDefaultClassValue(nuti, bvclass)) //uses scalar uti
	      {
		rtnok = ((NodeListClassInit *) m_nodeExpr)->initDataMembersConstantValue(bvclass); //at pos 0
	      }
	  }

	if(!rtnok)
	  return false;

	m_constSymbol->setInitValue(bvclass); //for consistency
      } //class, fall thru..

    if(bitsize <= MAXBITSPERINT)
      {
	u32 value = 0;
	AssertBool gotVal = m_constSymbol->getInitValue(value);
	assert(gotVal);
	bvref.Write(pos, bitsize, value);
      }
    else if(bitsize <= MAXBITSPERLONG)
      {
	u64 value = 0;
	AssertBool gotVal = m_constSymbol->getInitValue(value);
	assert(gotVal);
	bvref.WriteLong(pos, bitsize, value);
      }
    else
      {
	BV8K val8k;
	AssertBool gotVal = m_constSymbol->getInitValue(val8k);
	assert(gotVal);
	val8k.CopyBV<8192>(0, pos, bitsize, bvref);
      }
    return true; //pass on
  } //buildDefaultValue

  void NodeInitDM::genCodeDefaultValueStringRegistrationNumber(File * fp, u32 startpos)
  {
    m_state.abortNotImplementedYet(); //???
    return; //pass on
  }

  void NodeInitDM::genCodeElementTypeIntoDataMemberDefaultValue(File * fp, u32 startpos)
  {
    m_state.abortNotImplementedYet(); //???
    return;
  }

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
    return ERROR;
  }

  void NodeInitDM::packBitsInOrderOfDeclaration(u32& offset)
  {
    m_state.abortNotImplementedYet();
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
    assert(m_nodeExpr);
    return m_nodeExpr->isAConstant(); //t41169 (not m_constSymbol check)
  }

  void NodeInitDM::genCode(File * fp, UVPass& uvpass)
  {
    UTI nuti = getNodeType();
    assert(m_constSymbol);
    assert(m_state.isComplete(nuti));
    assert(UlamType::compare(m_constSymbol->getUlamTypeIdx(), nuti, m_state) == UTIC_SAME); //sanity check
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    ULAMTYPE etyp = nut->getUlamTypeEnum();
    u32 bitsize = nut->getTotalBitSize();
    TMPSTORAGE cstor = nut->getTmpStorageTypeForTmpVar();
    u32 pos = m_constSymbol->getPosOffset();

    const bool useLocalVar = (uvpass.getPassVarNum() == 0); //use variable name on stack t41171
    u32 cosSize = m_state.m_currentObjSymbolsForCodeGen.size();
    assert(!useLocalVar || cosSize==1);

    if(cosSize > 0)
      {
	//refresh 'pos' when a local variable (t41172)
	Symbol * asymptr = NULL;
	bool hazyKin = false;
	AssertBool isDef = m_state.findSymbolInAClass(m_cid, m_ofClassUTI, asymptr, hazyKin);
	assert(isDef);
	pos = asymptr->getPosOffset();
	((SymbolVariableDataMember *) m_constSymbol)->setPosOffset(pos); //sanity
      }

    Symbol * stgcos = NULL;
    Symbol * cos = NULL;
    s32 rtnstgidx = -1;
    if(cosSize > 0)
      {
	//avoid when empty since no "self" defined within initialization scope. t41170
	rtnstgidx = loadStorageAndCurrentObjectSymbols(stgcos, cos);
	assert(stgcos && cos);
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

	m_state.indent(fp);
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
	fp->write_decimal_unsigned(bitsize);
	fp->write("u); // ");
	fp->write(m_constSymbol->getMangledName().c_str()); //comment
	GCNL;

	//recurse to its NodeListClassInit with an immediate tmp var w default value(s)
	// (like NodeVarDeclDM) -- in case of Strings.
	s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
	m_state.indent(fp);
	fp->write(nut->getLocalStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, cstor).c_str());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum4, cstor).c_str());
	fp->write(");"); GCNL;

	UVPass uvpass2 = UVPass::makePass(tmpVarNum2, cstor, nuti, m_state.determinePackable(nuti), m_state, 0, 0); //default class data member as immediate

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

	m_state.indent(fp);
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
	fp->write_decimal_unsigned(bitsize);
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
	m_nodeExpr->genCode(fp, uvpass3);

	m_state.indent(fp);
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
	fp->write_decimal_unsigned(bitsize);
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

  void NodeInitDM::generateBuiltinConstantArrayInitializationFunction(File * fp, bool declOnly)
  {
    m_state.abortShouldntGetHere(); //the question!?
  } //generateBuiltinConstantArrayInitializationFunction

  void NodeInitDM::cloneAndAppendNode(std::vector<Node *> & cloneVec)
  {
    //include scalars for generated comments; arrays for constructor initialization
    //if(!m_state.isScalar(getNodeType()))
    {
      NodeInitDM * cloneofme = (NodeInitDM *) this->instantiate();
      assert(cloneofme);
      SymbolConstantValue * csymptr = NULL;
      AssertBool isSym = this->getSymbolPtr((Symbol *&) csymptr);
      assert(isSym);
      ((NodeInitDM *) cloneofme)->setSymbolPtr(csymptr); //another ptr to same symbol
      cloneVec.push_back(cloneofme);
    }
  }

} //end MFM
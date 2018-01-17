#include <stdlib.h>
#include "NodeInitDM.h"
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

  UTI NodeInitDM::checkAndLabelType()
  {
    UTI it = Nouti; //expression type

    // instantiate, look up in its class block
    if(m_constSymbol == NULL)
      checkForSymbol(); //for type and default value

    //short circuit, avoid assert
    if(!m_constSymbol)
      {
	setNodeType(Nav);
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

    // REQUIRED (e.g. for class dm init)
    if(m_nodeExpr)
      {
	it = m_nodeExpr->checkAndLabelType();
	if(it == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for";
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
	    msg << "Constant value expression for: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", is not ready, still hazy while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    m_state.setGoAgain();
	    setNodeType(Hzy);
	    return Hzy; //short-circuit
	  }

	if(!m_nodeExpr->isAConstant())
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for";
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
	    assert(!m_state.okUTItoContinue(suti) || !m_state.isScalar(suti));

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
	    msg << "Constant value expression for";
	    msg << " class data member: ";
	    msg << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << ", array/scalar mismatch";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }
      } //end node expression

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

	//Moved: esuti == Void 	    //void only valid use is as a func return type
	// (to be more like NodeVarDecl)

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

    if(!m_constSymbol->isReady() && m_nodeExpr)
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
	    if(!(m_constSymbol->isReady() || m_constSymbol->isInitValueReady()))
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
      }
    else
      {
	std::ostringstream msg;
	msg << "Class Data Member <" << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << "> is not defined, and cannot be initialized for this instance of class ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_ofClassUTI).c_str();
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
  } //checkForSymbols

  bool NodeInitDM::buildDefaultValue(u32 wlen, BV8K& bvref)
  {
    assert(m_constSymbol);

    //need updated POS for genCode
    Symbol * symptr = NULL;
    bool hazyKin = false;
    AssertBool gotIt = m_state.findSymbolInAClass(m_cid, m_ofClassUTI, symptr, hazyKin);
    assert(gotIt);
    u32 pos = symptr->getPosOffset();
    ((SymbolVariableDataMember *) m_constSymbol)->setPosOffset(pos); //update for consistency

    UTI nuti = m_constSymbol->getUlamTypeIdx();
    assert(UlamType::compare(nuti, getNodeType(), m_state) == UTIC_SAME);

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    u32 bitsize = nut->getBitSize();

    if(bitsize <= MAXBITSPERINT)
      {
	u32 value = 0;
	AssertBool gotVal = m_constSymbol->getValue(value);
	assert(gotVal);
	bvref.Write(pos, bitsize, value);
      }
    else if(bitsize <= MAXBITSPERLONG)
      {
	u64 value = 0;
	AssertBool gotVal = m_constSymbol->getValue(value);
	assert(gotVal);
	bvref.WriteLong(pos, bitsize, value);
      }
    else
      {
	BV8K val8k;
	AssertBool gotVal = m_constSymbol->getValue(val8k);
	assert(gotVal);
	val8k.CopyBV<8192>(0, pos, bitsize, bvref);
      }

    return true; //pass on
  }

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
    assert(m_constSymbol);
    //return m_constSymbol->isReady();
    return true;
  }

  void NodeInitDM::genCode(File * fp, UVPass& uvpass)
  {
    UTI nuti = getNodeType();
    assert(m_constSymbol);
    assert(m_state.isComplete(nuti));
    assert(UlamType::compare(m_constSymbol->getUlamTypeIdx(), nuti, m_state) == UTIC_SAME); //sanity check
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    ULAMTYPE etyp = nut->getUlamTypeEnum();
    u32 pos = m_constSymbol->getPosOffset();
    u32 bitsize = nut->getBitSize();
    TMPSTORAGE cstor = nut->getTmpStorageTypeForTmpVar();

    if(!nut->isScalar())
      {
	if(m_constSymbol->isLocalsFilescopeDef() ||  m_constSymbol->isDataMember() || m_constSymbol->isClassArgument())
	  {
	    //as a "data member", locals filescope, or class arguement: initialized in no-arg constructor (non-const)
	    m_state.indent(fp);
	    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_constSymbol->getMangledName().c_str());
	    fp->write(";"); GCNL;
	  }
	else
	  {
	    //immediate use (also, non-const)
	    assert(m_nodeExpr);
	    m_nodeExpr->genCode(fp, uvpass);

	    m_state.indent(fp);
	    fp->write(nut->getLocalStorageTypeAsString().c_str()); //for C++ local vars
	    fp->write(" ");
	    fp->write(m_constSymbol->getMangledName().c_str());
	    fp->write("("); // use constructor (not equals)
	    fp->write(m_state.getTmpVarAsString(nuti, uvpass.getPassVarNum(), uvpass.getPassStorage()).c_str()); //VALUE
	    fp->write(");"); GCNL;
	    m_state.clearCurrentObjSymbolsForCodeGen();
	  }
      }
    else if(etyp == Class)
      {
	ULAMCLASSTYPE classtype = nut->getUlamClassType();

	//recurse to its NodeListClassInit with an immediate tmp var w default value
	// (like NodeVarDeclDM)
	s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
	m_state.indent(fp);
	fp->write(nut->getLocalStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, cstor).c_str());
	fp->write(";"); GCNL;

	UVPass uvpass2 = UVPass::makePass(tmpVarNum2, cstor, nuti, m_state.determinePackable(nuti), m_state, pos, 0); //default class data member as immediate
	m_nodeExpr->genCode(fp, uvpass2);  //update initialized values before read (t41167)

	m_state.indent(fp);
	fp->write(m_state.getTmpVarAsString(nuti, uvpass.getPassVarNum(), uvpass.getPassStorage()).c_str()); //immediate class storage
	fp->write(".");
	fp->write(nut->writeMethodForCodeGen().c_str()); //e.g. Write, WriteLong, etc
	fp->write("(");
	fp->write_decimal_unsigned(pos);
	fp->write("u, ");
	fp->write_decimal_unsigned(bitsize);
	fp->write("u, ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, cstor).c_str());
	if((classtype == UC_ELEMENT))// && nut->isScalar()) yep.
	  fp->write(".GetBits()"); //T into BV
	else
	  fp->write(".read()");
	fp->write(");");
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
	//scalar, constant terminal, non-class primitive (32 or 64 bits), including String
	UVPass uvpass3;
	m_nodeExpr->genCode(fp, uvpass3);

	m_state.indent(fp);
	fp->write(m_state.getTmpVarAsString(nuti, uvpass.getPassVarNum(), uvpass.getPassStorage()).c_str()); //immediate class storage
	fp->write(".");
	fp->write(nut->writeMethodForCodeGen().c_str()); //e.g. Write, WriteLong, etc
	fp->write("(");
	fp->write_decimal_unsigned(pos);
	fp->write("u, ");
	fp->write_decimal_unsigned(bitsize);
	fp->write("u, ");
	fp->write(m_state.getTmpVarAsString(nuti, uvpass3.getPassVarNum(), uvpass3.getPassStorage()).c_str());
	//fp->write(m_nodeExpr->getName()); //only works for non-strings (without uvpass3)
	fp->write(");");
	GCNL;
      }
    return; //done
  } //genCode

  void NodeInitDM::genCodeConstantArrayInitialization(File * fp)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(nut->isScalar())
      return;

    //no-arg constructor to initialize const arrays using special method
    // (based on: 'Init' + constants's mangled name)
    m_state.m_currentIndentLevel+=2;
    m_state.indent(fp);
    fp->write(",");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("(Init");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("())\n");
    m_state.m_currentIndentLevel-=2;
  } //genCodeConstantArrayInitialization

  void NodeInitDM::generateBuiltinConstantArrayInitializationFunction(File * fp, bool declOnly)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if(nut->isScalar())
      return;

    u32 len = nut->getTotalNumberOfWords();

    if(declOnly)
      {
	//unique typedef for this constant array (number of u32's)
	m_state.indent(fp);
	fp->write("typedef u32 TypeForInit");
	fp->write(m_constSymbol->getMangledName().c_str());
	fp->write("[");
	fp->write_decimal_unsigned(len);
	fp->write("];\n");

	//unique function to initialize const array "data members" in class no-arg constructor
	m_state.indent(fp);
	fp->write("TypeForInit");
	fp->write(m_constSymbol->getMangledName().c_str());
	fp->write("& Init");
	fp->write(m_constSymbol->getMangledName().c_str());
	fp->write("();"); GCNL;
	fp->write("\n");
	return;
      }

    UTI cuti = m_state.getCompileThisIdx();
    //include the mangled class::
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    //unique function to initialize const array "data members" in class no-arg constructor
    m_state.indent(fp);
    fp->write("template<class EC>\n");

    m_state.indent(fp);
    fp->write("typename ");
    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write("TypeForInit");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("& ");

    fp->write(cut->getUlamTypeMangledName().c_str());
    fp->write("<EC>::");
    fp->write("Init");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("()\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    bool isString = (nut->getUlamTypeEnum() == String);
    if(isString)
      m_constSymbol->printPostfixValueArrayStringAsComment(fp); //t3953,4

    m_state.indent(fp);
    fp->write("static ");
    fp->write("u32 ");
    fp->write("initVal[");
    fp->write_decimal_unsigned(len);
    fp->write("] = ");
    m_constSymbol->printPostfixValue(fp);
    fp->write(";"); GCNL;

    // Note: Cannot initialize constants like data members in default class
    // (see CS::genCodeClassDefaultConstantArray);
    // Registration Number not yet available. (t3953)

    m_state.indent(fp);
    fp->write("return initVal;"); GCNL;

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("} //Init");
    fp->write(m_constSymbol->getMangledName().c_str()); GCNL;
    fp->write("\n");
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

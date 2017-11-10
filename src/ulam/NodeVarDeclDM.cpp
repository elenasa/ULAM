#include <stdlib.h>
#include "NodeVarDeclDM.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"
#include "NodeTerminal.h"
#include "MapDataMemberDesc.h"

namespace MFM {

  NodeVarDeclDM::NodeVarDeclDM(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeVarDecl(sym, nodetype, state) { }

  NodeVarDeclDM::NodeVarDeclDM(const NodeVarDeclDM& ref) : NodeVarDecl(ref) { }

  NodeVarDeclDM::~NodeVarDeclDM(){ }

  Node * NodeVarDeclDM::instantiate()
  {
    return new NodeVarDeclDM(*this);
  }

  void NodeVarDeclDM::updateLineage(NNO pno)
  {
    NodeVarDecl::updateLineage(pno);
  } //updateLineage

  bool NodeVarDeclDM::findNodeNo(NNO n, Node *& foundNode)
  {
    if(NodeVarDecl::findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  // see also SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarDeclDM::printPostfix(File * fp)
  {
    NodeVarDecl::printTypeAndName(fp);

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    if((nuti == Nav) || (nuti == Nouti))
      {
	fp->write("(");
	fp->write("unknown");
	fp->write("); ");
	return;
      }

    if(nuti == Hzy)
      {
	fp->write("(");
	fp->write("notready");
	fp->write("); ");
	return;
      }

    ULAMTYPE etyp = nut->getUlamTypeEnum();
    if(etyp == Class) //t3717, t3718, t3719, t3739, t3714, t3715, t3735
      {
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
	assert(isDefined);
	NodeBlockClass * classblock = csym->getClassBlockNode();
	assert(classblock);

	s32 arraysize = nut->getArraySize();
	//scalar has 'size=1'; empty array [0] is still '0'.
	s32 size = (arraysize > NONARRAYSIZE ? arraysize : 1);

	m_state.pushClassContext(csym->getUlamTypeIdx(), classblock, classblock, false, NULL);

	for(s32 i = 0; i < size; i++)
	  {
	    if(i > 0)
	      fp->write("), (");
	    else
	      fp->write("(");
	    classblock->printPostfixDataMembersParseTree(fp, csym->getUlamTypeIdx()); //same default values
	  }
	m_state.popClassContext();
      }
    else
      {
	fp->write("(");

	if(m_nodeInitExpr)
	  {
	    // only for primitive scalars and arrays
	    m_nodeInitExpr->printPostfix(fp);
	  }
	else if(etyp == String)
	  {
	    fp->write("UNINITIALIZED_STRING"); //t3987
	  }
	else
	  {
	    //default (uninitialized) values
	    char dstr[40];

	    nut->getDataAsString(0, dstr, 'z');
	    fp->write(dstr);

	    if(!m_state.isScalar(nuti))
	      {
		s32 arraysize = m_state.getArraySize(nuti);
		for(s32 i = 1; i < arraysize; i++)
		  {
		    nut->getDataAsString(0, dstr, ',');
		    fp->write(dstr);
		  }
	      }
	  }
      }
    fp->write(")");
    fp->write("; ");
  } //printPostfix

  void NodeVarDeclDM::noteTypeAndName(s32 totalsize, u32& accumsize)
  {
    UTI nuti = getNodeType();
    UlamKeyTypeSignature vkey = m_state.getUlamKeyTypeSignatureByIndex(nuti);
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nsize = nut->getTotalBitSize();

    std::ostringstream note;
    note << "(" << nsize << " of ";
    note << totalsize << " bits, at " << accumsize << ") ";

    //like NodeVarDecl::printNameAndType
    if(nut->getUlamTypeEnum() != Class)
      note << vkey.getUlamKeyTypeSignatureNameAndBitSize(&m_state).c_str();
    else
      note << nut->getUlamTypeNameBrief().c_str();

    note << " " << getName();

    s32 arraysize = nut->getArraySize();
    if(arraysize > NONARRAYSIZE)
      {
	note << "[" << arraysize << "]";
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	note << "[UNKNOWN]";
      }
    MSG(getNodeLocationAsString().c_str(), note.str().c_str(), NOTE);
    accumsize += nsize;

  } //noteTypeAndName

  const char * NodeVarDeclDM::getName()
  {
    return NodeVarDecl::getName();
  }

  const std::string NodeVarDeclDM::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeVarDeclDM::hasASymbolDataMember()
  {
    return true;
  }

  FORECAST NodeVarDeclDM::safeToCastTo(UTI newType)
  {
    UTI nuti = getNodeType();
    //cast RHS if necessary and safe
    //insure constant value fits in its declared type
    FORECAST rscr = m_nodeInitExpr->safeToCastTo(nuti);
    if(rscr != CAST_CLEAR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for data member (";
	msg << getName() << " = " << m_nodeInitExpr->getName();
	msg << ") initialization is not representable as ";
	msg<< m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	if(rscr == CAST_BAD)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    m_state.setGoAgain(); //since not error
	  }
      }
    else if(m_nodeInitExpr->isExplicitReferenceCast())
      {
	std::ostringstream msg;
	msg << "Explicit Reference cast for data member '";
	msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "' initialization is invalid";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rscr = CAST_BAD;
      }
    else
      {
	AssertBool isDefined = Node::makeCastingNode(m_nodeInitExpr, nuti, m_nodeInitExpr); //we know it's safe!
	assert(isDefined);
      }
    return rscr;
  } //safeToCastTo

  bool NodeVarDeclDM::checkReferenceCompatibility(UTI uti)
  {
    assert(m_state.okUTItoContinue(uti));
    if(m_state.getUlamTypeByIndex(uti)->isReference())
      {
	std::ostringstream msg;
	msg << "Data member '";
	msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "', is a reference";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }
    return true; //ok
  } //checkReferenceCompatibility

  UTI NodeVarDeclDM::checkAndLabelType()
  {
    UTI nuti = NodeVarDecl::checkAndLabelType(); //sets node type

    if(!m_state.okUTItoContinue(nuti))
      return nuti;

    UTI cuti = m_state.getCompileThisIdx();

    //don't allow unions to initialize its data members (t3782)
    if(m_state.isClassAQuarkUnion(cuti) && m_nodeInitExpr)
      {
	std::ostringstream msg;
	msg << "Data member '";
	msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "' belongs to a quark-union, and cannot be initialized";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav; //short-circuit
      }

    //don't allow unions to contain string data members (t41093)
    if(m_state.isClassAQuarkUnion(cuti) && UlamType::compareForString(nuti, m_state) == UTIC_SAME)
      {
	std::ostringstream msg;
	msg << "Data member '";
	msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "' belongs to a quark-union, and cannot be type String";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav; //short-circuit
      }

    //don't allow a subclass to shadow a superclass datamember
    UTI superuti = m_state.isClassASubclass(cuti);
    if(superuti != Nouti) //has ancestor
      {
	//is a subclass' DM..
	//check for shadowed superclass DM of same name
	NodeBlockClass * superclassblock = m_state.getAClassBlock(superuti);
	assert(superclassblock);
	m_state.pushClassContextUsingMemberClassBlock(superclassblock);

	Symbol * varSymbolOfSameName = NULL;
	bool hazyKin = false;
	if(m_state.alreadyDefinedSymbol(m_vid, varSymbolOfSameName, hazyKin))
	  {
	    std::ostringstream msg;
	    msg << "Data member '";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << "' is shadowing an ancestor";
	    if(hazyKin)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		setNodeType(Hzy);
		m_state.setGoAgain();
		m_state.popClassContext();
		return Hzy; //short-circuit
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav);
		m_state.popClassContext();
		return Nav; //short-circuit
	      }
	  }
	m_state.popClassContext();
      } //end subclass error checking

    //NodeVarDecl handles array initialization for both locals & dm
    // since initial expressions must be constant for both (unlike local scalars)
    if(m_nodeInitExpr && m_state.isScalar(getNodeType()))
      {
	if(!m_nodeInitExpr->isAConstant())
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for data member: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is not a constant";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	UTI it = m_nodeInitExpr->getNodeType();
	if(it == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for data member: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is invalid";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	if(it == Hzy)
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for data member: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is not ready";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    m_state.setGoAgain(); //since not error
	    setNodeType(Hzy);
	    return Hzy; //short-circuit
	  }

	//constant fold if possible, set symbol value
	assert(m_varSymbol);

	if(m_varSymbol->hasInitValue())
	  {
	    if(!(m_varSymbol->isInitValueReady()))
	      {
		if(!foldArrayInitExpression()) //sets init constant value
		  {
		    if((getNodeType() == Nav) || m_nodeInitExpr->getNodeType() == Nav)
		      return Nav;

		    if(!(m_varSymbol->isInitValueReady()))
		      {
			setNodeType(Hzy);
			m_state.setGoAgain(); //since not error
			return Hzy;
		      }
		  }
	      }
	  }

	//insure constant value fits in its declared type,
	//done in NodeVarDecl c&l: safeToCastTo(nuti)
      } //finished init expr node

    if(m_state.isComplete(getNodeType()))
      {
	if(!checkDataMemberSizeConstraints())
	  setNodeType(Nav); //err msgs, compiler counts;
      }

    return getNodeType();
  } //checkAndLabelType

  bool NodeVarDeclDM::checkDataMemberSizeConstraints()
  {
    bool rtnb = true;
    UTI it = m_varSymbol->getUlamTypeIdx();
    assert(m_state.isComplete(it)); //moved error check to separate pass
    assert(getNodeType() == it);
    UlamType * ut = m_state.getUlamTypeByIndex(it);
    ULAMCLASSTYPE dmclasstype = ut->getUlamClassType();
    u32 len = ut->getTotalBitSize();

    UTI cuti = m_state.getCompileThisIdx();
    ULAMCLASSTYPE thisclasstype = m_state.getUlamTypeByIndex(cuti)->getUlamClassType();
    if(thisclasstype != UC_TRANSIENT)
      {
	// this datamember belongs to an element or quark (e.g. t3190)
	// allows BIG numeric arrays, and BIG Bits and Bools Wed May 11 09:16:34 2016
	if((len > MAXBITSPERLONG) && (dmclasstype == UC_NOTACLASS) && ut->isScalar() && ut->isNumericType()) //BitField constraint
	  {
	    std::ostringstream msg;
	    msg << "Data member <" << getName() << "> of numeric scalar type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", total size: " << (s32) m_state.getTotalBitSize(it);
	    msg << " MUST fit into " << MAXBITSPERLONG << " bits;";
	    msg << " Local variables do not have this restriction";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnb = false;
	  }

	if(dmclasstype == UC_ELEMENT)
	  {
	    std::ostringstream msg;
	    msg << "Data member <" << getName() << "> of type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", is an element, and is NOT permitted; Local variables, quarks, ";
	    msg << "and Model Parameters do not have this restriction";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnb = false;
	  }

	if(dmclasstype == UC_QUARK)
	  {
	    if(!ut->isScalar() && (len > MAXSTATEBITS)) //was MAXBITSPERLONG
	      {
		std::ostringstream msg;
		msg << "Data member <" << getName() << "> of class array type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << ", total size: " << (s32) len;
		msg << " MUST fit into " << MAXSTATEBITS << " bits;";
		msg << "Local variables do not have this restriction";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		rtnb = false;
	      }
	  }

	if(dmclasstype == UC_TRANSIENT)
	  {
	    std::ostringstream msg;
	    msg << "Data member <" << getName() << "> of type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", is a transient, and is NOT permitted; Local variables, ";
	    msg << "do not have this restriction";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnb = false;
	  }
      } //not transient

    //this check is valid regardless of where quark resides
    if(dmclasstype == UC_QUARK)
      {
	if(ut->isScalar() && (len > MAXBITSPERINT))
	  {
	    std::ostringstream msg;
	    msg << "Data member <" << getName() << "> of class type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", total size: " << (s32) len;
	    msg << " MUST fit into " << MAXBITSPERINT << " bits";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnb = false;
	  }
      } //quarks
    return rtnb;
  } //checkDataMemberSizeConstraints

  void NodeVarDeclDM::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    NodeVarDecl::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  void NodeVarDeclDM::setInitExpr(Node * node)
  {
    NodeVarDecl::setInitExpr(node);
    if(m_varSymbol)
      m_varSymbol->setHasInitValue();
  }

  //from NodeConstantDef; applied here to init value.
  // called during parsing rhs of named constant;
  // Requires a constant expression, else error;
  // (SCOPE OF EVAL IS BASED ON THE BLOCK OF CONSTDEF.)
  bool NodeVarDeclDM::foldArrayInitExpression()
  {
    UTI nuti = getNodeType();

    if(nuti == Nav || !m_state.isComplete(nuti))
      return false; //e.g. not a constant

    assert(m_varSymbol);
    if(m_varSymbol->isInitValueReady())
      return true; //short-circuit

    if(!m_state.isScalar(nuti)) //arrays handled by NodeVarDecl (virtual)
      return NodeVarDecl::foldArrayInitExpression();

    assert(m_nodeInitExpr);
    // if here, must be a constant init value..
    UTI foldeduti = m_nodeInitExpr->constantFold(); //c&l redone
    if(!m_state.okUTItoContinue(foldeduti))
      return false;

    u64 newconst = 0; //UlamType format (not sign extended)

    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(nuti); //offset a constant expression
    EvalStatus evs = m_nodeInitExpr->eval();
    if(evs == NORMAL)
      {
	UlamValue cnstUV = m_state.m_nodeEvalStack.popArg();
	u32 wordsize = m_state.getTotalWordSize(nuti);
	if(wordsize <= MAXBITSPERINT)
	  newconst = cnstUV.getImmediateData(m_state);
	else if(wordsize <= MAXBITSPERLONG)
	  newconst = cnstUV.getImmediateDataLong(m_state);
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }

    evalNodeEpilog();

    if(evs == ERROR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for data member '";
	msg << m_state.m_pool.getDataAsString(m_varSymbol->getId()).c_str();
	msg << "' initialization failed while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return false;
      }

    if(evs == NOTREADY)
      {
	std::ostringstream msg;
	msg << "Constant value expression for data member '";
	msg << m_state.m_pool.getDataAsString(m_varSymbol->getId()).c_str();
	msg << "' initialization is not yet ready while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	setNodeType(Hzy);
	m_state.setGoAgain(); //since not error
	return false;
      }

    //insure constant value fits in its declared type
    FORECAST scr = m_nodeInitExpr->safeToCastTo(nuti);
    if(scr != CAST_CLEAR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for data member (";
	msg << getName() << " = " << m_nodeInitExpr->getName() ;
	msg << ") initialization is not representable as ";
	msg<< m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	if(scr == CAST_BAD)
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	  }
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    setNodeType(Hzy);
	    m_state.setGoAgain(); //since not error
	  }
	return false; //necessary if not just a warning.
      }

    if(updateConstant(newconst))
      {
	BV8K bvtmp;
	u32 len = m_state.getTotalBitSize(nuti);
	bvtmp.WriteLong(0u, len, newconst); //is newconst packed?
	m_varSymbol->setInitValue(bvtmp); //isReady now!
	return true;
      }
    return false;
  } //foldArrayInitExpression

  bool NodeVarDeclDM::updateConstant(u64 & newconst)
  {
    if(!m_varSymbol)
      return false;

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return false;
    else if(!m_state.isComplete(nuti))
      {
	m_state.setGoAgain(); //since not error
	return false;
      }

    //store in UlamType format
    bool rtnb = true;
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    AssertBool isKnownSize = (nut->getBitSize() > 0);
    assert(isKnownSize);
    u32 wordsize = nut->getTotalWordSize();
    if(wordsize <= MAXBITSPERINT)
      rtnb = updateConstant32(newconst);
    else if(wordsize <= MAXBITSPERLONG)
      rtnb = updateConstant64(newconst);
    else
      m_state.abortGreaterThanMaxBitsPerLong();

    if(!rtnb)
      {
	std::ostringstream msg;
	msg << "Constant Type Unknown: ";
	msg <<  m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return rtnb;
  } //updateConstant

  bool NodeVarDeclDM::updateConstant32(u64 & newconst)
  {
    u64 val = newconst; //wo any sign extend to start
    //store in UlamType format
    bool rtnb = true;
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());
    s32 nbitsize = nut->getBitSize();
    u32 srcbitsize = m_nodeInitExpr ? m_state.getBitSize(m_nodeInitExpr->getNodeType()) : nbitsize; //was MAXBITSPERINT WRONG!

    ULAMTYPE etyp = nut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	newconst = _Int32ToInt32((u32) val, srcbitsize, nbitsize); //signextended
	break;
      case Unsigned:
	newconst = _Unsigned32ToUnsigned32((u32) val, srcbitsize, nbitsize);
	break;
      case Bool:
	//newconst = _Unsigned32ToBool32(val, MAXBITSPERINT, nbitsize);
	newconst = _CboolToBool32( (bool) val, nbitsize);
	break;
      case Unary:
	newconst =  _Unsigned32ToUnary32((u32) val, srcbitsize, nbitsize);
	break;
      case Bits:
	newconst = _Unsigned32ToBits32((u32) val, srcbitsize, nbitsize);
	break;
      case String:
	break;
      default:
	rtnb = false;
      };
    return rtnb;
  } //updateConstant32

  bool NodeVarDeclDM::updateConstant64(u64 & newconst)
  {
    u64 val = newconst; //wo any sign extend to start
    //store in UlamType format
    bool rtnb = true;
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());
    s32 nbitsize = nut->getBitSize();
    u32 srcbitsize = m_nodeInitExpr ? m_state.getBitSize(m_nodeInitExpr->getNodeType()) : nbitsize; //was MAXBITSPERINT WRONG!
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	newconst = _Int64ToInt64(val, srcbitsize, nbitsize); //signextended
	break;
      case Unsigned:
	newconst = _Unsigned64ToUnsigned64(val, srcbitsize, nbitsize);
	break;
      case Bool:
	//newconst = _Unsigned64ToBool64(val, MAXBITSPERLONG, nbitsize);
	newconst = _CboolToBool64( (bool) val, nbitsize);
	break;
      case Unary:
	newconst =  _Unsigned64ToUnary64(val, srcbitsize, nbitsize);
	break;
      case Bits:
	newconst = _Unsigned64ToBits64(val, srcbitsize, nbitsize);
	break;
      case String:
	break;
      default:
	  rtnb = false;
      };
    return rtnb;
  } //updateConstant64

  bool NodeVarDeclDM::buildDefaultValue(u32 wlen, BV8K& dvref)
  {
    assert(m_varSymbol);
    assert(m_varSymbol->isDataMember());

    bool aok = false; //init as not ready
    UTI nuti = getNodeType(); //same as symbol uti, unless prior error
    assert(nuti == m_varSymbol->getUlamTypeIdx());

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    UTI cuti = m_state.getCompileThisIdx();
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    u32 pos = m_varSymbol->getPosOffset();
    s32 bitsize = nut->getBitSize();

    if(cut->getUlamClassType() == UC_ELEMENT)
      pos += ATOMFIRSTSTATEBITPOS; //atom-based (TypeField determined at runtime)

    if(nut->getUlamClassType() == UC_ELEMENT)
      bitsize = BITSPERATOM; //atom-based data member

    ULAMTYPE etyp = nut->getUlamTypeEnum();
    if(etyp == Class) //thisClass contains a different class
      {
	u32 dmwlen = nut->getTotalWordSize();
	if(dmwlen > 0)
	  {
	    assert(dmwlen <= wlen);
	    BV8K dmdv; //copies default BV

	    if(m_state.getDefaultClassValue(nuti, dmdv))
	      {
		s32 arraysize = nut->getArraySize();
		arraysize = ((arraysize == NONARRAYSIZE) ? 1 : arraysize); //could be 0
		//updates dvref in place at position 'pos' in dvref
		//from position 0 in dmdv (a copy)
		m_state.getDefaultAsArray(bitsize, arraysize, pos, dmdv, dvref); //both scalar and arrays
		aok = true;
	      }
	  }

	if(aok)
	  foldDefaultClass(); //init value for m_varSymbol
      }
    else if(m_nodeInitExpr)
      {
	//primitive (not a class!)
	//arrays may be initialized now
	//isn't this redundant??? Mon Dec 19 11:33:26 2016
	if(m_state.isScalar(nuti))
	  {
	    u64 val = 0;
	    if(m_varSymbol->getInitValue(val))
	      {
		s32 classsize = m_state.getBitSize(m_state.getCompileThisIdx());
		u64 packedval = 0;
		m_state.getDefaultAsPackedArray(classsize, bitsize, 1, pos, (u64) val, packedval);
		dvref.WriteLong(pos, bitsize, packedval);
		aok = true;
	      }
	  }
	else
	  {
	    assert(m_varSymbol->hasInitValue());
	    BV8K dval; //copies default BV
	    if(m_varSymbol->getInitValue(dval))
	      {
		u32 len = nut->getTotalBitSize();
		dval.CopyBV(0u, pos, len, dvref); //frompos, topos, len, destBV
		aok = true;
	      }
	  }
      }
    else
      aok = true; //no initialized value

    return aok;
  } //buildDefaultValue

  void NodeVarDeclDM::genCodeDefaultValueStringRegistrationNumber(File * fp, u32 startpos)
  {
    assert(m_varSymbol);
    assert(m_varSymbol->isDataMember());

    UTI nuti = getNodeType(); //same as symbol uti, unless prior error
    assert(nuti == m_varSymbol->getUlamTypeIdx());
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    u32 bits = nut->getBitSize();
    if(bits == 0)
      return;

    ULAMTYPE etyp = nut->getUlamTypeEnum();
    u32 pos = m_varSymbol->getPosOffset();
    u32 arraysize = nut->isScalar() ? 1 : nut->getArraySize();

    if(etyp == String)
      {
	//generate code to replace uti in string index with runtime registration number
	for(u32 i = 0; i < arraysize; i++)
	  {
	    m_state.indent(fp);
	    fp->write("initBV.Write(");
	    fp->write_decimal_unsigned(pos + startpos);
	    fp->write("u + ");
	    fp->write_decimal_unsigned(i * MAXBITSPERINT);
	    fp->write("u, ");
	    fp->write_decimal_unsigned(REGNUMBITS);
	    fp->write("u, myRegNum); //");
	    fp->write(m_varSymbol->getMangledName().c_str()); //comment
	    GCNL;
	  }
      }
    else if(etyp == Class)
      {
	ULAMCLASSTYPE classtype = nut->getUlamClassType();

	if(classtype == UC_ELEMENT)
	  bits = BITSPERATOM;

	u32 totbitsize = bits * arraysize;

	s32 tmpVarNum = m_state.getNextTmpVarNumber();
	TMPSTORAGE cstor = nut->getTmpStorageTypeForTmpVar();

	m_state.indent(fp);
	fp->write("const ");
	fp->write(nut->getLocalStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, cstor).c_str());
	fp->write(";"); GCNL;

	s32 tmpVarNum2 = m_state.getNextTmpVarNumber();
	m_state.indent(fp);
	fp->write("const ");
	fp->write(nut->getTmpStorageTypeAsString().c_str());
	fp->write(" ");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, cstor).c_str());
	fp->write("(");
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, cstor).c_str());
	fp->write(".read());"); GCNL;

	m_state.indent(fp);
	fp->write("initBV.");
	if((classtype == UC_ELEMENT) && nut->isScalar())
	  fp->write("WriteBV"); //t3968 don't want WriteAtom
	else
	  fp->write(nut->writeMethodForCodeGen().c_str());//Write(");
	fp->write("(");
	fp->write_decimal_unsigned(pos + startpos);
	fp->write("u, ");
	if((classtype == UC_QUARK) && (totbitsize <= BITSPERATOM))
	  {
	   fp->write_decimal_unsigned(totbitsize); //entire array (t3776, t3969)
	   fp->write(", ");
	  }
	fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum2, cstor).c_str());
	if((classtype == UC_ELEMENT) && nut->isScalar())
	  fp->write(".GetBits()"); //T into BV
	fp->write("); //");
	fp->write(m_varSymbol->getMangledName().c_str()); //comment
	GCNL;
      } //a class
  } //genCodeDefaultValueStringRegistrationNumber

  void NodeVarDeclDM::genCodeElementTypeIntoDataMemberDefaultValue(File * fp, u32 startpos)
  {
    assert(m_varSymbol);
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE nclasstype = nut->getUlamClassType();
    if(nclasstype == UC_ELEMENT)
      {
	s32 arraysize = nut->getArraySize();
	arraysize = ((arraysize == NONARRAYSIZE) ? 1 : arraysize); //allow zero

	m_state.indent(fp);
	fp->write("{\n"); //limit scope of 'dam'
	m_state.m_currentIndentLevel++;

	m_state.indent(fp);
	fp->write("AtomBitStorage<EC> gda(");
	fp->write(m_state.getTheInstanceMangledNameByIndex(nuti).c_str());
	fp->write(".GetDefaultAtom());"); GCNL;

	m_state.indent(fp);
	fp->write("u32 typefield = gda.Read(0u, T::ATOM_FIRST_STATE_BIT);"); GCNL; //can't use GetType");

	for(s32 i = 0; i < arraysize; i++) //e.g. t3714 (array of element dm); t3735
	  {
	    m_state.indent(fp);
	    fp->write("initBV.Write(");
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset() + startpos);
	    fp->write("u + ");
	    fp->write_decimal_unsigned(i * BITSPERATOM);
	    fp->write("u, T::ATOM_FIRST_STATE_BIT, typefield);"); GCNL;
	  }

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("}\n");
      }
    else if(nclasstype == UC_TRANSIENT)
      {
	s32 arraysize = nut->getArraySize();
	arraysize = (arraysize <= 0 ? 1 : arraysize);

	u32 len = nut->getBitSize(); //item
	//any transient data members that may have element data members
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(nuti, csym);
	assert(isDefined);

	NodeBlockClass * cblock = csym->getClassBlockNode();
	assert(cblock);

	for(s32 i = 0; i < arraysize; i++)
	  cblock->genCodeElementTypeIntoDataMemberDefaultValue(fp, m_varSymbol->getPosOffset() + startpos + i * len); //e.g. t3715
      }
    else if(m_state.isAtom(nuti))
      {
	s32 arraysize = nut->getArraySize();
	arraysize = (arraysize <= 0 ? 1 : arraysize);

	m_state.indent(fp);
	fp->write("{\n"); //limit scope of 'dam'
	m_state.m_currentIndentLevel++;

	m_state.indent(fp);
	fp->write("AtomBitStorage<EC> gda(");
	fp->write("Element_Empty<EC>::THE_INSTANCE.GetDefaultAtom());"); GCNL;

	m_state.indent(fp);
	fp->write("u32 typefield = gda.Read(0u, T::ATOM_FIRST_STATE_BIT);"); GCNL; //can't use GetType");

	for(s32 i = 0; i < arraysize; i++)
	  {
	    m_state.indent(fp);
	    fp->write("initBV.Write(");
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset() + startpos);
	    fp->write("u + ");
	    fp->write_decimal_unsigned(i * BITSPERATOM);
	    fp->write("u, T::ATOM_FIRST_STATE_BIT, typefield);"); GCNL;
	  }

	m_state.m_currentIndentLevel--;
	m_state.indent(fp);
	fp->write("}\n");
      }
  } //genCodeElementTypeIntoDataMemberDefaultValue

  void NodeVarDeclDM::foldDefaultClass()
  {
    if(m_varSymbol->isInitValueReady())
      return;

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    assert(m_state.okUTItoContinue(nuti));
    assert(m_state.isComplete(nuti));

    u32 bitsize = nut->getBitSize();
    s32 arraysize = nut->getArraySize();

    BV8K bvtmp;
    AssertBool gotDefault = m_state.getDefaultClassValue(nuti, bvtmp);
    assert(gotDefault); //maybe zeros

    BV8K bvarr;
    arraysize = arraysize > 0 ? arraysize : 1;
    m_state.getDefaultAsArray(bitsize, arraysize, 0u, bvtmp, bvarr);

    //(in this order) i thought this was for primitives only?
    m_varSymbol->setHasInitValue();
    m_varSymbol->setInitValue(bvarr); //t3512
  } //foldDefaultClass

  void NodeVarDeclDM::packBitsInOrderOfDeclaration(u32& offset)
  {
    assert((s32) offset >= 0); //neg is invalid
    assert(m_varSymbol);

    ((SymbolVariableDataMember *) m_varSymbol)->setPosOffset(offset);

    UTI it = m_varSymbol->getUlamTypeIdx();
    assert(m_state.isComplete(it)); //moved error check to separate pass
    assert(it == getNodeType()); //same as symbol, or shouldn't be here!

    UlamType * ut = m_state.getUlamTypeByIndex(it);
    u32 len = ut->getSizeofUlamType(); //ut->getTotalBitSize();
    offset += len; //uses atom-based size for element, and actual size for quark data members
  } //packBitsInOrderOfDeclaration

  void NodeVarDeclDM::printUnresolvedVariableDataMembers()
  {
    assert(m_varSymbol);
    UTI it = m_varSymbol->getUlamTypeIdx();
    if(!m_state.isComplete(it))
      {
	// e.g. error/t3298 Int(Fu.sizeof)
	std::ostringstream msg;
	msg << "Unresolved type <";
	msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	msg << "> used with variable symbol name '" << getName() << "'";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav); //compiler counts
      } //not complete
  } //printUnresolvedVariableDataMembers

  void NodeVarDeclDM::printUnresolvedLocalVariables()
  {
    m_state.abortShouldntGetHere(); //override
  }

  EvalStatus NodeVarDeclDM::eval()
  {
    assert(m_varSymbol);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    assert(m_varSymbol->getAutoLocalType() == ALT_NOT);

    if(m_state.isAtom(nuti))
      return NodeVarDecl::eval();

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    if((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() > MAXSTATEBITS))
      return UNEVALUABLE;

    // packedloadable class (e.g. quark) or nonclass data member;
    if(m_nodeInitExpr && m_varSymbol->hasInitValue())
      {
	return NodeVarDecl::evalInitExpr();
      }
    //else no side-effects needed..overrides NodeVarDecl.
    return NORMAL;
  } //eval

  EvalStatus NodeVarDeclDM::evalToStoreInto()
  {
    //called via NodeVarDecl eval (e.g. t3187)
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    if((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() > MAXSTATEBITS))
      return UNEVALUABLE;

    evalNodeProlog(0); //new current node eval frame pointer

    // (from NodeIdent's makeUlamValuePtr)
    // return ptr to this data member within the m_currentObjPtr
    // 'pos' modified by this data member symbol's packed bit position
    UlamValue rtnUVPtr = makeUlamValuePtr();

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  UlamValue NodeVarDeclDM::makeUlamValuePtr()
  {
    UlamValue ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());

    ptr.checkForAbsolutePtr(m_state.m_currentObjPtr);
    return ptr;
  } //makeUlamValuePtr

  // parse tree in order declared, unlike the ST.
  void NodeVarDeclDM::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_varSymbol);
    assert(m_state.isComplete(getNodeType()));

    assert(m_varSymbol->isDataMember());

    return genCodedBitFieldTypedef(fp, uvpass);
  } //genCode

  // variable is a data member; cannot be an element (unless a transient!)
  void NodeVarDeclDM::genCodedBitFieldTypedef(File * fp, UVPass& uvpass)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE netyp = nut->getUlamTypeEnum();

    m_state.indent(fp);
    if((netyp == Class) && nut->isScalar())
      {
	// usse typedef rather than atomic parameter for classes
	// (e.g. quarks within elements, element within transients,
	//       transients within transients, etc).
	// except if an array of quarks.
	fp->write("typedef ");
	fp->write(nut->getUlamTypeMangledName().c_str()); //for C++
	fp->write("<EC> ");
	//fp->write(m_varSymbol->getMangledNameForParameterType().c_str());
	fp->write(m_varSymbol->getMangledName().c_str()); //t41141
	fp->write("; //offset ");
	fp->write_decimal_unsigned(m_varSymbol->getPosOffset());
	fp->write("u"); GCNL; //func call parameters aren't NodeVarDecl's
      }
    else
      {
	//arrays and non-class data members
	fp->write("typedef UlamRefFixed");
	fp->write("<EC, "); //BITSPERATOM

	if(nut->getUlamClassType() == UC_ELEMENT) //t3714, t3779 Mob.h Up_Um_2sp
	  {
	    //elements only data members in transients
	    assert(m_state.getUlamClassForThisClass() == UC_TRANSIENT);
	    s32 arraysize = nut->getArraySize();
	    arraysize = ((arraysize == NONARRAYSIZE) ? 1 : arraysize); //Mon Jul  4 14:11:41 2016
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset());
	    if(nut->getBitSize() > 0)
	      fp->write("u, BPA * "); //atom-based
	    else
	      fp->write("u, 0u * "); //empty atom //t3400
	    fp->write_decimal(arraysize); //include arraysize
	    fp->write("u> ");
	  }
	else
	  {
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset());
	    fp->write("u, ");
	    fp->write_decimal(nut->getTotalBitSize()); //include arraysize
	    fp->write("u> ");
	  }
	//fp->write(m_varSymbol->getMangledNameForParameterType().c_str());
	fp->write(m_varSymbol->getMangledName().c_str());
	fp->write(";"); GCNL; //func call parameters aren't NodeVarDecl's
      }
  } //genCodedBitFieldTypedef

  void NodeVarDeclDM::genCodeConstantArrayInitialization(File * fp)
  { }

  void NodeVarDeclDM::generateBuiltinConstantArrayInitializationFunction(File * fp, bool declOnly)
  { }

  void NodeVarDeclDM::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    UTI nuti = getNodeType();

    //output a case of switch statement
    m_state.indent(fp);
    fp->write("case ");
    fp->write_decimal(dmcount);
    fp->write(": { static UlamClassDataMemberInfo i(\"");
    fp->write(m_state.getUlamTypeByIndex(nuti)->getUlamTypeMangledName().c_str());
    fp->write("\", \"");
    fp->write(m_state.m_pool.getDataAsString(m_varSymbol->getId()).c_str());
    fp->write("\", ");
    fp->write_decimal(m_varSymbol->getPosOffset());
    fp->write("u); return i; }"); GCNL;

    dmcount++; //increment data member count
  } //generateUlamClassInfo

  void NodeVarDeclDM::addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers)
  {
    assert(m_varSymbol);
    ClassMemberDesc * descptr = new DataMemberDesc((SymbolVariableDataMember *) m_varSymbol, classType, m_state);
    assert(descptr);

    //replace m_memberName with Ulam Type and Name (t3343, edit)
    std::ostringstream mnstr;
    if(m_nodeTypeDesc)
      mnstr << m_state.m_pool.getDataAsString(m_nodeTypeDesc->getTypeNameId()).c_str();
    else
      mnstr << m_state.getUlamTypeNameBriefByIndex(getNodeType()).c_str();
    mnstr << " " << descptr->m_memberName;

    descptr->m_memberName = ""; //clear base init
    descptr->m_memberName = mnstr.str();

    //concat mangled class and parameter names to avoid duplicate keys into map
    std::ostringstream fullMangledName;
    fullMangledName << descptr->m_mangledClassName << "_" << descptr->m_mangledMemberName;
    classmembers.insert(std::pair<std::string, ClassMemberDescHolder>(fullMangledName.str(), ClassMemberDescHolder(descptr)));
  } //addMemberDescriptionToInfoMap

} //end MFM

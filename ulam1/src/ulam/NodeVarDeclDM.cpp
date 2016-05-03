#include <stdlib.h>
#include "NodeVarDeclDM.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"
#include "NodeTerminal.h"

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

    if(nut->getUlamClassType() == UC_QUARK)
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
	    classblock->printPostfixDataMembersParseTree(fp); //same default values
	  }
	m_state.popClassContext();
      }
    else
      {
	fp->write("(");

	if(m_nodeInitExpr)
	  {
	    // only for quarks and scalars
	    m_nodeInitExpr->printPostfix(fp);
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

  const char * NodeVarDeclDM::getName()
  {
    return NodeVarDecl::getName();
  }

  const std::string NodeVarDeclDM::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
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
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
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

  UTI NodeVarDeclDM::checkAndLabelType()
  {
    NodeVarDecl::checkAndLabelType(); //sets node type

    //don't allow a subclass to shadow a superclass datamember
    UTI cuti = m_state.getCompileThisIdx();
    UTI superuti = m_state.isClassASubclass(cuti);
    if(superuti == Hzy)
      {
	setNodeType(Hzy);
	m_state.setGoAgain();
	return Hzy;
      }
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
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    m_state.popClassContext();
	    return Nav; //short-circuit
	  }
	m_state.popClassContext();
      } //end subclass error checking

    if(m_nodeInitExpr)
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
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_state.setGoAgain(); //since not error
	    setNodeType(Hzy);
	    return Hzy; //short-circuit
	  }

	//constant fold if possible, set symbol value
	if(m_varSymbol)
	  {
	    AssertBool isDefined = ((SymbolVariableDataMember *) m_varSymbol)->hasInitValue();
	    assert(isDefined);
	    if(!(((SymbolVariableDataMember *) m_varSymbol)->initValueReady()))
	      {
		foldInitExpression(); //sets init constant value
		if(!(((SymbolVariableDataMember *) m_varSymbol)->initValueReady()))
		  {
		    setNodeType(Hzy);
		    m_state.setGoAgain(); //since not error
		    return Hzy;
		  }
	      }
	  }
	else
	  assert(0);

	//insure constant value fits in its declared type,
	//done in NodeVarDecl c&l: safeToCastTo(nuti)
      } //finished init expr node

    return getNodeType();
  } //checkAndLabelType

  void NodeVarDeclDM::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    NodeVarDecl::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  } //countNavHzyNoutiNodes

  void NodeVarDeclDM::setInitExpr(Node * node)
  {
    NodeVarDecl::setInitExpr(node);
    if(m_varSymbol)
      ((SymbolVariableDataMember *) m_varSymbol)->setHasInitValue();
  }

  //from NodeConstantDef; applied here to init value.
  // called during parsing rhs of named constant;
  // Requires a constant expression, else error;
  // (scope of eval is based on the block of const def.)
  bool NodeVarDeclDM::foldInitExpression()
  {
    UTI nuti = getNodeType();

    if(nuti == Nav || !m_state.isComplete(nuti))
      return false; //e.g. not a constant

    assert(m_varSymbol);
    if(((SymbolVariableDataMember *) m_varSymbol)->initValueReady())
      return true; //short-circuit

    if(!m_nodeInitExpr)
      return false;

    // if here, must be a constant init value..
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
	  assert(0);
      }

    evalNodeEpilog();

    if(evs == ERROR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for data member '";
	msg << m_state.m_pool.getDataAsString(m_varSymbol->getId()).c_str();
	msg << "' initialization is not yet ready while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
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
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  {
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_state.setGoAgain(); //since not error
	  }
	return false; //necessary if not just a warning.
      }

    //folded here..(but no checkandlabel yet)
    if(updateConstant(newconst))
      {
	NodeTerminal * newnode;
	if(m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum() == Int)
	  newnode = new NodeTerminal((s64) newconst, nuti, m_state);
	else
	  newnode = new NodeTerminal(newconst, nuti, m_state);

	newnode->setNodeLocation(getNodeLocation());
	delete m_nodeInitExpr;
	m_nodeInitExpr = newnode;
      }
    else
      return false;

    ((SymbolVariableDataMember *) m_varSymbol)->setInitValue(newconst); //isReady now!
    return true;
  } //foldInitExpression

  bool NodeVarDeclDM::updateConstant(u64 & newconst)
  {
    if(!m_varSymbol)
      return false;

    UTI nuti = getNodeType();
    if(!m_state.isComplete(nuti))
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
      assert(0);

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

    ULAMTYPE etype = nut->getUlamTypeEnum();
    switch(etype)
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
    ULAMTYPE etype = nut->getUlamTypeEnum();
    switch(etype)
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
    UTI nuti = getNodeType(); //same as symbol uti
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    u32 pos = m_varSymbol->getPosOffset();
    s32 bitsize = nut->getBitSize();

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
		arraysize = ((arraysize == NONARRAYSIZE) ? 1 : arraysize);
		//updates dvref in place at position 'pos' in dvref
		//from position 0 in dmdv (a copy)
		m_state.getDefaultAsArray(bitsize, arraysize, pos, dmdv, dvref); //both scalar and arrays
		aok = true;
	      }
	  }

	//fold packloadable class (e.g. quark) here for node eval, printpostfix..
	if(aok)
	  foldDefaultClass(); //try if packedloadable
      }
    else if(m_nodeInitExpr)
      {
	//primitive (not a class!)
	//arrays not initialized at this time
	if(m_state.isScalar(nuti))
	  {
	    u64 val = 0;
	    if(((SymbolVariableDataMember *) m_varSymbol)->getInitValue(val))
	      {
		s32 classsize = m_state.getBitSize(m_state.getCompileThisIdx());
		u64 packedval = 0;
		m_state.getDefaultAsPackedArray(classsize, bitsize, 1, pos, (u64) val, packedval);
		dvref.WriteLong(pos, bitsize, packedval);
		aok = true;
	      }
	  }
      }
    else
      aok = true; //no initialized value

    return aok;
  } //buildDefaultValue

  bool NodeVarDeclDM::foldDefaultQuark(u32 dq)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    u64 dqval = 0;
    //bouild dqval array
    if(!nut->isScalar())
      {
	//array of quarks
	if(nut->getTotalWordSize() > MAXBITSPERLONG) //64
	  {
	    std::ostringstream msg;
	    msg << "Not supported at this time, UNPACKED Quark array type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    return false;
	  }
	m_state.getDefaultAsPackedArray(nuti, dq, dqval); //3rd arg ref
      }
    else
      dqval = dq;

    //folding into a terminal node
    NodeTerminal * newnode = new NodeTerminal(dqval, nuti, m_state);
    newnode->setNodeLocation(getNodeLocation());
    delete m_nodeInitExpr;
    m_nodeInitExpr = newnode;
    //(in this order)
    ((SymbolVariableDataMember *) m_varSymbol)->setHasInitValue();
    ((SymbolVariableDataMember *) m_varSymbol)->setInitValue(dqval);
    return true;
  } //foldDefaultQuark

  void NodeVarDeclDM::foldDefaultClass()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    u64 dpkval = 0;
    if(!m_state.getPackedDefaultClass(nuti, dpkval))
      return; //not pack loadable (or not ok to continue)

    //build dqval array
    if(!nut->isScalar())
      {
	//array of packed classes
	if(nut->getTotalWordSize() > MAXBITSPERLONG) //64
	  {
	    std::ostringstream msg;
	    msg << "Not supported at this time, UN-PACKEDLOADABLE Class array type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG); //was ERR
	    return;
	  }
	u64 dpkarr = 0;
	m_state.getDefaultAsPackedArray(nuti, dpkval, dpkarr); //3rd arg ref
	dpkval = dpkarr;
      }
    //else scalar in dpkval

    //folding into a terminal node
    NodeTerminal * newnode = new NodeTerminal(dpkval, nuti, m_state);
    newnode->setNodeLocation(getNodeLocation());
    delete m_nodeInitExpr;
    m_nodeInitExpr = newnode;
    //(in this order) i thought this was for primitives only????
    ((SymbolVariableDataMember *) m_varSymbol)->setHasInitValue(); //???
    ((SymbolVariableDataMember *) m_varSymbol)->setInitValue(dpkval); //???
  } //foldDefaultClass

  void NodeVarDeclDM::packBitsInOrderOfDeclaration(u32& offset)
  {
    assert((s32) offset >= 0); //neg is invalid
    assert(m_varSymbol);

    m_varSymbol->setPosOffset(offset);

    UTI it = m_varSymbol->getUlamTypeIdx();
    assert(m_state.isComplete(it)); //moved error check to separate pass
    UlamType * ut = m_state.getUlamTypeByIndex(it);
    ULAMCLASSTYPE classtype = ut->getUlamClassType();
    u32 len = ut->getTotalBitSize();

    UTI cuti = m_state.getCompileThisIdx();
    ULAMCLASSTYPE thisclasstype = m_state.getUlamTypeByIndex(cuti)->getUlamClassType();
    if(thisclasstype == UC_TRANSIENT)
      {
	if(m_state.isAtom(it))
	  {
	    if(ut->isScalar())
	      {
		offset += BITSPERATOM; //allocate full size of atom
	      }
	    else
	      {
		offset += (BITSPERATOM * ut->getArraySize());
	      }
	  }
	else
	  {
	    offset += len; //includes arraysize, and other transients
	  }
      }
    else
      {
	// this datamember belongs to an element or quark
	if((len > MAXBITSPERLONG) && (classtype == UC_NOTACLASS))
	  {
	    std::ostringstream msg;
	    msg << "Data member <" << getName() << "> of type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", total size: " << (s32) m_state.getTotalBitSize(it);
	    msg << " MUST fit into " << MAXBITSPERLONG << " bits;";
	    msg << " Local variables do not have this restriction";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav); //compiler counts
	  }

	if(classtype == UC_ELEMENT)
	  {
	    std::ostringstream msg;
	    msg << "Data member <" << getName() << "> of type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", is an element, and is NOT permitted; Local variables, quarks, ";
	    msg << "and Model Parameters do not have this restriction";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav); //compiler counts
	  }

	if(classtype == UC_QUARK)
	  {
	    if(!ut->isScalar() && (len > MAXBITSPERLONG))
	      {
		std::ostringstream msg;
		msg << "Data member <" << getName() << "> of class array type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << ", total size: " << (s32) len;
		msg << " MUST fit into " << MAXBITSPERLONG << " bits;";
		msg << "Local variables do not have this restriction";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		setNodeType(Nav); //compiler counts
	      }
	  }

	if(classtype == UC_TRANSIENT)
	  {
	    std::ostringstream msg;
	    msg << "Data member <" << getName() << "> of type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", is a transient, and is NOT permitted; Local variables, ";
	    msg << "do not have this restriction";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav); //compiler counts
	  }

	offset += len;
      } //not transient

    //this check is valid regardless of where quark resides
    if(classtype == UC_QUARK)
      {
	if(ut->isScalar() && (len > MAXBITSPERINT))
	  {
	    std::ostringstream msg;
	    msg << "Data member <" << getName() << "> of class type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
	    msg << ", total size: " << (s32) len;
	    msg << " MUST fit into " << MAXBITSPERINT << " bits";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav); //compiler counts
	  }
      } //quarks
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
    assert(0); //override
  }

  EvalStatus NodeVarDeclDM::eval()
  {
    assert(m_varSymbol);

    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    //ULAMCLASSTYPE classtype = nut->getUlamClassType();
    ULAMTYPE etyp = nut->getUlamTypeEnum();

    assert(m_varSymbol->getAutoLocalType() == ALT_NOT);

    if(m_state.isAtom(nuti))
      return NodeVarDecl::eval();

    // m_nodeInitExpr exists as result of a previous fold
    if(etyp == Class && (m_nodeInitExpr == NULL))
      {
	//element and transient can only be a data member of a transient
	//quarks go anywhere;
	u64 dpkval = 0;
	if(m_state.getPackedDefaultClass(nuti, dpkval))
	  foldDefaultClass();
	else
	  return ERROR;
      }

    //if(classtype == UC_TRANSIENT)
    //  return NodeVarDecl::eval(); //can only be a data member of a transient

    //if(m_state.isAtom(nuti) || (classtype == UC_ELEMENT))
    //  return NodeVarDecl::eval();
#if 0
    //should be correctly handled by generic class approach above
    // make terminal expression node so rest of eval works for quarks too
    if((classtype == UC_QUARK) && (m_nodeInitExpr == NULL))
      {
	u32 dqval = 0;
	if(m_state.getDefaultQuark(nuti, dqval))
	  {
	    if(!foldDefaultQuark(dqval))
	      return ERROR;
	  }
	else
	  return ERROR;
      }
#endif

    // packedloadable class (e.g. quark) or nonclass data member;
    if(((SymbolVariableDataMember *) m_varSymbol)->hasInitValue())
      {
	return NodeVarDecl::evalInitExpr();
      }
    return NORMAL;
  } //eval

  EvalStatus NodeVarDeclDM::evalToStoreInto()
  {
    evalNodeProlog(0); //new current node eval frame pointer

    // (from NodeIdent's makeUlamValuePtr)
    // return ptr to this data member within the m_currentObjPtr
    // 'pos' modified by this data member symbol's packed bit position
    UlamValue rtnUVPtr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());

    //copy result UV to stack, -1 relative to current frame pointer
    Node::assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  } //evalToStoreInto

  // parse tree in order declared, unlike the ST.
  void NodeVarDeclDM::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_varSymbol);
    assert(m_state.isComplete(getNodeType()));

    assert(m_varSymbol->isDataMember());

    return genCodedBitFieldTypedef(fp, uvpass);
  } //genCode

  // variable is a data member; cannot be an element
  void NodeVarDeclDM::genCodedBitFieldTypedef(File * fp, UVPass& uvpass)
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE netyp = nut->getUlamTypeEnum();
    //ULAMCLASSTYPE nclasstype = nut->getUlamClassType();
    ULAMCLASSTYPE classtype = m_state.getUlamClassForThisClass();

    m_state.indent(fp);
    //if(nclasstype == UC_QUARK && nut->isScalar())
    if((netyp == Class) && nut->isScalar())
      {
	// use typedef rather than atomic parameter for classes
	// (e.g. quarks within elements, element within transients,
	//       transients within transients, etc).
	// except if an array of quarks.
	fp->write("typedef ");
	fp->write(nut->getUlamTypeMangledName().c_str()); //for C++
	fp->write("<EC> ");
	fp->write(m_varSymbol->getMangledNameForParameterType().c_str());
	fp->write("; //offset ");
	fp->write_decimal_unsigned(m_varSymbol->getPosOffset());
	fp->write("u\n"); //func call parameters aren't NodeVarDecl's
      }
    else
      {
	fp->write("typedef UlamRefFixed");
	fp->write("<EC, "); //BITSPERATOM

	if(classtype == UC_QUARK)
	  {
	    fp->write_decimal(m_varSymbol->getPosOffset());
	  }
	else if(classtype == UC_ELEMENT)
	  {
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset());
	  }
	else if(classtype == UC_TRANSIENT)
	  {
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset());
	  }
	else
	  assert(0);

	fp->write("u, ");
	fp->write_decimal(nut->getTotalBitSize()); //include arraysize
	fp->write("u> ");

	fp->write(m_varSymbol->getMangledNameForParameterType().c_str());
	fp->write(";\n"); //func call parameters aren't NodeVarDecl's
      }
  } //genCodedBitFieldTypedef

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
    fp->write("u); return i; }\n");

    dmcount++; //increment data member count
  } //generateUlamClassInfo

} //end MFM

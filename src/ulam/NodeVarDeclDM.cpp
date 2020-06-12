#include <stdlib.h>
#include "NodeVarDeclDM.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"
#include "NodeListClassInit.h"
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
	if(hasInitExpr())
	  {
	    fp->write("("); // start open paren
	    m_nodeInitExpr->printPostfix(fp);
	  }
	else
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
		  fp->write("("); // start open paren
		classblock->printPostfixDataMembersParseTree(fp, csym->getUlamTypeIdx()); //same default values
	      }
	    m_state.popClassContext();
	  }
      }
    else
      {
	fp->write("("); // start open paren

	if(hasInitExpr())
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

  void NodeVarDeclDM::noteTypeAndName(UTI cuti, s32 totalsize, u32& accumsize)
  {
    assert(m_varSymbol);
    UTI nuti = m_varSymbol->getUlamTypeIdx(); //t41286, node type for class DMs maybe Hzy still.

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
      note << nut->getUlamTypeClassNameBrief(nuti).c_str();

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

    //all union data members start at pos 0
    if(!m_state.isClassAQuarkUnion(cuti))
      accumsize += nsize;
  } //noteTypeAndName

  void NodeVarDeclDM::genTypeAndNameEntryAsComment(File * fp, s32 totalsize, u32& accumsize)
  {
    assert(m_varSymbol);
    UTI nuti = m_varSymbol->getUlamTypeIdx(); //t41286, node type for class DMs maybe Hzy still.

    UlamKeyTypeSignature vkey = m_state.getUlamKeyTypeSignatureByIndex(nuti);
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nsize = nut->getSizeofUlamType(); //e.g. 96 for elements

    s32 pos = m_varSymbol->getPosOffset(); //better data (e.g. quarkunion)

    // "| Position\t| Bitsize\t| Name\t| Type\t| "
    m_state.indent(fp);
    fp->write("| ");
    fp->write_decimal_unsigned(pos); //at
    fp->write("\t| ");
    fp->write_decimal(nsize); // of totalsize
    fp->write("\t| "); //name
    fp->write(getName());
    s32 arraysize = nut->getArraySize();
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }

    fp->write("\t| "); //type
    //like NodeVarDecl::printNameAndType
    if(nut->getUlamTypeEnum() != Class)
      fp->write(vkey.getUlamKeyTypeSignatureNameAndBitSize(&m_state).c_str());
    else
      fp->write(nut->getUlamTypeClassNameBrief(nuti).c_str());;

    fp->write("\n"); //end

    accumsize += nsize;
  } //genTypeAndNameEntryAsComment

  const char * NodeVarDeclDM::getName()
  {
    return NodeVarDecl::getName();
  }

  const std::string NodeVarDeclDM::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeVarDeclDM::setNodeType(UTI uti)
  {
    Node::setNodeType(uti);
    if(m_state.okUTItoContinue(uti) && m_state.isAClass(uti))
      if(hasInitExpr() && m_nodeInitExpr->isClassInit()) //t41201,6
    	m_nodeInitExpr->setClassType(uti);
  }

  bool NodeVarDeclDM::hasASymbolDataMember()
  {
    return true;
  }

  FORECAST NodeVarDeclDM::safeToCastTo(UTI newType)
  {
    assert(m_nodeInitExpr);

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
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
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
    if(m_state.getUlamTypeByIndex(uti)->isAltRefType())
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
    // but a quark/union data member may as long as they don't clobber.
    if(m_state.isClassAQuarkUnion(cuti) && hasInitExpr())
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
    if(m_state.isClassAQuarkUnion(cuti) && m_state.isAStringType(nuti))
      {
	std::ostringstream msg;
	msg << "Data member '";
	msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "' belongs to a quark-union, and cannot be type String";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav; //short-circuit
      }

    //don't allow a subclass to shadow a base class datamember (error/t41331)
    TBOOL shadowt = checkForNoShadowingSubclass(cuti);
    if(shadowt != TBOOL_TRUE)
      return getNodeType();
    //else continue...

    //NodeVarDecl handles array initialization for both locals & dm
    // since initial expressions must be constant for both (unlike local scalars)
    if(hasInitExpr() && m_state.isScalar(getNodeType()))
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

	if(m_state.isStillHazy(it))
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for data member: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is not ready";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	    setNodeType(Hzy);
	    m_state.setGoAgain(); //since not error
	    return Hzy; //short-circuit
	  }

	//constant fold if possible, set symbol value
	assert(m_varSymbol);

	if(m_varSymbol->hasInitValue())
	  {
	    if(!(m_varSymbol->isInitValueReady()))
	      {
		bool foldok = false;
		if(m_state.isAClass(it))
		  {
		    //pos still not updated; wait for after c&l
		    foldok = true; //t41167 noop here, folding part of c&l for each dm
		  }
		else
		  foldok = foldArrayInitExpression(); //sets init constant value

		if(!foldok)
		  {
		    assert(m_nodeInitExpr);
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

  TBOOL NodeVarDeclDM::checkForNoShadowingSubclass(UTI cuti)
  {
    std::set<UTI> kinset;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbolByAncestorsOf(cuti, m_vid, kinset, hazyKin))
      {
	u32 kinsetsize = kinset.size();

	std::ostringstream msg;
	msg << "Data member '";
	msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	msg << "' is shadowing ";
	if(kinsetsize == 1)
	  msg << "an ancestor: ";
	else
	  msg << kinsetsize << " ancestors: ";

	u32 k=0;
	std::set<UTI>::iterator it;
	for(it = kinset.begin(); it != kinset.end(); it++, k++)
	  {
	    UTI ancestor = *it;
	    if(m_state.okUTItoContinue(ancestor))
	      {
		if(k > 0)
		  msg << ", ";
		msg << m_state.getUlamTypeNameBriefByIndex(ancestor).c_str();
	      }
	  }

      if(hazyKin)
	{
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	  setNodeType(Hzy);
	  m_state.setGoAgain();
	  return TBOOL_HAZY;
	}
      else
	{
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  setNodeType(Nav);
	  return TBOOL_FALSE;
	}
      }
    return TBOOL_TRUE; //aok
  } //checkForNoShadowingSubclass

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
	    msg << "Data member '" << getName() << "' of numeric scalar type: ";
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
	    msg << "Data member '" << getName() << "' of type: ";
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
		msg << "Data member '" << getName() << "' of class array type: ";
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
	    msg << "Data member '" << getName() << "' of type: ";
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
	    msg << "Data member '" << getName() << "' of class type: ";
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
    //called during parsing
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

    if(!m_state.okUTItoContinue(nuti) || !m_state.isComplete(nuti))
      return false; //e.g. not a constant

    assert(m_varSymbol);
    if(m_varSymbol->isInitValueReady())
      return true; //short-circuit

    if(!m_state.isScalar(nuti)) //arrays handled by NodeVarDecl (virtual)
      return NodeVarDecl::foldArrayInitExpression();

    if(!hasInitExpr())
      return true;

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
      return false;

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
    u32 srcbitsize = hasInitExpr() ? m_state.getBitSize(m_nodeInitExpr->getNodeType()) : nbitsize; //was MAXBITSPERINT WRONG!

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
    u32 srcbitsize = hasInitExpr() ? m_state.getBitSize(m_nodeInitExpr->getNodeType()) : nbitsize; //was MAXBITSPERINT WRONG!
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
    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UTI cuti = m_state.getCompileThisIdx();

    if(!m_varSymbol->isPosOffsetReliable())
      {
	if(m_state.isAClass(vuti))
	  m_state.tryToPackAClass(vuti);

	if(!m_varSymbol->isPosOffsetReliable())
	  m_state.tryToPackAClass(cuti); //t41262

	if(!m_varSymbol->isPosOffsetReliable())
	  return false;
	//else continue
      }

    UTI nuti = getNodeType(); //same as symbol uti, unless prior error
    assert(nuti == vuti);

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    UlamType * cut = m_state.getUlamTypeByIndex(cuti);

    u32 pos = m_varSymbol->getPosOffset();
    u32 len = nut->getSizeofUlamType();

    if(len == 0)
      return true; //no size to initialize (e.g. System)

    if(cut->getUlamClassType() == UC_ELEMENT)
      pos += ATOMFIRSTSTATEBITPOS; //atom-based (ulam-4 TypeField known at compile-time)

    bool aok = false; //init as not ready
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    if(etyp == Class) //thisClass contains a different class
      {
	if(!hasInitExpr())
	  {
	    //no node expression -- will use default class value (t41232)
	    aok = true;
	  }
	else
	  {
	    BV8K dmdv; //copies default BV
	    if(m_varSymbol->isInitValueReady())
	      {
		//NodeVarDecl tries to build initialized constants during c&l (ulam-4)
		aok = true;
	      }
	    else if(m_nodeInitExpr->isAConstantClass())
	      {
		if((aok = m_nodeInitExpr->getConstantValue(dmdv)))
		  m_varSymbol->setInitValue(dmdv); //t41229
	      }
	    else if(m_nodeInitExpr->isAList())
	      {
		if(m_state.getDefaultClassValue(nuti, dmdv)) //uses scalar uti
		  {
		    if(!((NodeList *) m_nodeInitExpr)->isEmptyList())
		      {
			BV8K bvmask;
			if((aok = m_nodeInitExpr->initDataMembersConstantValue(dmdv, bvmask)))
			  m_varSymbol->setInitValue(dmdv); //t41167,8 t41185(handles arrays)
		      }
		    else
		      {
			m_varSymbol->setInitValue(dmdv); //t41167,8 t41185(handles arrays)
			aok = true; //no init expression, just the default (t3143)
		      }
		  } //else default value not ok
	      }
	    else
	      m_state.abortShouldntGetHere();
	  } //has init expr

	if(aok)
	  {
	    foldDefaultClass(); //init value for m_varSymbol (if not already done), handles arrays t3512
	    if((aok = m_varSymbol->isInitValueReady()))
	      {
		BV8K bvarr;
		m_varSymbol->getInitValue(bvarr);
		bvarr.CopyBV(0, pos, len, dvref); //both scalar and arrays (t41185, t41267, t3818)
	      } //else not ready (t41184)
	  }
      }
    else if(etyp == UAtom) //this Transient contains an empty atom (t3802)
      {
	BV8K bvatom; //copy default EMPTY Element with Type (ulam-4)
	UTI emptyuti = m_state.getEmptyElementUTI(); //first class seen by compiler
	AssertBool gotDefault = m_state.getDefaultClassValue(emptyuti, bvatom);
	assert(gotDefault);

	s32 arraysize = nut->getArraySize();
	arraysize = ((arraysize == NONARRAYSIZE) ? 1 : arraysize); //could be 0

	BV8K darrval;
	m_state.getDefaultAsArray(BITSPERATOM, arraysize, 0, bvatom, darrval);

	m_varSymbol->setInitValue(darrval); //t3818

	//updates dvref in place at position 'pos'
	darrval.CopyBV(0, pos, len, dvref); //both scalar and arrays
	aok = true;
      }
    else if(hasInitExpr())
      {
	//primitive (neither a class, nor an atom!)
	//arrays may be initialized now
	assert(m_varSymbol->hasInitValue());
	BV8K dval; //copies default BV
	if(m_varSymbol->getInitValue(dval))
	  {
	    dval.CopyBV(0u, pos, len, dvref); //frompos, topos, len, destBV
	    aok = true; //e.g. t3512
	  }
      }
    else
      aok = true; //no initialized value

    return aok;
  } //buildDefaultValue

  bool NodeVarDeclDM::buildDefaultValueForClassConstantDefs()
  {
    return true;
  }

  void NodeVarDeclDM::foldDefaultClass()
  {
    if(m_varSymbol->isInitValueReady())
      return;

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    assert(m_state.okUTItoContinue(nuti));
    assert(m_state.isComplete(nuti));

    UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
    UlamType * scalarut = m_state.getUlamTypeByIndex(scalaruti);
    u32 bitsize = scalarut->getSizeofUlamType(); //was BitSize (t41185)
    s32 arraysize = nut->getArraySize();

    BV8K bvtmp;
    if(m_state.getDefaultClassValue(nuti, bvtmp)) //uses scalar
      {
	BV8K bvarr;
	arraysize = arraysize > 0 ? arraysize : 1;
	m_state.getDefaultAsArray(bitsize, arraysize, 0u, bvtmp, bvarr);

	m_varSymbol->setInitValue(bvarr); //t3512
      }
    //else initValue not ready (e.g. t41184)
  } //foldDefaultClass

  TBOOL NodeVarDeclDM::packBitsInOrderOfDeclaration(u32& offset)
  {
    //can be called any time during c&l resolving loop; may not be ready yet
    assert((s32) offset >= 0); //neg is invalid

    UTI nuti = getNodeType();
    if(!m_state.okUTItoContinue(nuti))
      {
	if(nuti == Nav)
	  return TBOOL_FALSE;
	return TBOOL_HAZY; //t3875
      }
    if(!m_state.isComplete(nuti))
      return TBOOL_HAZY;

    if(m_varSymbol==NULL)
      return TBOOL_HAZY;

    assert(nuti == m_varSymbol->getUlamTypeIdx()); //same as symbol, or shouldn't be here!

    ((SymbolVariableDataMember *) m_varSymbol)->setPosOffset(offset);

    if(m_state.isClassAQuarkUnion(m_state.getCompileThisIdx()))
      return TBOOL_TRUE; //offset not incremented; all DM at pos 0 (t3209, t41145)

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    u32 len = nut->getSizeofUlamType();
    offset += len; //uses atom-based size for element, and complete size for quark data members
    return TBOOL_TRUE;
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
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    assert(m_varSymbol->getAutoLocalType() == ALT_NOT);

    if(m_state.isAtom(nuti))
      return NodeVarDecl::eval();

    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMCLASSTYPE classtype = nut->getUlamClassType();
    if((classtype == UC_TRANSIENT) && (nut->getTotalBitSize() > MAXSTATEBITS))
      return evalStatusReturnNoEpilog(UNEVALUABLE);

    // packedloadable class (e.g. quark) or nonclass data member; t41167
    if(hasInitExpr())
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
      return evalStatusReturnNoEpilog(UNEVALUABLE);

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

    UVPass uvpass2clear;
    uvpass = uvpass2clear; //refresh

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
	// use typedef rather than atomic parameter for classes
	// (e.g. quarks within elements, element within transients,
	//       transients within transients, etc).
	// except if an array of quarks.
	fp->write("typedef ");
	fp->write(nut->getUlamTypeMangledName().c_str()); //for C++
	fp->write("<EC> ");
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

	u32 len = nut->getSizeofUlamType();
	if(nut->getUlamClassType() == UC_ELEMENT) //t3714, t3779 Mob.h Up_Um_2sp
	  {
	    //elements only data members in transients
	    assert(m_state.getUlamClassForThisClass() == UC_TRANSIENT);
	    s32 arraysize = nut->getArraySize();
	    arraysize = ((arraysize == NONARRAYSIZE) ? 1 : arraysize); //Mon Jul  4 14:11:41 2016
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset());
	    fp->write("u, BPA * "); //atom-based
	    fp->write_decimal(arraysize); //include arraysize
	    fp->write("u> ");
	  }
	else
	  {
	    fp->write_decimal_unsigned(m_varSymbol->getPosOffset());
	    fp->write("u, ");
	    fp->write_decimal(len); //include arraysize
	    fp->write("u> ");
	  }
	fp->write(m_varSymbol->getMangledName().c_str());
	fp->write(";"); GCNL; //func call parameters aren't NodeVarDecl's
      }
  } //genCodedBitFieldTypedef

  void NodeVarDeclDM::genCodeConstantArrayInitialization(File * fp)
  { }

  void NodeVarDeclDM::generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly)
  { }

  void NodeVarDeclDM::generateTestInstance(File * fp, bool runtest)
  {
    UTI nuti = getNodeType();
    if(m_state.isAClass(nuti))
      {
	UTI scalaruti = m_state.getUlamTypeAsScalar(nuti);
	SymbolClass * csym = NULL;
	AssertBool isDefined = m_state.alreadyDefinedSymbolClass(scalaruti, csym);
	assert(isDefined);
	csym->generateTestInstance(fp, runtest);
      }
  }

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
    fp->write("\", \"");

    //ulam-5, needs baseclass relative start pos
    UTI dmclass = m_varSymbol->getDataMemberClass();
    u32 dmclassrelpos = UNRELIABLEPOS;
    AssertBool gotRelPos = m_state.getABaseClassRelativePositionInAClass(m_state.getCompileThisIdx(), dmclass, dmclassrelpos);
    assert(gotRelPos);

    fp->write(m_state.getUlamTypeByIndex(dmclass)->getUlamTypeMangledName().c_str());
    fp->write("\", ");
    fp->write_decimal(m_varSymbol->getPosOffset() + dmclassrelpos);
    fp->write("u); return i; }\n");

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

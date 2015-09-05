#include <stdlib.h>
#include "NodeVarDeclDM.h"
#include "Token.h"
#include "CompilerState.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "NodeIdent.h"
#include "NodeTerminal.h"

namespace MFM {

  NodeVarDeclDM::NodeVarDeclDM(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeVarDecl(sym, nodetype, state), m_nodeInitExpr(NULL)
  { }

  NodeVarDeclDM::NodeVarDeclDM(const NodeVarDeclDM& ref) : NodeVarDecl(ref), m_nodeInitExpr(NULL)
  {
    if(ref.m_nodeInitExpr)
      m_nodeInitExpr = (Node *) ref.m_nodeInitExpr->instantiate();
  }

  NodeVarDeclDM::~NodeVarDeclDM()
  {
    delete m_nodeInitExpr;
    m_nodeInitExpr = NULL;
  }

  Node * NodeVarDeclDM::instantiate()
  {
    return new NodeVarDeclDM(*this);
  }

  // see SymbolVariable: printPostfixValuesOfVariableDeclarations via ST.
  void NodeVarDeclDM::printPostfix(File * fp)
  {
    NodeVarDecl::printPostfix(fp);
  } //printPostfix

  const char * NodeVarDeclDM::getName()
  {
    return NodeVarDecl::getName();
  }

  const std::string NodeVarDeclDM::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeVarDeclDM::checkAndLabelType()
  {
    NodeVarDecl::checkAndLabelType(); //sets node type

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

	UTI it = m_nodeInitExpr->checkAndLabelType();
	if(it == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for data member: ";
	    msg << m_state.m_pool.getDataAsString(m_vid).c_str();
	    msg << ", initialization is invalid";
	    if(m_nodeInitExpr->isReadyConstant())
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    else
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    setNodeType(Nav);
	    return Nav; //short-circuit
	  }

	//constant fold if possible, set symbol value
	if(m_varSymbol)
	  {
	    assert(((SymbolVariableDataMember *) m_varSymbol)->hasInitValue());
	    if(!(((SymbolVariableDataMember *) m_varSymbol)->initValueReady()))
	      {
		foldConstantExpression(); //sets init constant value
		if(!(((SymbolVariableDataMember *) m_varSymbol)->initValueReady()))
		  setNodeType(Nav);
	      }
	  }
	else
	  assert(0);
      }

    return getNodeType();
  } //checkAndLabelType

  void NodeVarDeclDM::countNavNodes(u32& cnt)
  {
    if(m_nodeInitExpr)
      m_nodeInitExpr->countNavNodes(cnt);

    NodeVarDecl::countNavNodes(cnt);
  } //countNavNodes

  void NodeVarDeclDM::setConstantExpr(Node * node)
  {
    assert(node);
    m_nodeInitExpr = node;
    m_nodeInitExpr->updateLineage(getNodeNo()); //for unknown subtrees
    if(m_varSymbol)
      ((SymbolVariableDataMember *) m_varSymbol)->setHasInitValue();
  }

  //from NodeConstantDef; applied here to init value.
  // called during parsing rhs of named constant;
  // Requires a constant expression, else error;
  // (scope of eval is based on the block of const def.)
  bool NodeVarDeclDM::foldConstantExpression()
  {
    UTI uti = getNodeType();

    if(uti == Nav || !m_state.isComplete(uti))
      return false; //e.g. not a constant

    assert(m_varSymbol);
    if(((SymbolVariableDataMember *) m_varSymbol)->initValueReady())
      return true;

    if(!m_nodeInitExpr)
      return false;

    // if here, must be a constant init value..
    u64 newconst = 0; //UlamType format (not sign extended)
    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(uti); //offset a constant expression
    EvalStatus evs = m_nodeInitExpr->eval();
    if(evs == NORMAL)
      {
	UlamValue cnstUV = m_state.m_nodeEvalStack.popArg();
	u32 wordsize = m_state.getTotalWordSize(uti);
	if(wordsize == MAXBITSPERINT)
	  newconst = cnstUV.getImmediateData(m_state);
	else if(wordsize == MAXBITSPERLONG)
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
	return false;
      }

    //insure constant value fits in its declared type
    FORECAST scr = m_nodeInitExpr->safeToCastTo(uti);
    if(scr != CAST_CLEAR)
      {
	std::ostringstream msg;
	msg << "Constant value expression for data member (";
	msg << getName() << " = " << m_nodeInitExpr->getName() ;
	msg << ") initialization is not representable as ";
	msg<< m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	if(scr == CAST_BAD)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	return false; //necessary if not just a warning.
      }

    //folded here..
    if(updateConstant(newconst))
      {
	NodeTerminal * newnode;
	if(m_state.getUlamTypeByIndex(uti)->getUlamTypeEnum() == Int)
	  newnode = new NodeTerminal((s64) newconst, uti, m_state);
	else
	  newnode = new NodeTerminal(newconst, uti, m_state);

	newnode->setNodeLocation(getNodeLocation());
	delete m_nodeInitExpr;
	m_nodeInitExpr = newnode;
      }
    else
      return false;

    ((SymbolVariableDataMember *) m_varSymbol)->setInitValue(newconst); //isReady now!
    return true;
  } //foldConstantExpression

  bool NodeVarDeclDM::updateConstant(u64 & newconst)
  {
    if(!m_varSymbol)
      return false;

    UTI nuti = getNodeType();
    if(!m_state.isComplete(nuti))
      return false;

    //store in UlamType format
    bool rtnb = true;
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nbitsize = nut->getBitSize();
    assert(nbitsize > 0);
    u32 wordsize = nut->getTotalWordSize();
    if(wordsize == MAXBITSPERINT)
      rtnb = updateConstant32(newconst);
    else if(wordsize == MAXBITSPERLONG)
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

  EvalStatus NodeVarDeclDM::eval()
  {
    return NodeVarDecl::eval();
  } //eval

  // parse tree in order declared, unlike the ST.
  void NodeVarDeclDM::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_varSymbol);
    assert(getNodeType() != Nav);

    if(m_varSymbol->isDataMember())
      {
	return genCodedBitFieldTypedef(fp, uvpass);
      }

    if(m_varSymbol->isAutoLocal())
      {
	return genCodedAutoLocal(fp, uvpass);
      }

    UTI vuti = m_varSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    m_state.indent(fp);
    if(!m_varSymbol->isDataMember())
      {
	fp->write(vut->getImmediateStorageTypeAsString().c_str()); //for C++ local vars
      }
    else
      {
	fp->write(vut->getUlamTypeMangledName().c_str()); //for C++
	assert(0); //doesn't happen anymore..
      }

    fp->write(" ");
    fp->write(m_varSymbol->getMangledName().c_str());

    ULAMCLASSTYPE vclasstype = vut->getUlamClass();

    //initialize T to default atom (might need "OurAtom" if data member ?)
    if(vclasstype == UC_ELEMENT)
      {
	fp->write(" = ");
	fp->write(m_state.getUlamTypeByIndex(vuti)->getUlamTypeMangledName().c_str());
	fp->write("<EC>");
	fp->write("::THE_INSTANCE");
	fp->write(".GetDefaultAtom()"); //returns object of type T
      }

    if(vclasstype == UC_QUARK)
      {
	//right-justified?
      }

    fp->write(";\n"); //func call parameters aren't NodeVarDeclDM's
  } //genCode

} //end MFM

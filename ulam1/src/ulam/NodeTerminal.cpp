#include <stdlib.h>
#include <errno.h>
#include "NodeTerminal.h"
#include "CompilerState.h"

namespace MFM {

  NodeTerminal::NodeTerminal(CompilerState & state) : Node(state) {}

  NodeTerminal::NodeTerminal(Token tok, CompilerState & state) : Node(state)
  {
    Node::setNodeLocation(tok.m_locator); //in case of errors
    setConstantValue(tok);
    setConstantTypeForNode(tok);
  }

  //cannot convert using utype at construction since bitsize may be UNKNOWN
  NodeTerminal::NodeTerminal(s32 val, UTI utype, CompilerState & state) : Node(state)
  {
    m_constant.sval = val;
    setNodeType(utype);
    //uptocaller to set node location.
  }

  //cannot convert using utype at construction since bitsize may be UNKNOWN
  NodeTerminal::NodeTerminal(u32 val, UTI utype, CompilerState & state) : Node(state)
  {
    m_constant.uval = val;
    setNodeType(utype);
    //uptocaller to set node location.
  }

  NodeTerminal::NodeTerminal(const NodeTerminal& ref) : Node(ref), m_constant(ref.m_constant) {}

  NodeTerminal::NodeTerminal(const NodeIdent& iref) : Node(iref) {}

  NodeTerminal::~NodeTerminal(){}

  Node * NodeTerminal::instantiate()
  {
    return new NodeTerminal(*this);
  }

  void NodeTerminal::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  } //printPostfix

  const char * NodeTerminal::getName()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nbitsize = nut->getBitSize();
    assert(nbitsize > 0);
    u32 wordsize = nut->getTotalWordSize();
    assert(wordsize == 32);
    ULAMTYPE etype = nut->getUlamTypeEnum();
    std::ostringstream num;
    switch(etype)
      {
      case Bool:
	num << ( _Bool32ToCbool(m_constant.uval, nbitsize) ? "true" : "false");
	break;
      case Int:
	num << _Int32ToInt32(m_constant.sval, wordsize, nbitsize);
	break;
      case Unsigned:
	num << _Unsigned32ToUnsigned32(m_constant.uval, wordsize, nbitsize) << "u";
	break;
      case Unary:
	num << _Unsigned32ToUnary32(m_constant.uval, wordsize, nbitsize) << "u"; //y
	break;
      case Bits:
	num << _Unsigned32ToBits32(m_constant.uval, wordsize, nbitsize) << "u";  //t
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Constant Type Unknown: " <<  m_state.getUlamTypeNameByIndex(nuti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  num << "CONSTANT?";
	}
      };
    u32 id = m_state.m_pool.getIndexForDataString(num.str());
    return m_state.m_pool.getDataAsString(id).c_str();
  } //getName

  const std::string NodeTerminal::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  // this is the application of unary minus to produce a negative number
  void NodeTerminal::constantFoldAToken(Token tok)
  {
    if(tok.m_type == TOK_MINUS)
      {
	UTI nuti = getNodeType();
	ULAMTYPE etype = m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum();
	if(etype == Int)
	  {
	    m_constant.sval = -m_constant.sval;
	    fitsInBits(nuti);  //check self
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Negating an unsigned constant: <" << m_constant.uval <<  ">, type: ";
	    msg << m_state.getUlamTypeNameByIndex(nuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Constant Folding Token: <" << m_state.getTokenDataAsString(&tok).c_str();
	msg << ">, currently unsupported";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	assert(0);
      }
  } //constantFoldAToken

  bool NodeTerminal::isAConstant()
  {
    return true;
  }

  bool NodeTerminal::isReadyConstant()
  {
    return true;
  }

  UTI NodeTerminal::checkAndLabelType()
  {
    return getNodeType(); //done by constructor
  } //checkAndLabelType

  EvalStatus NodeTerminal::eval()
  {
    EvalStatus evs = NORMAL; //init ok

    if(!m_state.isComplete(getNodeType()))
       return ERROR;

    evalNodeProlog(0); //new current frame pointer

    UlamValue rtnUV;
    evs = makeTerminalValue(rtnUV);

    //copy result UV to stack, -1 relative to current frame pointer
    if(evs == NORMAL)
      Node::assignReturnValueToStack(rtnUV);

    evalNodeEpilog();
    return evs;
  } //eval

  EvalStatus NodeTerminal::makeTerminalValue(UlamValue& uvarg)
  {
    UlamValue rtnUV; //init to Nav error case
    EvalStatus evs = NORMAL; //init ok
    UTI nuti = getNodeType();
    assert(nuti != Nav);
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nbitsize = nut->getBitSize();
    assert(nbitsize > 0);
    u32 wordsize = nut->getTotalWordSize();
    assert(wordsize == 32);
    ULAMTYPE etype = nut->getUlamTypeEnum();
    switch(etype)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(nuti, _Int32ToInt32(m_constant.sval, wordsize, nbitsize), m_state);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(nuti, _Unsigned32ToUnsigned32(m_constant.uval, wordsize, nbitsize), m_state);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(nuti, _Unsigned32ToBool32(m_constant.uval, wordsize, nbitsize), m_state);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(nuti, _Unsigned32ToUnary32(m_constant.uval, wordsize, nbitsize), m_state);
	break;
      case Bits:
	rtnUV = UlamValue::makeImmediate(nuti, _Unsigned32ToBits32(m_constant.uval, wordsize, nbitsize), m_state);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Constant Type Unknown: " <<  m_state.getUlamTypeNameByIndex(nuti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  evs = ERROR;
	}
      };
    uvarg = rtnUV;
    return evs;
  } //makeTerminalValue

  //used during check and label for binary arith and compare ops that have a constant term
  bool NodeTerminal::fitsInBits(UTI fituti)
  {
    bool rtnb = false;
    UTI nuti = getNodeType(); //constant type
    UlamType * fit = m_state.getUlamTypeByIndex(fituti);
    if(!fit->isComplete())
      {
	std::ostringstream msg;
	msg << "Unknown size!! constant type: ";
	msg << m_state.getUlamTypeNameByIndex(nuti).c_str();
	msg << ", to fit into type: " << m_state.getUlamTypeNameByIndex(fituti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	return false;
      }

    if(fit->getTotalWordSize() != MAXBITSPERINT) //32
      {
	std::ostringstream msg;
	msg << "Not supported at this time, constant type: ";
	msg << m_state.getUlamTypeNameByIndex(nuti).c_str() << ", to fit into type: ";
	msg << m_state.getUlamTypeNameByIndex(fituti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }

    if(!fit->isMinMaxAllowed())
      {
	std::ostringstream msg;
	msg << "Cannot check: <" << m_constant.uval << ">, fits into a non-arithmetic type: ";
	msg << m_state.getUlamTypeNameByIndex(fituti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }

    if(getNodeType() == Nav)
      {
	std::ostringstream msg;
	msg << "Constant Type Unknown: " <<  m_state.getUlamTypeNameByIndex(nuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	return false;
      }

    ULAMTYPE etype = m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum();
    switch(etype)
      {
      case Int:
	{
	  s32 numval = m_constant.sval;
	  if(fit->getUlamTypeEnum() == Int)
	    {
	      rtnb = (numval <= (s32) fit->getMax()) && (numval >= fit->getMin());
	    }
	  else
	    {
	      rtnb = (UABS32(numval) <= fit->getMax()) && (numval >= 0);
	    }
	}
	break;
      case Unsigned:
	{
	  u32 numval = m_constant.uval;
	  rtnb = (numval <= fit->getMax()) && (numval >= 0);
	}
	break;
      case Bool:
	rtnb = true;
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Constant Type Unknown: " <<  m_state.getUlamTypeNameByIndex(nuti).c_str();
	  msg << ", to fit into type: " << m_state.getUlamTypeNameByIndex(fituti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      };
    return rtnb;
  } //fitsInBits

  //used during check and label for bitwise shift op that has a negative constant term
  bool NodeTerminal::isNegativeConstant()
  {
    bool rtnb = false;
    if(m_state.getUlamTypeByIndex(getNodeType())->getUlamTypeEnum() == Int)
      {
	rtnb = (m_constant.sval < 0);
      }
    return rtnb;
  } //isNegativeConstant

  // used during check and label for bitwise shift op that has a constant term gt/ge 32;
  // false is ok.
  bool NodeTerminal::isWordSizeConstant()
  {
    bool rtnb = false;
    ULAMTYPE etype = m_state.getUlamTypeByIndex(getNodeType())->getUlamTypeEnum();
    if(etype == Int)
      {
	rtnb = (m_constant.sval >= m_state.getDefaultBitSize(Int));
      }
    else if(etype == Unsigned)
      {
	rtnb = (m_constant.uval > (u32) m_state.getDefaultBitSize(Unsigned)); // may be ==
      }
    return rtnb;
  } //isWordSizeConstant

  void NodeTerminal::genCode(File * fp, UlamValue& uvpass)
  {
    UlamValue tv;
    assert(makeTerminalValue(tv) == NORMAL);
    // unclear to do this read or not; squarebracket not happy, or cast not happy ?
    genCodeReadIntoATmpVar(fp, tv);  //tv updated to Ptr with a tmpVar "slot"
    uvpass = tv;
  } //genCode

  void NodeTerminal::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    UlamValue tv;
    assert(makeTerminalValue(tv) == NORMAL);
    uvpass = tv; //uvpass is an immediate UV, not a PTR
  } //genCodeToStoreInto

  // reads into a tmp var
  // (for BitVector use Node::genCodeConvertATmpVarIntoBitVector)
  void NodeTerminal::genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    // into tmp storage first, in case of casts
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32	tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");

    fp->write(nut->getTmpStorageTypeAsString().c_str());
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum).c_str());

    fp->write(" = ");

    fp->write(getName());
    fp->write(";\n");

    //substitute Ptr for uvpass to contain the tmpVar number;
    //save id of constant string in Ptr;
    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0);  //POS 0 rightjustified (atom-based);
    uvpass.setPtrPos(0); //entire register

    m_state.m_currentObjSymbolsForCodeGen.clear(); //missing or just needed by NodeTerminalProxy?
  } //genCodeReadIntoATmpVar

  bool NodeTerminal::setConstantValue(Token tok)
  {
    bool rtnok = false;
    switch(tok.m_type)
      {
      case TOK_NUMBER_SIGNED:
	{
	  std::string numstr = m_state.getTokenDataAsString(&tok);
	  const char * numlist = numstr.c_str();
	  char * nEnd;

	  m_constant.sval = strtol(numlist, &nEnd, 0);   //base 10, 8, or 16
	  if (*numlist == 0 || *nEnd != 0)
	    {
	      std::ostringstream msg;
	      msg << "Invalid signed constant: <" << numstr.c_str() << ">, errno=";
	      msg << errno << " " << strerror(errno);
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    }
	  else
	    rtnok = true;
	}
	break;
      case TOK_NUMBER_UNSIGNED:
	{
	  std::string numstr = m_state.getTokenDataAsString(&tok);
	  const char * numlist = numstr.c_str();
	  char * nEnd;

	  m_constant.uval = strtoul(numlist, &nEnd, 0);   //base 10, 8, or 16
	  if (*numlist == 0 || !(*nEnd == 'u' || *nEnd == 'U') || *(nEnd + 1) != 0)
	    {
	      std::ostringstream msg;
	      msg << "Invalid unsigned constant: <" << numstr.c_str() << ">, errno=";
	      msg << errno << " " << strerror(errno);
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    }
	  else
	    rtnok = true;
	}
	break;
      case TOK_KW_TRUE:
	m_constant.uval = 1u;
	rtnok = true;
	break;
      case TOK_KW_FALSE:
	m_constant.uval = 0u;
	rtnok = true;
	break;
      default:
	{
	    std::ostringstream msg;
	    msg << "Token is neither a number, nor a boolean: <";
	    msg <<  m_state.getTokenDataAsString(&tok).c_str() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	}
      };
    return rtnok;
  } //setConstantValue

  UTI NodeTerminal::setConstantTypeForNode(Token tok)
  {
    UTI newType = Nav; //init
    switch(tok.m_type)
      {
      case TOK_NUMBER_SIGNED:
	newType = Int; //m_state.getUlamTypeOfConstant(Int);
	break;
      case TOK_NUMBER_UNSIGNED:
	newType = Unsigned; //m_state.getUlamTypeOfConstant(Unsigned);
	break;
      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
	newType = Bool; //m_state.getUlamTypeOfConstant(Bool);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Token not a number or boolean: <";
	  msg << m_state.getTokenDataAsString(&tok).c_str() << ">";
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      };
    setNodeType(newType);
    setStoreIntoAble(false);
    return newType;
  } //setConstantTypeForNode

} //end MFM

#include <stdlib.h>
#include <errno.h>
#include "NodeTerminal.h"
#include "CompilerState.h"

namespace MFM {

  NodeTerminal::NodeTerminal(CompilerState & state) : Node(state) {}

  NodeTerminal::NodeTerminal(Token tok, CompilerState & state) : Node(state)
  {
    Node::setNodeLocation(tok.m_locator); //in case of errors
    setConstantTypeForNode(tok);
    setConstantValue(tok);
  }

  //cannot convert using utype at construction since bitsize may be UNKNOWN
  NodeTerminal::NodeTerminal(s64 val, UTI utype, CompilerState & state) : Node(state)
  {
    setNodeType(utype);
    m_constant.sval = val;
    //uptocaller to set node location.
  }

  //cannot convert using utype at construction since bitsize may be UNKNOWN
  NodeTerminal::NodeTerminal(u64 val, UTI utype, CompilerState & state) : Node(state)
  {
    setNodeType(utype);
    m_constant.uval = val;
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

    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    ULAMTYPE etype = nut->getUlamTypeEnum();
    if(etype == Bool)
      fp->write((_Bool64ToCbool(m_constant.uval, nut->getBitSize()) ? "true" : "false"));
    else
      fp->write(getName());
  } //printPostfix

  const char * NodeTerminal::getName()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nbitsize = nut->getBitSize();
    assert(nbitsize >= 0);
    u32 wordsize = nut->getTotalWordSize();
    ULAMTYPE etype = nut->getUlamTypeEnum();
    std::ostringstream num;
    switch(etype)
      {
      case Int:
	if(wordsize <= MAXBITSPERINT)
	  {
	    s32 sval = _Int32ToCs32((u32) m_constant.uval, nbitsize);
	    num << sval;
	  }
	else
	  {
	    s64 sval = _Int64ToCs64(m_constant.uval, nbitsize);
	    num << sval;
	  }
	break;
      case Bool:
      case Unsigned:
      case Unary:
      case Bits:
	// NO CASTING NEEDED, assume saved in its ulam-native format
	if(wordsize <= MAXBITSPERINT)
	  num << (u32) m_constant.uval << "u";
	else
	  num << m_constant.uval << "u";
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Constant Type Unknown: " <<  m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
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
	    fitsInBits(nuti); //check self
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Negating an unsigned constant '" << m_constant.uval <<  "'";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "Constant Folding Token <" << m_state.getTokenDataAsString(&tok).c_str();
	msg << "> currently unsupported";
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

  FORECAST NodeTerminal::safeToCastTo(UTI newType)
  {
    UTI nuti = getNodeType();
    ULAMTYPE ntypEnum = m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum();
    UlamType * newut = m_state.getUlamTypeByIndex(newType);
    ULAMTYPE typEnum = newut->getUlamTypeEnum();

    //special cases: not a matter of fitting
    if(typEnum == Bool || ntypEnum == Bool || typEnum == UAtom || typEnum == Class || typEnum == Void)
      return m_state.getUlamTypeByIndex(newType)->safeCast(nuti);

    //for non-bool terminal check for complete types and arrays before fits.
    FORECAST scr = m_state.getUlamTypeByIndex(newType)->UlamType::safeCast(nuti);
    if(scr != CAST_CLEAR)
      return scr;

    return fitsInBits(newType) ? CAST_CLEAR : CAST_BAD;
  } //safeToCastTo

  UTI NodeTerminal::checkAndLabelType()
  {
    UTI nuti = getNodeType(); //done by constructor using default size
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    assert(nut->isComplete());

    s32 bs = nut->getBitSize();
    //update bitsize to fit the constant value
    if(bs == m_state.getDefaultBitSize(nuti))
      {
	s32 newbs = bs;
	u32 wordsize = nut->getTotalWordSize();
	ULAMTYPE typEnum = nut->getUlamTypeEnum();
	switch(typEnum)
	  {
	  case Int:
	    {
	      if(wordsize <= MAXBITSPERINT)
		{
		  s32 sval = _Int32ToCs32((u32) m_constant.uval, bs);
		  newbs = (s32) (_getLogBase2(UABS32(sval)) + 1 + 1); //fits into signed
		}
	      else
		{
		  s64 sval = _Int64ToCs64(m_constant.uval, bs);
		  newbs = (s32) (_getLogBase2Long(UABS64(sval)) + 1 + 1); //fits into signed
		}
	    }
	    break;
	  case Bits:
	  case Unsigned:
	    {
	      if(wordsize <= MAXBITSPERINT)
		newbs = (s32) _getLogBase2((u32) m_constant.uval) + 1; //fits into unsigned
	      else
		newbs = (s32) _getLogBase2Long(m_constant.uval) + 1; //fits into unsigned
	    }
	    break;
	  case Unary:
	    {
	      u32 wordsize = nut->getTotalWordSize();
	      newbs = (s32) (m_constant.uval > wordsize ? wordsize : m_constant.uval);
	    }
	    break;
	  case Bool:
	    newbs = BITS_PER_BOOL;
	    break;
	  default:
	    assert(0);
	  };

	//use UTI with same base type and new bitsize:
	u32 enumStrIdx = m_state.m_pool.getIndexForDataString(UlamType::getUlamTypeEnumAsString(typEnum));
	UlamKeyTypeSignature newkey(enumStrIdx, newbs);
	UTI newType = m_state.makeUlamType(newkey, typEnum);
	setNodeType(newType);
      }
    return getNodeType();
  } //checkAndLabelType

  EvalStatus NodeTerminal::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav)
      return ERROR;

    if(nuti == Hzy)
      return NOTREADY;

    if(!m_state.isComplete(nuti))
      return NOTREADY;

    EvalStatus evs = NORMAL; //init ok
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
    UTI nuti = getNodeType();
    u32 wordsize = m_state.getUlamTypeByIndex(nuti)->getTotalWordSize();
    if(wordsize <= MAXBITSPERINT)
      return makeTerminalValue(uvarg, (u32) m_constant.uval, nuti);
    else if(wordsize <= MAXBITSPERLONG)
      return makeTerminalValueLong(uvarg, m_constant.uval, nuti);
    else
      assert(0);
    return ERROR;
  } //makeTerminalValue

  EvalStatus NodeTerminal::makeTerminalValue(UlamValue& uvarg, u32 data, UTI uti)
  {
    UlamValue rtnUV; //init to Nav error case
    EvalStatus evs = NORMAL; //init ok
    assert(m_state.isComplete(uti));
    UlamType * ut = m_state.getUlamTypeByIndex(uti);
    assert(ut->getBitSize() >= 0);

    ULAMTYPE etype = ut->getUlamTypeEnum();
    switch(etype)
      {
	// assumes val is in proper format for its type
      case Int:
	  rtnUV = UlamValue::makeImmediate(uti, (s32) data, m_state);
	  break;
      case Bool:
      case Unsigned:
      case Unary:
      case Bits:
	rtnUV = UlamValue::makeImmediate(uti, data, m_state);
	break;
      case Class:
	{
	  if(ut->getUlamClass() == UC_QUARK)
	    {
	      rtnUV = UlamValue::makeImmediate(uti, data, m_state);
	      break;
	    }
	}
      default:
	{
	  std::ostringstream msg;
	  msg << "Constant Type Unknown: ";
	  msg <<  m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  evs = ERROR;
	}
      };
    uvarg = rtnUV;
    return evs;
  } //makeTerminalValue

  EvalStatus NodeTerminal::makeTerminalValueLong(UlamValue& uvarg, u64 data, UTI uti)
  {
    UlamValue rtnUV; //init to Nav error case
    EvalStatus evs = NORMAL; //init ok
    assert(m_state.isComplete(uti));
    UlamType * ut = m_state.getUlamTypeByIndex(uti);
    assert(ut->getBitSize() >= 0);

    ULAMTYPE etype = ut->getUlamTypeEnum();
    switch(etype)
      {
	// assumes val is in proper format for its type
      case Int:
	rtnUV = UlamValue::makeImmediateLong(uti, (s64) data, m_state);
	break;
      case Bool:
      case Unsigned:
      case Unary:
      case Bits:
	rtnUV = UlamValue::makeImmediateLong(uti, data, m_state);
	break;
      case Class:
	{
	  if(ut->getUlamClass() == UC_QUARK)
	    {
	      assert(!ut->isScalar());
	      rtnUV = UlamValue::makeImmediateQuarkArrayLong(uti, data, ut->getTotalBitSize());
	      break;
	    }
	}
      default:
	{
	  std::ostringstream msg;
	  msg << "Constant Type Unknown: ";
	  msg <<  m_state.getUlamTypeNameBriefByIndex(uti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  evs = ERROR;
	}
      };
    uvarg = rtnUV;
    return evs;
  } //makeTerminalValueLong

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
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	msg << ", to fit into type: " << m_state.getUlamTypeNameBriefByIndex(fituti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	m_state.setGoAgain(); //since not an error
	return false;
      }
    if(nuti == Nav)
      {
	std::ostringstream msg;
	msg << "Constant is not-a-valid type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }
    if(nuti == Hzy)
      {
	std::ostringstream msg;
	msg << "Constant is not-a-valid type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	m_state.setGoAgain(); //since not an error
	return false;
      }

    u32 fwordsize = fit->getTotalWordSize();
    if(fwordsize <= MAXBITSPERINT) //32
      {
	rtnb = fitsInBits32(fituti);
      }
    else if(fwordsize <= MAXBITSPERLONG) //64
      {
	rtnb = fitsInBits64(fituti);
      }
    else
      {
	std::ostringstream msg;
	msg << "Not supported at this time, constant type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str() << ", to fit into type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(fituti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }
    return rtnb;
  } //fitsInBits

  //used during check and label for binary arith and compare ops that have a constant term
  bool NodeTerminal::fitsInBits32(UTI fituti)
  {
    bool rtnb = false;
    u32 rtnc = 0;
    UTI nuti = getNodeType(); //constant type
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nbitsize = nut->getBitSize();
    assert(nbitsize > 0);

    //convert forth and back, then compare.
    ULAMTYPE etype = nut->getUlamTypeEnum();
    switch(etype)
      {
      case Int:
	{
	  u32 cdata = convertForthAndBack((u32) m_constant.uval, fituti);
	  rtnc = _BinOpCompareEqEqInt32((u32) m_constant.uval, cdata, nbitsize);
	}
	break;
      case Unsigned:
	{
	  u32 cdata = convertForthAndBack((u32) m_constant.uval, fituti);
	  rtnc = _BinOpCompareEqEqUnsigned32((u32) m_constant.uval, cdata, nbitsize);
	}
	break;
      case Unary:
	{
	  //right-justify first
	  u32 jdata = _Unary32ToUnary32((u32) m_constant.uval, nbitsize, nbitsize);
	  u32 cdata = convertForthAndBack(jdata, fituti);
	  rtnc = _BinOpCompareEqEqUnary32(jdata, cdata, nbitsize);
	}
	break;
      case Bits:
	{
	  u32 cdata = convertForthAndBack((u32) m_constant.uval, fituti);
	  rtnc = _BinOpCompareEqEqBits32((u32) m_constant.uval, cdata, nbitsize);
	}
	break;
      case Bool:
	{
	  u32 jdata = _Bool32ToBool32((u32) m_constant.uval, nbitsize, nbitsize);
	  u32 cdata = convertForthAndBack(jdata, fituti);
	  rtnc = _BinOpCompareEqEqBool32(jdata, cdata, nbitsize);
	}
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Constant Type Unknown: ";
	  msg <<  m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	  msg << ", to fit into type: ";
	  msg << m_state.getUlamTypeNameBriefByIndex(fituti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      };

    rtnb = _Bool32ToCbool(rtnc, BITSPERBOOL);
    return rtnb;
  } //fitsInBits32

  //used during check and label for binary arith and compare ops that have a constant term
  bool NodeTerminal::fitsInBits64(UTI fituti)
  {
    bool rtnb = false;
    u64 rtnc = 0;
    UTI nuti = getNodeType(); //constant type
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nbitsize = nut->getBitSize();
    assert(nbitsize > 0);

    //convert forth and back, then compare.
    ULAMTYPE etype = nut->getUlamTypeEnum();
    switch(etype)
      {
      case Int:
	{
	  u64 cdata = convertForthAndBackLong(m_constant.uval, fituti);
	  rtnc = _BinOpCompareEqEqInt64(m_constant.uval, cdata, nbitsize);
	}
	break;
      case Unsigned:
	{
	  u64 cdata = convertForthAndBackLong(m_constant.uval, fituti);
	  rtnc = _BinOpCompareEqEqUnsigned64(m_constant.uval, cdata, nbitsize);
	}
	break;
      case Unary:
	{
	  //right-justify first
	  u64 jdata = _Unary64ToUnary64(m_constant.uval, nbitsize, nbitsize);
	  u64 cdata = convertForthAndBackLong(jdata, fituti);
	  rtnc = _BinOpCompareEqEqUnary64(jdata, cdata, nbitsize);
	}
	break;
      case Bits:
	{
	  u64 cdata = convertForthAndBackLong(m_constant.uval, fituti);
	  rtnc = _BinOpCompareEqEqBits64(m_constant.uval, cdata, nbitsize);
	}
	break;
      case Bool:
	{
	  u64 jdata = _Bool64ToBool64(m_constant.uval, nbitsize, nbitsize);
	  u64 cdata = convertForthAndBackLong(jdata, fituti);
	  rtnc = _BinOpCompareEqEqBool64(jdata, cdata, nbitsize);
	}
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Constant Type Unknown: ";
	  msg <<  m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	  msg << ", to fit into type: ";
	  msg << m_state.getUlamTypeNameBriefByIndex(fituti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      };

    rtnb = _Bool64ToCbool(rtnc, BITSPERBOOL);
    return rtnb;
  } //fitsInBits64

  u32 NodeTerminal::convertForthAndBack(const u32 data, UTI fituti)
  {
    UTI nuti = getNodeType();
    UlamValue uv;
    makeTerminalValue(uv, data, nuti);
    //first cast to fit type:
    bool fb1 = m_state.getUlamTypeByIndex(fituti)->cast(uv, fituti);
    //2nd cast back to node type:
    bool fb2 = m_state.getUlamTypeByIndex(nuti)->cast(uv, nuti);
    return ((fb1 && fb2) ? uv.getImmediateData(MAXBITSPERINT, m_state) : ~data);
  } //convertForthAndBack

  u64 NodeTerminal::convertForthAndBackLong(const u64 data, UTI fituti)
  {
    UTI nuti = getNodeType();
    UlamValue uv;
    makeTerminalValueLong(uv, data, nuti);
    //first cast to fit type:
    bool fb1 = m_state.getUlamTypeByIndex(fituti)->cast(uv, fituti);
    //2nd cast back to node type:
    bool fb2 = m_state.getUlamTypeByIndex(nuti)->cast(uv, nuti);
    return ((fb1 && fb2) ? uv.getImmediateDataLong() : ~data);
  } //convertForthAndBackLong

  //used during check and label for bitwise shift op that has a negative constant term
  bool NodeTerminal::isNegativeConstant()
  {
    bool rtnb = false;
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());
    if(nut->getUlamTypeEnum() == Int)
      {
	u32 wordsize = nut->getTotalWordSize();
	s32 nbitsize = nut->getBitSize();
	assert(nbitsize > 0);
	if(wordsize <= MAXBITSPERINT)
	  {
	    s32 sval = _Int32ToCs32((u32) m_constant.uval, (u32) nbitsize);
	    if(sval < 0)
	      rtnb = true;
	  }
	else if(wordsize <= MAXBITSPERLONG)
	  {
	    s64 sval = _Int64ToCs64(m_constant.uval, (u32) nbitsize);
	    if(sval < 0)
	      rtnb = true;
	  }
	else
	  assert(0);
      }
    return rtnb;
  } //isNegativeConstant

  // used during check and label for bitwise shift op that has a RHS constant term gt/ge 32;
  // since node is not the shiftee, unsigned/int distinction not pertinent
  // false is ok.
  bool NodeTerminal::isWordSizeConstant()
  {
    bool rtnb = false;
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());
    u32 wordsize = nut->getTotalWordSize();
    ULAMTYPE etype = nut->getUlamTypeEnum();
    if(etype == Int)
      {
	if(wordsize <= MAXBITSPERINT)
	  rtnb = (m_constant.sval >= MAXBITSPERINT);
	else if(wordsize <= MAXBITSPERLONG)
	  rtnb = (m_constant.sval >= MAXBITSPERLONG);
	else
	  assert(0);
      }
    else if(etype == Unsigned)
      {
	if(wordsize <= MAXBITSPERINT)
	rtnb = (m_constant.uval >= (u32) MAXBITSPERINT);
	else if(wordsize <= MAXBITSPERLONG)
	  rtnb = (m_constant.uval >= (u32) MAXBITSPERLONG);
	else
	  assert(0);
      }
    return rtnb;
  } //isWordSizeConstant

  void NodeTerminal::genCode(File * fp, UlamValue& uvpass)
  {
    UlamValue tv;
    AssertBool isNormal = (makeTerminalValue(tv) == NORMAL);
    assert(isNormal);
    // unclear to do this read or not; squarebracket not happy, or cast not happy ?
    genCodeReadIntoATmpVar(fp, tv);  //tv updated to Ptr with a tmpVar "slot"
    uvpass = tv;
  } //genCode

  void NodeTerminal::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    UlamValue tv;
    AssertBool isNormal = (makeTerminalValue(tv) == NORMAL);
    assert(isNormal);
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

    if(isNegativeConstant())
      {
	u32 wordsize = nut->getTotalWordSize();
	if(wordsize <= MAXBITSPERINT)
	  fp->write("(u32) ");
	else if(wordsize <= MAXBITSPERLONG)
	  fp->write("(u64) ");
	else
	  assert(0);
      }

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
	      msg << "Invalid signed constant <" << numstr.c_str() << ">, errno=";
	      msg << errno << ": " << strerror(errno);
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
	      msg << "Invalid unsigned constant <" << numstr.c_str() << ">, errno=";
	      msg << errno << ": " << strerror(errno);
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    }
	  else
	    rtnok = true;
	}
	break;
      case TOK_KW_TRUE:
	m_constant.uval = _CboolToBool64(true, m_state.getBitSize(getNodeType()));
	rtnok = true;
	break;
      case TOK_KW_FALSE:
	m_constant.uval = 0u;
	rtnok = true;
	break;
      case TOK_SQUOTED_STRING:
	{
	  std::string numstr = m_state.getTokenDataAsString(&tok);
	  assert(numstr.length() == 1);
	  m_constant.uval = (u8) numstr[0];
	  rtnok = true;
	}
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
      case TOK_SQUOTED_STRING:
	{
	  u32 uid = m_state.m_pool.getIndexForDataString("Unsigned");
	  UlamKeyTypeSignature key(uid, SIZEOFACHAR, NONARRAYSIZE, 0);
	  newType = m_state.makeUlamType(key, Unsigned);
	}
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

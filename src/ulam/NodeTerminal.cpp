#include <stdlib.h>
#include <errno.h>
#include "NodeTerminal.h"
#include "CompilerState.h"
#include "strto64.h"

namespace MFM {

  NodeTerminal::NodeTerminal(CompilerState & state) : Node(state), m_etyp(Hzy) {}

  NodeTerminal::NodeTerminal(const Token& tok, CompilerState & state) : Node(state), m_etyp(Hzy)
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
    m_etyp = state.getUlamTypeByIndex(utype)->getUlamTypeEnum();
    //uptocaller to set node location.
  }

  //cannot convert using utype at construction since bitsize may be UNKNOWN
  NodeTerminal::NodeTerminal(u64 val, UTI utype, CompilerState & state) : Node(state)
  {
    setNodeType(utype);
    m_constant.uval = val;
    m_etyp = state.getUlamTypeByIndex(utype)->getUlamTypeEnum();
    //uptocaller to set node location.
  }

  NodeTerminal::NodeTerminal(const NodeTerminal& ref) : Node(ref), m_etyp(ref.m_etyp), m_constant(ref.m_constant) { }

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
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    //assert(etyp == m_etyp); //true? fails as constant value in hazy template (e.g. t41222)
    if(etyp == Bool)
      fp->write((_Bool64ToCbool(m_constant.uval, nut->getBitSize()) ? "true" : "false"));
    else
      fp->write(getName());
  } //printPostfix

  const char * NodeTerminal::getName()
  {
    u32 id = getNameId();
    return m_state.m_pool.getDataAsString(id).c_str();
  } //getName

  u32 NodeTerminal::getNameId()
  {
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nbitsize = nut->getBitSize();
    assert(nbitsize >= 0);
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    std::ostringstream num;
    switch(etyp)
      {
      case Int:
	if(nut->getTotalWordSize() <= MAXBITSPERINT)
	  {
	    s32 sval = _Int32ToCs32((u32) m_constant.uval, nbitsize);
	    num << sval;
	  }
	else
	  {
	    s64 sval = _Int64ToCs64(m_constant.uval, nbitsize);
	    num << ToSignedDecimal(sval);
	  }
	break;
      case Bool:
      case Unsigned:
      case Unary:
      case Bits:
	// NO CASTING NEEDED, assume saved in its ulam-native format
	if(nut->getTotalWordSize() <= MAXBITSPERINT)
	  num << (u32) m_constant.uval << "u";
	else
	  num << ToUnsignedDecimal(m_constant.uval);
	break;
      case String:
	num << m_state.getDataAsFormattedUserString(m_constant.uval);
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
    return id;
  } //getNameId

  const std::string NodeTerminal::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

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
    if(m_state.isAltRefType(newType))
      return CAST_BAD; //t3965

    UTI nuti = getNodeType();
    ULAMTYPE ntypEnum = m_state.getUlamTypeByIndex(nuti)->getUlamTypeEnum();
    UlamType * newut = m_state.getUlamTypeByIndex(newType);
    ULAMTYPE typEnum = newut->getUlamTypeEnum();

    //special cases: not a matter of fitting
    if((typEnum == Bool) || (ntypEnum == Bool) || (typEnum == UAtom) || (typEnum == Class) || (typEnum == Void))
      return m_state.getUlamTypeByIndex(newType)->safeCast(nuti);

    //special case: FROM Bits, not just a matter of fitting, e.g. logical shifts (t3850)
    if((ntypEnum == Bits) && (typEnum != Bits))
      return m_state.getUlamTypeByIndex(newType)->safeCast(nuti);

    //special case: FROM/TO String, not just a matter of fitting (t41164)
    if((ntypEnum == String) ^ (typEnum == String))
      return m_state.getUlamTypeByIndex(newType)->safeCast(nuti);

    //for non-bool terminal check for complete types and arrays before fits.
    FORECAST scr = m_state.getUlamTypeByIndex(newType)->UlamType::safeCast(nuti);
    if(scr != CAST_CLEAR)
      return scr;

    return fitsInBits(newType) ? CAST_CLEAR : CAST_BAD;
  } //safeToCastTo

  UTI NodeTerminal::checkAndLabelType()
  {
    //numeric tokens are implicitily 64-bits
    // o.w. 64-bit constants got truncated; but no 32-bit sign extension.
    s32 bs = MAXBITSPERLONG;

    //update bitsize to fit the constant value
    if(!m_state.okUTItoContinue(getNodeType()))
      {
	s32 newbs = bs;
	switch(m_etyp)
	  {
	  case Int:
	    {
	      s64 sval = _Int64ToCs64(m_constant.uval, bs);
	      newbs = (s32) (_getLogBase2Long(UABS64(sval)) + 1 + 1); //fits into signed
	      newbs = CLAMP<s32>(1, MAXBITSPERLONG, newbs);
	    }
	    break;
	  case Bits:
	  case Unsigned:
	    newbs = (s32) _getLogBase2Long(m_constant.uval) + 1; //fits into unsigned
	    break;
	  case Unary:
	    newbs = (s32) (m_constant.uval > MAXBITSPERLONG ? MAXBITSPERLONG : m_constant.uval);
	    break;
	  case Bool:
	    newbs = BITS_PER_BOOL;
	    break;
	  case String: //always ok, 32 bits
	  default:
	    m_state.abortUndefinedUlamPrimitiveType();
	  };

	//use UTI with same base type and new bitsize:
	u32 enumStrIdx = m_state.m_pool.getIndexForDataString(UlamType::getUlamTypeEnumAsString(m_etyp));
	UlamKeyTypeSignature newkey(enumStrIdx, newbs);
	UTI newType = m_state.makeUlamType(newkey, m_etyp, UC_NOTACLASS);
	setNodeType(newType);
      }

    if(getNodeType() == Hzy)
      m_state.setGoAgain();
    return getNodeType();
  } //checkAndLabelType

  EvalStatus NodeTerminal::eval()
  {
    UTI nuti = getNodeType();
    if(nuti == Nav) return evalErrorReturn();

    if(nuti == Hzy) return evalStatusReturnNoEpilog(NOTREADY);

    if(!m_state.isComplete(nuti)) return evalStatusReturnNoEpilog(NOTREADY);

    EvalStatus evs = NORMAL; //init ok
    evalNodeProlog(0); //new current frame pointer

    UlamValue rtnUV;
    evs = makeTerminalValue(rtnUV);

    //copy result UV to stack, -1 relative to current frame pointer
    if(evs != NORMAL) return evalStatusReturn(evs);

    Node::assignReturnValueToStack(rtnUV);

    evalNodeEpilog();
    return NORMAL;
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
      m_state.abortGreaterThanMaxBitsPerLong();
    return ERROR;
  } //makeTerminalValue

  EvalStatus NodeTerminal::makeTerminalValue(UlamValue& uvarg, u32 data, UTI uti)
  {
    UlamValue rtnUV; //init to Nav error case
    EvalStatus evs = NORMAL; //init ok
    assert(m_state.isComplete(uti));
    UlamType * ut = m_state.getUlamTypeByIndex(uti);
    assert(ut->getBitSize() >= 0);

    ULAMTYPE etyp = ut->getUlamTypeEnum();
    switch(etyp)
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
	rtnUV = UlamValue::makeImmediate(uti, data, m_state);
	break;
      case String:
	rtnUV = UlamValue::makeImmediate(uti, data, m_state);
	break;
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

    ULAMTYPE etyp = ut->getUlamTypeEnum();
    switch(etyp)
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
	rtnUV = UlamValue::makeImmediateLongClass(uti, data, ut->getTotalBitSize());
	break;
      case String:
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

  void NodeTerminal::makeTerminalPassForCodeGen(UVPass& uvpass)
  {
    UTI nuti = getNodeType();
    u32 tid = getNameId();

    //TMPSTORAGE is TERMINAL, and VarNum is zero.
    uvpass = UVPass::makePass(0, TERMINAL, nuti, m_state.determinePackable(nuti), m_state, 0, tid);
  } //makeTerminalValueForCodeGen

  //used during check and label for binary arith and compare ops that have a constant term
  bool NodeTerminal::fitsInBits(UTI destuti)
  {
    bool rtnb = false;
    UTI nuti = getNodeType(); //constant type
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    UlamType * dest = m_state.getUlamTypeByIndex(destuti);
    if(!dest->isComplete())
      {
	std::ostringstream msg;
	msg << "Unknown size!! constant type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	msg << ", to fit into type: " << m_state.getUlamTypeNameBriefByIndex(destuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	return false; //t3461
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
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
	return false;
      }

    u32 nwordsize = nut->getTotalWordSize(); //from
    u32 fwordsize = dest->getTotalWordSize(); //to
    if(nwordsize > fwordsize)
      {
	std::ostringstream msg;
	msg << "Word size incompatible. Not supported at this time, constant type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str() << ", to fit into type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(destuti).c_str();
	fwordsize = nwordsize; //t3851
      }

    if(fwordsize <= MAXBITSPERINT) //32
      {
	rtnb = fitsInBits32(destuti);
      }
    else if(fwordsize <= MAXBITSPERLONG) //64
      {
	rtnb = fitsInBits64(destuti);
      }
    else
      {
	std::ostringstream msg;
	msg << "Not supported at this time, constant type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str() << ", to fit into type: ";
	msg << m_state.getUlamTypeNameBriefByIndex(destuti).c_str();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }
    return rtnb;
  } //fitsInBits

  //used during check and label for binary arith and compare ops that have a constant term
  bool NodeTerminal::fitsInBits32(UTI destuti)
  {
    bool rtnb = false;
    u32 rtnc = 0;
    UTI nuti = getNodeType(); //constant type
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nbitsize = nut->getBitSize();
    assert(nbitsize > 0);

    //convert forth and back, then compare.
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	{
	  u32 cdata = convertForthAndBack((u32) m_constant.uval, destuti);
	  rtnc = _BinOpCompareEqEqInt32((u32) m_constant.uval, cdata, nbitsize);
	}
	break;
      case Unsigned:
	{
	  u32 cdata = convertForthAndBack((u32) m_constant.uval, destuti);
	  rtnc = _BinOpCompareEqEqUnsigned32((u32) m_constant.uval, cdata, nbitsize);
	}
	break;
      case Unary:
	{
	  //right-justify first
	  u32 jdata = _Unary32ToUnary32((u32) m_constant.uval, nbitsize, nbitsize);
	  u32 cdata = convertForthAndBack(jdata, destuti);
	  rtnc = _BinOpCompareEqEqUnary32(jdata, cdata, nbitsize);
	}
	break;
      case Bits:
	{
	  u32 cdata = convertForthAndBack((u32) m_constant.uval, destuti);
	  rtnc = _BinOpCompareEqEqBits32((u32) m_constant.uval, cdata, nbitsize);
	}
	break;
      case Bool:
	{
	  u32 jdata = _Bool32ToBool32((u32) m_constant.uval, nbitsize, nbitsize);
	  u32 cdata = convertForthAndBack(jdata, destuti);
	  rtnc = _BinOpCompareEqEqBool32(jdata, cdata, nbitsize);
	}
	break;
      case String:
	rtnc = 1; //true since 32-bit index
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Constant Type Unknown: ";
	  msg <<  m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	  msg << ", to fit into type: ";
	  msg << m_state.getUlamTypeNameBriefByIndex(destuti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      };

    rtnb = _Bool32ToCbool(rtnc, BITSPERBOOL);
    return rtnb;
  } //fitsInBits32

  //used during check and label for binary arith and compare ops that have a constant term
  bool NodeTerminal::fitsInBits64(UTI destuti)
  {
    bool rtnb = false;
    u64 rtnc = 0;
    UTI nuti = getNodeType(); //constant type
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32 nbitsize = nut->getBitSize();
    assert(nbitsize > 0);

    //convert forth and back, then compare.
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	{
	  u64 cdata = convertForthAndBackLong(m_constant.uval, destuti);
	  rtnc = _BinOpCompareEqEqInt64(m_constant.uval, cdata, nbitsize);
	}
	break;
      case Unsigned:
	{
	  u64 cdata = convertForthAndBackLong(m_constant.uval, destuti);
	  rtnc = _BinOpCompareEqEqUnsigned64(m_constant.uval, cdata, nbitsize);
	}
	break;
      case Unary:
	{
	  //right-justify first
	  u64 jdata = _Unary64ToUnary64(m_constant.uval, nbitsize, nbitsize);
	  u64 cdata = convertForthAndBackLong(jdata, destuti);
	  rtnc = _BinOpCompareEqEqUnary64(jdata, cdata, nbitsize);
	}
	break;
      case Bits:
	{
	  u64 cdata = convertForthAndBackLong(m_constant.uval, destuti);
	  rtnc = _BinOpCompareEqEqBits64(m_constant.uval, cdata, nbitsize);
	}
	break;
      case Bool:
	{
	  u64 jdata = _Bool64ToBool64(m_constant.uval, nbitsize, nbitsize);
	  u64 cdata = convertForthAndBackLong(jdata, destuti);
	  rtnc = _BinOpCompareEqEqBool64(jdata, cdata, nbitsize);
	}
	break;
      case String:
	rtnc = 1; //true since 32-bit index
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Constant Type Unknown: ";
	  msg <<  m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	  msg << ", to fit into type: ";
	  msg << m_state.getUlamTypeNameBriefByIndex(destuti).c_str();
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      };

    rtnb = _Bool64ToCbool(rtnc, BITSPERBOOL);
    return rtnb;
  } //fitsInBits64

  u32 NodeTerminal::convertForthAndBack(const u32 data, UTI destuti)
  {
    UTI nuti = getNodeType();
    UlamValue uv;
    makeTerminalValue(uv, data, nuti);

    //first cast to dest type:
    bool fb1 = m_state.getUlamTypeByIndex(destuti)->cast(uv, destuti);
    //2nd cast back to node type:
    bool fb2 = m_state.getUlamTypeByIndex(nuti)->cast(uv, nuti);
    return ((fb1 && fb2) ? uv.getImmediateData(MAXBITSPERINT, m_state) : ~data);
  } //convertForthAndBack

  u64 NodeTerminal::convertForthAndBackLong(const u64 data, UTI destuti)
  {
    UTI nuti = getNodeType();
    UlamValue uv;
    makeTerminalValueLong(uv, data, nuti);
    //first cast to dest type:
    bool fb1 = m_state.getUlamTypeByIndex(destuti)->cast(uv, destuti);
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
	  m_state.abortGreaterThanMaxBitsPerLong();
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
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    if(etyp == Int)
      {
	if(wordsize <= MAXBITSPERINT)
	  rtnb = (m_constant.sval >= MAXBITSPERINT);
	else if(wordsize <= MAXBITSPERLONG)
	  rtnb = (m_constant.sval >= MAXBITSPERLONG);
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }
    else if(etyp == Unsigned)
      {
	if(wordsize <= MAXBITSPERINT)
	rtnb = (m_constant.uval >= (u32) MAXBITSPERINT);
	else if(wordsize <= MAXBITSPERLONG)
	  rtnb = (m_constant.uval >= (u32) MAXBITSPERLONG);
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }
    return rtnb;
  } //isWordSizeConstant

  void NodeTerminal::genCode(File * fp, UVPass& uvpass)
  {
    UVPass tvpass;
    makeTerminalPassForCodeGen(tvpass);

    // unclear to do this read or not; squarebracket not happy, or cast not happy ?
    genCodeReadIntoATmpVar(fp, tvpass);  //tv updated to a tmpVar "num"
    uvpass = tvpass;
  } //genCode

  void NodeTerminal::genCodeToStoreInto(File * fp, UVPass& uvpass)
  {
    makeTerminalPassForCodeGen(uvpass);
  } //genCodeToStoreInto

  // reads into a tmp var
  // (for BitVector use Node::genCodeConvertATmpVarIntoBitVector)
  void NodeTerminal::genCodeReadIntoATmpVar(File * fp, UVPass & uvpass)
  {
    // into tmp storage first, in case of casts
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32	tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indentUlamCode(fp);
    fp->write("const ");

    fp->write(nut->getTmpStorageTypeAsString().c_str());
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum, TMPREGISTER).c_str());

    fp->write(" = ");

    u32 wordsize = nut->getTotalWordSize();
    if(isNegativeConstant())
      {
	if(wordsize <= MAXBITSPERINT)
	  fp->write("(u32) ");
	else if(wordsize <= MAXBITSPERLONG)
	  fp->write("(u64) ");
	else
	  m_state.abortGreaterThanMaxBitsPerLong();
      }

    if(UlamType::compareForString(nuti, m_state) == UTIC_SAME)
      {
	u32 sidx = (m_constant.uval & STRINGIDXMASK);
	assert((sidx > 0));
	//String, String array or array item (t3929, t3950)
	fp->write_decimal_unsigned(sidx);
	fp->write("u; //user string pool index for ");
      }

    fp->write(getName());
    fp->write(";"); GCNL;

    //substitute Ptr for uvpass to contain the tmpVar number;
    //save id of constant string in Ptr;
    uvpass = UVPass::makePass(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0, 0);  //POS 0 rightjustified (atom-based);
    m_state.clearCurrentObjSymbolsForCodeGen(); //missing or just needed by NodeTerminalProxy?
  } //genCodeReadIntoATmpVar

  bool NodeTerminal::setConstantValue(const Token& tok)
  {
    bool rtnok = false;
    errno = 0; //to check for ERANGE
    switch(tok.m_type)
      {
      case TOK_NUMBER_SIGNED:
	{
	  std::string numstr = m_state.getTokenDataAsString(tok);
	  const char * numlist = numstr.c_str();
	  const char * errMsg;

	  m_constant.sval = strtos64(numlist, errMsg);
	  if(errMsg)
	    {
	      std::ostringstream msg;
	      msg << "Invalid signed constant <" << numstr.c_str() << ">: ";
              msg << errMsg;
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    }
	  else
	    rtnok = true;
	}
	break;
      case TOK_NUMBER_UNSIGNED:
	{
	  std::string numstr = m_state.getTokenDataAsString(tok);
	  const char * numlist = numstr.c_str();
	  const char * errMsg;

	  m_constant.uval = strtou64(numlist, errMsg);
	  if(errMsg)
	    {
	      std::ostringstream msg;
	      msg << "Invalid unsigned constant <" << numstr.c_str() << ">: ";
              msg << errMsg;
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
	  std::string numstr = m_state.getTokenDataAsString(tok);
	  assert(numstr.length() == 1);
	  m_constant.uval = (u8) numstr[0];
	  rtnok = true;
	}
	break;
      case TOK_DQUOTED_STRING:
	{
	  u32 stringidx = m_state.m_upool.getIndexForDataString(m_state.m_tokenupool.getDataAsString(tok.m_dataindex));
	  m_constant.uval = stringidx; //global user string pool (ulam-4)
	  rtnok = true;
	}
	break;
      default:
	{
	    std::ostringstream msg;
	    msg << "Token is neither a number, nor a boolean: <";
	    msg <<  m_state.getTokenDataAsString(tok).c_str() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	}
      };
    return rtnok;
  } //setConstantValue

  UTI NodeTerminal::setConstantTypeForNode(const Token& tok)
  {
    UTI newType = Nav; //init
    switch(tok.m_type)
      {
      case TOK_NUMBER_SIGNED:
	m_etyp = Int;
	newType = Hzy;
	break;
      case TOK_NUMBER_UNSIGNED:
	m_etyp = Unsigned;
	newType = Hzy;
	break;
      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
	m_etyp = Bool;
	newType = Bool;
	break;
      case TOK_SQUOTED_STRING:
	{
	  u32 uid = m_state.m_pool.getIndexForDataString("Unsigned");
	  UlamKeyTypeSignature key(uid, SIZEOFACHAR, NONARRAYSIZE, 0);
	  newType = m_state.makeUlamType(key, Unsigned, UC_NOTACLASS);
	}
	m_etyp = Unsigned;
	break;
      case TOK_DQUOTED_STRING:
	newType = String;
	m_etyp = String;
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Token not a number or boolean: <";
	  msg << m_state.getTokenDataAsString(tok).c_str() << ">";
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      };
    setNodeType(newType);
    Node::setStoreIntoAble(TBOOL_FALSE);
    return newType;
  } //setConstantTypeForNode

} //end MFM

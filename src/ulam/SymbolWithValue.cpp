#include <iomanip>
#include "SymbolWithValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolWithValue::SymbolWithValue(const Token& id, UTI utype, CompilerState & state) : Symbol(id, utype, state), m_isReady(false), m_hasInitVal(false), m_isReadyInitVal(false), m_classParameter(false), m_classArgument(Nouti), m_declnno(0) { }

  SymbolWithValue::SymbolWithValue(const SymbolWithValue & sref) : Symbol(sref), m_isReady(sref.m_isReady), m_hasInitVal(sref.m_hasInitVal), m_isReadyInitVal(false), m_classParameter(false), m_classArgument(Nouti), m_declnno(sref.m_declnno)
  {
    if((sref.m_classArgument != Nouti) || sref.m_classParameter)
      m_classArgument = m_state.getCompileThisIdx(); //t41229
    //classArg is copying from a classParameter
    m_constantValue = sref.m_constantValue;
    m_initialValue = sref.m_initialValue;

  }

  SymbolWithValue::SymbolWithValue(const SymbolWithValue & sref, bool keepType) : Symbol(sref, keepType), m_isReady(sref.m_isReady), m_hasInitVal(sref.m_hasInitVal), m_isReadyInitVal(false), m_classParameter(false), m_classArgument(sref.m_classArgument), m_declnno(sref.m_declnno)
  {
    //classArg is copying from a classParameter
    m_constantValue = sref.m_constantValue;
    m_initialValue = sref.m_initialValue;
  }

  SymbolWithValue::~SymbolWithValue()
  { }


  bool SymbolWithValue::isClassParameter()
  {
    return m_classParameter;
  }

  void SymbolWithValue::setClassParameterFlag()
  {
    m_classParameter = true;
  }

  bool SymbolWithValue::isClassArgument()
  {
    return (m_classArgument != Nouti);
  }

  UTI SymbolWithValue::getClassArgumentOfClassInstance()
  {
    return m_classArgument;
  }

  void SymbolWithValue::setClassArgumentFlag(UTI cuti)
  {
    m_classArgument = cuti;
  }

  u32 SymbolWithValue::getPosOffset()
  {
    return 0; //always zero?
  }

  bool SymbolWithValue::isPosOffsetReliable()
  {
    return false; //non-data member.
  }

  bool SymbolWithValue::isReady()
  {
    return m_isReady; //constant value
  }

  bool SymbolWithValue::getLexValue(std::string& vstr)
  {
    if(!isReady())
      return false;

    u64 constantval = 0;
    AssertBool gotVal = getValue(constantval);
    assert(gotVal);

    return convertValueToALexString(constantval, getUlamTypeIdx(), vstr, m_state);
  } //getLexValue

  bool SymbolWithValue::getValue(u32& val)
  {
    if(isReady())
      {
	u32 len = m_state.getTotalBitSize(getUlamTypeIdx());
	assert(len <= MAXBITSPERINT);
	val = m_constantValue.Read(0u, len); //return value
	return true;
      }
    return false;
  }

  bool SymbolWithValue::getValue(u64& val)
  {
    if(isReady())
      {
	u32 len = m_state.getTotalBitSize(getUlamTypeIdx());
	if(len > MAXBITSPERLONG)
	  m_state.abortGreaterThanMaxBitsPerLong();
	val = m_constantValue.ReadLong(0u, len); //return value
	return true;
      }
    return false;
  }

  bool SymbolWithValue::getValue(BV8K& val)
  {
    if(isReady())
      {
	val = m_constantValue; //return value
	return true;
      }
    return false;
  }

  void SymbolWithValue::setValue(const BV8K& val)
  {
    u32 len = m_state.getUlamTypeByIndex(getUlamTypeIdx())->getSizeofUlamType();
    val.CopyBV(0u, 0u, len, m_constantValue); //frompos, topos, len, destBV
    m_isReady = true;
  }

  bool SymbolWithValue::getArrayItemValue(u32 item, u32& rtnitem)
  {
    if(isReady())
      {
	UTI suti = getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	u32 bs = sut->getBitSize();
	s32 arrsize = sut->getArraySize();
	assert(bs <= MAXBITSPERINT);
	assert((arrsize >= 0) && (item < (u32) arrsize));
	//no casting!
	rtnitem = m_constantValue.Read(item * bs, bs);
	return true;
      }
    return false;
  }

  bool SymbolWithValue::getArrayItemValue(u32 item, u64& rtnitem)
  {
    if(isReady())
      {
	UTI suti = getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	u32 bs = sut->getBitSize();
	s32 arrsize = sut->getArraySize();
	assert(bs <= MAXBITSPERLONG);
	assert((arrsize >= 0) && (item < (u32) arrsize));
	//no casting!
	rtnitem = m_constantValue.ReadLong(item * bs, bs);
	return true;
      }
    return false;
  }

  bool SymbolWithValue::hasInitValue()
  {
    return m_hasInitVal;
  }

  bool SymbolWithValue::getInitValue(u32& val)
  {
    assert(hasInitValue());
    if(isInitValueReady())
      {
	u32 len = m_state.getTotalBitSize(getUlamTypeIdx());
	val = m_initialValue.Read(0u, len); //return value
	return true;
      }
    return false;
  }

  bool SymbolWithValue::getInitValue(u64& val)
  {
    assert(hasInitValue());
    if(isInitValueReady())
      {
	u32 len = m_state.getTotalBitSize(getUlamTypeIdx());
	val = m_initialValue.ReadLong(0u, len); //return value
	return true;
      }
    return false;
  }

  bool SymbolWithValue::getInitValue(BV8K& val)
  {
    assert(hasInitValue());
    if(isInitValueReady())
      {
	u32 len = m_state.getUlamTypeByIndex(getUlamTypeIdx())->getSizeofUlamType();
	m_initialValue.CopyBV(0u, 0u, len, val);
	return true;
      }
    return false;
  }

  void SymbolWithValue::setInitValue(const BV8K& val)
  {
    u32 len = m_state.getUlamTypeByIndex(getUlamTypeIdx())->getSizeofUlamType();
    val.CopyBV(0u, 0u, len, this->m_initialValue); //frompos, topos, len, destBV
    m_hasInitVal = true;
    m_isReadyInitVal = true;
  }

  bool SymbolWithValue::isInitValueReady()
  {
    return m_isReadyInitVal;
  }

  void SymbolWithValue::setHasInitValue()
  {
    m_hasInitVal = true;
    m_isReadyInitVal = false;
  }

  bool SymbolWithValue::getArrayItemInitValue(u32 item, u32& rtnitem)
  {
    assert(hasInitValue());
    if(isInitValueReady())
      {
	UTI suti = getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	u32 bs = sut->getBitSize();
	s32 arrsize = sut->getArraySize();
	assert(bs <= MAXBITSPERINT);
	assert((arrsize >= 0) && (item < (u32) arrsize));
	//no casting!
	rtnitem = m_initialValue.Read(item * bs, bs);
	return true;
      }
    return false;
  }

  bool SymbolWithValue::getArrayItemInitValue(u32 item, u64& rtnitem)
  {
    assert(hasInitValue());
    if(isInitValueReady())
      {
	UTI suti = getUlamTypeIdx();
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	u32 bs = sut->getBitSize();
	s32 arrsize = sut->getArraySize();
	assert(bs <= MAXBITSPERLONG);
	assert((arrsize >= 0) && (item < (u32) arrsize));
	//no casting!
	rtnitem = m_initialValue.ReadLong(item * bs, bs);
	return true;
      }
    return false;
  }

  bool SymbolWithValue::getValueReadyToPrint(u32 & uv)
  {
    bool oktoprint = true;
    if(isReady())
      getValue(uv);
    else if(hasInitValue() && isInitValueReady())
      getInitValue(uv);
    else
      oktoprint = false;
    return oktoprint;
  } //getValueReadyToPrint (helper)

  bool SymbolWithValue::getValueReadyToPrint(u64 & dv)
  {
    bool oktoprint = true;
    if(isReady())
      getValue(dv);
    else if(hasInitValue() && isInitValueReady())
      getInitValue(dv);
    else
      oktoprint = false;
    return oktoprint;
  } //getValueReadyToPrint (helper)

  bool SymbolWithValue::getValueReadyToPrint(BV8K & bv)
  {
    bool oktoprint = true;
    if(isReady())
      getValue(bv);
    else if(hasInitValue() && isInitValueReady())
      getInitValue(bv);
    else
      oktoprint = false;
    return oktoprint;
  } //getValueReadyToPrint (helper)

  bool SymbolWithValue::foldConstantExpression()
  {
    return true; //stub
  }

  void SymbolWithValue::printPostfixValue(File * fp)
  {
    if(m_state.isScalar(getUlamTypeIdx()))
      printPostfixValueScalar(fp);
    else
      printPostfixValueArray(fp);
  } //printPostfixValue

  void SymbolWithValue::printPostfixValueScalar(File * fp)
  {
    u64 val = 0;
    bool oktoprint = getValueReadyToPrint(val);

    if(oktoprint)
      {
	UTI tuti = getUlamTypeIdx();
	UlamType * tut = m_state.getUlamTypeByIndex(tuti);
	u32 twordsize =  tut->getTotalWordSize(); //must be commplete
	s32 tbs = tut->getBitSize();
	ULAMTYPE etyp = tut->getUlamTypeEnum();

	switch(etyp)
	  {
	  case Int:
	    {
	      if(twordsize <= MAXBITSPERINT)
		{
		  s32 sval = _Int32ToCs32((u32) val, tbs);
		  fp->write_decimal(sval);
		}
	      else if(twordsize <= MAXBITSPERLONG)
		{
		  s64 sval = _Int64ToCs64(val, tbs);
		  fp->write_decimal_long(sval);
		}
	      else
		m_state.abortGreaterThanMaxBitsPerLong();
	    }
	    break;
	  case Bool:
	    {
	      bool bval = _Bool64ToCbool(val, tbs);
	      if(bval)
		fp->write("true");
	      else
		fp->write("false");
	    }
	    break;
	  case Unary:
	  case Unsigned:
	  case Bits:
	    {
	      // NO CASTING NEEDED, assume saved in its ulam-native format
	      if( tbs <= MAXBITSPERINT)
		fp->write_decimal_unsigned(val);
	      else if( tbs <= MAXBITSPERLONG)
		fp->write_decimal_unsignedlong(val);
	      else
		m_state.abortGreaterThanMaxBitsPerLong(); //TBD > 64
	      fp->write("u");
	    }
	    break;
	  case String:
	    // scalar strings generate comments (output value rather than index) e.g. t3951,2
	    fp->write(m_state.getDataAsFormattedUserString(val).c_str());
	    break;
	  default:
	    m_state.abortUndefinedUlamPrimitiveType();
	  };
      }
    else
      fp->write("NONREADYCONST");
  } //printPostfixValueScalar

  void SymbolWithValue::printPostfixValueArray(File * fp)
  {
    BV8K dval;
    bool oktoprint = getValueReadyToPrint(dval);

    if(!oktoprint)
      {
	fp->write("NONREADYCONSTARRAY");
	return;
      }

    UTI tuti = getUlamTypeIdx();
    UlamType * tut = m_state.getUlamTypeByIndex(tuti);
    s32 tbs = tut->getTotalBitSize();
    if(tbs == 0)
      {
	fp->write("{ }");
	return; //nothing to do
      }

    //required as hex for String arrays too (needed for initialization) (t3974)
    //like the code generated in CS::genCodeClassDefaultConstantArray
    std::string dhex;
    bool nonZero = SymbolWithValue::getHexValueAsString(tbs, dval, dhex);

    //short-circuit if all zeros
    if(!nonZero)
      {
	if(tut->getUlamTypeEnum() == String) //t3953
	  m_state.abortShouldntGetHere();

	fp->write("{ 0 }");
	return; //nothing else to do
      }

    fp->write("{ ");
    fp->write(dhex.c_str());
    fp->write(" }");
  } //printPostfixValueArray

  void SymbolWithValue::printPostfixValueArrayStringAsComment(File * fp)
  {
    BV8K dval;
    bool oktoprint = getValueReadyToPrint(dval);

    UTI tuti = getUlamTypeIdx();
    UlamType * tut = m_state.getUlamTypeByIndex(tuti);
    bool isString = (tut->getUlamTypeEnum() == String); //t3953
    assert(isString);

    if(!oktoprint)
      {
	fp->write("// ");
	fp->write(getMangledName().c_str());
	fp->write(": NONREADYCONSTARRAY OF STRINGS"); GCNL;
	return;
      }

    //like the code generated in CS::genCodeClassDefaultConstantArray
    u32 uvals[ARRAY_LEN8K];
    dval.ToArray(uvals);

    u32 nwords = tut->getTotalNumberOfWords();

    //indented comments of string value items (one per line); e.g. t3953,4
    for(u32 w = 0; w < nwords; w++)
      {
	m_state.indent(fp);
	fp->write("// ");
	fp->write("[");
	fp->write_decimal_unsigned(w);
	fp->write("] = ");
	fp->write(m_state.getDataAsFormattedUserString(uvals[w]).c_str());
	fp->write("\n");
      }
    m_state.indent(fp);
    fp->write("// = ");
    fp->write(getMangledName().c_str());
    fp->write("[");
    fp->write_decimal_unsigned(nwords);
    fp->write("]");
    GCNL;
  } //printPostfixValueArrayStringAsComment

 bool SymbolWithValue::getValueAsHexString(std::string& vstr)
  {
    BV8K dval;
    bool oktoprint = getValueReadyToPrint(dval);

    if(!oktoprint) return false;

    UTI tuti = getUlamTypeIdx();
    UlamType * tut = m_state.getUlamTypeByIndex(tuti);
    s32 tbs = tut->getTotalBitSize();

    if(tbs == 0)
      {
	vstr = "0"; //no "0x" hex indicates empty array
	return true;
      }

    s32 tnybbles = int(tbs/4); //4 bits per nybble (one Hex digit)

    std::ostringstream ostream;
    //ostream << "0x"; //no "0x"

    for(s32 i = 0; i < tnybbles; i++)
      {
	ostream << std::hex << dval.Read(i, 4); //per bit?
      }

    s32 remainder = tbs - tnybbles*4;
    if(remainder > 0)
      {
	//shift left for continguous bits
	ostream << std::hex << ((dval.Read(tnybbles*4, remainder)) << (4 - remainder));
      }

    vstr = ostream.str();
    return true;
  } //getValueAsHexString

  bool SymbolWithValue::getArrayValueAsString(std::string& vstr)
  {
    BV8K dval;
    bool oktoprint = getValueReadyToPrint(dval);

    if(!oktoprint) return false;

    UTI tuti = getUlamTypeIdx();
    UlamType * tut = m_state.getUlamTypeByIndex(tuti);

    if(tut->getTotalBitSize() == 0)
      {
	vstr = "10"; //empty array
	return true;
      }

    //get the number of bits for this type into u64
    // convert to a lex-number as a string, applying type specifics
    // return the completed string of all the array values in arg vstr.
    std::ostringstream tovstr;
    s32 bs = tut->getBitSize();
    s32 arraysize = tut->getArraySize();
    for(s32 i=0; i < arraysize; i++)
      {
	u64 thisval = dval.ReadLong(i * bs, bs); //pos and len
	std::string str;
	convertValueToALexString(thisval, tuti, str, m_state);
	tovstr << str;
      }
    vstr = tovstr.str();
    return true;
  } //getArrayValueAsString

  bool SymbolWithValue::getScalarValueAsString(std::string& vstr)
  {
    u64 constantval;
    bool oktoprint = getValueReadyToPrint(constantval);

    if(!oktoprint) return false;

    return SymbolWithValue::convertValueToAPrettyString(constantval, getUlamTypeIdx(), vstr, m_state);
  } //getScalarValueAsString

  //return false if not ready, o.w. true; rtnstr updated
  // NodeListArrayInitialization::buildClassArrayItemInitialValue
  // expanded data to fill atom-size per element; left space for MFM
  // Element Type. Handles scalar and array.
  bool SymbolWithValue::getClassValueAsHexString(std::string& rtnstr)
  {
    BV8K dval;
    bool oktoprint = getValueReadyToPrint(dval);

    if(!oktoprint) return false;

    u32 totlen = m_state.getUlamTypeByIndex(getUlamTypeIdx())->getSizeofUlamType();
    SymbolWithValue::getHexValueAsString(totlen, dval, rtnstr);
    return true;
  } //getClassValueAsHexString

  //static: return false if all zeros, o.w. true; rtnstr updated
  bool SymbolWithValue::getLexValueAsString(u32 ntotbits, const BV8K& bval, std::string& rtnstr)
  {
    //like the code generated in CS::genCodeClassDefaultConstantArray
    u32 uvals[ARRAY_LEN8K];
    bval.ToArray(uvals);

    u32 nwords = (ntotbits + 31)/MAXBITSPERINT;

    //short-circuit if all zeros
    bool isZero = true;
    s32 x = nwords - 1;
    for(; x >= 0; x--)
      {
	if(uvals[x] != 0)
	  {
	    isZero = false;
	    break;
	  }
      }

    if(isZero)
      {
	rtnstr = "10"; //all zeros
	return false;
      }

    //compress to output only non-zero uval items (left-justified)
    nwords = (u32) x + 1;

    std::ostringstream ostream;
    //output number of non-zero words first
    ostream << ToLeximitedNumber(nwords);

    for(u32 i = 0; i < nwords; i++)
      {
	ostream << ToLeximitedNumber(uvals[i]); //no spaces
      }
    rtnstr = ostream.str();
    return true;
  } //getLexValueAsString

  //static: return false if all zeros, o.w. true; rtnstr updated
  bool SymbolWithValue::getHexValueAsString(u32 ntotbits, const BV8K& bval, std::string& rtnstr)
  {
    //used for code generated in CS::genCodeClassDefaultConstantArray
    u32 uvals[ARRAY_LEN8K];
    bval.ToArray(uvals);

    u32 nwords = (ntotbits + 31)/MAXBITSPERINT;

    //short-circuit if all zeros
    bool isZero = true;
    for(u32 x = 0; x < nwords; x++)
      {
	if(uvals[x] != 0)
	  {
	    isZero = false;
	    break;
	  }
      }

    if(isZero)
      {
	rtnstr = "0x0"; //nothing to xodo
	return false;
      }

    std::ostringstream ostream;
    for(u32 i = 0; i < nwords; i++)
      {
	if(i > 0)
	  ostream << ", ";

	ostream << "0x" << std::hex << uvals[i];
      }
    rtnstr = ostream.str();
    return true;
  } //getHexValueAsString

  bool SymbolWithValue::convertValueToAPrettyString(u64 varg, UTI tuti, std::string& vstr, CompilerState & state)
  {
    std::ostringstream ostr;
    UlamType * tut = state.getUlamTypeByIndex(tuti);
    s32 bs = tut->getBitSize();
    ULAMTYPE etyp = tut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	{
	  if(bs <= MAXBITSPERINT)
	    {
	      s32 sval = _Int32ToInt32((u32) varg, bs, MAXBITSPERINT);
	      ostr << sval;
	    }
	  else if(bs <= MAXBITSPERLONG)
	    {
	      s64 sval = _Int64ToInt64(varg, bs, MAXBITSPERLONG);
	      ostr << sval;
	    }
	  else
	    state.abortGreaterThanMaxBitsPerLong();
	}
	break;
      case Bool:
	{
	  bool bval = _Bool64ToCbool(varg, bs);
	  if(bval)
	    ostr << "true";
	  else
	    ostr << "false";
	}
	break;
      case Unary:
	{
	  u32 pval = _Unary64ToUnsigned64(varg, bs, MAXBITSPERINT);
	  ostr << pval;
	}
	break;
      case Unsigned:
	{
	  if( bs <= MAXBITSPERINT)
	    ostr << (u32) varg << "u";
	  else if( bs <= MAXBITSPERLONG)
	    ostr << varg << "u";
	  else
	    state.abortGreaterThanMaxBitsPerLong();
	}
	break;
      case Bits:
	{
	  ostr << "0x" << std::hex << varg;
	}
	break;
      case String:
	{
	  // Human readable strings:
	  // t3959,60,61,62,67, t3981,82,86, t41005,6,7,12,68,83, error/t41013
	  ostr << state.getDataAsFormattedUserString((u32) varg);
	}
	break;
      default:
	state.abortUndefinedUlamPrimitiveType();
      };
    vstr = ostr.str();
    return true;
  } //convertValueToAPrettyString (static helper)

  bool SymbolWithValue::convertValueToANonPrettyString(u64 varg, UTI tuti, std::string& vstr, CompilerState & state)
  {
    std::ostringstream ostr;
    UlamType * tut = state.getUlamTypeByIndex(tuti);
    s32 bs = tut->getBitSize();
    ULAMTYPE etyp = tut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	{
	  if(bs <= MAXBITSPERINT)
	    {
	      s32 sval = _Int32ToInt32((u32) varg, bs, MAXBITSPERINT);
	      ostr << sval;
	    }
	  else if(bs <= MAXBITSPERLONG)
	    {
	      s64 sval = _Int64ToInt64(varg, bs, MAXBITSPERLONG);
	      ostr << sval;
	    }
	  else
	    state.abortGreaterThanMaxBitsPerLong();
	}
	break;
      case Unsigned:
	{
	  if( bs <= MAXBITSPERINT)
	    ostr << (u32) varg << "u";
	  else if( bs <= MAXBITSPERLONG)
	    ostr << varg << "u";
	  else
	    state.abortGreaterThanMaxBitsPerLong();
	}
	break;
      case Unary:
      case Bool:
      case Bits:
      case Class:
	{
	  ostr << "0x" << std::hex << varg;
	}
	break;
      case String:
	{
	  std::string fstr = state.getDataAsUnFormattedUserString((u32) varg);
	  u32 flen = fstr.length() - 1; //exclude null terminator
	  for(u32 i = 0; i < flen; i++)
	    ostr << std::hex << std::setfill('0') << std::setw(2) << (u32) fstr[i];
	}
	break;
      default:
	state.abortUndefinedUlamType();
      };
    vstr = ostr.str();
    return true;
  } //convertValueToANonPrettyString (static helper)

  bool SymbolWithValue::convertValueToALexString(u64 varg, UTI tuti, std::string& vstr, CompilerState & state)
  {
    //UTI tuti = getUlamTypeIdx();
    UlamType * tut = state.getUlamTypeByIndex(tuti);
    s32 bs = tut->getBitSize();
    ULAMTYPE etyp = tut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	{
	  if(bs <= MAXBITSPERINT)
	    {
	      s32 sval = _Int32ToInt32((u32) varg, bs, MAXBITSPERINT);
	      vstr = ToLeximitedNumber(sval);
	    }
	  else if(bs <= MAXBITSPERLONG)
	    {
	      s64 sval = _Int64ToInt64(varg, bs, MAXBITSPERLONG);
	      vstr = ToLeximitedNumber64(sval);
	    }
	  else
	    state.abortGreaterThanMaxBitsPerLong();
	}
	break;
      case Bool:
	{
	  bool bval = _Bool64ToCbool(varg, bs);
	  if(bval)
	    vstr = ToLeximitedNumber(1); //true
	  else
	    vstr = ToLeximitedNumber(0); //false
	}
	break;
      case Unary:
	{
	  s32 pval = _Unary64ToInt64(varg, bs, MAXBITSPERINT);
	  vstr = ToLeximitedNumber(pval);
	}
	break;
      case Unsigned:
      case Bits:
	{
	  if( bs <= MAXBITSPERINT)
	    vstr = ToLeximitedNumber((u32) varg);
	  else if( bs <= MAXBITSPERLONG)
	    vstr = ToLeximitedNumber64(varg);
	  else
	    state.abortGreaterThanMaxBitsPerLong();
	}
	break;
      case String:
	{
	  std::ostringstream fhex;
	  std::string fstr = state.getDataAsUnFormattedUserString((u32) varg);
	  u32 flen = fstr.length() - 1; //exclude null terminator
	  for(u32 i = 0; i < flen; i++)
	    fhex << std::hex << std::setfill('0') << std::setw(2) << (u32) fstr[i];
	  vstr = fhex.str();
	}
	break;
      default:
	state.abortUndefinedUlamPrimitiveType();
      };
    return true;
  } //convertValueToLexString (static helper)

  //static: return true if all zeros, o.w. true;
  bool SymbolWithValue::isValueAllZeros(u32 startbit, u32 ntotbits, const BV8K& bval)
  {
    bool isZero = true;
    if(ntotbits <= MAXBITSPERINT)
      {
	u32 val32 = bval.Read(startbit, ntotbits);
	isZero = (val32 == 0);
      }
    else if(ntotbits <= MAXBITSPERLONG)
      {
	u64 val64 = bval.ReadLong(startbit, ntotbits);
	isZero = (val64 == 0);
      }
    else
      {
	for(u32 x = 0; x < ntotbits; x++)
	  {
	    if(bval.Read(x + startbit, 1) != 0) //per bit
	      {
		isZero = false;
		break;
	      }
	  }
      }
    return isZero;
  } //isValueAllZeros

  //warning: this change also requires an update to the ST's key.
  void SymbolWithValue::changeConstantId(u32 fmid, u32 toid)
  {
    assert(getId() == fmid);
    setId(toid);
  }

  NNO SymbolWithValue::getDeclNodeNo()
  {
    assert(!isDataMember());
    return m_declnno;
  }

  void SymbolWithValue::setDeclNodeNo(NNO nno)
  {
    m_declnno = nno;
  }

} //end MFM

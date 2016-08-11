#include "SymbolWithValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolWithValue::SymbolWithValue(const Token& id, UTI utype, CompilerState & state) : Symbol(id, utype, state), m_isReady(false), m_hasInitVal(false), m_isReadyInitVal(false), m_classParameter(false), m_classArgument(false) { }

  SymbolWithValue::SymbolWithValue(const SymbolWithValue & sref) : Symbol(sref), m_isReady(sref.m_isReady), m_hasInitVal(sref.m_hasInitVal), m_isReadyInitVal(false), m_classParameter(false), m_classArgument(sref.m_classArgument || sref.m_classParameter)
  {
    //classArg is copying from a classParameter
    m_constantValue = sref.m_constantValue;
    m_initialValue = sref.m_initialValue;
  }

  SymbolWithValue::SymbolWithValue(const SymbolWithValue & sref, bool keepType) : Symbol(sref, keepType), m_isReady(sref.m_isReady), m_hasInitVal(sref.m_hasInitVal), m_isReadyInitVal(false), m_classParameter(false), m_classArgument(sref.m_classArgument || sref.m_classParameter)
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
    return m_classArgument;
  }

  void SymbolWithValue::setClassArgumentFlag()
  {
    m_classArgument = true;
  }

  u32 SymbolWithValue::getPosOffset()
  {
    return 0; //always zero
  }

  bool SymbolWithValue::isReady()
  {
    return m_isReady; //constant value
  }

  bool SymbolWithValue::getValue(u32& val)
  {
    if(isReady())
      {
	u32 len = m_state.getBitSize(getUlamTypeIdx());
	val = m_constantValue.Read(0u, len); //return value
	return true;
      }
    return false;
  }

  bool SymbolWithValue::getValue(u64& val)
  {
    if(isReady())
      {
	u32 len = m_state.getBitSize(getUlamTypeIdx());
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
    //m_constantValue = val;
    u32 tbs = m_state.getTotalBitSize(getUlamTypeIdx());
    val.CopyBV(0u, 0u, tbs, m_constantValue); //frompos, topos, len, destBV
    m_isReady = true;
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
	//val = m_initialValue; //return value
	u32 tbs = m_state.getTotalBitSize(getUlamTypeIdx());
	m_initialValue.CopyBV(0u, 0u, tbs, val);
	return true;
      }
    return false;
  }

  void SymbolWithValue::setInitValue(const BV8K& val)
  {
    //m_initialValue = val;
    u32 tbs = m_state.getTotalBitSize(getUlamTypeIdx());
    val.CopyBV(0u, 0u, tbs, this->m_initialValue); //frompos, topos, len, destBV
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
    bool oktoprint = true;
    u64 val = 0;
    if(isReady())
      getValue(val);
    else if(hasInitValue() && isInitValueReady())
      getInitValue(val);
    else
      oktoprint = false;

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
		assert(0);
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
		assert(0); //TBD > 64
	      fp->write("u");
	    }
	    break;
	  default:
	    assert(0);
	  };
      }
    else
      fp->write("NONREADYCONST");
  } //printPostfixValueScalar

  void SymbolWithValue::printPostfixValueArray(File * fp)
  {
    bool oktoprint = true;
    BV8K dval;
    if(isReady())
      getValue(dval);
    else if(hasInitValue() && isInitValueReady())
      getInitValue(dval);
    else
      oktoprint = false;

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

    //like the code generated in CS::genCodeClassDefaultConstantArray
    u32 uvals[ARRAY_LEN8K];
    dval.ToArray(uvals);

    u32 nwords = tut->getTotalNumberOfWords();

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
	fp->write("{ 0..0 }");
	return; //nothing else to do
      }

    fp->write("{ ");
    for(u32 w = 0; w < nwords; w++)
      {
	std::ostringstream dhex;
	dhex << "0x" << std::hex << uvals[w];

	if(w > 0)
	  fp->write(", ");

	fp->write(dhex.str().c_str());
      }
    fp->write(" }");
  } //printPostfixValueArray

  bool SymbolWithValue::getLexValue(std::string& vstr)
  {
    UTI tuti = getUlamTypeIdx();

    if(!isReady())
      return false;

    u64 constantval = 0;
    AssertBool gotVal = getValue(constantval);
    assert(gotVal);

    u32 twordsize =  m_state.getTotalWordSize(tuti); //must be commplete
    s32 tbs = m_state.getBitSize(tuti);
    ULAMTYPE etyp = m_state.getUlamTypeByIndex(tuti)->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	{
	  if(twordsize <= MAXBITSPERINT)
	    {
	      s32 sval = _Int32ToInt32((u32) constantval, tbs, MAXBITSPERINT);
	      vstr = ToLeximitedNumber(sval);
	    }
	  else if(twordsize <= MAXBITSPERLONG)
	    {
	      s64 sval = _Int64ToInt64(constantval, tbs, MAXBITSPERLONG);
	      vstr = ToLeximitedNumber64(sval);
	    }
	  else
	    assert(0);
	}
	break;
      case Bool:
	{
	  bool bval = _Bool64ToCbool(constantval, tbs);
	  if(bval)
	    vstr = ToLeximitedNumber(1); //true
	  else
	    vstr = ToLeximitedNumber(0); //false
	}
	break;
      case Unary:
	{
	  s32 pval = _Unary64ToInt64(constantval, tbs, MAXBITSPERINT);
	  vstr = ToLeximitedNumber(pval);
	}
	break;
      case Unsigned:
      case Bits:
	{
	  //oddly write_decimal wants a signed int..
	  if( tbs <= MAXBITSPERINT)
	    vstr = ToLeximitedNumber((u32) constantval);
	  else if( tbs <= MAXBITSPERLONG)
		vstr = ToLeximitedNumber64(constantval);
	  else
	    assert(0);
	}
	break;
      default:
	assert(0);
      };
    return true;
  } //getLexValue

  //warning: this change also requires an update to the ST's key.
  void SymbolWithValue::changeConstantId(u32 fmid, u32 toid)
  {
    assert(getId() == fmid);
    setId(toid);
  }

} //end MFM

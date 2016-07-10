#include "SymbolWithValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolWithValue::SymbolWithValue(Token id, UTI utype, CompilerState & state) : Symbol(id, utype, state), m_isReady(false), m_parameter(false)
  {
    m_constant.sval = 0;
  }

  SymbolWithValue::SymbolWithValue(const SymbolWithValue & sref) : Symbol(sref), m_isReady(sref.m_isReady), m_parameter(sref.m_parameter)
  {
    m_constant = sref.m_constant;
  }

  SymbolWithValue::SymbolWithValue(const SymbolWithValue & sref, bool keepType) : Symbol(sref, keepType), m_isReady(sref.m_isReady), m_parameter(sref.m_parameter)
  {
    m_constant = sref.m_constant;
  }

  SymbolWithValue::~SymbolWithValue()
  { }

  bool SymbolWithValue::isReady()
  {
    return m_isReady;
  }

  bool SymbolWithValue::getValue(s64& val)
  {
    val = m_constant.sval;
    return m_isReady;
  }

  bool SymbolWithValue::getValue(u64& val)
  {
    val = m_constant.uval;
    return m_isReady;
  }

  void SymbolWithValue::setValue(s64 val)
  {
    m_constant.sval = val;
    m_isReady = true;
  }

  void SymbolWithValue::setValue(u64 val)
  {
    m_constant.uval = val;
    m_isReady = true;
  }

  bool SymbolWithValue::foldConstantExpression()
  {
    return true; //stub
  }

  void SymbolWithValue::printPostfixValue(File * fp)
  {
    UTI tuti = getUlamTypeIdx();

    if(isReady())
      {
	u32 twordsize =  m_state.getTotalWordSize(tuti); //must be commplete
	s32 tbs = m_state.getBitSize(tuti);
	ULAMTYPE etype = m_state.getUlamTypeByIndex(tuti)->getUlamTypeEnum();

	switch(etype)
	  {
	  case Int:
	    {
	      if(twordsize == MAXBITSPERINT)
		{
		  s32 sval = _Int32ToInt32((u32) m_constant.uval, tbs, MAXBITSPERINT);
		  fp->write_decimal(sval);
		}
	      else if(twordsize == MAXBITSPERLONG)
		{
		  s64 sval = _Int64ToInt64(m_constant.uval, tbs, MAXBITSPERLONG);
		  fp->write_decimal_long(sval);
		}
	      else
		assert(0);
	    }
	    break;
	  case Bool:
	    {
	      bool bval = _Bool64ToCbool(m_constant.uval, tbs);
	      if(bval)
		fp->write("true");
	      else
		fp->write("false");
	    }
	    break;
	  case Unary:
	    {
	      s32 pval = _Unary64ToInt64(m_constant.uval, tbs, MAXBITSPERINT);
	      fp->write_decimal(pval);
	    }
	    break;
	  case Unsigned:
	  case Bits:
	    {
	      //oddly write_decimal wants a signed int..
	      if( tbs <= MAXBITSPERINT)
		fp->write_decimal((s32) m_constant.sval);
	      else if( tbs <= MAXBITSPERLONG)
		fp->write_decimal_long(m_constant.sval);
	      else
		assert(0);
	    }
	    break;
	  default:
	    assert(0);
	  };
      }
    else
      fp->write("NONREADYCONST");
    fp->write("; ");
  } //printPostfixValue

  bool SymbolWithValue::getLexValue(std::string& vstr)
  {
    UTI tuti = getUlamTypeIdx();

    if(!isReady())
      return false;

    u32 twordsize =  m_state.getTotalWordSize(tuti); //must be commplete
    s32 tbs = m_state.getBitSize(tuti);
    ULAMTYPE etype = m_state.getUlamTypeByIndex(tuti)->getUlamTypeEnum();
    switch(etype)
      {
      case Int:
	{
	  if(twordsize == MAXBITSPERINT)
	    {
	      s32 sval = _Int32ToInt32((u32) m_constant.uval, tbs, MAXBITSPERINT);
	      vstr = ToLeximitedNumber(sval);
	    }
	  else if(twordsize == MAXBITSPERLONG)
	    {
	      s64 sval = _Int64ToInt64(m_constant.uval, tbs, MAXBITSPERLONG);
	      vstr = ToLeximitedNumber64(sval);
	    }
	  else
	    assert(0);
	}
	break;
      case Bool:
	{
	  bool bval = _Bool64ToCbool(m_constant.uval, tbs);
	  if(bval)
	    vstr = ToLeximitedNumber(1); //true
	  else
	    vstr = ToLeximitedNumber(0); //false
	}
	break;
      case Unary:
	{
	  s32 pval = _Unary64ToInt64(m_constant.uval, tbs, MAXBITSPERINT);
	      vstr = ToLeximitedNumber(pval);
	}
	break;
      case Unsigned:
      case Bits:
	{
	  //oddly write_decimal wants a signed int..
	  if( tbs <= MAXBITSPERINT)
	    vstr = ToLeximitedNumber((u32) m_constant.uval);
	  else if( tbs <= MAXBITSPERLONG)
		vstr = ToLeximitedNumber64(m_constant.uval);
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

  void SymbolWithValue::setParameterFlag()
  {
    m_parameter = true;
  }

  bool SymbolWithValue::isParameter()
  {
    return m_parameter;
  }

  u32 SymbolWithValue::getPosOffset()
  {
    return 0; //always zero
  }


} //end MFM

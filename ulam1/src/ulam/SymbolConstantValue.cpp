#include "SymbolConstantValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolConstantValue::SymbolConstantValue(Token id, UTI utype, CompilerState & state) : Symbol(id, utype, state), m_isReady(false), m_parameter(false)
  {
    m_constant.sval = 0; //was NONREADYCONST
  }

  SymbolConstantValue::SymbolConstantValue(const SymbolConstantValue & sref) : Symbol(sref), m_isReady(sref.m_isReady), m_parameter(sref.m_parameter)
  {
    m_constant = sref.m_constant;
  }

  SymbolConstantValue::SymbolConstantValue(const SymbolConstantValue & sref, bool keepType) : Symbol(sref, keepType), m_isReady(sref.m_isReady), m_parameter(sref.m_parameter)
  {
    m_constant = sref.m_constant;
  }

  SymbolConstantValue::~SymbolConstantValue()
  { }

  Symbol * SymbolConstantValue::clone()
  {
    return new SymbolConstantValue(*this);
  }

  Symbol * SymbolConstantValue::cloneKeepsType()
  {
    return new SymbolConstantValue(*this, true);
  }

  bool SymbolConstantValue::isConstant()
  {
    return true;
  }

  bool SymbolConstantValue::isReady()
  {
    return m_isReady;
  }

  bool SymbolConstantValue::getValue(s32& val)
  {
    val = m_constant.sval;
    return m_isReady;
  }

  bool SymbolConstantValue::getValue(u32& val)
  {
    val = m_constant.uval;
    return m_isReady;
  }

  bool SymbolConstantValue::getValue(bool& val)
  {
    val = m_constant.bval;
    return m_isReady;
  }

  void SymbolConstantValue::setValue(s32 val)
  {
    m_constant.sval = val;
    m_isReady = true;
  }

  void SymbolConstantValue::setValue(u32 val)
  {
    m_constant.uval = val;
    m_isReady = true;
  }

  void SymbolConstantValue::setValue(bool val)
  {
    m_constant.bval = val;
    m_isReady = true;
  }

  bool SymbolConstantValue::foldConstantExpression()
  {
    return true; //stub
  }

  const std::string SymbolConstantValue::getMangledPrefix()
  {
    return "Uc_";  //?
  }

  // replaces NodeConstantValue:printPostfix to learn the values of Class' storage in center site
  void SymbolConstantValue::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    UTI tuti = getUlamTypeIdx();

    fp->write(" constant");

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(tuti).c_str());
    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(getId()).c_str());
    fp->write(" = ");
    if(isReady())
      {
	s32 tbs = m_state.getBitSize(tuti);
	if( tbs <= MAXBITSPERINT)
	  fp->write_decimal((s32) m_constant.sval);
	else if( tbs <= MAXBITSPERLONG)
	  fp->write_decimal_long(m_constant.sval);
	else
	  assert(0);
      }
    else
      fp->write("NONREADYCONST");
    fp->write("; ");
  } //printPostfixValuesOfVariableDeclarations

  //warning: this change also requires an update to the ST's key.
  void SymbolConstantValue::changeConstantId(u32 fmid, u32 toid)
  {
    assert(getId() == fmid);
    setId(toid);
  }

  void SymbolConstantValue::setParameterFlag()
  {
    m_parameter = true;
  }

  bool SymbolConstantValue::isParameter()
  {
    return m_parameter;
  }

} //end MFM

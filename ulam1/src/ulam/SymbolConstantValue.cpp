#include "SymbolConstantValue.h"
#include "CompilerState.h"

namespace MFM {

  SymbolConstantValue::SymbolConstantValue(u32 id, UTI utype, CompilerState & state) : Symbol(id, utype, state), m_isReady(false), m_expr(NULL)
  {
    m_constant.sval = NONREADYCONST;
  }

  SymbolConstantValue::~SymbolConstantValue()
  {
    delete m_expr;
    m_expr = NULL;
  }


  bool SymbolConstantValue::isConstant()
  {
    return true;
  }


  bool SymbolConstantValue::isReady()
  {
    return m_isReady;
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
	fp->write_decimal(m_constant.sval);
      }
    else
      fp->write("NONREADYCONST");
    fp->write("; ");
  }

} //end MFM

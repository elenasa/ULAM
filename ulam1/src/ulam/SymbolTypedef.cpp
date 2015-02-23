#include "SymbolTypedef.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTypedef::SymbolTypedef(u32 id, UTI utype, CompilerState & state) : Symbol(id, utype, state){}
  SymbolTypedef::SymbolTypedef(const SymbolTypedef& sref) : Symbol(sref) {}
  SymbolTypedef::~SymbolTypedef()
  {}

  Symbol * SymbolTypedef::clone()
  {
    return new SymbolTypedef(*this);
  }

  bool SymbolTypedef::isTypedef()
  {
    return true;
  }

  const std::string SymbolTypedef::getMangledPrefix()
  {
    return "Ut_";  //?
  }

  // replaces NodeTypedef:printPostfix to learn the values of Class' storage in center site
  void SymbolTypedef::printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    UTI tuti = getUlamTypeIdx();

    fp->write(" typedef");

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(tuti).c_str());
    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(getId()).c_str());

    s32 arraysize = m_state.getArraySize(tuti);
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }

    fp->write("; ");
  } //printPostfixValuesOfVariableDeclarations

} //end MFM

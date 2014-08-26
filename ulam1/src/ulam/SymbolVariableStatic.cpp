#include "SymbolVariableStatic.h"
#include "CompilerState.h"

namespace MFM {

  SymbolVariableStatic::SymbolVariableStatic(u32 id, UlamType * utype, u32 slot) : SymbolVariable(id, utype), m_dataMemberSlotIndex(slot)
  {
    setDataMember();
  }


  SymbolVariableStatic::~SymbolVariableStatic()
  {
    //   m_static.clearAllocatedMemory(); //clean up arrays,etc.
  }


  u32  SymbolVariableStatic::getDataMemberSlotIndex()
  {
    return m_dataMemberSlotIndex;
  }


  UlamValue SymbolVariableStatic::getUlamValue(CompilerState& m_state)
  {
    if(getUlamType()->isScalar())
      return m_state.m_selectedAtom.getDataMemberAt(getDataMemberSlotIndex());

    // should return a header if array!!!
    return makeUlamValuePtr();
  }


  UlamValue SymbolVariableStatic::getUlamValueToStoreInto()
  {
    return makeUlamValuePtr();
  }


  UlamValue SymbolVariableStatic::getUlamValueAt(s32 idx, CompilerState& m_state)
  {
    assert(idx < (s32) getUlamType()->getArraySize() && idx >= 0);
    return m_state.m_selectedAtom.getDataMemberAt(getDataMemberSlotIndex() + idx);
  }


  UlamValue SymbolVariableStatic::getUlamValueAtToStoreInto(s32 idx, CompilerState& state)
  {
    assert(idx < (s32) getUlamType()->getArraySize() && idx >= 0);
    return makeUlamValuePtrAt(idx, state);
  }


  UlamValue SymbolVariableStatic::makeUlamValuePtr()
  {
    return UlamValue(getUlamType(),getDataMemberSlotIndex(), true, ATOM);
  }


  UlamValue SymbolVariableStatic::makeUlamValuePtrAt(u32 idx, CompilerState& state)
  {
    assert(idx < getUlamType()->getArraySize() && idx >= 0);
    UlamValue tmpuv = getUlamValueAt(idx, state); //for scalar type
    
    return UlamValue(tmpuv.getUlamValueType(),getDataMemberSlotIndex()+idx, true, ATOM);
  }


  s32 SymbolVariableStatic::getBaseArrayIndex()
  {
    return (s32) getDataMemberSlotIndex();
  }


  const std::string SymbolVariableStatic::getMangledPrefix()
  {
    return "Um_"; 
  }


  void SymbolVariableStatic::generateCodedVariableDeclarations(File * fp, ULAMCLASSTYPE classtype, CompilerState * state)
  {
    UlamType * vut = getUlamType(); 
    ULAMCLASSTYPE vclasstype = vut->getUlamClassType();

    state->indent(fp);
    fp->write(vut->getUlamTypeMangledName(state).c_str()); //for C++
    
    if(vclasstype == UC_QUARK)       // called on classtype elements only
      {
	fp->write("<");
	fp->write_decimal(getPosOffset());
	fp->write(">");
      }    
    fp->write(" ");
    fp->write(getMangledName(state).c_str());

#if 0
    //???
    u32 arraysize = vut->getArraySize();
    if(arraysize > 0)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
#endif

    fp->write(";\n");  
  }

} //end MFM

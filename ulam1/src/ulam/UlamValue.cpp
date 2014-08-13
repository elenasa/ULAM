#include <stdio.h>
#include <assert.h>
#include "UlamValue.h"
#include "UlamType.h"
#include "CompilerState.h"

namespace MFM {

  UlamValue::UlamValue() : m_utype(NULL), m_valInt(0), m_storage(IMMEDIATE) {}
  UlamValue::UlamValue(UlamType * utype, s32 v, STORAGE place) : m_valInt(v), m_storage(place){ m_utype = utype;}

  UlamValue::UlamValue(UlamType * utype, bool v, STORAGE place) : m_valBool(v), m_storage(place) { m_utype = utype;}

  UlamValue::UlamValue(UlamType * utype, float v, STORAGE place) : m_valFloat(v), m_storage(place){ m_utype = utype;}

  UlamValue::UlamValue(UlamType * arrayUType, s32 baseArrayIndex, bool headerOnly, STORAGE place) : m_baseArraySlotIndex(baseArrayIndex), m_storage(place) { m_utype = arrayUType; } //header for array

  UlamValue::~UlamValue()
  {
    //do not do this automatically; up to Symbol 
    //clearAllocatedMemory();
  }

  void UlamValue::init(UlamType * utype, s32 v) { m_valInt = v; m_utype = utype;}
  void UlamValue::init(UlamType * utype, bool v) { m_valBool = v; m_utype = utype;}
  void UlamValue::init(UlamType * utype, float v) { m_valFloat = v; m_utype = utype;}
  void UlamValue::init(UlamType * arrayUType, s32 baseArrayIndex, bool headerOnly, STORAGE place){ m_baseArraySlotIndex = baseArrayIndex; m_storage = place; m_utype = arrayUType; } //header for array

  UlamType * UlamValue::getUlamValueType()
  {
    return m_utype;  
  }


  void UlamValue::getUlamValueAsString(char * valstr, CompilerState& state)
  {
    if(m_utype)
      m_utype->getUlamValueAsString(*this, valstr, state);
    else
      sprintf(valstr,"<NOVALUE>");
  }


  bool UlamValue::isZero()
  {
    if(m_utype)
      return m_utype->isZero(*this);

    return true;  //let zero be the default???
  }


  u32 UlamValue::isArraySize()
  {
    return m_utype->getUlamKeyTypeSignature().getUlamKeyTypeSignatureArraySize();
  }


  UlamValue UlamValue::getValAt(s32 arrayindex, CompilerState& state) const
  { 
    assert(m_utype->getArraySize() > 0);
    s32 index = m_baseArraySlotIndex + arrayindex;
   
    if(m_storage == ATOM)
      {
	return state.m_selectedAtom.getDataMemberAt(index);
      }
    else if(m_storage == STACK)
      {
	return state.m_funcCallStack.getFrameSlotAt(index);
      }
    else if(m_storage == EVALRETURN)
      {
	assert(0);  //not valid i think
	return state.m_nodeEvalStack.getFrameSlotAt(index);
      }
    else 
      {
	assert(0);
      }

    return UlamValue(state.getUlamTypeByIndex(Nav), 0, IMMEDIATE); //for compiler
  }


} //end MFM

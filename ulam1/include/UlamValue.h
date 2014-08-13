/* -*- c++ -*- */

#ifndef ULAMVALUE_H
#define ULAMVALUE_H

#include "itype.h"

namespace MFM{

  enum STORAGE { IMMEDIATE = 0, ATOM, STACK, EVALRETURN};

  class UlamType;      //forward
  class CompilerState; //forward

  struct UlamValue
  {
    UlamType * m_utype;
    union{
      s32 m_valInt;
      bool m_valBool;
      float m_valFloat;
      //UlamValue * m_valPtr;
      s32 m_baseArraySlotIndex;
    };
    STORAGE m_storage;
    
    UlamValue();   //requires init to avoid Null ptr for type
    UlamValue(UlamType * utype, s32 v, STORAGE place);
    UlamValue(UlamType * utype, bool v, STORAGE place);
    UlamValue(UlamType * utype, float v, STORAGE place);
    //UlamValue(UlamType * arrayUType, UlamType * baseUType, STORAGE place);
    UlamValue(UlamType * arrayUType, s32 slot, bool headerOnly, STORAGE place);

    ~UlamValue();

    void init(UlamType * utype, s32 v);
    void init(UlamType * utype, bool v);
    void init(UlamType * utype, float v);
    //void init(UlamType * arrayUType, UlamType * baseUType, STORAGE place);

    UlamType * getUlamValueType();
    void getUlamValueAsString(char * valstr, CompilerState& state);
    bool isZero();
    u32 isArraySize();

    UlamValue getValAt(s32 arrayindex, CompilerState& state) const;
  };

}

#endif //end ULAMVALUE_H

/* -*- c++ -*- */

#ifndef CALLSTACK_H
#define CALLSTACK_H

#include <stdio.h>
#include <vector>
#include "itype.h"
#include "UlamValue.h"
#include "UlamType.h"

namespace MFM
{

  class CompilerState; //forward

  class CallStack
  {
  public:

    CallStack();
    ~CallStack();

    void init(UlamType * intType);

    //returns new "zero"
    u32 addFrameSlots(u32 depth);

    u32 returnFrame();

    u32 getCurrentFramePointer();

    u32 getRelativeTopOfStackNextSlot();

    UlamValue getFrameSlotAt(s32 idx);

    UlamValue * getPointerToFrameSlotAt(s32 idx); //dangerous, pointer can get reallocated by vector

    u32 getAbsoluteIndexToFrameSlotAt(s32 idx);

    UlamValue getAbsoluteSlotAt(u32 idx);

    void assignUlamValue(UlamValue luv, UlamValue ruv, CompilerState & state);
    void assignUlamValuePtr(UlamValue pluv, UlamValue puv, CompilerState & state);

    //returns number of ulamvalues pushed; one per array element, o.w. 1
    u32 pushArg(UlamValue arg, CompilerState & state);   //doesn't change framepointer

    void popArgs(u32 num);        //doesn't change framepointer

    UlamValue popArg();                //doesn't change framepointer

    UlamValue loadUlamValueFromSlot(s32 slot);
    void storeUlamValueInSlot(UlamValue uv, s32 slot);

  private: 
    std::vector<UlamValue> m_frames;  
    u32 m_currentFrame;
    UlamType * m_intType;

  };
}

#endif  /* CALLSTACK_H */

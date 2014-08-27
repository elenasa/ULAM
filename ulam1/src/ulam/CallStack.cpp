#include <assert.h>
#include "CallStack.h"
#include "CompilerState.h"

namespace MFM {

  CallStack::CallStack() : m_currentFrame(0), m_intType(NULL)
  { }  //don't forget to call init..


  CallStack::~CallStack()
  {
    m_frames.clear();
    m_currentFrame = 0;
  }


  void CallStack::init(UlamType * intType)
  {
    m_intType = intType;
    // push a slot for main's return value; assume no arg's
    UlamValue uv(m_intType, -1, IMMEDIATE);
    m_frames.push_back(uv);
    m_currentFrame = m_frames.size() - 1;
  }


  u32 CallStack::addFrameSlots(u32 depth)
  {
    //first slot is frame pointer, contains previous frame pointer
    UlamValue uv(m_intType, (s32) m_currentFrame, IMMEDIATE);
    m_frames.push_back(uv); 
    m_currentFrame = m_frames.size() - 1;

    //initialize remaining depth slots to a zero ulamvalue
    uv.init(m_intType, 0); 

    for(u32 i = 0; i < depth; i++)
      {
	m_frames.push_back(uv);
      }

    //    m_currentFrame = (m_frames.size() - depth - 1);
    return m_currentFrame;  //new frame pointer
  }


  u32 CallStack::returnFrame()
  {
    s32 prevZero = m_frames[m_currentFrame].m_valInt;
    assert(prevZero < (s32) m_frames.size());

    //let caller pop its args!
    //u32 popslots = m_currentFrame - prevZero;
    s32 popslots = m_frames.size() - m_currentFrame;
    for(s32 i = 0; i < popslots; i++)
      m_frames.pop_back();

    m_currentFrame = prevZero;
    return m_currentFrame;
  }


  u32 CallStack::getCurrentFramePointer()
  {
    return m_currentFrame;
  }


  u32 CallStack::getRelativeTopOfStackNextSlot()
  {
    return m_frames.size() - m_currentFrame;
  }


  //duplicates loadUlamValueFromSlot
  UlamValue CallStack::getFrameSlotAt(s32 idx)
  {
    assert((m_currentFrame + idx < m_frames.size()) && (m_currentFrame + idx >= 0));
    return m_frames[m_currentFrame + idx];
  }


  UlamValue CallStack::getAbsoluteSlotAt(u32 idx)
  {
    return m_frames[idx];
  }


  UlamValue * CallStack::getPointerToFrameSlotAt(s32 idx)
  {
    return  &m_frames[m_currentFrame + idx];  //dangerous!!!
  }


  u32 CallStack::getAbsoluteIndexToFrameSlotAt(s32 idx)
  {
    return  (m_currentFrame + idx);
  }


  void CallStack::storeUlamValueInSlot(UlamValue uv, s32 slot)
  {
    assert((m_currentFrame + slot < m_frames.size()) && (m_currentFrame + slot >= 0));
    m_frames[m_currentFrame + slot] = uv; 
  }


  UlamValue CallStack::loadUlamValueFromSlot(s32 slot)
  {
    assert((m_currentFrame + slot < m_frames.size()) && (m_currentFrame + slot >= 0));
    return m_frames[m_currentFrame + slot]; 
  }


  void CallStack::assignUlamValue(UlamValue pluv, UlamValue ruv, CompilerState & state)
  {
    assert(pluv.m_utype == ruv.m_utype);

    u32 arraysize = pluv.isArraySize();
    s32 leftbaseslot = pluv.m_baseArraySlotIndex; //even for scalars

    if(arraysize == 0 && ruv.m_storage == IMMEDIATE)
      {
	m_frames[m_currentFrame + leftbaseslot] = ruv;  //must be immediate
      }
    else
      {
	s32 rightbaseslot = ruv.m_baseArraySlotIndex;
	arraysize = arraysize > 0 ? arraysize : 1;

	for(u32 i=0; i < arraysize; i++)
	  {
	    if(ruv.m_storage == ATOM)
	      {
		m_frames[m_currentFrame + leftbaseslot + i] = state.m_selectedAtom.getDataMemberAt(rightbaseslot + i);
	      }
	    else if(ruv.m_storage == STACK)
	      {
		m_frames[m_currentFrame + leftbaseslot + i] = state.m_funcCallStack.getFrameSlotAt(rightbaseslot + i);
	      }
	    else if(ruv.m_storage == EVALRETURN)
	      {
		m_frames[m_currentFrame + leftbaseslot + i] = state.m_nodeEvalStack.getFrameSlotAt(rightbaseslot + i);
	      }
	    else
	      {
		assert(0);
		//error! how can an immediate be an array?
	      }
	  } //end for each array value
      } //end if array
  }


  void CallStack::assignUlamValuePtr(UlamValue pluv, UlamValue puv, CompilerState & state)
  {
    assert(pluv.m_utype == puv.m_utype);
    s32 leftbaseslot = pluv.m_baseArraySlotIndex; //even for scalars
    m_frames[m_currentFrame + leftbaseslot] = puv;  
  }


  u32 CallStack::pushArg(UlamValue arg, CompilerState & state)   //doesn't change framepointer
  {
    u32 pushsize = 1;
    if(arg.m_storage == IMMEDIATE)
      {
	m_frames.push_back(arg);
      }
    else
      {
	s32 baseslot = arg.m_baseArraySlotIndex;
	pushsize = arg.getUlamValueType()->getArraySize();
	pushsize = (pushsize > 0 ? pushsize : 1);

	for(u32 i=0; i < pushsize; i++)
	  {
	    if(arg.m_storage == ATOM)
	      {
		m_frames.push_back(state.m_selectedAtom.getDataMemberAt(baseslot + i));
	      }
	    else if(arg.m_storage == STACK)
	      {
		m_frames.push_back(state.m_funcCallStack.getFrameSlotAt(baseslot + i));
	      }
	    else if(arg.m_storage == EVALRETURN)
	      {
		m_frames.push_back(state.m_nodeEvalStack.getFrameSlotAt(baseslot + i));
	      }
	    else
	      {
		assert(0);
		//error! how can an immediate be an array?
	      }
	  }
      }
    return pushsize;
  }


  void CallStack::popArgs(u32 num)                 //doesn't change framepointer
  {
    for(u32 i = 0; i < num; i++)
      {
	popArg();  //drop values
      }
  }


  UlamValue CallStack::popArg()                 //doesn't change framepointer
  {
    assert(!m_frames.empty());
    UlamValue rtnUV = m_frames.back();
    m_frames.pop_back();
    return rtnUV;
  }


} //MFM

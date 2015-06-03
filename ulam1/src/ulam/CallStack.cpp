#include <assert.h>
#include "CallStack.h"
#include "CompilerState.h"

namespace MFM {

  CallStack::CallStack() : m_currentFrame(0), m_intType(0)
  { }  //don't forget to call init..


  CallStack::~CallStack()
  {
    m_frames.clear();
    m_currentFrame = 0;
  }


  void CallStack::init(UTI intType)
  {
    m_intType = intType;
    // push a slot for main's return value; assume no arg's
    UlamValue uv = UlamValue::makeImmediate(m_intType, -1);
    m_frames.push_back(uv);
    m_currentFrame = m_frames.size() - 1;
  }


  u32 CallStack::addFrameSlots(u32 depth)
  {
    //first slot is frame pointer, contains previous frame pointer
    UlamValue uv = UlamValue::makeImmediate(m_intType, (s32) m_currentFrame);
    m_frames.push_back(uv);
    m_currentFrame = m_frames.size() - 1;

    //initialize remaining depth slots to a zero ulamvalue
    UlamValue uv0 = UlamValue::makeImmediate(m_intType, 0);

    for(u32 i = 0; i < depth; i++)
      {
	m_frames.push_back(uv0);
      }

    return m_currentFrame;  //new frame pointer
  }


  void CallStack::returnFrame()
  {
    s32 prevZero = m_frames[m_currentFrame].getImmediateData();
    assert(prevZero < (s32) m_frames.size());

    //let caller pop its args!
    s32 popslots = m_frames.size() - m_currentFrame;
    for(s32 i = 0; i < popslots; i++)
      m_frames.pop_back();

    m_currentFrame = prevZero;
  }


  u32 CallStack::getCurrentFramePointer()
  {
    return m_currentFrame;
  }


  u32 CallStack::getRelativeTopOfStackNextSlot()
  {
    return m_frames.size() - m_currentFrame;
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


  //called by CompilerState
  void CallStack::assignUlamValue(UlamValue pluv, UlamValue ruv, CompilerState& state)
  {
    assert(pluv.getUlamValueTypeIdx() == Ptr);
    assert(ruv.getUlamValueTypeIdx() != Ptr);

    s32 leftbaseslot = pluv.getPtrSlotIndex();    //even for scalars

    if(pluv.isTargetPacked() == UNPACKED)       //???
      {
	m_frames[m_currentFrame + leftbaseslot] = ruv;  //must be immediate
      }
    else if(pluv.isTargetPacked() == PACKED && state.isScalar(pluv.getPtrTargetType()))
      {
	m_frames[m_currentFrame + leftbaseslot] = ruv;  //entire element, perhaps?
      }
    else
      {
	//target is either packed array or packedloadable into a single int,
	// use pos & len in ptr, unless there's no type
	UlamValue lvalAtIdx = loadUlamValueFromSlot(leftbaseslot);

	// if uninitialized, set the type
	if(lvalAtIdx.getUlamValueTypeIdx() == Nav)
	  {
	    lvalAtIdx.setUlamValueTypeIdx(pluv.getPtrTargetType());
	  }

	// never is lvalAtIdx a Ptr
	UTI luti = lvalAtIdx.getUlamValueTypeIdx();
	assert(luti != Ptr);

	s32 len = pluv.getPtrLen(); //state.getBitSize(luti);
	assert(len != UNKNOWNSIZE);
	if(len <= MAXBITSPERINT)
	  {
	    lvalAtIdx.putDataIntoAtom(pluv, ruv, state);
	    storeUlamValueInSlot(lvalAtIdx, leftbaseslot);
	  }
	else if(len <= MAXBITSPERLONG)
	  {
	    lvalAtIdx.putDataIntoAtom(pluv, ruv, state);
	    storeUlamValueInSlot(lvalAtIdx, leftbaseslot);
	  }
	else
	  assert(0);
      }
  } //assignUlamValue


  void CallStack::assignUlamValuePtr(UlamValue pluv, UlamValue puv)
  {
    assert(pluv.getPtrTargetType() == puv.getPtrTargetType());

    s32 leftbaseslot = pluv.getPtrSlotIndex(); //even for scalars
    m_frames[m_currentFrame + leftbaseslot] = puv;
  }


  void CallStack::pushArg(UlamValue arg)   //doesn't change framepointer
  {
    m_frames.push_back(arg);               //doesn't care what type.
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

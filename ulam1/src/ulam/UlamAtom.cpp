#include <assert.h>
#include "UlamAtom.h"
#include "CompilerState.h"

namespace MFM {

  UlamAtom::UlamAtom()
  { }


  UlamAtom::~UlamAtom()
  {
    m_dataMembers.clear();
  }


  void UlamAtom::init(UlamType * intType)
  {
    UlamValue uv(intType, -1, IMMEDIATE);
    m_dataMembers.push_back(uv);
  }


  u32 UlamAtom::getUlamAtomTypeNumber()
  {
    return m_elementTypeNumber;
  }


  void UlamAtom::setUlamAtomTypeNumber(u32 typenumber)
  {
    m_elementTypeNumber = typenumber;
  }


  UlamValue UlamAtom::getDataMemberAt(u32 idx)
  {
    assert( (idx < m_dataMembers.size()) && (idx >= 0));
    return m_dataMembers[idx];
  }


  void UlamAtom::storeDataMemberAt(UlamValue uv, u32 idx)
  {
    assert((idx < m_dataMembers.size()) && (idx >= 0));
    m_dataMembers[idx] = uv; 
  }


  void UlamAtom::assignUlamValue(UlamValue pluv, UlamValue ruv, CompilerState & state)
  {
    assert(pluv.m_utype == ruv.m_utype);

    u32 arraysize = pluv.isArraySize();
    s32 leftbaseslot = pluv.m_baseArraySlotIndex; //even for scalars

    if(arraysize == 0 && ruv.m_storage == IMMEDIATE)
      {
	m_dataMembers[leftbaseslot] = ruv;  //must be immediate ???
      }
    else
      {
	s32 rightbaseslot = ruv.m_baseArraySlotIndex;
	arraysize = arraysize > 0 ? arraysize : 1;

	for(u32 i=0; i < arraysize; i++)
	  {
	    if(ruv.m_storage == ATOM)
	      {
		m_dataMembers[leftbaseslot+i] = m_dataMembers[rightbaseslot+i];
	      }
	    else if(ruv.m_storage == STACK)
	      {
		m_dataMembers[leftbaseslot+i] = state.m_funcCallStack.getFrameSlotAt(rightbaseslot+i);
	      }
	    else if(ruv.m_storage == EVALRETURN)
	      {
		m_dataMembers[leftbaseslot+i] = state.m_nodeEvalStack.getFrameSlotAt(rightbaseslot+i);
	      }
	    else
	      {
		assert(0);
		//error! how can an immediate be an array???
	      }
	  } //end for each array value
      } //end if array
  }


  void UlamAtom::assignUlamValuePtr(UlamValue pluv, UlamValue puv, CompilerState & state)
  {
    assert(pluv.m_utype == puv.m_utype);
    s32 leftbaseslot = pluv.m_baseArraySlotIndex; //even for scalars
    m_dataMembers[leftbaseslot] = puv; 
  }


  u32 UlamAtom::pushDataMember(UlamType * arrayType, UlamType * scalarType)
  {
    UlamValue uv(scalarType,0,IMMEDIATE);
    u32 baseIndex = m_dataMembers.size();

    if(arrayType != scalarType)
      {
	u32 pushsize = arrayType->getArraySize();
	for(u32 i=0; i < pushsize; i++)
	  {
	    m_dataMembers.push_back(uv);
	  }
      }
    else
      {
	m_dataMembers.push_back(uv);
      }
    return baseIndex;
  }



} //MFM

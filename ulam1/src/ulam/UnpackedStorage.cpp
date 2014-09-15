#include <assert.h>
#include "UnpackedStorage.h"
#include "CompilerState.h"

namespace MFM {

  UnpackedStorage::UnpackedStorage(CompilerState& state): m_state(state)
  { }


  UnpackedStorage::~UnpackedStorage()
  {
    m_values.clear();
  }


  void UnpackedStorage::init(UTI intType)
  {
    UlamValue uv = UlamValue::makeImmediate(intType, -1, m_state);
    m_values.push_back(uv);
  }


  UlamValue UnpackedStorage::loadDataMemberAt(u32 idx)
  {
    assert( (idx < m_values.size()) && (idx >= 0));
    return m_values[idx];
  }


  void UnpackedStorage::storeDataMemberAt(UlamValue uv, u32 idx)
  {
    assert((idx < m_values.size()) && (idx >= 0));
    m_values[idx] = uv; 
  }


  void UnpackedStorage::assignUlamValue(UlamValue pluv, UlamValue ruv)
  {
    assert(pluv.getUlamValueTypeIdx() == Ptr);
    assert(ruv.getUlamValueTypeIdx() != Ptr);

    s32 leftbaseslot = pluv.getPtrSlotIndex();    //even for scalars

    if(pluv.isTargetPacked() == UNPACKED)
      {
	m_values[leftbaseslot] = ruv;  //must be immediate
      }
    else
      {
	//target is packed or packedloadable, use pos & len in ptr
	UlamValue lvalAtIdx = loadDataMemberAt(leftbaseslot);
	assert(lvalAtIdx.getUlamValueTypeIdx() == Atom);  //?
	lvalAtIdx.putDataIntoAtom(pluv, ruv, m_state);
	storeDataMemberAt(lvalAtIdx, leftbaseslot);
      }
  }


  void UnpackedStorage::assignUlamValuePtr(UlamValue pluv, UlamValue puv)
  {
    assert(pluv.getPtrTargetType() == puv.getPtrTargetType());   

    s32 leftbaseslot = pluv.getPtrSlotIndex(); //even for scalars
    m_values[leftbaseslot] = puv;  
  }


  u32 UnpackedStorage::pushDataMember(UTI arrayType, UTI scalarType)
  {
    UlamValue uv = UlamValue::makeImmediate(scalarType,0);
    u32 baseIndex = m_values.size();

    if(arrayType != scalarType)
      {
	u32 pushsize = m_state.getArraySize(arrayType); //unpacked! no Ptr header???
	for(u32 i=0; i < pushsize; i++)
	  {
	    m_values.push_back(uv);
	  }
      }
    else
      {
	m_values.push_back(uv);
      }
    return baseIndex;
  }

} //MFM

#include <assert.h>
#include <stdio.h>
#include "CompilerState.h"
#include "UlamType.h"
#include "UlamTypeAtom.h"
#include "UlamValue.h"

namespace MFM {

  UlamValue::UlamValue()
  {
    m_uv.m_storage.m_atom.Clear();  //default type is Nav == 0
  }


  UlamValue::~UlamValue()
  {
    //do not do this automatically; up to Symbol 
    //clearAllocatedMemory();
  }


  void UlamValue::init(UTI utype, u32 v, CompilerState& state)
  {
    u32 len = state.getBitSize(utype);
    assert(len <=32 && len > 0);  //very important!
    m_uv.m_storage.m_atom.Clear();
    putData(BITSPERATOM - len, len, v);  //starts from end, for 32 bit boundary case
    setUlamValueTypeIdx(utype);
  }


  UlamValue UlamValue::makeAtom(UTI elementType)
  {
    UlamValue rtnValue = UlamValue::makeAtom();
    rtnValue.setAtomElementTypeIdx(elementType);
    return rtnValue;
  }


  UlamValue UlamValue::makeAtom()
  {
    UlamValue rtnVal;   //static
    rtnVal.m_uv.m_storage.m_atom.Clear();
    rtnVal.setAtomElementTypeIdx(Atom);
    return rtnVal;
  }


  UlamValue UlamValue::makeImmediate(UTI utype, u32 v, CompilerState& state)
  {
    u32 len = state.getBitSize(utype);
    return UlamValue::makeImmediate(utype, v, len);
  }


  UlamValue UlamValue::makeImmediate(UTI utype, u32 v, u32 len)
  {
    UlamValue rtnVal;             //static
    assert(len <=32 && len > 0);  //very important!
    rtnVal.m_uv.m_storage.m_atom.Clear();
    rtnVal.putData(BITSPERATOM - len, len, v);  //starts from end, for 32 bit boundary case
    rtnVal.setUlamValueTypeIdx(utype);
    return rtnVal;
  }


  UlamValue UlamValue::makePtr(u32 slot, STORAGE storage, UTI targetType, PACKFIT packed, CompilerState& state, u32 pos)
  {
    UlamValue rtnUV;  //static method
    rtnUV.m_uv.m_ptrValue.m_utypeIdx = Ptr;
    rtnUV.m_uv.m_ptrValue.m_slotIndex = slot;
    
    //NOTE: 'len' of a packed-array, 
    //       becomes the total size (bits * arraysize)
    u32 len = state.getBitSize(targetType);
    u32 arraysize = state.getArraySize(targetType);
    arraysize = (arraysize > 0 ? arraysize : 1);
    if(packed == PACKED || packed == PACKEDLOADABLE) 
      {
	len *= arraysize;
      }

    if(pos == 0)
      {
	//figure out the pos based on targettype:
	if(targetType == Atom || state.getUlamTypeByIndex(targetType)->getUlamClass() == UC_ELEMENT)
	  rtnUV.m_uv.m_ptrValue.m_posInAtom = ATOMFIRSTSTATEBITPOS; //len is predetermined 
	else
	  {
	    rtnUV.m_uv.m_ptrValue.m_posInAtom = BITSPERATOM - len;  //base position
	  }
      }
    else
      {
	assert(pos >= 0 && (pos + len) <= BITSPERATOM);
	rtnUV.m_uv.m_ptrValue.m_posInAtom = pos; 
      }

    rtnUV.m_uv.m_ptrValue.m_bitlenInAtom = len; //if packed, entire array len
    rtnUV.m_uv.m_ptrValue.m_storage = storage;
    rtnUV.m_uv.m_ptrValue.m_packed = packed;
    rtnUV.m_uv.m_ptrValue.m_targetType = targetType;
    rtnUV.m_uv.m_ptrValue.m_pad = 0;
    return rtnUV;
  }

  // has scalar target type and len; base position depends on packedness; 
  // incrementPtr increments index or pos depending on packness.
  UlamValue UlamValue::makeScalarPtr(UlamValue arrayPtr, CompilerState& state)
  {
    UTI scalarType = state.getUlamTypeAsScalar(arrayPtr.getPtrTargetType());
    UlamValue rtnUV = UlamValue::makePtr(arrayPtr.getPtrSlotIndex(),
					 arrayPtr.getPtrStorage(),
					 scalarType,
					 state.determinePackable(scalarType), //PACKEDLOADABLE
					 state,
					 arrayPtr.getPtrPos() /* base pos of array */
					 );
    return rtnUV;
  }


  UTI UlamValue::getUlamValueTypeIdx() const
  {
    return m_uv.m_rawAtom.m_utypeIdx;  
  }


  void UlamValue::setUlamValueTypeIdx(UTI utype)
  {
     m_uv.m_rawAtom.m_utypeIdx = utype;  
  }


  UTI UlamValue::getAtomElementTypeIdx()
  {
    return (UTI) getDataFromAtom(0, 16);
  }


  void UlamValue::setAtomElementTypeIdx(UTI utype)
  {
    //assert(utype == Atom || state.getUlamTypeByIndex(utype)->getUlamClass() == UC_ELEMENT);
    putData(0, 16, utype);
    //m_uv.m_rawAtom.m_utypeIdx = utype;
  }


  // for iterating an entire array see CompilerState::assignArrayValues
  UlamValue UlamValue::getValAt(u32 offset, CompilerState& state) const
  { 
    assert(getUlamValueTypeIdx() == Ptr);
    assert(state.getArraySize(m_uv.m_ptrValue.m_targetType) > 0);
    
    UlamValue scalarPtr = UlamValue::makeScalarPtr(*this, state);
  
    scalarPtr.incrementPtr(state, offset);   //incr appropriately by packed-ness
    UlamValue atval = state.getPtrTarget(scalarPtr);
    
    // redo what getPtrTarget use to do, when types didn't match due to
    // an element/quark or a requested scalar of an arraytype
    UTI suti = scalarPtr.getPtrTargetType();
    if(atval.getUlamValueTypeIdx() != suti)
      {
	u32 datavalue = atval.getDataFromAtom(scalarPtr, state); 
	atval= UlamValue::makeImmediate(suti, datavalue, state);
      }
    
    return atval;
  }


  PACKFIT UlamValue::isTargetPacked()
  {
    assert(getUlamValueTypeIdx() == Ptr);
    return (PACKFIT) m_uv.m_ptrValue.m_packed;
  }
    

  STORAGE UlamValue::getPtrStorage()
  {
    assert(getUlamValueTypeIdx() == Ptr);
    return (STORAGE) m_uv.m_ptrValue.m_storage;
  }


  // possibly negative into call stack (e.g. args, rtn values)
  // use: adjust "hidden" arg's pointer to atom in call stack
  void UlamValue::setPtrSlotIndex(s32 s)
  {
    assert(getUlamValueTypeIdx() == Ptr);
    m_uv.m_ptrValue.m_slotIndex = s;
  }


  // possibly negative into call stack (e.g. args, rtn values)
  s32 UlamValue::getPtrSlotIndex()
  {
    assert(getUlamValueTypeIdx() == Ptr);
    return m_uv.m_ptrValue.m_slotIndex;
  }


  u32 UlamValue::getPtrPos()
  {
    assert(getUlamValueTypeIdx() == Ptr);
    return m_uv.m_ptrValue.m_posInAtom;
  }


  u32 UlamValue::getPtrLen()
  {
    assert(getUlamValueTypeIdx() == Ptr);
    return m_uv.m_ptrValue.m_bitlenInAtom;
  }


  UTI UlamValue::getPtrTargetType()
  {
    assert(getUlamValueTypeIdx() == Ptr);
    return m_uv.m_ptrValue.m_targetType;
  }

  void UlamValue::setPtrTargetType(UTI type)
  {
    assert(getUlamValueTypeIdx() == Ptr);
    m_uv.m_ptrValue.m_targetType = type;
  }


  void UlamValue::incrementPtr(CompilerState& state, s32 offset)
  {
    assert(getUlamValueTypeIdx() == Ptr);
    assert(state.isScalar(getPtrTargetType())); //?
    if(WritePacked((PACKFIT) m_uv.m_ptrValue.m_packed))
      m_uv.m_ptrValue.m_posInAtom += (m_uv.m_ptrValue.m_bitlenInAtom * offset);
    else
      m_uv.m_ptrValue.m_slotIndex += offset; 
  }


  // bigger than an int, yet packable, extract array from data 
  // where ptr p refers to its start pos/len in data;
  // returns data right justified so that its type will indicate its start.
  // note: cannot be an array of elements since they aren't packable; may be loadable
  UlamValue UlamValue::getPackedArrayDataFromAtom(UlamValue p, UlamValue data, CompilerState& state)
  {
    UTI tuti = p.getPtrTargetType();
    //assert(data.getUlamValueTypeIdx() == tuti); not if 'data' is an element

    UlamValue rtnUV;  //static return
    rtnUV.setUlamValueTypeIdx(tuti);

    u32 arraysize = state.getArraySize(tuti);
    assert(arraysize > 0);
    u32 bitsize = state.getBitSize(tuti);
    assert( (bitsize * arraysize) == p.getPtrLen());  

    if(p.isTargetPacked() == PACKEDLOADABLE)
      {
	u32 len = bitsize * arraysize;
	u32 datavalue = data.getData(p.getPtrPos(), len); //entire array
	rtnUV.putData((BITSPERATOM-len), len, datavalue); //immediate
      }
    else
      {
	assert(p.isTargetPacked() == PACKED);
	// base [0] is furthest from the end
	UlamValue nextPtr = UlamValue::makeScalarPtr(p,state);
	for(u32 i = 0; i < arraysize; i++)
	  {
	    u32 datavalue = data.getData(nextPtr.getPtrPos(), nextPtr.getPtrLen());
	    rtnUV.putData((BITSPERATOM-(bitsize * (arraysize - i))), bitsize, datavalue);
	    nextPtr.incrementPtr(state);
	  }
      }
    return rtnUV;
  }


  u32 UlamValue::getDataFromAtom(UlamValue p, CompilerState& state) const
  {
    UTI duti = getUlamValueTypeIdx();
    //assert(duti == p.getPtrTargetType()); this could be an 'unpacked' element

    if(!state.isScalar(p.getPtrTargetType()) && p.isTargetPacked() == PACKED)
      {
	//must get data piecemeal, too big to fit into one int
	//use getPackedArrayDataIntoAtom(p, data, state);
	assert(0);
      }

    assert(p.getPtrLen() <= MAXBITSPERINT);

    //either a packed-loadable array or scalar
    u32 datavalue;
    PACKFIT packedData = state.determinePackable(duti);
    //assert(p.isTargetPacked() == packedData); untrue if this is an element

    if(WritePacked(packedData)) //scalar packed within
      {
	datavalue = getData(p.getPtrPos(), p.getPtrLen());
      }
    else
      {
	ULAMCLASSTYPE dclasstype = state.getUlamTypeByIndex(duti)->getUlamClass();
	if(dclasstype == UC_NOTACLASS)
	  datavalue = getImmediateData(p.getPtrLen());
	else
	  datavalue = getData(p.getPtrPos(), p.getPtrLen());
      }
    
    return datavalue;
  } //getDataFromAtom overloaded


  u32 UlamValue::getDataFromAtom(u32 pos, u32 len) const
  {
    //assert(getUlamValueTypeIdx() == Atom); ///??? not an atom, element?
    return getData(pos,len);
  }


  u32 UlamValue::getImmediateData(CompilerState & state) const
  {
    return getImmediateData(state.getBitSize(getUlamValueTypeIdx()));
  }


  u32 UlamValue::getImmediateData(u32 len) const
  {
    assert(getUlamValueTypeIdx() != Atom);
    assert(getUlamValueTypeIdx() != Ptr);
    assert(getUlamValueTypeIdx() != Nav);
    assert(len > 0 && len <= MAXBITSPERINT);
    return getData(BITSPERATOM - len, len);
  }


  // 'p' is Ptr into this destination; assumes 'data' is right-justified
  void UlamValue::putDataIntoAtom(UlamValue p, UlamValue data, CompilerState& state)
  {
    // apparently amused by other types..
    UTI duti = data.getUlamValueTypeIdx();
    assert( (duti == p.getPtrTargetType()) || (getUlamValueTypeIdx() == p.getPtrTargetType())); //ALL-PURPOSE!

    if(!state.isScalar(p.getPtrTargetType()) && p.isTargetPacked() == PACKED)
      {
	//must get data piecemeal, too big to fit into one int
	putPackedArrayDataIntoAtom(p, data, state);
      }
    else 
	{
	  assert(p.isTargetPacked() == PACKEDLOADABLE);
	  u32 datavalue = data.getImmediateData(p.getPtrLen());
	  putData(p.getPtrPos(), p.getPtrLen(), datavalue);
	}
  }


  // bigger than an int, yet packable, array data; assume 'data' is
  // right-justified, and 'p' refers to this' destination.
  void UlamValue::putPackedArrayDataIntoAtom(UlamValue p, UlamValue data, CompilerState& state)
  {
    UTI tuti = p.getPtrTargetType();
    assert(data.getUlamValueTypeIdx() == tuti);
    u32 arraysize = state.getArraySize(tuti);
    assert(arraysize > 0);
    u32 bitsize = state.getBitSize(tuti);
    
    UlamValue nextPPtr = UlamValue::makeScalarPtr(p,state);
    assert(bitsize == nextPPtr.getPtrLen()); 

    for(u32 i = 0; i < arraysize; i++)
      {
	//assume data is right-justified, base [0] is furthest from the end
	u32 datavalue = data.getData((BITSPERATOM-(bitsize * (arraysize - i))), bitsize); 
	putData(nextPPtr.getPtrPos(), nextPPtr.getPtrLen(), datavalue);
	nextPPtr.incrementPtr(state);
      }
  }


  u32 UlamValue::getData(u32 pos, u32 len) const
  {
    return m_uv.m_storage.m_atom.Read(pos, len);
  }



  void UlamValue::putData(u32 pos, u32 len, u32 data)
  {
    m_uv.m_storage.m_atom.Write(pos, len, data);
  }


  UlamValue& UlamValue::operator=(const UlamValue& rhs)
  {
    m_uv.m_storage.m_atom = rhs.m_uv.m_storage.m_atom;
    return *this;
  }


  bool UlamValue::operator==(const UlamValue& rhs)
  {
    return m_uv.m_storage.m_atom == rhs.m_uv.m_storage.m_atom;
  }


} //end MFM

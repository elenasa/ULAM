#include <assert.h>
#include <stdio.h>
#include "CompilerState.h"
#include "UlamType.h"
#include "UlamTypeAtom.h"
#include "UlamValue.h"

namespace MFM {

  UlamValue::UlamValue()
  {
    clear(); //default type is Nav == 0
  }

  UlamValue::~UlamValue()
  {
    //do not do this automatically; up to Symbol
    //clearAllocatedMemory();
  }

  void UlamValue::clear()
  {
    AtomBitVector a; //clear storage
    a.ToArray(m_uv.m_storage.m_atom);
  }

  void UlamValue::init(UTI utype, u32 v, CompilerState& state)
  {
    s32 len = state.getBitSize(utype);
    assert(len <=32 && len >= 0); //very important!
    clear();
    setUlamValueTypeIdx(utype);

    if(len == 0)
      return;

    putData(BITSPERATOM - len, len, v); //starts from end, for 32 bit boundary case
  } //init

  UlamValue UlamValue::makeDefaultAtom(UTI elementType, CompilerState& state)
  {
    UlamValue rtnValue = UlamValue::makeAtom();
    rtnValue.setAtomElementTypeIdx(elementType);

    SymbolClass * csym = NULL;
    AssertBool isDefined = state.alreadyDefinedSymbolClass(elementType, csym);
    assert(isDefined);
    NodeBlockClass * cblock = csym->getClassBlockNode();
    assert(cblock);
    cblock->initElementDefaultsForEval(rtnValue);

    return rtnValue;
  } //makeDefaultAtom

  UlamValue UlamValue::makeAtom(UTI elementType)
  {
    UlamValue rtnValue = UlamValue::makeAtom();
    rtnValue.setAtomElementTypeIdx(elementType);
    return rtnValue;
  } //makeAtom

  UlamValue UlamValue::makeAtom()
  {
    UlamValue rtnVal; //static
    rtnVal.clear();
    rtnVal.setAtomElementTypeIdx(UAtom);
    return rtnVal;
  } //makeAtom overload

  UlamValue UlamValue::makeImmediate(UTI utype, u32 v, CompilerState& state)
  {
    s32 len = state.getTotalBitSize(utype); //possible packed array (e.g. default qks)
    assert(len != UNKNOWNSIZE);
    if(state.getUlamTypeByIndex(utype)->getUlamTypeEnum() == Class)
      return UlamValue::makeImmediateQuark(utype, v, len);
    return UlamValue::makeImmediate(utype, v, len);
  } //makeImmediate

  UlamValue UlamValue::makeImmediate(UTI utype, u32 v, s32 len)
  {
    UlamValue rtnVal; //static
    assert(len <= MAXBITSPERINT && (s32) len >= 0); //very important!
    rtnVal.clear();
    rtnVal.setUlamValueTypeIdx(utype);
    rtnVal.putData(BITSPERATOM - len, len, v); //starts from end, for 32 bit boundary case (primitives)
    return rtnVal;
  } //makeImmediate overloaded

  UlamValue UlamValue::makeImmediateQuark(UTI utype, u32 v, s32 len)
  {
    UlamValue rtnVal; //static
    assert(len <= MAXBITSPERINT && (s32) len >= 0); //very important!
    rtnVal.clear();
    rtnVal.setUlamValueTypeIdx(utype);
    rtnVal.putData(ATOMFIRSTSTATEBITPOS, len, v); //left-justified
    return rtnVal;
  } //makeImmediateQuark

  UlamValue UlamValue::makeImmediateLong(UTI utype, u64 v, CompilerState& state)
  {
    s32 len = state.getTotalBitSize(utype);
    assert(len != UNKNOWNSIZE);
    assert(state.getUlamTypeByIndex(utype)->getUlamTypeEnum() != Class);
    return UlamValue::makeImmediateLong(utype, v, len);
  } //makeImmediateLong

  UlamValue UlamValue::makeImmediateLong(UTI utype, u64 v, s32 len)
  {
    UlamValue rtnVal; //static
    assert(len <= MAXBITSPERLONG && (s32) len >= 0); //very important!
    rtnVal.clear();
    rtnVal.setUlamValueTypeIdx(utype);
    rtnVal.putDataLong(BITSPERATOM - len, len, v); //starts from end for 32-bit boundary case
    return rtnVal;
  } //makeImmediateLong overloaded

  UlamValue UlamValue::makePtr(u32 slot, STORAGE storage, UTI targetType, PACKFIT packed, CompilerState& state, u32 pos, u32 id)
  {
    UlamValue rtnUV = UlamValue::makePtr(slot,storage,targetType,packed,state,pos);
    assert(id >= U16_MIN && id <= U16_MAX);
    rtnUV.setPtrNameId(id);
    return rtnUV;
  } //makePtr

  UlamValue UlamValue::makePtr(u32 slot, STORAGE storage, UTI targetType, PACKFIT packed, CompilerState& state, u32 pos)
  {
    UlamValue rtnUV; //static method
    rtnUV.m_uv.m_ptrValue.m_utypeIdx = Ptr;
    assert((s16) slot <= S16_MAX && (s16) slot >= S16_MIN);
    rtnUV.m_uv.m_ptrValue.m_slotIndex = (s16) slot;

    //NOTE: 'len' of a packed-array,
    //       becomes the total size (bits * arraysize);
    //       'len' is item bitsize for unpacked-array;
    //       constants become default len;
    s32 len;
    if(packed == UNPACKED)
      len = state.getBitSize(targetType);
    else
      len = state.getTotalBitSize(targetType);

    if(pos == 0)
      {
	//figure out the pos based on targettype; both elements and quarks start at first state bit (25)
	ULAMTYPE ttenum = state.getUlamTypeByIndex(targetType)->getUlamTypeEnum();
	if(ttenum == UAtom || ttenum == Class)
	  rtnUV.m_uv.m_ptrValue.m_posInAtom = ATOMFIRSTSTATEBITPOS; //len is predetermined
	else
	  {
	    u32 basepos = BITSPERATOM - len;
	    assert(basepos <= U8_MAX && basepos >= U8_MIN);
	    rtnUV.m_uv.m_ptrValue.m_posInAtom = basepos; //base position
	  }
      }
    else
      {
	assert((packed == UNPACKED) || (pos > 0 && (pos + len) <= BITSPERATOM));
	rtnUV.m_uv.m_ptrValue.m_posInAtom = pos;
      }

    assert(len <= S8_MAX && len >= S8_MIN);
    rtnUV.m_uv.m_ptrValue.m_bitlenInAtom = len; //if packed, entire array len
    rtnUV.m_uv.m_ptrValue.m_storagetype = storage;
    rtnUV.m_uv.m_ptrValue.m_packed = packed;
    rtnUV.m_uv.m_ptrValue.m_targetType = targetType;
    rtnUV.m_uv.m_ptrValue.m_nameid = 0;
    return rtnUV;
  } //makePtr overload

  // has scalar target type and len; base position depends on packedness;
  // incrementPtr increments index or pos depending on packness.
  UlamValue UlamValue::makeScalarPtr(UlamValue arrayPtr, CompilerState& state)
  {
    UTI scalarType = state.getUlamTypeAsScalar(arrayPtr.getPtrTargetType());
    UlamValue rtnUV = UlamValue::makePtr(arrayPtr.getPtrSlotIndex(),
					 arrayPtr.getPtrStorage(),
					 scalarType,
					 state.determinePackable(arrayPtr.getPtrTargetType()), //PACKEDLOADABLE, or UNPACKED?
					 state,
					 arrayPtr.getPtrPos(),    /* base pos of array */
					 arrayPtr.getPtrNameId()  /* incl nameid of array */
					 );
    if(arrayPtr.getUlamValueTypeIdx() == PtrAbs)
      rtnUV.setUlamValueTypeIdx(PtrAbs);
    return rtnUV;
  } //makeScalarPtr

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
    putData(0, 16, utype);
  }

  // for iterating an entire array see CompilerState::assignArrayValues
  UlamValue UlamValue::getValAt(u32 offset, CompilerState& state) const
  {
    assert(isPtr());

    UTI auti = m_uv.m_ptrValue.m_targetType;
    if(state.isClassACustomArray(auti))
      {
	UTI caType = state.getAClassCustomArrayType(auti);
	UlamType * caut = state.getUlamTypeByIndex(caType);
	s32 calen = caut->getBitSize();
	if( calen > MAXBITSPERLONG)
	  return UlamValue::makeAtom(caType);
	if(calen > MAXBITSPERINT)
	  return UlamValue::makeImmediateLong(caType, 0, state); //quietly skip for now

	return UlamValue::makeImmediate(caType, 0, state); //quietly skip for now
      }

    assert(state.getArraySize(auti) >= 0); //allow zero length arrays

    UlamValue scalarPtr = UlamValue::makeScalarPtr(*this, state);

    AssertBool isNext = scalarPtr.incrementPtr(state, offset); //incr appropriately by packed-ness
    assert(isNext);
    UlamValue atval = state.getPtrTarget(scalarPtr);

    while(atval.isPtr())
      atval = state.getPtrTarget(atval); //instead of getPtrTarget doing it (t3615 vs t3611)

    // redo what getPtrTarget use to do, when types didn't match due to
    // an element/quark or a requested scalar of an arraytype
    UTI suti = scalarPtr.getPtrTargetType();
    s32 slen = scalarPtr.getPtrLen();
    assert(slen != UNKNOWNSIZE);
    if(UlamType::compareForUlamValueAssignment(atval.getUlamValueTypeIdx(),suti, state) == UTIC_NOTSAME)
      {
	if(slen <= MAXBITSPERINT)
	  {
	    u32 datavalue = atval.getDataFromAtom(scalarPtr, state);
	    atval= UlamValue::makeImmediate(suti, datavalue, state);
	  }
	else if(slen <= MAXBITSPERLONG)
	  {
	    u64 datavalue = atval.getDataLongFromAtom(scalarPtr, state);
	    atval= UlamValue::makeImmediateLong(suti, datavalue, state);
	  }
	else
	  assert(0);
      }
    return atval;
  } //getValAt

  bool UlamValue::isPtr() const
  {
    UTI puti = getUlamValueTypeIdx();
    return ((puti == Ptr) || (puti == PtrAbs));
  } //isPtr

  PACKFIT UlamValue::isTargetPacked()
  {
    assert(isPtr());
    return (PACKFIT) m_uv.m_ptrValue.m_packed;
  } //isTargetPacked

  void UlamValue::setPtrStorage(STORAGE s)
  {
    assert(isPtr());
    m_uv.m_ptrValue.m_storagetype = s;
  } //setPtrStorage

  STORAGE UlamValue::getPtrStorage()
  {
    assert(isPtr());
    return (STORAGE) m_uv.m_ptrValue.m_storagetype;
  } //getPtrStorage

  // possibly negative into call stack (e.g. args, rtn values)
  // use: adjust "hidden" arg's pointer to atom in call stack
  void UlamValue::setPtrSlotIndex(s32 s)
  {
    assert(isPtr());
    m_uv.m_ptrValue.m_slotIndex = s;
  } //setPtrSlotIndex

  // possibly negative into call stack (e.g. args, rtn values)
  s32 UlamValue::getPtrSlotIndex()
  {
    assert(isPtr());
    return m_uv.m_ptrValue.m_slotIndex;
  } //getPtrSlotIndex

  void UlamValue::setPtrPos(u32 pos)
  {
    assert(isPtr());
    assert(pos <= BITSPERATOM && pos >= 0);
    m_uv.m_ptrValue.m_posInAtom = pos;
    return;
  } //setPtrPos

  u32 UlamValue::getPtrPos()
  {
    assert(isPtr());
    u32 pos = m_uv.m_ptrValue.m_posInAtom;
    // this will blow up the smaller BITVECTORS used in code gen for immmediates.
    //assert(pos <= BITSPERATOM && pos >= ATOMFIRSTSTATEBITPOS);
    assert(pos <= BITSPERATOM && pos >= 0);
    return pos;
  } //getPtrPos

  s32 UlamValue::getPtrLen()
  {
    assert(isPtr());
    s32 len = m_uv.m_ptrValue.m_bitlenInAtom;
    assert(len >= 0 && len <= MAXSTATEBITS); //up to caller to fix negative len
    return len;
  } //getPtrLen

  UTI UlamValue::getPtrTargetType()
  {
    assert(isPtr());
    return m_uv.m_ptrValue.m_targetType;
  } //getPtrTargetType

  void UlamValue::setPtrTargetType(UTI type)
  {
    assert(isPtr());
    m_uv.m_ptrValue.m_targetType = type;
  } //setPtrTargetType

  u16 UlamValue::getPtrNameId()
  {
    assert(isPtr());
    return m_uv.m_ptrValue.m_nameid;
  } //getPtrNameId

  void UlamValue::setPtrNameId(u32 id)
  {
    assert(isPtr());
    assert(id <= ((1 << 17) - 1));
    m_uv.m_ptrValue.m_nameid = (u16) id;
  } //setPtrNameId

  //default offset is 1
  bool UlamValue::incrementPtr(CompilerState& state, s32 offset)
  {
    assert(isPtr());
    assert(state.isScalar(getPtrTargetType())); //?
    bool rtnb = false;
    if(WritePacked((PACKFIT) m_uv.m_ptrValue.m_packed))
      {
	m_uv.m_ptrValue.m_posInAtom += (m_uv.m_ptrValue.m_bitlenInAtom * offset);
	rtnb = (m_uv.m_ptrValue.m_posInAtom < BITSPERATOM && m_uv.m_ptrValue.m_posInAtom >= 0);
      }
    else
      {
	m_uv.m_ptrValue.m_slotIndex += offset;
	if(getUlamValueTypeIdx() == Ptr)
	  {
	    //rtnb = (m_uv.m_ptrValue.m_slotIndex < BITSPERATOM && m_uv.m_ptrValue.m_slotIndex >= 0);

	    STORAGE place = getPtrStorage();
	    switch(place)
	      {
	      case STACK:
		{
		  u32 absidx = state.m_funcCallStack.getAbsoluteStackIndexOfSlot(m_uv.m_ptrValue.m_slotIndex);
		  rtnb = ((absidx >=0) && (absidx < state.m_funcCallStack.getAbsoluteTopOfStackIndexOfNextSlot()));
		}
		break;
	      case EVALRETURN:
		{
		  u32 absidx = state.m_nodeEvalStack.getAbsoluteStackIndexOfSlot(m_uv.m_ptrValue.m_slotIndex);
		  rtnb = ((absidx >=0) && (absidx < state.m_nodeEvalStack.getAbsoluteTopOfStackIndexOfNextSlot()));
		}
		break;
	      case EVENTWINDOW:
		{
		  Coord c;
		  rtnb = state.m_eventWindow.isValidSite(m_uv.m_ptrValue.m_slotIndex, c);
		}
		break;
	      default:
		assert(0);
	      };
	  }
	else if(getUlamValueTypeIdx() == PtrAbs)
	  {
	    STORAGE place = getPtrStorage();
	    switch(place)
	      {
	      case STACK:
		rtnb = (m_uv.m_ptrValue.m_slotIndex > 0) && ((u32) m_uv.m_ptrValue.m_slotIndex < state.m_funcCallStack.getAbsoluteTopOfStackIndexOfNextSlot());
		break;
	      case EVALRETURN:
		rtnb = (m_uv.m_ptrValue.m_slotIndex > 0) && ((u32) m_uv.m_ptrValue.m_slotIndex < state.m_nodeEvalStack.getAbsoluteTopOfStackIndexOfNextSlot());
		break;
	      case EVENTWINDOW:
		{
		  Coord c;
		  rtnb = state.m_eventWindow.isValidSite(m_uv.m_ptrValue.m_slotIndex, c);
		}
		break;
	      default:
		assert(0);
	      };
	  }
	else
	  assert(0);
      }
    return rtnb;
  } //incrementPtr

  // bigger than an int, yet packable, extract array from data
  // where ptr p refers to its start pos/len in data;
  // returns data right justified so that its type will indicate its start.
  // note: cannot be an array of elements since they aren't packable; may be loadable
  UlamValue UlamValue::getPackedArrayDataFromAtom(UlamValue p, UlamValue data, CompilerState& state)
  {
    UTI tuti = p.getPtrTargetType();
    //assert(data.getUlamValueTypeIdx() == tuti); not if 'data' is an element

    UlamValue rtnUV; //static return
    rtnUV.setUlamValueTypeIdx(tuti);

    s32 arraysize = state.getArraySize(tuti);
    arraysize = (arraysize == NONARRAYSIZE ? 1 : arraysize); //possible scalar if element or quark
    assert(arraysize > NONARRAYSIZE);
    s32 bitsize = state.getBitSize(tuti);
    s32 len = p.getPtrLen();
    assert( (bitsize * arraysize) == len);

    if(p.isTargetPacked() == PACKEDLOADABLE)
      {
	if(len <= MAXBITSPERINT)
	  {
	    u32 datavalue = data.getData(p.getPtrPos(), len); //entire array
	    rtnUV.putData((BITSPERATOM-len), len, datavalue); //immediate
	  }
	else if(len <= MAXBITSPERLONG)
	  {
	    u64 datavalue = data.getDataLong(p.getPtrPos(), len); //entire array
	    rtnUV.putDataLong((BITSPERATOM-len), len, datavalue); //immediate
	  }
	else
	  assert(0);
      }
    else
      {
	assert(p.isTargetPacked() == PACKED);
	// base [0] is furthest from the end
	UlamValue nextPtr = UlamValue::makeScalarPtr(p,state);
	s32 itemlen = nextPtr.getPtrLen();
	for(s32 i = 0; i < arraysize; i++)
	  {
	    if(itemlen <= MAXBITSPERINT)
	      {
		u32 datavalue = data.getData(nextPtr.getPtrPos(), nextPtr.getPtrLen());
		rtnUV.putData((BITSPERATOM-(bitsize * (arraysize - i))), bitsize, datavalue);
	      }
	    else if(len <= MAXBITSPERLONG)
	      {
		u64 datavalue = data.getDataLong(nextPtr.getPtrPos(), nextPtr.getPtrLen());
		rtnUV.putDataLong((BITSPERATOM-(bitsize * (arraysize - i))), bitsize, datavalue);
	      }
	    else
	      assert(0);

	    nextPtr.incrementPtr(state);
	  }
      }
    return rtnUV;
  } //getPackedArrayDataFromAtom

  u32 UlamValue::getDataFromAtom(UlamValue p, CompilerState& state) const
  {
    UTI duti = getUlamValueTypeIdx();
    //assert(duti == p.getPtrTargetType()); this could be an 'unpacked' element
    if(isPtr())
      duti = (const_cast<UlamValue *>(this))->getPtrTargetType();

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
	UlamType * dut = state.getUlamTypeByIndex(duti);
	if((dut->getUlamClass() == UC_NOTACLASS) && (dut->getUlamTypeEnum() != UAtom))
	  datavalue = getImmediateData(p.getPtrLen(), state);
	else
	  datavalue = getData(p.getPtrPos(), p.getPtrLen());
      }
    return datavalue;
  } //getDataFromAtom overloaded

  u32 UlamValue::getDataFromAtom(u32 pos, s32 len) const
  {
    assert(len >= 0);
    ///if not an atom, element
    return getData(pos,len);
  }

  u64 UlamValue::getDataLongFromAtom(UlamValue p, CompilerState& state) const
  {
    UTI duti = getUlamValueTypeIdx();
    //assert(duti == p.getPtrTargetType()); this could be an 'unpacked' element

    if(!state.isScalar(p.getPtrTargetType()) && p.isTargetPacked() == PACKED)
      {
	//must get data piecemeal, too big to fit into one int
	//use getPackedArrayDataIntoAtom(p, data, state);
	assert(0);
      }

    assert(p.getPtrLen() <= MAXBITSPERLONG);

    //either a packed-loadable array or scalar
    u64 datavalue;
    PACKFIT packedData = state.determinePackable(duti);
    //assert(p.isTargetPacked() == packedData); untrue if this is an element

    if(WritePacked(packedData)) //scalar packed within
      {
	datavalue = getDataLong(p.getPtrPos(), p.getPtrLen());
      }
    else
      {
	UlamType * dut = state.getUlamTypeByIndex(duti);
	if((dut->getUlamClass() == UC_NOTACLASS) && (dut->getUlamTypeEnum() != UAtom))
	  datavalue = getImmediateDataLong(p.getPtrLen());
	else
	  datavalue = getDataLong(p.getPtrPos(), p.getPtrLen());
      }
    return datavalue;
  } //getDataLongFromAtom overloaded

  u64 UlamValue::getDataLongFromAtom(u32 pos, s32 len) const
  {
    assert(len >= 0);
    ///if not an atom, element
    return getDataLong(pos,len);
  }

  u32 UlamValue::getImmediateData(CompilerState & state) const
  {
    UTI utype = getUlamValueTypeIdx();
    s32 len = state.getBitSize(utype); //scalar
    assert(len != UNKNOWNSIZE);
    assert(len <= MAXBITSPERINT);

    if(len == 0)
      return 0;

    return getImmediateData(len, state);
  } //getImmediateData

  u32 UlamValue::getImmediateData(s32 len, CompilerState & state) const
  {
    UTI utype = getUlamValueTypeIdx();
    ULAMTYPE eutype = state.getUlamTypeByIndex(utype)->getUlamTypeEnum();
    assert(eutype != UAtom);
    assert(utype != Ptr);
    assert(utype != Nav);
    assert(len >= 0 && len <= MAXBITSPERINT);

    if(eutype == Class)
      return getImmediateQuarkData(len);

    return getData(BITSPERATOM - len, len);
  } //getImmediateData const

  u32 UlamValue::getImmediateQuarkData(CompilerState & state) const
  {
    s32 len = state.getBitSize(getUlamValueTypeIdx()); //scalar
    assert(len != UNKNOWNSIZE);
    assert(len <= MAXBITSPERINT);

    if(len == 0)
      return 0;

    return getImmediateQuarkData(len); //left-justified
  } //getImmediateQuark

  u32 UlamValue::getImmediateQuarkData(s32 len) const
  {
    UTI utype = getUlamValueTypeIdx();
    //ULAMTYPE eutype = m_state.getUlamTypeByIndex(utype)->getUlamTypeEnum();
    AssertBool utypOk = ((utype != UAtom) && (utype != Ptr) && (utype != Nav));
    assert(utypOk);
    assert(len >= 0 && len <= MAXBITSPERINT);
    return getData(ATOMFIRSTSTATEBITPOS, len);
  } //getImmediateQuarkData const

  u64 UlamValue::getImmediateDataLong(CompilerState & state) const
  {
    s32 len = state.getBitSize(getUlamValueTypeIdx());
    assert(len != UNKNOWNSIZE);
    assert(len <= MAXBITSPERLONG);

    if(len == 0)
      return 0;

    return getImmediateDataLong(len);
  } //getImmediateDataLong

  u64 UlamValue::getImmediateDataLong(s32 len) const
  {
    UTI utype = getUlamValueTypeIdx();
    //ULAMTYPE eutype = state.getUlamTypeByIndex(utype)->getUlamTypeEnum();
    AssertBool utypOk = ((utype != UAtom) && (utype != Ptr) && (utype != Nav));
    assert(utypOk);
    assert(len >= 0 && len <= MAXBITSPERLONG);
    return getDataLong(BITSPERATOM - len, len);
  } //getImmediateDataLong const

  // 'p' is Ptr into this destination; assumes 'data' is right-justified, unless a quark!
  void UlamValue::putDataIntoAtom(UlamValue p, UlamValue data, CompilerState& state)
  {
    // apparently amused by other types..
    UTI duti = data.getUlamValueTypeIdx();
    UTI puti = p.getPtrTargetType();
    AssertBool comparesOk = (UlamType::compareForUlamValueAssignment(duti, puti, state) == UTIC_SAME) || (UlamType::compareForUlamValueAssignment(getUlamValueTypeIdx(), puti, state));
    assert(comparesOk); //ALL-PURPOSE!

    if(p.isTargetPacked() == PACKED)
      {
	if(!state.isScalar(puti))
	  {
	    //must get data piecemeal, too big to fit into one int
	    putPackedArrayDataIntoAtom(p, data, state);
	  }
	else if(state.getUlamTypeByIndex(puti)->getPackable() == PACKEDLOADABLE)
	  {
	    // array item that's loadable
	    s32 len = p.getPtrLen();
	    assert(len != UNKNOWNSIZE);

	    if(len <= MAXBITSPERINT)
	      {
		u32 datavalue = data.getImmediateData(len, state);
		putData(p.getPtrPos(), len, datavalue);
	      }
	    else if(len <= MAXBITSPERLONG)
	      {
		u64 datavalue = data.getImmediateDataLong(len);
		putDataLong(p.getPtrPos(), len, datavalue);
	      }
	    else
	      assert(0);
	  }
	else
	  // e.g. an element, take wholesale
	  assert(0);
      }
    else
      {
	assert(p.isTargetPacked() == PACKEDLOADABLE);
	s32 len = p.getPtrLen();
	assert(len != UNKNOWNSIZE);
	if(len <= MAXBITSPERINT)
	  {
	    u32 datavalue = data.getImmediateData(len, state);
	    putData(p.getPtrPos(), len, datavalue);
	  }
	else if(len <= MAXBITSPERLONG)
	  {
	    u64 datavalue = data.getImmediateDataLong(len);
	    putDataLong(p.getPtrPos(), len, datavalue);
	  }
	else
	  assert(0);
      }
  } //putDataIntoAtom

  // bigger than an int, yet packable, array data; assume 'data' is
  // right-justified, and 'p' refers to this' destination.
  void UlamValue::putPackedArrayDataIntoAtom(UlamValue p, UlamValue data, CompilerState& state)
  {
    UTI tuti = p.getPtrTargetType();
    assert(UlamType::compareForUlamValueAssignment(data.getUlamValueTypeIdx(),tuti, state) == UTIC_SAME);
    s32 arraysize = state.getArraySize(tuti);
    assert(arraysize > NONARRAYSIZE);
    s32 bitsize = state.getBitSize(tuti);

    UlamValue nextPPtr = UlamValue::makeScalarPtr(p,state);
    assert(bitsize == nextPPtr.getPtrLen());

    for(s32 i = 0; i < arraysize; i++)
      {
	if(bitsize <= MAXBITSPERINT)
	  {
	    //assume data is right-justified, base [0] is furthest from the end
	    u32 datavalue = data.getData((BITSPERATOM-(bitsize * (arraysize - i))), bitsize);
	    putData(nextPPtr.getPtrPos(), nextPPtr.getPtrLen(), datavalue);
	  }
	else if(bitsize <= MAXBITSPERLONG)
	  {
	    //assume data is right-justified, base [0] is furthest from the end
	    u64 datavalue = data.getDataLong((BITSPERATOM-(bitsize * (arraysize - i))), bitsize);
	    putDataLong(nextPPtr.getPtrPos(), nextPPtr.getPtrLen(), datavalue);
	  }
	AssertBool isNext = nextPPtr.incrementPtr(state);
	assert(isNext);
      }
  } //putPackedArrayDataIntoAtom

  u32 UlamValue::getData(u32 pos, s32 len) const
  {
    assert(len >= 0);
    AtomBitVector a(m_uv.m_storage.m_atom); //copy
    return a.Read(pos, len);
  } //getData

  u64 UlamValue::getDataLong(u32 pos, s32 len) const
  {
    assert(len >= 0);
    AtomBitVector a(m_uv.m_storage.m_atom); //copy
    return a.ReadLong(pos, len);
  } //getData

  void UlamValue::putData(u32 pos, s32 len, u32 data)
  {
    assert(len >= 0);
    AtomBitVector a(m_uv.m_storage.m_atom); //copy
    a.Write(pos, len, data);
    a.ToArray(m_uv.m_storage.m_atom);
  } //putData

  void UlamValue::putDataLong(u32 pos, s32 len, u64 data)
  {
    assert(len >= 0);
    assert(pos + len <= BITSPERATOM);
    AtomBitVector a(m_uv.m_storage.m_atom); //copy
    a.WriteLong(pos, len, data);
    a.ToArray(m_uv.m_storage.m_atom);
  } //putData

  UlamValue& UlamValue::operator=(const UlamValue& rhs)
  {
    for(u32 i = 0; i < AtomBitVector::ARRAY_LENGTH; i++)
      {
	m_uv.m_storage.m_atom[i] = rhs.m_uv.m_storage.m_atom[i];
      }
    return *this;
  } //op=

  bool UlamValue::operator==(const UlamValue& rhs)
  {
    bool allTheSame = true;
    for(u32 i = 0; i < AtomBitVector::ARRAY_LENGTH; i++)
      {
	if(m_uv.m_storage.m_atom[i] != rhs.m_uv.m_storage.m_atom[i])
	  {
	    allTheSame = false;
	    break;
	  }
      }
    return allTheSame;
  } //op==

  void UlamValue::genCodeBitField(File * fp, CompilerState& state)
  {
    assert(isPtr());
    UTI vuti = getPtrTargetType();
    UlamType * vut = state.getUlamTypeByIndex(vuti);

    fp->write("BitField<BPA,");
    fp->write(vut->getUlamTypeVDAsStringForC().c_str()); //VD type
    fp->write(",");
    fp->write_decimal(vut->getTotalBitSize());
    fp->write(",");
    fp->write_decimal(m_uv.m_ptrValue.m_posInAtom); //use directly for smaller bitvectors in gen code.
    fp->write(">");
  } //genCodeBitField

} //end MFM

#include <assert.h>
#include <stdio.h>
#include "CompilerState.h"
#include "UlamType.h"
#include "UVPass.h"

namespace MFM {

  UVPass::UVPass() : m_varNum(0), m_posInStorage(0), m_bitlenInStorage(0), m_storagetype(TMPREGISTER), m_packed(UNPACKED), m_targetType(Nouti), m_nameid(0) { }

  UVPass::~UVPass() { }

  UVPass UVPass::makePass(u32 varnum, TMPSTORAGE storage, UTI targetType, PACKFIT packed, CompilerState& state, u32 pos, u32 id)
  {
    UVPass rtnUV; //static method
    rtnUV.m_varNum = varnum;
    rtnUV.m_posInStorage = pos;

    //NOTE: 'len' of a packed-array,
    //       becomes the total size (bits * arraysize);
    //       'len' is item bitsize for unpacked-array;
    //       constants become default len;
    u32 len;
    if(packed == UNPACKED)
      len = state.getBitSize(targetType);
    else
      len = state.getTotalBitSize(targetType);

    rtnUV.m_bitlenInStorage = len; //if packed, entire array len
    rtnUV.m_storagetype = storage;
    rtnUV.m_packed = packed;
    rtnUV.m_targetType = targetType;
    rtnUV.setPassNameId(id);
    return rtnUV;
  } //makePass

  PACKFIT UVPass::isTargetPacked()
  {
    return (PACKFIT) m_packed;
  } //isTargetPacked

  void UVPass::setPassStorage(TMPSTORAGE s)
  {
    m_storagetype = s;
  } //setPassStorage

  TMPSTORAGE UVPass::getPassStorage()
  {
    return (TMPSTORAGE) m_storagetype;
  } //getPassStorage

  void UVPass::setPassVarNum(s32 s)
  {
    m_varNum = s;
  } //setPassSlotIndex

  s32 UVPass::getPassVarNum()
  {
    return m_varNum;
  } //getPassSlotIndex

  void UVPass::setPassPos(u32 pos)
  {
    assert((pos <= getPassLen()) && pos >= 0);
    m_posInStorage = pos;
    return;
  } //setPassPos

  void UVPass::setPassPosForElementType(u32 pos, CompilerState& state)
  {
    assert((pos < BITSPERATOM) && pos >= 0);
    assert(state.getUlamTypeByIndex(this->getPassTargetType())->getUlamClassType() == UC_ELEMENT); //sanity
    m_posInStorage = pos;
    return;
  } //setPassPosForElementType

  u32 UVPass::getPassPos()
  {
    u32 pos = m_posInStorage;
    //assert(pos <= getPassLen() && pos >= 0);
    assert(pos >= 0); //data member pos may go beyonds its own length
    return pos;
  } //getPassPos

  u32 UVPass::getPassLen()
  {
    u32 len = m_bitlenInStorage;
    assert(len >= 0);
    return len;
  } //getPassLen

  UTI UVPass::getPassTargetType()
  {
    return m_targetType;
  } //getPassTargetType

  void UVPass::setPassTargetType(UTI type)
  {
    m_targetType = type;
  } //setPassTargetType

  u32 UVPass::getPassNameId()
  {
    return m_nameid;
  } //getPassNameId

  void UVPass::setPassNameId(u32 id)
  {
    m_nameid = id;
  } //setPassNameId

  const std::string UVPass::getTmpVarAsString(CompilerState & state)
  {
    return state.getTmpVarAsString(getPassTargetType(), getPassVarNum(), getPassStorage());
  }

} //end MFM

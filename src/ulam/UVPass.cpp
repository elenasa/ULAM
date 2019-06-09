#include <assert.h>
#include <stdio.h>
#include "CompilerState.h"
#include "UlamType.h"
#include "UVPass.h"

namespace MFM {

  UVPass::UVPass() : m_varNum(0), m_posInStorage(0), m_applyDelta(false), m_bitlenInStorage(0), m_storagetype(TMPREGISTER), m_packed(UNPACKED), m_targetType(Nouti), m_nameid(0) { }

  UVPass::~UVPass() { }

  UVPass UVPass::makePass(u32 varnum, TMPSTORAGE storage, UTI targetType, PACKFIT packed, CompilerState& state, u32 pos, u32 id)
  {
    return UVPass::makePass(varnum, storage, targetType, packed, state, pos, false, id);
  }

  UVPass UVPass::makePass(u32 varnum, TMPSTORAGE storage, UTI targetType, PACKFIT packed, CompilerState& state, u32 pos, bool applydelta, u32 id)
  {
    UVPass rtnUV; //static method
    rtnUV.m_varNum = varnum;
    rtnUV.m_posInStorage = pos;
    rtnUV.m_applyDelta = applydelta;

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

  PACKFIT UVPass::isTargetPacked() const
  {
    return (PACKFIT) m_packed;
  }

  void UVPass::setPassStorage(TMPSTORAGE s)
  {
    m_storagetype = s;
  }

  TMPSTORAGE UVPass::getPassStorage() const
  {
    return (TMPSTORAGE) m_storagetype;
  }

  void UVPass::setPassVarNum(s32 s)
  {
    m_varNum = s;
  }

  s32 UVPass::getPassVarNum() const
  {
    return m_varNum;
  }

  void UVPass::setPassPos(u32 pos)
  {
    assert((pos <= getPassLen()));
    m_posInStorage = pos;
    return;
  }

  void UVPass::setPassPosForced(u32 pos)
  {
    //assert((pos <= getPassLen()));
    m_posInStorage = pos;
    return;
  }

  void UVPass::setPassPosForElementType(u32 pos, CompilerState& state)
  {
    //t3968 element dm in transient can have pos > 96
    assert(((this->getPassLen() + ATOMFIRSTSTATEBITPOS) <= BITSPERATOM));
    assert(state.getUlamTypeByIndex(this->getPassTargetType())->getUlamClassType() == UC_ELEMENT); //sanity
    m_posInStorage = pos + ATOMFIRSTSTATEBITPOS;
    return;
  }

  u32 UVPass::getPassPos() const
  {
    return m_posInStorage; //data member pos may go beyonds its own length
  }

  u32 UVPass::getPassLen() const
  {
    return m_bitlenInStorage;
  }

  bool UVPass::getPassApplyDelta() const
  {
    return m_applyDelta;
  }

  void UVPass::setPassApplyDelta(bool apply)
  {
    m_applyDelta = apply;
  }

  UTI UVPass::getPassTargetType() const
  {
    return m_targetType;
  }

  void UVPass::setPassTargetType(UTI type)
  {
    m_targetType = type;
  }

  u32 UVPass::getPassNameId() const
  {
    return m_nameid;
  }

  void UVPass::setPassNameId(u32 id)
  {
    m_nameid = id;
  }

  const std::string UVPass::getTmpVarAsString(CompilerState & state) const
  {
    return state.getTmpVarAsString(getPassTargetType(), getPassVarNum(), getPassStorage());
  }

} //end MFM

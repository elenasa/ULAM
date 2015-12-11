#include "MapClassMemberDesc.h"
#include "CompilerState.h"

namespace MFM {

  ClassMemberDesc::ClassMemberDesc(Symbol * sym, UTI classtype, CompilerState & state)
  {
    assert(sym);
    //similar to SymbolClass' addTargetDescriptionMapEntry for class targets
    m_loc = sym->getLoc();
    m_mangledClassName = state.getUlamTypeByIndex(classtype)->getUlamTypeMangledName();
    m_mangledType = state.getUlamTypeByIndex(sym->getUlamTypeIdx())->getUlamTypeMangledName();
    m_memberName = state.m_pool.getDataAsString(sym->getId());
    m_mangledMemberName = sym->getMangledName();
    Token scTok;
    if(sym->getStructuredComment(scTok))
      {
	std::ostringstream sc;
	sc << "/**";
	sc << state.m_pool.getDataAsString(scTok.m_dataindex).c_str();
	sc << "*/";
	m_structuredComment = sc.str();
      }
    else
      m_structuredComment = "NONE";
  }

  ClassMemberDesc::ClassMemberDesc(const ClassMemberDesc& cref) : m_loc(cref.m_loc), m_mangledClassName(cref.m_mangledClassName), m_mangledType(cref.m_mangledType), m_memberName(cref.m_memberName), m_mangledMemberName(cref.m_mangledMemberName), m_structuredComment(cref.m_structuredComment) {}

  ClassMemberDesc::~ClassMemberDesc() {}

  bool ClassMemberDesc::getValue(u64& vref) const
  {
    return false;
  }

} //MFM

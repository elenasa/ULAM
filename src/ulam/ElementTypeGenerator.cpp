#include "ElementTypeGenerator.h"
#include <assert.h>

namespace MFM {

  ElementTypeGenerator::ElementTypeGenerator() : m_maxNumberOfTypes(MAX_ELEMENT_TYPE), m_next(0), m_counter(2) { }

  ElementTypeGenerator::~ElementTypeGenerator()
  {
    m_sequentialTypeMap.clear();
    m_setOfKnownTypes.clear();
    m_mapOfKnownTypes.clear();
  }

  ELE_TYPE ElementTypeGenerator::makeNextType(UTI cuti)
  {
    //incremental version
    assert(m_next < m_maxNumberOfTypes);
    ELE_TYPE type = makeAType(cuti);
    m_sequentialTypeMap[m_next] = type;
    AssertBool added = addTypeToMap(type,cuti);
    assert(added);
    return m_sequentialTypeMap[m_next++];
  }

  UTI ElementTypeGenerator::findElementTypeClass(ELE_TYPE ele)
  {
    std::map<ELE_TYPE, UTI>::iterator it = m_mapOfKnownTypes.find(ele);

    if(it != m_mapOfKnownTypes.end())
      {
	assert(ele == it->first);
	return it->second;
      }
    return Nouti; //not found
  }

  /////////////////////////////////////////////////
  //
  // PRIVATE METHODS
  //
  ////////////////////////////////////////////////

  //borrowed from MFM ElementTypeNumberMap<EC>: a fixed sequence,
  // where types are far apart, and get progressively closer with more elements;
  // Empty is a special case that is handled separately; zero is the undefined type;
  ELE_TYPE ElementTypeGenerator::makeAType(UTI cuti)
  {
    u32 type;
    do {
      ++m_counter;
      s32 highidx = 31;
      while (highidx > 1 && ((m_counter & (1 << highidx)) == 0))
        --highidx;

      u32 bitsPerBit = ELEMENT_TYPE_BITS / highidx;
      u32 bits = (1<<bitsPerBit) - 1;

      type = 0;
      while (--highidx >= 0) {
        type <<= bitsPerBit;
        if (m_counter & (1 << highidx)) {
          type |= bits;
        }
      }
      //      LOG.Debug("trying 0x%04x",type);
    } while (type == EMPTY_ELEMENT_TYPE || type == UNDEFINED_ELEMENT_TYPE || !addTypeToSet(type));
    return type;
  } //makeAType

  bool ElementTypeGenerator::addTypeToSet(ELE_TYPE type)
  {
    std::pair<std::set<ELE_TYPE>::iterator,bool> ret;
    ret = m_setOfKnownTypes.insert(type);
    return ret.second; //false if already in the set, true if newly added
  }

  bool ElementTypeGenerator::addTypeToMap(ELE_TYPE type, UTI cuti)
  {
    std::pair<std::map<ELE_TYPE,UTI>::iterator,bool> ret;
    ret = m_mapOfKnownTypes.insert(std::pair<ELE_TYPE, UTI>(type,cuti));
    return ret.second; //false if already in the set, true if newly added
  }

} //MFM

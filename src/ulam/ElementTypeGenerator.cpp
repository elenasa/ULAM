#include "ElementTypeGenerator.h"
#include <assert.h>

namespace MFM {

  ElementTypeGenerator::ElementTypeGenerator() : m_maxNumberOfTypes(MAX_ELEMENT_TYPE), m_next(0), m_counter(2) { }

  ElementTypeGenerator::ElementTypeGenerator(u32 n) : m_maxNumberOfTypes(n), m_next(U32_MAX), m_counter(2) {
    assert(n > 0 && n < MAX_ELEMENT_TYPE);
    generateTypes();
  }

  ElementTypeGenerator::~ElementTypeGenerator() {}

  void ElementTypeGenerator::generateTypes()
  {
    for(u32 i = 0; i < m_maxNumberOfTypes; i++)
      {
	//m_sequentialTypeMap[i] = i + 1; LAME FIRST-CUT
	u32 type = makeAType();
	m_sequentialTypeMap[i] = type;
      }
  } //generateTypes

  void ElementTypeGenerator::beginIteration()
  {
    m_next = 0;
  }

  ELE_TYPE ElementTypeGenerator::getNextType()
  {
    assert(m_next < m_maxNumberOfTypes);
    return m_sequentialTypeMap[m_next++];
  }

  ELE_TYPE ElementTypeGenerator::makeNextType()
  {
    //incremental version
    assert(m_next < m_maxNumberOfTypes);
    ELE_TYPE type = makeAType();
    m_sequentialTypeMap[m_next] = type;
    return m_sequentialTypeMap[m_next++];
  }

  /////////////////////////////////////////////////
  //
  // PRIVATE METHODS
  //
  ////////////////////////////////////////////////

  //borrowed from MFM ElementTypeNumberMap<EC>: a fixed sequence,
  // where types are far apart, and get progressively closer with more elements;
  // Empty is a special case that is handled separately; zero is the undefined type;
  ELE_TYPE ElementTypeGenerator::makeAType()
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

} //MFM

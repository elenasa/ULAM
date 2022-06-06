#include "Locator.h"

namespace MFM {

  Locator::Locator(u32 idx) :
    m_pathIdx(idx), m_fullPathIdx(0), m_lineNo(0), m_byteNo(0) {}
  Locator::~Locator() {}

  u32 Locator::getPathIndex() const
  {
    return m_pathIdx;
  }

  u32 Locator::getFullPathIndex() const
  {
    return m_fullPathIdx;
  }

  void Locator::setPathIndex(u32 idx)
  {
    m_pathIdx = idx;
  }

  void Locator::setFullPathIndex(u32 idx)
  {
    m_fullPathIdx = idx;
  }

  u16 Locator::getLineNo() const
  {
    return m_lineNo;
  }

  u16 Locator::getByteNo() const
  {
    return m_byteNo;
  }

  void Locator::updateLineByteNo(s32 c)
  {
    if(c < 0) //end of file
      {
	return;
      }

    if( m_lineNo == 0)  // uninitialized locator?
      {
	m_lineNo = 1;   // init it
      }

    if( c == '\n')
      {
	m_byteNo = 0;
	m_lineNo++;
      }
    else
      {
	m_byteNo++;
      }
  }

  bool Locator::hasNeverBeenRead() const
  {
    return ( m_lineNo == 0 && m_byteNo == 0);
  }

  bool Locator::operator<(const Locator & loc2) const
  {
    if(m_pathIdx < loc2.m_pathIdx) return true;  //?
    if(m_pathIdx > loc2.m_pathIdx) return false; //?

    if(m_lineNo < loc2.m_lineNo) return true;
    if(m_lineNo > loc2.m_lineNo) return false;

    if(m_byteNo < loc2.m_byteNo) return true;
    if(m_byteNo > loc2.m_byteNo) return false;

    return false;
  }

  bool Locator::operator==(const Locator & loc2) const
  {
    return((m_pathIdx == loc2.m_pathIdx) && (m_lineNo == loc2.m_lineNo) &&(m_byteNo == loc2.m_byteNo));
  }


} //end MFM

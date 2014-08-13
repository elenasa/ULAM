#include "Locator.h"

namespace MFM {

  Locator::Locator(u32 idx) : m_pathIdx(idx), m_lineNo(0), m_byteNo(0) {}
  Locator::~Locator() {}

  u32 Locator::getPathIndex()
  {
    return m_pathIdx;
  }

  void Locator::setPathIndex(u32 idx)
  {
    m_pathIdx = idx;
  }

  u16 Locator::getLineNo()
  {
    return m_lineNo;
  }

  u16 Locator::getByteNo()
  {
    return m_byteNo;
  }

  void Locator::updateLineByteNo(s32 c)
  {
    if(c < 0) //end of file
      {
	return;
      }
    
    if( c == '\n')
      {
	m_byteNo = 0;
	m_lineNo++;
      }
    else if( m_lineNo == 0)
      {
	m_lineNo++;
	m_byteNo=1;
      }
    else
      {
	m_byteNo++;
      }
  }

  bool Locator::hasNeverBeenRead()
  {
    return ( m_lineNo == 0 && m_byteNo == 0);
  }

} //end MFM

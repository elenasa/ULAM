/* -*- c++ -*- */

#ifndef LOCATOR_H
#define LOCATOR_H

#include "itype.h"

namespace MFM{

class Locator
{
 public:
  Locator(u32 id=0);
  ~Locator();

  u32 getPathIndex();
  u16 getLineNo();
  u16 getByteNo();

  void setPathIndex(u32 idx);

  void updateLineByteNo(s32 c);

  bool hasNeverBeenRead();

 private:
  u32 m_pathIdx;
  u16 m_lineNo;
  u16 m_byteNo;
};

}

#endif  /* LOCATOR_H */

/**                                        -*- mode:C++ -*/

#ifndef ULAMBITSIZETEST_H
#define ULAMBITSIZETEST_H

#include "itype.h"
#include "/home/elenas/WORK/ulam/repo/MFMv2/src/core/include/Util.h"
#include "./bpstat.h"

namespace MFM{

  enum TestMode { _byInt, _byUnsigned, _byUnary, _byLast };

#define DOABS 1
#define TESTCASES 4

class UlamBitsizeTest
{

public:
  UlamBitsizeTest(u32 odds, u32 testval, enum TestMode mode);
  ~UlamBitsizeTest();

  void play();
  void done();

private:
  const u32 m_odds;
  const u32 m_testval;  //max testvalue, 1 to
  const enum TestMode m_mode;

  bpstat m_bpstatsarray[TESTCASES];

  void init();
  void playme(u32 bz, u32 index);

};

} //MFM

#endif //ULAMBITSIZETEST_H

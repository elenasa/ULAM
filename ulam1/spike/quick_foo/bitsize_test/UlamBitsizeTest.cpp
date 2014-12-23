#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "UlamBitsizeTest.h"

namespace MFM
{

  UlamBitsizeTest::UlamBitsizeTest(u32 odds, u32 testval, enum TestMode mode) : m_odds(odds), m_testval(testval), m_mode(mode) { init(); }

  UlamBitsizeTest::~UlamBitsizeTest() {}


  void UlamBitsizeTest::init()
  {
    srand(time(NULL));
    for(u32 i = 0; i < 32; i++)
      {
	m_bpstatsarray[i].clear();
    }
  }

  void UlamBitsizeTest::play()
  {
    for(u32 i = 0; i < 32; i++)
      {
	playme(i);
      }
  }


  void UlamBitsizeTest::playme(u32 bz)
  {
    u32 mask = 0;
    for(u32 i = 0; i < bz; i++)
      {
	u32 r = rand() % m_odds;
	if(r == 0)
	  {
	    mask |= (1 << i);
	  }
      }

    u32 v = m_testval ^ mask;
    s32 d = -1;  // = v - m_testval;
    switch(m_mode)
      {
      case _byInt:
	d = _SignExtend32(v, bz) - m_testval;
	break;
      case _byUnsigned:
	d = v - m_testval;
	break;
      case _byUnary:
	d = PopCount(v) - m_testval;
	break;
      default:
	assert(0);
      };

    // update statistics
    m_bpstatsarray[bz].update(DOABS ? abs(d) : (d * d));
  } //playme


  void UlamBitsizeTest::done()
  {
    printf("#bits mean std max min n\n");
    for(u32 i = 0; i < 32; i++)
      {
	//playme(i);
	printf("%d %f %f %d %d %d\n", i,
	       m_bpstatsarray[i].getMean(),
	       m_bpstatsarray[i].getSD(),
	       m_bpstatsarray[i].getMax(),
	       m_bpstatsarray[i].getMin(),
	       m_bpstatsarray[i].getNumValues()
	       );
      }
  }

} //MFM

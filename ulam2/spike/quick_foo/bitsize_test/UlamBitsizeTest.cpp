#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "UlamBitsizeTest.h"

namespace MFM
{

  static const u32 CseBitSize[] = { 0, 8, 16, 32 };
  static const u32 minbitsizebymode[_byLast] = {4, 3, 6};

  UlamBitsizeTest::UlamBitsizeTest(u32 odds, u32 testval, enum TestMode mode) : m_odds(odds), m_testval(testval), m_mode(mode) { init(); }

  UlamBitsizeTest::~UlamBitsizeTest() {}


  void UlamBitsizeTest::init()
  {
    //srand(time(NULL));
    // for each type/size: min, 8, 32
    for(u32 i = 0; i < TESTCASES; i++)
      {
	m_bpstatsarray[i].clear();
      }
  }


  void UlamBitsizeTest::play()
  {
    for(u32 i = 0; i < TESTCASES; i++)
      {
	int bz = (i == 0 ? minbitsizebymode[m_mode] : CseBitSize[i]);
	playme(bz, i);
      }
  }


  void UlamBitsizeTest::playme(u32 bz, u32 index)
  {
    assert(DOABS);
    // t goes from 1 to 6
    for(u32 t = 1; t < m_testval; t++)
      {
	//unary is right-justified
	u32 n = m_mode == _byUnary ? _GetNOnes32(t) : t;

	//bz is min per mode, 8 and 32; e.g. p is 0 to 31
	for(u32 p = 0; p < bz; p++)
	  {
	    u32 v = n ^ (1 << p);
	    s32 d;
	    switch(m_mode)
	      {
	      case _byInt:
		{
		  d = _SignExtend32(v, bz+1) - n;
		  assert(labs(d) == labs(1 << p));
		}
		break;
	      case _byUnsigned:
		{
		  d =  v - n;
		  assert(labs(d) == labs(1 << p));
		}
		break;
	      case _byUnary:
		{
		  d = PopCount(v) - (s32) t;  //converted to binary
		  assert(d == 1 || d == -1);
		}
		break;
	      default:
		assert(0);
	      };

	    u32 err = labs(d);
	    double relerr = err/(t * 1.0);

	    // update statistics
	    m_bpstatsarray[index].update(relerr);
	  }
      }
  } //playme

#if 0
  void UlamBitsizeTest::PLAYME(u32 bz)
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

    u32 t = (rand() % m_testval) + 1;
    u32 v = t ^ mask;
    s32 d = -1;  // = v - m_testval;
    switch(m_mode)
      {
      case _byInt:
	d = _SignExtend32(v, bz) - t;
	break;
      case _byUnsigned:
	d = v - t;
	break;
      case _byUnary:
	d = PopCount(v) - t;
	break;
      default:
	assert(0);
      };

    // update statistics
    m_bpstatsarray[bz].update(DOABS ? abs(d) : (d * d));
  } //playme
#endif

  void UlamBitsizeTest::done()
  {

    printf("#bits mean std max min n\n");
    for(u32 i = 0; i < TESTCASES; i++)
      {
	int bz = (i == 0 ? minbitsizebymode[m_mode] : CseBitSize[i]);
	//playme(i);
	printf("%d %f %f %f %f %d\n", bz,
	       m_bpstatsarray[i].getMean(),
	       m_bpstatsarray[i].getSD(),
	       m_bpstatsarray[i].getMax(),
	       m_bpstatsarray[i].getMin(),
	       m_bpstatsarray[i].getNumValues()
	       );
      }
  }

} //MFM

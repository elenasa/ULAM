/**                                        -*- mode:C++ -*/

#include "../../include/itype.h"
//#include "foo-bitsize_stat.h"
#include "UlamBitsizeTest.h"

#define RUNS 1000000
#define ODDS 1000000
#define TESTVAL 1
#define MODES 3

int main()
{

  for(MFM::u32 k = 0; k < MODES; k++)
    {
      MFM::UlamBitsizeTest utest(ODDS, TESTVAL, (MFM::TestMode) k);  //Int, Unsigned, Unary
      for(MFM::u32 j = 0; j < RUNS; j++)
	{
	  utest.play();
	}
      utest.done();
    }
  return 0;
} //main

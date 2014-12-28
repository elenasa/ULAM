/**                                        -*- mode:C++ -*/

#include "itype.h"
//#include "foo-bitsize_stat.h"
#include "UlamBitsizeTest.h"

#define RUNS 10000000
#define ODDS 1000000
#define MAXTESTVAL 7
#define MODES 3

int main()
{
  for(MFM::u32 k = 0; k < MODES; k++)
    {
      MFM::UlamBitsizeTest utest(ODDS, MAXTESTVAL, (MFM::TestMode) k);  //Int, Unsigned, Unary
      //for(MFM::u32 j = 0; j < RUNS; j++)
	{
	  utest.play();
	}
      utest.done();
    }
  return 0;
} //main

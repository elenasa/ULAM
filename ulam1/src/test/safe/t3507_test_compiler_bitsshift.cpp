#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3507_test_compiler_bitsshift)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 7\nUe_A { Unsigned(3) u(7);  Unsigned(3) v(4);  Int(32) test() {  u 12 cast = v 12u cast = u cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // the example in the reference manual:
      //./A.ulam:4:3: ERROR: Use explicit cast to convert Int(7) to Unsigned(3) for operator=.
      // ./A.ulam:5:5: ERROR: Use explicit cast to convert Bits(11) to Unsigned(3) for operator=.
      bool rtn1 = fms->add("A.ulam","element A {\nUnsigned(3) u, v;\nInt test() {\nu = (Unsigned(3)) (3 * 4);\n  v = (Unsigned(3)) (3 << 2);\n  return u;\n }\n }\n");


      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3507_test_compiler_bitsshift)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3433_test_compiler_boolshift_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:7:8: ERROR: Bool is not currently supported for bitwise shift operator<<; suggest casting Bool(3) to Bits.
      //./A.ulam:8:8: ERROR: Bool is not currently supported for bitwise shift operator>>; suggest casting Bool(3) to Bits.
      //./A.ulam:9:8: ERROR: Bool is not currently supported for bitwise shift operator<<; suggest casting Bool(3) to Bits.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //conversion to Bits then shift then back to Bool. so, 7 << 1 drops to 6 (t); 7 >> 1 = 3 (t); 3 << 2 = 4 (f)
      bool rtn1 = fms->add("A.ulam","element A {\nBool(3) u, v, z;\n Int test() {\n v = true;\n u = v << 1;\n v = v >> 1;\n z = v << 2;\n return 0;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3433_test_compiler_boolshift_error)

} //end MFM

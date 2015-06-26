#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3433_test_compiler_boolshift_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./A.ulam:5:8: ERROR: Bits is the supported type for bitwise shift operator<<; Suggest casting Bool(3) to Bits(3).
	./A.ulam:6:8: ERROR: Bits is the supported type for bitwise shift operator>>; Suggest casting Bool(3) to Bits(3).
	./A.ulam:7:8: ERROR: Bits is the supported type for bitwise shift operator<<; Suggest casting Bool(3) to Bits(3).
      */
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

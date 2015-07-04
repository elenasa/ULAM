#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3488_test_compiler_bigshift_error)
  {
    std::string GetAnswerKey()
    {
      /*
	 ./A.ulam:5:22: Warning: Shifting more than a wordsize, operator<<.
	 ./A.ulam:6:18: Warning: Shifting more than a wordsize, operator<<.
	 ./A.ulam:7:23: Warning: Shifting more than a wordsize, operator>>.
	 ./A.ulam:8:18: Warning: Shifting more than a wordsize, operator>>.
      */

      // regardless signed or unsigned
      //note: g++ shift == 32 produces 1 << 32 = 1, 1 >> 32 = 1; for ulam: 1 << 32 = 0; 1 >> 32 = 1; DIFFERENT!
      //note: g++ shift == 33 produces 1 << 33 = 2, 1 >> 33 = 0; for ulam: 1 << 33 = 2; 1 >> 33 = 0; SAME!
      return std::string("Exit status: 0\nUe_A { Unsigned(8) d(0);  Unsigned(8) f(1);  Int(8) e(0);  Int(8) g(1);  Int(32) test() {  32u = shift const d 0u cast = e 0u cast = f 1u cast = g 1u cast = 0 return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3480
      bool rtn1 = fms->add("A.ulam","element A {\nUnsigned(8) d,f;\nInt(8) e,g;\n Int test(){constant Unsigned shift = 32;\nd = (Unsigned(8)) (1 << shift);\n e = (Int(8)) (1 << shift);\n f = (Unsigned(8)) (1 >> shift);\n g = (Int(8)) (1 >> shift);\n return 0;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3488_test_compiler_bigshift_error)

} //end MFM

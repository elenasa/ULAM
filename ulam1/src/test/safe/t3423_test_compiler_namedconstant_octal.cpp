#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3423_test_compiler_namedconstant_octal)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Int(32) test() {  255u = c255 const 73u = c73 const 64u = c64 const 8u = c8 const 1u = c1 const 0u = cZ const 0 cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // one to three octal digits (0 - 7)
      // note: extra backslash required since test is within a quoted string.
      bool rtn1 = fms->add("A.ulam","element A{\n  Int test () {\n constant Unsigned(8) c255 = '\\377';\n constant Unsigned(8) c73 = '\\111';\n constant Unsigned(8) c64 = '\\100';\n constant Unsigned(8) c8 = '\\010';\n constant Unsigned(8) c1 = '\\01';\n constant Unsigned(8) cZ = '\\0';\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3423_test_compiler_namedconstant_octal)

} //end MFM

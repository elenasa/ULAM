#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3422_test_compiler_namedconstant_specialchars)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Int(32) test() {  7u = cA const 8u = cB const 9u = cT const 10u = cN const 11u = cV const 12u = cF const 13u = cR const 34u = cDQ const 39u = cTIC const 63u = cQ const 92u = cESC const 0 cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3414, except constant type is unsigned char
      // note: extra backslash required since test is within a quoted string.
      bool rtn1 = fms->add("A.ulam","element A{\n  Int test () {\n constant Unsigned(8) cA = '\\a';\n constant Unsigned(8) cB = '\\b';\n constant Unsigned(8) cT = '\\t';\n constant Unsigned(8) cN = '\\n';\n constant Unsigned(8) cV = '\\v';\n constant Unsigned(8) cF = '\\f';\n constant Unsigned(8) cR = '\\r';\n  constant Unsigned(8) cDQ = '\\\"';\n constant Unsigned(8) cTIC = '\\'';\n constant Unsigned(8) cQ = '\\?';\n constant Unsigned(8) cESC = '\\\\';\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3422_test_compiler_namedconstant_specialchars)

} //end MFM

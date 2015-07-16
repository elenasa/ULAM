#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3424_test_compiler_namedconstant_hex)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Int(32) test() {  255u = c255 const 225u = c225 const 136u = c136 const 59u = c59 const 14u = c14 const 13u = c13 const 12u = c12 const 1u = c1 const 0u = cZ const 0 cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {


      // one or two hex digits (0 - 9, A-Z, a-z)
      // note: extra backslash required since test is within a quoted string.
      bool rtn1 = fms->add("A.ulam","element A{\n  Int test () {\n constant Unsigned(8) c255 = '\\xFf';\n constant Unsigned(8) c225 = '\\xE1';\n constant Unsigned(8) c136 = '\\x88';\n constant Unsigned(8) c59 = '\\x3b';\n constant Unsigned(8) c14 = '\\xE';\n constant Unsigned(8) c13 = '\\xd';\n constant Unsigned(8) c12 = '\\xC';\n  constant Unsigned(8) c1 = '\\x01';\n  constant Unsigned(8) cZ = '\\x0';\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3424_test_compiler_namedconstant_hex)

} //end MFM

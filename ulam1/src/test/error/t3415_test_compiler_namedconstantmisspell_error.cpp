#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3415_test_compiler_namedconstantmisspell_error)
  {
    std::string GetAnswerKey()
    {
      // don't assert!
      //./A.ulam:4:46: ERROR: (2) <c_PRINT_VALUE> is not defined, and cannot be used with class: A.
      return std::string("Exit status: 0\nUe_A { constant Bits(3) cPRINT_NAME = 2;  constant Bits(3) cPRINT_VALUE = 1;  constant Bits(3) cPRINT_ALL = 3;  Int(32) test() {  0 return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3314, except misspelled c_PRINT_VALUE
      //      bool rtn1 = fms->add("A.ulam","element A{\nconstant Bits(3) cPRINT_NAME = 2;\n constant Bits(3) cPRINT_VALUE = 1;\n constant Bits(3) cPRINT_ALL = cPRINT_NAME | c_PRINT_VALUE;\nInt test () {\n return 0;\n}\n}\n");
      bool rtn1 = fms->add("A.ulam","element A{\nconstant Bits(3) cPRINT_NAME = 2;\n constant Bits(3) cPRINT_VALUE = 1;\n constant Bits(3) cPRINT_ALL = cPRINT_NAME | PRINT_VALUE;\nInt test () {\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3415_test_compiler_namedconstantmisspell_error)

} //end MFM

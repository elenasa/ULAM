#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3416_test_compiler_namedconstantmisspellastype_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:4:46: ERROR: Incomplete Type: PRINT_VALUE was never defined, fails sizing.
      //./A.ulam:4:46: ERROR: Incomplete Type: PRINT_VALUE was never defined, fails labeling.

      // using ulam compiler (yay! points to use!!!):
      //{Compilation initialization FAILURE: <PRINT_VALUE.ulam>
      //./A.ulam:5:45: ERROR: Invalid Type: PRINT_VALUE.
      //Unrecoverable Program Parse FAILURE: <PRINT_VALUE.ulam>

      return std::string("Exit status: 0\nUe_A { constant Bits(3) cPRINT_NAME = 2;  constant Bits(3) cPRINT_VALUE = 1;  constant Bits(3) cPRINT_ALL = 3;  Int(32) test() {  0 return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3415, except misspelled PRINT_VALUE; culam thinks it's a class!
      bool rtn1 = fms->add("A.ulam","element A{\nconstant Bits(3) cPRINT_NAME = 2;\n constant Bits(3) cPRINT_VALUE = 1;\n constant Bits(3) cPRINT_ALL = cPRINT_NAME | PRINT_VALUE;\nInt test () {\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3416_test_compiler_namedconstantmisspellastype_error)

} //end MFM

#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t213_test_parser_lval)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(16) a[2](3,7);  Int(32) test() {  Int(32) j;  a 2 1 -b [] 7 cast = j 10 cast a 1 [] cast -b = a 0 [] j cast = j return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Int(16) a[2];\n Int test() {\n Int j;\n a[2-1] = 7;\n j = 10 - a[1];\n a[0] = j;\n return j;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASEPARSER(t213_test_parser_lval)

} //end MFM



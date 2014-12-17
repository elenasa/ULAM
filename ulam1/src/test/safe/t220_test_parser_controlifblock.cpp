#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t220_test_parser_controlifblock)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) a(6);  Int(32) b(0);  Int(32) test() {  a 7 cast = b 0 cast = a cast cond { Int(32) b;  b 1 cast b +b = a a 1 cast -b = } if b a = else b return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Int a, b;\n Int test() {\na = 7;\n b = 0;\n if( a ){\n Int b; b = 1 + b;\n a = a - 1;\n }\n else\n b = a;\n return b;\n}\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASEPARSER(t220_test_parser_controlifblock)

} //end MFM



#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t218_test_parser_controlwhile)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) a(0);  Int(32) b(8);  Int(32) test() {  a 5 cast = b 0 cast = a a 1 cast -b = cast cond b b 2 cast +b = while b return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Int a, b;\n Int test() {\n a = 5;\n b = 0;\n while( a = a - 1 )\n b = b + 2;\n return b;\n }\n }\n");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t218_test_parser_controlwhile)
  
} //end MFM



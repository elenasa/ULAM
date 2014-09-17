#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t218_test_parser_controlwhile)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) a(0);  Int(32) b(8);  Int(32) test() {  a 5 = b 0 = a a 1 -b = cast cond b b 2 +b = while b return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a, b; Int test() { a = 5; b = 0; while( a = a - 1 ) b = b + 2; return b; } }");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t218_test_parser_controlwhile)
  
} //end MFM



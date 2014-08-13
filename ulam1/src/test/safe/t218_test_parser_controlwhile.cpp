#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t218_test_parser_controlwhile)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(0);  Int b(8);  Int main() {  a 5 = b 0 = a a 1 -b = cast cond b b 2 +b = while b } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a, b; Int main() { a = 5; b = 0; while( a = a - 1 ) b = b + 2; b; } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t218_test_parser_controlwhile)
  
} //end MFM



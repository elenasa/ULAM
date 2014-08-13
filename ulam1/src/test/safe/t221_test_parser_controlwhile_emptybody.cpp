#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t221_test_parser_controlwhile_emptybody)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(0);  Int main() {  a 5 = a a 1 -b = cast cond {} while 0 } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a; Int main() {a = 5; while( a = a - 1 ) { } 0; } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t221_test_parser_controlwhile_emptybody)
  
} //end MFM



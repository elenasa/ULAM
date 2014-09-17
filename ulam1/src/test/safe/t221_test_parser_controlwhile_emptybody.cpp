#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t221_test_parser_controlwhile_emptybody)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) a(0);  Int(32) test() {  a 5 = a a 1 -b = cast cond {} while a return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a; Int test() {a = 5; while( a = a - 1 ) { } return a; } }");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t221_test_parser_controlwhile_emptybody)
  
} //end MFM



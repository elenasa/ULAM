#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t219_test_parser_controlif)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(0);  Int b(2);  Int test() {  a 1 = b 0 = a a 1 -b = cast cond b 1 = ifthen b 2 = else a return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a, b; Int test() { a = 1; b = 0; if( a = a - 1 ) b = 1; else b = 2; return a; } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t219_test_parser_controlif)
  
} //end MFM



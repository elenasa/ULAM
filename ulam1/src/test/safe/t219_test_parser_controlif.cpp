#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t219_test_parser_controlif)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) a(0);  Int(32) b(2);  Int(32) test() {  a 1 cast = b 0 cast = a a cast 1 -b cast = cast cond b 1 cast = if b 2 cast = else a return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a, b; Int test() { a = 1; b = 0; if( a = a - 1 ) b = 1; else b = 2; return a; } }");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t219_test_parser_controlif)
  
} //end MFM



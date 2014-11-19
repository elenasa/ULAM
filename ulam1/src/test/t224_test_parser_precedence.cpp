#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t224_test_parser_precedence)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) j(7);  Int(32) test() {  j 1 2 3 * +b cast = j return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {Int j; Int test() { j = 1 + 2 * 3; return j; } }\n"); // precedence test
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t224_test_parser_precedence)
  
} //end MFM



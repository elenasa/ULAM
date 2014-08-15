#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t224_test_parser_precedence)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int j(7);  Int test() {  j 1 2 3 * +b = j return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam {Int j; Int test() { j = 1 + 2 * 3; return j; } }\n"); // precedence test
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t224_test_parser_precedence)
  
} //end MFM



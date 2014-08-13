#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t224_test_parser_precedence)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int j(7);  Int main() {  j 1 2 3 * +b = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam {Int j; Int main() { j = 1 + 2 * 3 ;} }\n"); // precedence test
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t224_test_parser_precedence)
  
} //end MFM



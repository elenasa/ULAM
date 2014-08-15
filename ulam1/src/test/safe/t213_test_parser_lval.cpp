#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t213_test_parser_lval)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a[5](0,0,0,0,7);  Int j(3);  Int test() {  a 1 3 +b [] 7 = j 10 a 4 [] -b = j return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a[5]; Int j; Int test() { a[1+3] = 7; j = 10 - a[4]; return j; } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t213_test_parser_lval)
  
} //end MFM



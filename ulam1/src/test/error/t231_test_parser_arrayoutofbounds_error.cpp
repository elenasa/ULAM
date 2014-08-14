#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t231_test_parser_arrayoutofbounds_error)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a[2](4,8);  Int test() {  a 0 [] 1 3 +b = a 1 [] 2 4 * = a 2 [] return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a[2]; Int test() { a[0] = 1 + 3; a[1] = 2 * 4; return a[2]; } }"); 
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t231_test_parser_arrayoutofbounds_error)
  
} //end MFM



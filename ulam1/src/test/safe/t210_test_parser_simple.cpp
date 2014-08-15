#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t210_test_parser_simple)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a[2](4,8);  Int test() {  a 0 [] 1 3 +b = a 1 [] 2 4 * = a 1 [] return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a[2]; Int test() { a[0] = 1 + 3; a[1] = 2 * 4; return a[1]; } }"); // simple case
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t210_test_parser_simple)
  
} //end MFM



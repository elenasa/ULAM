#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t216_test_parser_vardecls)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a[5](0,0,0,2,0);  Int b(2);  Int test() {  b 2 = a 1 b +b [] b = a 3 [] return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a[5], b; Int test() { b = 2; a[1+b] = b; return a[3]; } }"); //not error?
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t216_test_parser_vardecls)
  
} //end MFM



#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t215_test_parser_lvalintarray)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a[5](0,3,0,8,2);  Int main() {  a 1 3 +b [] 2 = a 1 [] a 4 [] 1 +b = a a 1 [] [] a 1 [] 5 +b = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a[5]; Int main() { a[1+3] = 2; a[1] = a[4] + 1; a[a[1]] = a[1] + 5;} }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t215_test_parser_lvalintarray)
  
} //end MFM



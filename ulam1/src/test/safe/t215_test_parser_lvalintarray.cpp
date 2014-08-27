#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t215_test_parser_lvalintarray)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int a[3](0,2,7);  Int test() {  a 1 1 +b [] 1 = a 1 [] a 2 [] 1 +b = a a 1 [] [] a 1 [] 5 +b = a a 1 [] [] return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a[3]; Int test() { a[1+1] = 1; a[1] = a[2] + 1; a[a[1]] = a[1] + 5; return a[a[1]]; } }");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t215_test_parser_lvalintarray)
  
} //end MFM



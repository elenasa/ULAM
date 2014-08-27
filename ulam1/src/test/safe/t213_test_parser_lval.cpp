#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t213_test_parser_lval)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int a[3](3,0,7);  Int test() {  Int j;  a 1 1 +b [] 7 = j 10 a 2 [] -b = a 0 [] j = j return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a[3]; Int test() { Int j; a[1+1] = 7; j = 10 - a[2]; a[0] = j; return j; } }");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t213_test_parser_lval)
  
} //end MFM



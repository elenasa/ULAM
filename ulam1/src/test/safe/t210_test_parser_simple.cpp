#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t210_test_parser_simple)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) a[2](4,8);  Int(32) test() {  a 0 [] 1 3 +b = a 1 [] 2 4 * = a 1 [] return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a[2]; Int test() { a[0] = 1 + 3; a[1] = 2 * 4; return a[1]; } }"); // simple case
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t210_test_parser_simple)
  
} //end MFM



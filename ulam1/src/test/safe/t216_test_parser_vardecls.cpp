#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t216_test_parser_vardecls)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) c(1);  Int(32) test() {  Int(32) a[2];  Int(32) b;  b 1 = a 0 b +b [] b = c a 1 [] = a 1 [] return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int c; Int test() { Int a[2], b; b = 1; a[0+b] = b; c = a[1]; return a[1]; } }"); //not error?
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t216_test_parser_vardecls)
  
} //end MFM



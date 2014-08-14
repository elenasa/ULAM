#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t230_test_parser_arraynonconstant_error)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(7);  Int test() {  a 7 = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam{ Int a; Int test() {a = 7; Int b[a]; b[4] = 2; return b[4]; } }"); 
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t230_test_parser_arraynonconstant_error)
  
} //end MFM



#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t226_test_parser_emptysimplestmt)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int main() {  ; ; ; } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam{ Int main() {;;; } }\n");  //empty statement
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t226_test_parser_emptysimplestmt)
  
} //end MFM



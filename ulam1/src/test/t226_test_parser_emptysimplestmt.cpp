#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t226_test_parser_emptysimplestmt)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) test() {  ; ; ; 0 cast return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A{ Int test() {;;; return 0; } }\n");  //empty statement
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t226_test_parser_emptysimplestmt)
  
} //end MFM



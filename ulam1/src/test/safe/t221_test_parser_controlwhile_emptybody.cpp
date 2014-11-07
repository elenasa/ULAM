#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t221_test_parser_controlwhile_emptybody)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) a(0);  Int(32) test() {  a 5 cast = a a 1 cast -b = cast cond {} while a return } }
      return std::string("Ue_A { Bool(7) b(false);  Int(32) a(0);  Int(32) test() {  a 5 cast = a a 1 cast -b = cast cond {} while a return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\nBool(7) b;\n Int a;\n Int test() {\na = 5;\n while( a = a - 1 ) { }\n return a;\n }\n }\n");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t221_test_parser_controlwhile_emptybody)
  
} //end MFM



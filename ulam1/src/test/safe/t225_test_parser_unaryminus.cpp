#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t225_test_parser_unaryminus)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) bar(-18);  Int(32) test() {  bar 18 - = bar - - bar return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // newline lead to valgrind detecting leak in Lexer destructor: NULLifying sourcestream ref.
      bool rtn1 = fms->add("A.ulam","element A {Int bar; Int test() {bar = -18; - - bar; return bar; } }\n"); 
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t225_test_parser_unaryminus)
  
} //end MFM



#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t225_test_parser_unaryminus)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int bar(-18);  Int test() {  bar 18 - = bar - - bar return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // newline lead to valgrind detecting leak in Lexer destructor: NULLifying sourcestream ref.
      bool rtn1 = fms->add("a.ulam","ulam {Int bar; Int test() {bar = -18; - - bar; return bar; } }\n"); 
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t225_test_parser_unaryminus)
  
} //end MFM



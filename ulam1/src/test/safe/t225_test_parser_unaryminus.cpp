#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t225_test_parser_unaryminus)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) bar(-18);  Int(32) test() {  bar 18 cast - = bar - - bar return } }
      return std::string("Ue_A { Bool(7) b(false);  System s();  Int(32) bar(-18);  Int(32) test() {  bar 18 cast - = bar - - s ( bar )print . bar return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // newline lead to valgrind detecting leak in Lexer destructor: NULLifying sourcestream ref.
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\nInt bar;\n Int test() {\nbar = -18;\n - - bar;\ns.print(bar);\n return bar;\n }\n }\n"); 
      
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid assert(Bool b) native;\n}\n");      
      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t225_test_parser_unaryminus)
  
} //end MFM



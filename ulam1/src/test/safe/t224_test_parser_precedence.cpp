#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t224_test_parser_precedence)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) j(7);  Int(32) test() {  j 1 2 3 * +b cast = j return } }
      return std::string("Ue_A { Bool(7) b(false);  System s();  Int(32) j(7);  Int(32) test() {  j 1 2 3 * +b cast = s ( j )print . j return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\nInt j;\n Int test() {\n j = 1 + 2 * 3;\ns.print(j);\n return j;\n }\n }\n"); // precedence test

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid assert(Bool b) native;\n}\n");      
      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t224_test_parser_precedence)
  
} //end MFM



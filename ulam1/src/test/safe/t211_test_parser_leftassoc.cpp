#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t211_test_parser_leftassoc)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) a(1);  Int(32) test() {  a 1 1 -b 1 +b cast = a return } }
      return std::string("Ue_A { Bool(7) b(false);  System s();  Int(32) a(1);  Int(32) test() {  a 1 1 -b 1 +b cast = s ( a )print . a return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\n Int a;\n Int test() {\n a = 1 - 1 + 1;\ns.print(a);\n return a;\n }\n }\n"); // we want 1, not -1
      
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid assert(Bool b) native;\n}\n");      

      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t211_test_parser_leftassoc)
  
} //end MFM



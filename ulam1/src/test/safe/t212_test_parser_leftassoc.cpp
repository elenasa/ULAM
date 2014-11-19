#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t212_test_parser_leftassoc)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) a(2);  Int(32) test() {  a 2 2 / 2 * cast = a return } }
      return std::string("Ue_A { Bool(7) b(false);  System s();  Int(32) a(2);  Int(32) test() {  a 2 2 / 2 * cast = s ( a )print . a return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","element A {\n Int a;\n Int test() {\n a = 2 / 2 * 2;\n return a;\n }\n }\n"); // we want 2, not 0

      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\nInt a;\n Int test() {\n a = 2 / 2 * 2;\ns.print(a);\n return a;\n }\n }\n"); // we want 2, not 0

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid assert(Bool b) native;\n}\n");      

      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t212_test_parser_leftassoc)
  
} //end MFM



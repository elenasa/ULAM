#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t223_test_parser_simplewithparens)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) barne(32);  Int(32) test() {  barne 1 3 +b 8 * cast = barne return } }
      return std::string("Ue_A { Bool(7) b(false);  System s();  Int(32) barne(32);  Int(32) test() {  barne 1 3 +b 8 * cast = s ( barne )print . barne return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\n Int barne;\n Int test() {\n barne = (1 + 3) * 8;\ns.print(barne);\n return barne;\n }\n }\n"); // case with parens
      
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid assert(Bool b) native;\n}\n");      
      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t223_test_parser_simplewithparens)
  
} //end MFM



#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3102_test_compiler_bitwiseand)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) a(2);  Int(32) b(2);  Bool(1) d(true);  Int(32) test() {  a 3 cast = b 2 cast = d a a b & = cast = a return } }\nExit status: 2

      return std::string("Ue_A { Int(3) b(2);  System s();  Bool(1) d(true);  Int(3) a(2);  Int(32) test() {  a 3 cast = b 2 cast = d a a b & = cast = s ( a )print . s ( d )assert . a cast return } }/nExit status: 2");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","element A { Int a, b;\nBool d;\nuse test;\na = 3;\nb = 2;\nd = a = a & b;\n return a;\n}\n }\n");
      //bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool d;\nInt(3) a, b;\nInt test() {\na = 3;\nb = 2;\nd = a = a & b;\ns.print(a);\ns.assert(d);\nreturn a;\n}\n}\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3102_test_compiler_bitwiseand)
  
} //end MFM



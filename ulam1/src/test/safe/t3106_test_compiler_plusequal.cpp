#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3106_test_compiler_plusequal)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) a(3);  Int(32) b(2);  Int(32) test() {  a 1 cast = b 2 cast = a b += a return } }\nExit status: 3

      /* gen code output:
	 Int(3) Arg: 0x3
	 assert: arg is 1
	 after assert's abort: arg is 1
      */

      return std::string("Ue_A { Int(3) b(2);  System s();  Bool(1) d(true);  Int(3) a(3);  Int(32) test() {  a 1 cast = b 2 cast = a b += s ( a )print . d a cast = s ( d )assert . a cast return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","element A { Int a, b;\nuse test;\n a = 1;\nb = 2;\na+=b;\nreturn a;\n}\n}\n");
      //bool rtn2 = fms->add("test.ulam", "Int test() {\n");
      
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool d;\nInt(3) a, b;\nInt test() {\na = 1;\nb = 2;\na+=b;\ns.print(a);\nd=a;\ns.assert(d);\nreturn a;\n}\n}\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3106_test_compiler_plusequal)
  
} //end MFM



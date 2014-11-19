#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3108_test_compiler_multiplyequal)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) a(6);  Int(32) b(2);  Int(32) test() {  a 3 cast = b 2 cast = a b *= a return } }\nExit status: 6
      return std::string("Ue_A { Int(3) b(2);  System s();  Bool(1) l(false);  Int(3) a(6);  Int(32) test() {  a 3 cast = b 2 cast = a b *= s ( a )print . a cast return } }\nExit status: -2");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool l;\nInt(3) a, b;\nInt test() {\na = 3;\nb = 2;\na*=b;\ns.print(a);\nreturn a;\n}\n}\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid assert(Bool b) native;\n}\n");
      
      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3108_test_compiler_multiplyequal)
  
} //end MFM



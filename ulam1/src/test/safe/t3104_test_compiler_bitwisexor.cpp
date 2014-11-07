#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3104_test_compiler_bitwisexor)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Bool(1) d(true);  Int(3) a(1);  Int(3) b(2);  Int(32) test() {  a 3 cast = b 2 cast = d a a b ^ = cast = a cast return } }\nExit status: 1");
//Ue_A { Int(32) a(1);  Int(32) b(2);  Bool(1) d(true);  Int(32) test() {  a 3 cast = b 2 cast = d a a b ^ = cast = a return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","element A {Bool d;\nInt(3) a, b;\nuse test;\na = 3;\nb = 2;\nd = a = a ^ b;\nreturn a;\n}\n}\n");
      bool rtn1 = fms->add("A.ulam","ulam 1; use System; element A {System s;\nBool d;\nInt(3) a, b;\nuse test;\na = 3;\nb = 2;\nd = a = a ^ b;\ns.print(a);\nreturn a;\n}\n}\n");
      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3104_test_compiler_bitwisexor)
  
} //end MFM



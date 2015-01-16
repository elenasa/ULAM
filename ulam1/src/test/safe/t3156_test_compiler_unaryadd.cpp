#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3156_test_compiler_unaryadd)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Unary(3) a(2);  Unary(3) b(1);  Unary(3) c(3);  Int(32) test() {  a 2 cast = b 1 cast = c a cast b cast +b cast = c cast return } }\nExit status: 3
      return std::string("Ue_A { Unary(3) b(1);  System s();  Bool(1) sp(false);  Unary(3) a(2);  Unary(3) c(3);  Int(32) test() {  a 2 cast = b 1 cast = c a cast b cast +b cast = s ( c )print . c cast return } }\nExit status: 3");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool sp;\n Unary(3) a, b, c;\n use test;\n  a = 2;\n b = 1;\n c = a + b;\ns.print(c);\n return c;\n }\n }\n");

      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3156_test_compiler_unaryadd)

} //end MFM



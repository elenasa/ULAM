#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3160_test_compiler_unarybitwiseor)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Unary(3) a(2);  Unary(3) b(1);  Unary(3) c(2);  Int(3) e(2);  Int(3) f(1);  Int(3) g(3);  Int(32) test() {  a e 2 cast = cast = b f 1 cast = cast = c a b | = g e f | = c cast return } }\nExit status: 2
      return std::string("Ue_A { Unary(3) b(1);  System s();  Bool(1) sp(false);  Unary(3) a(2);  Unary(3) c(2);  Int(3) e(2);  Int(3) f(1);  Int(3) g(3);  Int(32) test() {  a e 2 cast = cast = b f 1 cast = cast = c a b | = s ( c )print . g e f | = s ( g )print . c cast return } }\nExit status: 2");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // a is 2 bits; b is 1 bit; a | b = 2 bit; notice as Int.3 (g), 2 | 1 = 3
      //bool rtn1 = fms->add("A.ulam","element A { Unary(3) a, b, c; Int(3) e, f, g; use test;  a = e = 2; b = f = 1; c = a | b; g = e | f; return c; } }");
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool sp;\nUnary(3) a, b, c;\n Int(3) e, f, g;\n use test;\n  a = e = 2;\n b = f = 1;\n c = a | b;\ns.print(c);\ng = e | f;\ns.print(g);\n return c;\n }\n }\n");

      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3160_test_compiler_unarybitwiseor)
  
} //end MFM



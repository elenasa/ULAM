#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3158_test_compiler_booladd)
  {
    std::string GetAnswerKey()
    {
      /* gencode output:
	 Bool(3) Arg: 0x7 (true)
	 Unary(3) Arg: 0x1
      */
      return std::string("Exit status: 1\nUe_A { Bool(3) b(false);  System s();  Bool(3) a(true);  Bool(3) c(true);  Unary(3) d(1);  Int(32) test() {  a true cast = b false cast = c a cast b cast +b cast = s ( c )print . d c cast = s ( d )print . c cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //./A.ulam:10:8: ERROR: Incompatible Bool type for binary operator+. Suggest casting to a numeric type first.
      //here's what happens when we try to add bools and save in a bool;
      // note1: cast as a unary the sum is true
      // note2: cast as an Int(32), the exit status is 1
      //bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool sp;\n Bool(3) a, b, c;\n Unary(3) d;\n use test;\na = true;\nb = false;\n c = a + b;\ns.print(c);\n d = c;\ns.print(d);\n return c;\n }\n }\n");

      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(3) a, b, c;\n Unary(3) d;\n use test;\na = true;\nb = false;\n c = (Unsigned(3)) a + (Unsigned(3)) b;\n s.print(c);\n d = c;\ns.print(d);\n return c;\n }\n }\n");

      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3158_test_compiler_booladd)

} //end MFM

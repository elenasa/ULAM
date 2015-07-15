#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3222_test_compiler_localatom)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 96\nUe_A { Int(3) b(0);  System s();  Bool(1) d(false);  Int(3) a(0);  Int(3) c(0);  Int(32) test() {  typedef Atom(96) A;  Atom(96) tm;  96u cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool d;\nInt(3) a, b, c;\nInt test() {\ntypedef Atom A;\n A tm;\n return A.sizeof;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3222_test_compiler_localatom)

} //end MFM

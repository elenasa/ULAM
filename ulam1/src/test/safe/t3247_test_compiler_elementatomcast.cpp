#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3247_test_compiler_elementatomcast)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { System s();  Bool(1) sp(false);  Bool(3) bi(true);  Bool(3) bh(true);  Int(32) d(0);  Int(32) test() {  Atom(96) a;  Foo f;  a f cast = a Foo is cast cond bi true cast = if f a cast = f System has cast cond bh true cast = if 0 cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // 'a has System' doesn't appy to eval because atoms have no class declarations; but testable for gencode
      bool rtn1 = fms->add("Foo.ulam","use System;\nelement Foo {\nSystem s;\nBool sp;\nBool(3) bi, bh;\nInt d;\nInt test(){Atom a;\nFoo f;\na = f; //easy\nif(a is Foo)\n bi = true;\nf = a; //make sure a is a foo\nif(f has System)\nbh = true;\n return 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3247_test_compiler_elementatomcast)

} //end MFM



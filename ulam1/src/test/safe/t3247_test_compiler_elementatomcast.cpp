#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3247_test_compiler_elementatomcast)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 assert: arg is 1
	 after assert's abort: arg is 1
	 assert: arg is 1
	 after assert's abort: arg is 1
      */
      // no cast cond
      //constant fold minus

      //./Foo.ulam:17:4: ERROR: Cannot cast to type: Foo (UTI16); use Conditional-As.
      //./Foo.ulam:17:4: ERROR: Cannot CAST type: Atom(96) as a Foo(39)<16>.

      return std::string("Exit status: 3\nUe_Foo { System s();  Bool(1) sp(false);  Bool(3) bi(true);  Bool(3) bh(true);  Int(32) d(3);  Int(32) test() {  Atom(96) a;  Foo f;  Bool(1) b;  b a System has = s ( b ! )assert . a f cast = a Foo is cond bi true cast = if b a System has = s ( b )assert . f a cast = f System has cond bh true cast = if d a System has cast 3 cast +b = d return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // 'a has System' doesn't apply to eval because atoms have no class declarations; but testable for gencode
      bool rtn1 = fms->add("Foo.ulam","use System;\nelement Foo {\nSystem s;\nBool sp;\nBool(3) bi, bh;\nInt d;\nInt test(){Atom a;\nFoo f;\nBool b;\nb=a has System;\ns.assert(!b);\n a = f; //easy\nif(a is Foo)\n bi = true;\n b=a has System;\ns.assert(b);\n f = (Foo) a; //make sure a is a foo\nif(f has System)\nbh = true;\n d = (Int) (a has System) + 3;\nreturn d;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3247_test_compiler_elementatomcast)

} //end MFM

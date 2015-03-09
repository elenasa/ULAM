#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3348_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef_sizeof)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Unsigned Arg: 1
      */
      //Constants have explicit types
      return std::string("Exit status: 1\nUe_P { Bool(UNKNOWN) b(false);  constant Unsigned(32) a = NONREADYCONST;  Int(32) foo(0);  Int(32) test() {  System s;  3u = var const P(1u) pel;  s ( pel -2 . )print . pel -2 . return } }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) Foo;  <NOMAIN> }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // informed by 3339: P is now parametric
      // recursive typedefs as local variable type
      // must already be parsed!
      //pel.b.sizeof
      //P.ulam:9:15: ERROR: Member selected must be either a quark or an element, not type: Bool(1).
      bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\nuse System;\n element P(Unsigned a) {\nBool(a) b;\nInt foo;\nInt test() {\nSystem s;\n constant V.Woof.Foo var = 3u;\n P(1u) pel;\n s.print(pel.b.sizeof);\n return pel.b.sizeof;\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q(Int s) {\ntypedef Unsigned(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn4 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("P.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3348_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef_sizeof)

} //end MFM

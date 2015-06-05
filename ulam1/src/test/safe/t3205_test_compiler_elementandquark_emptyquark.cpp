#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3205_test_compiler_elementandquark_emptyquark)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: -1
	 Int(4) Arg: 0xf
      */

      return std::string("Exit status: -1\nUe_Foo { Bool(3) b(false);  System s();  Empty e();  Int(4) i(0);  Int(4) j(15);  Int(32) test() {  i 0 cast = j ( i 1 cast -b cast )update = s ( j cast )print . s ( j )print . j cast return } }\nUq_System { <NOMAIN> }\nUq_Empty { /* empty class block */ }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nelement Foo {\nBool(3) b;\nInt(4) i, j;\nInt(4) update(Int x)\n{\nreturn x;\n}\nInt test(){\ni = 0;\nj = update(i - 1);\n\nreturn j;\n}\n}\n");

      // test explicit cast in arg
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Empty;\nelement Foo {\nSystem s;\nEmpty e;\nBool(3) b;\nInt(4) i, j;\nInt(4) update(Int(4) x)\n{\nreturn x;\n}\nInt test(){\ni = 0;\nj = update((Int(4)) (i - 1));\ns.print((Int) j);\ns.print(j);\nreturn j;\n}\n}\n");

      bool rtn2 = fms->add("Empty.ulam","ulam 1;\nuse System;\n quark Empty {\n /* empty quark comment */ }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3205_test_compiler_elementandquark_emptyquark)

} //end MFM

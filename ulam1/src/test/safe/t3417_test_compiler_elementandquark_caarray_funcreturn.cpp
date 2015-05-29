#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3417_test_compiler_elementandquark_caarray_funcreturn)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int(3) Arg: 0x2
      */
      // correct answer in gen code output, not exit status.
      return std::string("Exit status: 0\nUe_IntArrayTest { typedef Int(3) Intn;  constant Int(32) isn = 3;  IAN(3) ia( constant Int(32) n = 3;  typedef Int(3) Intn; );  Int(32) test() {  System s;  Int(3) a;  a ( )func = s ( a )print . a cast return } }\nUq_IAN { constant Int(32) n = NONREADYCONST;  typedef Int(UNKNOWN) Intn;  <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3416 only return is Int("3") instead of atom; using class args for bitsize "3"
      // (don't have to use native aref).
      bool rtn1 = fms->add("IntArrayTest.ulam", "ulam 1;\nuse IAN;\n use System;\n element IntArrayTest {\nconstant Int isn = 3;\n typedef Int(isn) Intn;\n IAN(isn) ia;\n  Intn func() {\n return ia[1];\n }\n Int test() {\nSystem s;\n Intn a;\n a = func();\n s.print(a);\n return a;\n  }\n}\n");
      bool rtn2 = fms->add("IAN.ulam", "ulam 1;\nquark IAN(Int n) {\ntypedef Int(n) Intn;\n Intn aref(Int index){\n return n - index;\n}\n}\n");

      // simplify for debug..wo class param, use hardcoded Int(3)
      // return isn't 3+1, it's zero; might need system to see..
      // bool rtn1 = fms->add("IntArrayTest.ulam", "ulam 1;\nuse IAN;\nelement IntArrayTest {\ntypedef IAN.Intn Intn;\n IAN ia;\n  Intn func() {\n return ia[1];\n }\n Int test() {\n Intn a;\n a = func();\n return a;\n  }\n}\n");
      //bool rtn2 = fms->add("IAN.ulam", "ulam 1;\nquark IAN {\ntypedef Int(3) Intn;\n Intn aref(Int index){\n return 3-index;\n}\n}\n");


      //works! BUT return isnt' 3-1, it's 0
      //bool rtn1 = fms->add("IntArrayTest.ulam", "ulam 1;\nuse IAN;\nelement IntArrayTest {\ntypedef Int(3) Intn;\n IAN(3) ia;\n  Intn func() {\n return ia[1];\n }\n Int test() {\n Intn a = func();\n return a;\n  }\n}\n");
      //bool rtn2 = fms->add("IAN.ulam", "ulam 1;\nquark IAN(Int n) {\ntypedef Int(n) Intn;\n Intn aref(Int index){\n return n-index;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("IntArrayTest.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3417_test_compiler_elementandquark_caarray_funcreturn)

} //end MFM

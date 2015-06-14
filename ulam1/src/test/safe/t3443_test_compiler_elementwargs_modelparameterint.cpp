#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3443_test_compiler_elementwargs_modelparameterint)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int(3) Arg: 0x7
	 Int Arg: -1
      */

      return std::string("Exit status: -1\nUe_R { System s();  constant Int(3) ra = -1;  Int(32) i(-1);  Int(32) test() {  Foo(-1) f;  s ( f chance(-1) . )print . i f chance(-1) . cast = s ( i )print . i return } }\nUq_System { <NOMAIN> }\nUe_Foo { constant Int(3) a = NONREADYCONST;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //also tests using a negative constant, and model parameter
      bool rtn2 = fms->add("R.ulam","ulam 1;\nuse System;\nuse Foo;\n element R {\nSystem s;\nconstant Int(3) ra = -1;\n Int i;\n Int test() {\n Foo(ra) f;\n s.print(f.chance);\n i = f.chance;\n s.print(i);\n return i;\n }\n }\n");

      // MP not storeintoable/
      bool rtn1 = fms->add("Foo.ulam","element Foo(Int(3) a) {\n parameter Int(3) chance = a;\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	//if(rtn1)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3443_test_compiler_elementwargs_modelparameterint)

} //end MFM

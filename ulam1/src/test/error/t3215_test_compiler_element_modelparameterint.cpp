#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3215_test_compiler_element_modelparameterint)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 1
	 Int Arg: 1
      */

      return std::string("Exit status: 1\nUe_Foo { System s();  Int(32) a(1);  Int(32) test() {  Foo f;  s ( chance(1) )print . s ( f chance(1) . )print . a f chance(1) . = a return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // MP not storeintoable/
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\n parameter Int chance = 1;\n Int a;\n Int test() {\n Foo f;\n s.print(chance);\n s.print(f.chance);\n a = f.chance;\n return a;\n }\n }\n");

      //simplify for debugging
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\n parameter Int chance = 1;\n Int a;\n Int test() {\n Foo f;\n a = f.chance;\n return a;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	//if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3215_test_compiler_element_modelparameterint)

} //end MFM

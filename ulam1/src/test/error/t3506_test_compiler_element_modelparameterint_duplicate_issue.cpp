#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3506_test_compiler_element_modelparameterint_duplicate_issue)
  {
    std::string GetAnswerKey()
    {

      // using TESTPARAMETERMAP in Compiler.cpp: confirm 2 parameter map records exist
      //  when the names are identical.
      //Size of model parameter map is 2
      //ULAM INFO: PARAMETER ./Foo.ulam:6:16: Ue_102323Foo10 Ut_102321i chance Up_6chance 0x1 NONE
      //ULAM INFO: PARAMETER ./Foo2.ulam:5:16: Ue_102324Foo210 Ut_102321i chance Up_6chance 0x1 NONE
      return std::string("Exit status: 1\nUe_Foo { System s();  Int(32) a(1);  Int(32) test() {  Foo f;  s ( chance(1) )print . s ( f chance(1) . )print . a f chance(1) . = a return } }\nUq_System { <NOMAIN> }\nUe_Foo2 { System s();  Int(32) a(1);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // MP not storeintoable/
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n use Foo2;\n element Foo {\nSystem s;\n parameter Int chance = 1;\n Int a;\n Int test() {\n Foo f;\n s.print(chance);\n s.print(f.chance);\n a = f.chance;\n return a;\n }\n }\n");

      //simplify for debugging
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\n parameter Int chance = 1;\n Int a;\n Int test() {\n Foo f;\n a = f.chance;\n return a;\n }\n }\n");

      bool rtn2 = fms->add("Foo2.ulam","ulam 1;\nuse System;\n element Foo2 {\nSystem s;\n parameter Int chance = 1;\n Int a;\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	//if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3506_test_compiler_element_modelparameterint_duplicate_issue)

} //end MFM

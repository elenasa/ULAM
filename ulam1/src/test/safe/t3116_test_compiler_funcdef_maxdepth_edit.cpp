#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3116_test_compiler_funcdef_maxdepth_edit)
  {
    //turn on INFO message in NodeBlockFunctionDefinition::setMaxDepth

    std::string GetAnswerKey()
    {
      //A.ulam:1:13: (NodeBlockFunctionDefinition.cpp:setMaxDepth:193) fyi: Max Depth is: <9>.
      return std::string("Ue_A { Bool(1) c(true);  Int(32) test() {  c ( 1 cast true cast )foo = c cast return } }\nExit status: 1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Bool foo(Int m, Bool b) {\n Int d;\n { Int e[8];\n b = true;\n}\n Bool c;\n c = d = m;\n return c;\n } Int test() {\n c = foo(1, true);\n return c;\n } Bool c;\n }\n");  // max depth is 9; should cast return to Int.

      // max depth == 2 (packed array)
      //bool rtn1 = fms->add("A.ulam","element A { Bool foo(Int m, Bool b) {\n Int d;\n {\n Int(4) e[8];\n b = true;\n}\n Bool c;\n c = d = m;\n return c;\n }\n Int test() {\n c = foo(1, true);\n return c;\n } Bool c;\n }\n");  // max depth is 9; should cast return to Int.

      //currently, can't handle arrays, SO substitute 8 Int's instead.
      //bool rtn1 = fms->add("A.ulam","element A { Bool foo(Int m, Bool b) {\n Int d;\n {\nInt e,f,g,h,i,j,k,l;\n b = true;\n}\n Bool c;\n c = d = m;\n return c;\n }\n Int test() {\n c = foo(1, true);\n return c;\n } Bool c;\n }\n");
      // max depth is 9; should cast return to Int.


      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3116_test_compiler_funcdef_maxdepth_edit)

} //end MFM



#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3319_test_compiler_errortok)
  {
    std::string GetAnswerKey()
    {
      /*
	A.ulam:1:34: ERROR: Weird! Lexer could not find match for: <@>.
	A.ulam:1:34: ERROR: Unexpected input!! Token: <ContinueError>.
	A.ulam:1:39: ERROR: Weird! Lexer could not find match for: <@>.
	A.ulam:1:39: ERROR: Unexpected input!! Token: <ContinueError>.
	ERROR: No parse tree.
      */
      return std::string(" { Int a(5);  Int test() {  a 3 2 +b = a return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use foo;  Int a; Int test() {a = @3 + @2; use bar; return a; load bar;");
      bool rtn2 = fms->add("foo.ulam","element A {  ");  //recovers from invalid character
      bool rtn3 = fms->add("bar.ulam","} \n ");

      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3319_test_compiler_errortok)

} //end MFM



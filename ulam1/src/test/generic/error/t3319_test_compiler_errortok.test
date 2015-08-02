#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3319_test_compiler_errortok)
  {
    std::string GetAnswerKey()
    {
      /*
	./A.ulam:1:34: ERROR: (Low Level) Weird! Lexer could not find match for: <@>.
	./A.ulam:1:39: ERROR: (Low Level) Weird! Lexer could not find match for: <@>.
	./foo.ulam:1:1: ERROR: Empty/Incomplete Class Definition: <A>; possible missing ending curly brace.
	./A.ulam:1:52: ERROR: Invalid Class Type: <return>; KEYWORD should be: 'element', 'quark', or 'union'.
	./foo.ulam:1:9: ERROR: Invalid Type: A.
	ERROR: No parse tree for This Class: A.
	Unrecoverable Program Parse FAILURE.
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

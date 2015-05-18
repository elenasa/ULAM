#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3101_test_compiler_uses_invalidchars)
  {
    std::string GetAnswerKey()
    {
      /*
	./foo.ulam:1:13: ERROR: Weird! Lexer could not find match for: <@>.
	./foo.ulam:1:1: Warning: Empty/Incomplete Class Definition: <D>; possible missing ending curly brace.
	./foo.ulam:1:13: ERROR: Invalid Class Type: <ContinueError>; KEYWORD should be: 'element', 'quark', or 'union'.
      */
      return std::string("Ue_D { Int(32) test() { Int(32) a(5);  a 3 2 +b = } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // ok to have more than 'atom' inside a function
      // also tests 'use' and 'load'
      bool rtn1 = fms->add("D.ulam","use foo; Int a; use test;  a = @3 + @2; return a; use bar; load bar; ");
      bool rtn2 = fms->add("foo.ulam","element D { @ ");  //recovers from invalid character
      bool rtn3 = fms->add("bar.ulam","} \n ");
      bool rtn4 = fms->add("test.ulam","Int test() { ");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3101_test_compiler_uses_invalidchars)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3487_test_compiler_ulamversion_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./Foo.ulam:3:1: ERROR: Unexpected token <AbortError> (check 'ulam' version number, 'use' or 'load' a missing .ulam file) -- exiting now.
	./Foo.ulam:1:2: ERROR: Empty/Incomplete Class Definition: <Foo>; possible missing ending curly brace.
	./Foo.ulam:3:6: ERROR: Invalid Class Type: <1.1>; KEYWORD should be: 'element', 'quark', or 'union'.
	./Foo.ulam:1:10: ERROR: Invalid Type: Foo.
	ERROR: No parse tree for This Class: Foo.
	Unrecoverable Program Parse FAILURE.
       */
      return std::string("Exit status: 0\nUe_Foo { NoParms(1) boom( constant Unary(32) a = 1; );  Int(32) test() {  0 return } }\nUq_NoParms { /* empty class block */ }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      /*
	ulam 1;
	quark NoParms {
	}
	element Foo {
	NoParms(1) boom;
	}
      */

      // invalid ulam version (note '1a' is fine, just no floats!)
      //informed by 3340, added arg to NoParms
      bool rtn1 = fms->add("NoParms.ulam", "ulam 1;\nquark NoParms(Unary a) {\n }\n");
      bool rtn2 = fms->add("Foo.ulam", "element Foo{\nNoParms(1) boom;\nulam 1.1\nuse test;\n}\nuse NoParms;\n");
      bool rtn3 = fms->add("test.ulam", "Int test() {\n return 0;\n}\n");
      if(rtn2 && rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3487_test_compiler_ulamversion_error)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3487_test_compiler_ulamversion_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./Foo.ulam:3:1: ERROR: (Low Level) ulam version ending with <Int>.
	./Foo.ulam:1:1: ERROR: Empty/Incomplete Class Definition: <Foo>; possible missing ending curly brace.
	./test.ulam:1:5: ERROR: Invalid Class Type: <test>; KEYWORD should be: 'element', 'quark', or 'union'.
	./Foo.ulam:1:9: ERROR: Invalid Type: Foo.
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
      //informed by 3340
      bool rtn1 = fms->add("NoParms.ulam", "ulam 1;\nquark NoParms {\n }\n");
      bool rtn2 = fms->add("Foo.ulam", "element Foo{\nNoParms(1) boom;\nulam 1.1\nuse test;\n}\nuse NoParms;\n");
      bool rtn3 = fms->add("test.ulam", "Int test() {\n return 0;\n}\n");
      if(rtn2 && rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3487_test_compiler_ulamversion_error)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3472_test_compiler_atomcomparison_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./A.ulam:5:10: ERROR: Non-primitive type: <Atom(96)> is not supported as LHS for binary operator<.
	./A.ulam:5:10: ERROR: Non-primitive type: <Atom(96)> is not supported as RHS for binary operator<.
	./A.ulam:6:10: ERROR: Non-primitive type: <Atom(96)> is not supported as LHS for binary operator!=.
	./A.ulam:6:10: ERROR: Non-primitive type: <Atom(96)> is not supported as RHS for binary operator!=.
      */
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\nInt test() {\n Bool bad;\n Atom a, b;\n bad = a < b;\n bad = a != b;\n return bad;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3472_test_compiler_atomcomparison_error)

} //end MFM

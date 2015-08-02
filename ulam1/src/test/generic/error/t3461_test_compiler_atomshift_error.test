#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3461_test_compiler_atomshift_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./A.ulam:4:8: ERROR: Non-primitive type: <Atom(96)> is not supported as LHS for binary operator<<.
	./A.ulam:5:8: ERROR: Non-primitive type: <Atom(96)> is not supported as RHS for binary operator<<.
	./A.ulam:4:8: ERROR: Non-primitive type: <Atom(96)> is not supported as LHS for binary operator<<.
	./A.ulam:5:8: ERROR: Non-primitive type: <Atom(96)> is not supported as RHS for binary operator<<.
      */
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\nInt test() {\n Atom a, b;\n b = a << 1;\n b = 1 << a;\n return -1;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3461_test_compiler_atomshift_error)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3466_test_compiler_voidshift_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./A.ulam:6:13: ERROR: Void is not supported for binary operator<<.
	./A.ulam:7:8: ERROR: Void is not supported for binary operator<<.
      */
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\nVoid func() {\n return;\n}\n Int test() {\n b = func() << 1;\n b = 1 << func();\n return -1;\n }\n Int b;\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3466_test_compiler_voidshift_error)

} //end MFM

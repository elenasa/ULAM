#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3213_test_compiler_elementboolmultiplyequal_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:7:7: ERROR: Incompatible Bool type for binary operator*. Suggest casting to a numeric type first.
      //./Foo.ulam:8:3: ERROR: Arithmetic Operations are invalid on 'Bool' type.

      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\nBool(3) a, b, c;\nInt test() {\na = false;\n b = true;\nc = a * b;\n a*=b;\nreturn a;\n }\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3213_test_compiler_elementboolmultiplyequal_error)

} //end MFM

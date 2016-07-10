#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3219_test_compiler_element_modelparameterasconstantexpression_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:4:2: ERROR: Type Bitsize specifier: Int(32), within (), is not a valid constant expression.
      //./Foo.ulam:4:2: ERROR: Type Bitsize specifier for base type: Int is not a constant expression.
      // fixed bitsize get:
      //./Foo.ulam:6:9: ERROR: Array size specifier in [] is not a constant integer.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\nparameter Int chance = 2;\n Int(chance) arr[chance];\nBool(1) last;\nInt test() {\nreturn chance;\n }\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3219_test_compiler_element_modelparameterasconstantexpression_error)

} //end MFM

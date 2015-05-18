#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3370_test_compiler_unseentemplateclass_error)
  {
    std::string GetAnswerKey()
    {
      //./Eltypo.ulam:4:2: ERROR: Incomplete Class: Typ0(UNKNOWN)<13> was never defined, fails sizing.
      return std::string("Exit status: 0\nUe_Eltypo { Typo t();  Int(32) test() {  0 cast return } }\nUq_Typo { /* empty class block */ }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //quark misspelled
      bool rtn1 = fms->add("Typo.ulam", "ulam 1;\nquark Typo(Int a) {\n }\n");

      bool rtn2 = fms->add("Eltypo.ulam", "ulam 1;\nuse Typo;\n element Eltypo {\n Typ0(1) t;\n Int test(){\n return 0;\n}\n }\n");

      if(rtn1 && rtn2)
	return std::string("Eltypo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3370_test_compiler_unseentemplateclass_error)

} //end MFM

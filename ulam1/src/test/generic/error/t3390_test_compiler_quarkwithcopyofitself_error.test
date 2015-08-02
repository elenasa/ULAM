#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3390_test_compiler_quarkwithcopyofitself_error)
  {
    std::string GetAnswerKey()
    {
      //./Typo.ulam:4:2: ERROR:  Quark Typo (UTI11) cannot contain a copy of itself.
      //....
      //./Typo.ulam:3:1: ERROR: Incomplete Variable Decl for type: Typo(UNKNOWN)<11> used with variable symbol name 't2' (UTI11) while bit packing class: Typo.
      return std::string("Uq_Typo { /* empty class block */ }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Typo.ulam", "ulam 1;\nquark Typo {\nTypo t2;\n }\n");

      if(rtn1)
	return std::string("Typo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3390_test_compiler_quarkwithcopyofitself_error)

} //end MFM

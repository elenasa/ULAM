#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3390_test_compiler_quarkwithcopyofitself_error)
  {
    std::string GetAnswerKey()
    {
      //./Typo.ulam:4:2: ERROR:  Quark Typo (UTI11) cannot contain a copy of itself.
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

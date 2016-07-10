#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3369_test_compiler_invalidclasstype_error)
  {
    std::string GetAnswerKey()
    {
      //./Typo.ulam:2:1: ERROR: Invalid Class Type: <quaak>; should be: 'element' , 'quark' , or 'union'.
      //Unrecoverable Program Parse FAILURE.
      return std::string("Uq_Typo { /* empty class block */ }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //quark misspelled
      bool rtn1 = fms->add("Typo.ulam", "ulam 1;\nquaak Typo {\n }\n");

      if(rtn1)
	return std::string("Typo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3369_test_compiler_invalidclasstype_error)

} //end MFM

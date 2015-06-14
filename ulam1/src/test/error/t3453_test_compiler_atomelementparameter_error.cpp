#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3453_test_compiler_atomelementparameter_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:6:6: ERROR: Invalid lefthand side of equals: <aep>, type: Atom(96).
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // cannot write into an EP in ulam (only MFM simulator!)
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\ntypedef Atom A;\n element A aep;\n  Int test(){\n aep = self;\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3453_test_compiler_atomelementparameter_error)

} //end MFM

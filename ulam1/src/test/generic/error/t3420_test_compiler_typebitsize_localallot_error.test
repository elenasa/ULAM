#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3420_test_compiler_typebitsize_localallot_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./C.ulam:5:2: ERROR: Type Bitsize specifier for base type: Int has a constant value of 65 that exceeds the maximum bitsize 64.
      */
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("C.ulam","ulam 1;\n element C {\nInt test() {\n Int(34) way;\n Int(65) noway;\n return -1;\n}\n }\n");

      if(rtn1)
	return std::string("C.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3420_test_compiler_typebitsize_localallot_error)

} //end MFM

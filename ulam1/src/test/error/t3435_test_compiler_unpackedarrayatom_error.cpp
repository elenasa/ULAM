#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3435_test_compiler_unpackedarrayatom_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:4:9: ERROR: Invalid non-scalar type: Atom(96)[10]'. Requires a custom array.
      return std::string("Exit status: 10\n\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\ntypedef Atom BigSite[10];\nInt test(){ BigSite site;\n for(Int i = 0; i < 10; ++i){\n site[i] = self;\n}\n return 10;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3435_test_compiler_unpackedarrayatom_error)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3436_test_compiler_unpackedarrayelements_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:2:13: ERROR: Invalid non-scalar type: A(0)[10]<14>'. Requires a custom array.
      return std::string("Exit status: 10\n\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // taken from 3436
      //bool rtn1 = fms->add("A.ulam","element A {\ntypedef A BigSite[10];\nInt test(){ BigSite site;\n for(Int i = 0; i < 10; ++i){\n site[i] = self;\n}\n return 10;\n }\n }\n");

      // here, tests non-typedef decl
      bool rtn1 = fms->add("A.ulam","element A {\nInt test(){ A site[10];\n for(Int i = 0; i < 10; ++i){\n site[i] = self;\n}\n return 10;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3436_test_compiler_unpackedarrayelements_error)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3444_test_compiler_elementwithclassparameterunpackedarray_error)
  {
    //informed by t3445
    std::string GetAnswerKey()
    {
      /*
	./S.ulam:2:17: ERROR: Named Constant 'y' cannot be based on a class type: BigSite.
	./S.ulam:2:17: ERROR: Invalid constant-def of Type: <BigSite> and Name: <y> (problem with []).
	./T.ulam:7:4: ERROR: Too many Class Arguments (2), class template: S expects 1.
      */
      return std::string("Exit status: -1\n\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse S;\n element T{\ntypedef Int BigSite[10];\n Int test(){\nBigSite site;\n S(site, 3) n;\n return 0;\n}\n }\n");

      bool rtn1 = fms->add("S.ulam"," ulam 1;\nquark S(BigSite y, Int i){\n }\n");

      if(rtn1 && rtn2)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3444_test_compiler_elementwithclassparameterunpackedarray_error)

} //end MFM

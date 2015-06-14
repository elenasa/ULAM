#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3446_test_compiler_constantdefunpackedarray_error)
  {
    //informed by t3445
    std::string GetAnswerKey()
    {
      /*
	./T.ulam:4:19: ERROR: Arraysize [] is included in typedef: <BigSite>, (UTI0), and cannot be used by a named constant: 'x'.
	./T.ulam:4:19: ERROR: Invalid constant-def of Type: <Int> and Name: <x> (problem with []).
      */
      return std::string("Exit status: -1\n\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("T.ulam"," ulam 1;\n element T{\ntypedef Int BigSite[10];\n constant BigSite x = 0;\n Int test(){\nBigSite site;\n return 0;\n}\n }\n");

      if(rtn2)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3446_test_compiler_constantdefunpackedarray_error)

} //end MFM

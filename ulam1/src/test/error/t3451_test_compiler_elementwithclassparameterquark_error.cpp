#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3451_test_compiler_elementwithclassparameterquark_error)
  {
    //informed by t3333
    std::string GetAnswerKey()
    {
      //./S.ulam:2:11: ERROR: Named Constant 'v' cannot be based on a class type: Q.
      //./S.ulam:2:11: ERROR: Invalid constant-def of Type: <Q> and Name: <v>.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse S;\n element T{\n Int test(){\n return 0;\n}\n }\n");

      bool rtn1 = fms->add("S.ulam"," ulam 1;\nuse Q;\n quark S(Q v){\n }\n");

      bool rtn3 = fms->add("Q.ulam"," ulam 1;\nunion Q{\nBool b;\n Int(3) i;\n}\n");

      if(rtn1 && rtn2 && rtn3)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3451_test_compiler_elementwithclassparameterquark_error)

} //end MFM

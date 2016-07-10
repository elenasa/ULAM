#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3399_test_compiler_arraysizeof)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Eltypo { typedef Int(3) IArr[0];  Int(32) test() {  Int(3) arr[0];  0u cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //arraysize zero ok, but not bitsize zero for nonClass primitive types!
      //bool rtn2 = fms->add("Eltypo.ulam", "ulam 1;\n element Eltypo {\ntypedef Int(3) IArr[4];\n Int test(){\nIArr arr;\n return arr.sizeof; \n}\n }\n");
      bool rtn2 = fms->add("Eltypo.ulam", "ulam 1;\n element Eltypo {\ntypedef Int(3) IArr[0];\n Int test(){\nIArr arr;\n return arr.sizeof; \n}\n }\n");

      if(rtn2)
      return std::string("Eltypo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3399_test_compiler_arraysizeof)

} //end MFM

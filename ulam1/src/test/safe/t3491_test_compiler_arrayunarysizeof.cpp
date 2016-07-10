#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3491_test_compiler_arrayunarysizeof)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 9\nUe_Eltypo { constant Unary(3) cy = 3;  typedef Int(3) IArr[3];  Int(32) test() {  Int(3) arr[3];  9u cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Eltypo.ulam", "ulam 1;\n element Eltypo {\nconstant Unary(3) cy = 3u;\n typedef Int(3) IArr[cy];\n Int test(){\nIArr arr;\n return arr.sizeof; \n}\n }\n");

      if(rtn2)
      return std::string("Eltypo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3491_test_compiler_arrayunarysizeof)

} //end MFM

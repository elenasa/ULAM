#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3448_test_compiler_shiftfunccallreturns)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 4\nUe_T { Int(32) i(4);  Int(32) test() {  i ( )func1 cast ( )func2 cast << cast = i return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("T.ulam"," ulam 1;\n element T{\nInt i;\n Int func1() {\n return 1;\n}\n Int func2() {\n return 2;\n}\n Int test(){\n i = (Int) ((Bits) func1() << func2()); return i;\n}\n }\n");

      if(rtn2)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3448_test_compiler_shiftfunccallreturns)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3447_test_compiler_comparefunccallreturns)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_T { Bool(1) b(true);  Int(32) test() {  b ( )func1 ( )func2 == = b cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("T.ulam"," ulam 1;\n element T{\nBool b;\n Int func1() {\n return 1;\n}\n Int func2() {\n return Bool.sizeof;\n}\n Int test(){\n b = func1() == func2(); return b;\n}\n }\n");

      if(rtn2)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3447_test_compiler_comparefunccallreturns)

} //end MFM

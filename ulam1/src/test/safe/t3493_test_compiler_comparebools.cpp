#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3493_test_compiler_comparebools)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_El { constant Unary(3) cy = 3;  Int(32) test() {  Bool(1) b;  b true = Bool(1) c;  c false = b cast 7u cast < c cast 0u cast < && cond 0 return if -1 return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn2 = fms->add("El.ulam", "ulam 1;\n element El {\n constant Unary(3) cy = 3u;\n Int test(){\nBool b = true, c = false;\n if( b < cy /*&& c <= 0u*/) return 0; \n return -1;\n}\n }\n");
      bool rtn2 = fms->add("El.ulam", "ulam 1;\n element El {\n constant Unary(3) cy = 3u;\n Int test(){\nBool(3) b = true, c = false;\n if( (Unary(3)) b < cy /*&& c <= 0u*/) return 0; \n return -1;\n}\n }\n");

      if(rtn2)
      return std::string("El.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3493_test_compiler_comparebools)

} //end MFM

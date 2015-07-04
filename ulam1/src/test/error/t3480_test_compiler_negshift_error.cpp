#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3480_test_compiler_negshift_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:7:1: ERROR: Converting Int(32) to Bool(1) requires explicit casting for while.
      //./A.ulam:8:26: ERROR: Unsigned is the supported type for RHS bitwise shift value, operator<<; Suggest casting Int(32) to Unsigned.
      //./A.ulam:9:22: ERROR: Unsigned is the supported type for RHS bitwise shift value, operator>>; Suggest casting Int(32) to Unsigned.
      return std::string("Exit status: -1\n\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3246
      bool rtn1 = fms->add("A.ulam","element A {\nBool(7) b;\nUnsigned d;\nInt test(){Int a;\n a = 8;\nd = 1;\nwhile(a){\nd = (Unsigned) (d << -1);\n a = (Int) (a >> -1);\n}\nreturn (Int) d;\n }\n }\n");

      // oops! >> is infinite WHILE loop
      //bool rtn1 = fms->add("A.ulam","element A {\nBool(7) b;\nUnsigned d;\nInt test(){Int a;\n a = 8;\nd = 1;\nwhile(a){\nd = (Unsigned) ((Bits) d << (Unsigned) -1);\n a = (Int) ((Bits) a >> (Unsigned) -1);\n}\nreturn (Int) d;\n }\n }\n");
      //ok.
      //bool rtn1 = fms->add("A.ulam","element A {\nBool(7) b;\nUnsigned d;\nInt test(){Int a;\n a = 8;\nd = 1;\n d = (Unsigned) ((Bits) d << (Unsigned) -1);\n a = (Int) ((Bits) a >> (Unsigned) -1);\nreturn (Int) d;\n }\n }\n");



      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3480_test_compiler_negshift_error)

} //end MFM

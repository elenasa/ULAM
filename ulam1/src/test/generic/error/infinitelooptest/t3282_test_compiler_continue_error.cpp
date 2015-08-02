#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3282_test_compiler_continue_error)
  {
    std::string GetAnswerKey()
    {
      // must turn off eval testing to avoid infinite loop; only gencode testable
      return std::string("Ue_Tu { Int(32) me(2);  Int(32) test() {  System s;  me 2 cast = s ( me )print . me return } }\nExit status: 2");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // ends in code generated compile
      //bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nelement Fu {\nInt test(){\nfor(Int i = 0; i < 5; ++i){\n if(i==0) continue;\n }\nreturn 0;\n}\n}\n");

      // ends in code generated compile
      //bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nelement Fu {\nInt test(){\nfor(Int i = 0; i < 5; ++i) continue;\nreturn 0;\n}\n}\n");

      // ends in code generated compile
      //bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nelement Fu {\nInt test(){\n Int j=5;\nwhile(--j > 0)\n if(j == 1) continue;\nreturn 0;\n}\n}\n");

      bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nelement Fu {\nInt test(){\nfor(Int i = 0; i < 5; ++i)\n continue;\n Int j=5;\nwhile(--j > 0)\n if(j == 1) continue;\nreturn 0;\n}\n}\n");

      if(rtn1)
	return std::string("Fu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3282_test_compiler_continue_error)

} //end MFM



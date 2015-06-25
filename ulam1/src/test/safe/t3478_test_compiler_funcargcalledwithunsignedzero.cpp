#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3478_test_compiler_funcargcalledwithunsignedzero)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Tu { typedef Unsigned(3) I;  Int(32) test() {  ( 0 cast )func cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\ntypedef Unsigned(3) I;\n I func(I arg) {\nreturn arg;\n}\n Int test(){\n return func(0);\n}\n}\n");


      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3478_test_compiler_funcargcalledwithunsignedzero)

} //end MFM

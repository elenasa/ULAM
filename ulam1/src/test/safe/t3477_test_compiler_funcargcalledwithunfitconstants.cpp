#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3477_test_compiler_funcargcalledwithunfitconstants)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 7\nUe_Tu { typedef Int(3) I;  typedef Int(4) J;  Int(32) test() {  ( 7 )func cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\ntypedef Int(3) I;\n I func(I arg) {\nreturn arg;\n}\n typedef Int(4) J;\n J func(J arg) {\nreturn arg;\n}\n Int test(){\n return func(7);\n}\n}\n");


      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3477_test_compiler_funcargcalledwithunfitconstants)

} //end MFM

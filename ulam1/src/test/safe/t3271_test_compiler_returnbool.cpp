#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3271_test_compiler_returnbool)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { typedef Unsigned(6) SiteNum;  Bool(1) sp(true);  Int(32) test() {  sp ( 7 cast )dup = 7 cast return } }\nExit status: 7");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\ntypedef Unsigned(6) SiteNum;\n Bool dup(SiteNum m) {\nBool b = m;\n return b;\n}\nBool sp;\nInt test(){ sp = dup((SiteNum) 7);\n return 7;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3271_test_compiler_returnbool)

} //end MFM



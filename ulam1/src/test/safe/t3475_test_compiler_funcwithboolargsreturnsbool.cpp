#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3475_test_compiler_funcwithboolargsreturnsbool)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Tu { Bool(3) me(false);  Int(32) test() {  me ( )func cast = me ( false me )func2 cast = me ( false true cast )func2 cast = me cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //func args do not cast implicitly except for constants: (see t3474)
      //./Tu.ulam:13:7: ERROR: (1) <func2> has no defined function with 2 matching argument types: Bool(3), Bool(3), and cannot be called, while compiling class: Tu.

      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\nBool(3) me;\nBool func() {\nBool(3) b = true;\n return b;\n}\n Bool func2(Bool b, Bool(3) c) {\n return b && c;\n}\n Int test(){\n me = func();\n me = func2(false, me);\n me = func2(false, true);\nreturn me;\n}\n}\n");


      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3475_test_compiler_funcwithboolargsreturnsbool)

} //end MFM

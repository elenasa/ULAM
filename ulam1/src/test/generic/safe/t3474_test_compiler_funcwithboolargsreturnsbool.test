#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3474_test_compiler_funcwithboolargsreturnsbool)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:13:7: ERROR: (1) <func2> has no defined function with 2 matching argument types: Bool(3), Bool(3), and cannot be called, while compiling class: Tu.
      return std::string("Exit status: 0\nUe_Tu { Bool(3) me(false);  Int(32) test() {  me ( )func cast = me ( me me )func2 cast = me cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\nBool(3) me;\nBool func() {\nBool(3) b = true;\n return b;\n}\n Bool func2(Bool b, Bool(3) c) {\n return b && c;\n}\n Bool func2(Bool(3) b, Bool(3) c) {\n return false;\n}\n Int test(){\n me = func();\n me = func2(me, me);\n return (Int) me;\n}\n}\n");


      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3474_test_compiler_funcwithboolargsreturnsbool)

} //end MFM

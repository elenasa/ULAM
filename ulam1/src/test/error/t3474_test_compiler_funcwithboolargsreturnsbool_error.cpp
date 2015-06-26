#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3474_test_compiler_funcwithboolargsreturnsbool_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:13:7: ERROR: (1) <func2> has no defined function with 2 matching argument types: Bool(3), Bool(3), and cannot be called, while compiling class: Tu.
      return std::string("Ue_Tu { Int(32) me(2);  Int(32) test() {  System s;  me 2 cast = s ( me )print . me return } }\nExit status: 2");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\nBool(3) me;\nBool func() {\nBool(3) b = true;\n return b;\n}\n Bool func2(Bool b, Bool(3) c) {\n return b && c;\n}\n Int test(){\n me = func();\n me = func2(me, me);\n return me;\n}\n}\n");


      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3474_test_compiler_funcwithboolargsreturnsbool_error)

} //end MFM

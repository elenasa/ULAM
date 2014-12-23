#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3266_test_compiler_forloop_scope_issue)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 2
	 Int Arg: 1
	 Int Arg: 0
      */
      return std::string("Ue_A { Bool(7) b(true);  System s();  Int(32) d(0);  Int(32) test() {  d 3 cast = { Int(32) index;  index 0 cast = index d < cast cond b index cast = index 1 cast += while } { Int(32) index;  index 0 cast = index 3 cast < cast cond { d 1 cast -= s ( d )print . } index 1 cast += while } d return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by: WindowUtils
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\nInt d;\nInt test(){\nd = 3;\n for(Int index = 0; index < d; index+=1)\n b = index;\n for(Int index = 0; index < 3; index+=1){\n d-=1;\ns.print(d);\n }\n return d;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3266_test_compiler_forloop_scope_issue)

} //end MFM



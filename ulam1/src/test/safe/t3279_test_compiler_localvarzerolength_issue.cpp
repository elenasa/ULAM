#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3279_test_compiler_localvarzerolength_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Tu { Int(32) me(2);  Int(32) test() {  System s;  me 2 cast = s ( me )print . me return } }\nExit status: 2");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nuse System;\n element Tu {\nInt me;\nInt test(){\nSystem s;\n me = 2;\n s.print(me);\n return me;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3279_test_compiler_localvarzerolength_issue)

} //end MFM



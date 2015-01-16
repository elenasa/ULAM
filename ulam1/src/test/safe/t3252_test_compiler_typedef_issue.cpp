#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3252_test_compiler_typedef_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Bool(7) b(false);  System s();  typedef Unsigned(8) Byte;  Unsigned(8) arr[2](1,0);  Int(32) test() {  arr 0 [] 1 cast = arr 0 [] cast 0 cast == cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\ntypedef Unsigned(8) Byte;\nByte arr[2];\n Int test(){ arr[0] = 1;\n return arr[0] == 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3252_test_compiler_typedef_issue)

} //end MFM



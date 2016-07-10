#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3293_test_compiler_typebitsize_minmaxsizeof)
  {
    std::string GetAnswerKey()
    {
      /* gen code output: array size (3 x 3):
	 Unsigned Arg: 9
      */

      return std::string("Exit status: 0\nUe_B { System s();  Int(32) test() {  Int(3) arr[3];  s ( 9u cast )print . 0 cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("B.ulam","use System;\nelement B {\nSystem s;\nInt test(){ Int(Int(3).maxof) arr[Int(3).maxof];\ns.print((Unsigned) arr.sizeof);\n return 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("B.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3293_test_compiler_typebitsize_minmaxsizeof)

} //end MFM

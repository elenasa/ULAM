#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3291_test_compiler_typedefarray_minmaxsizeof)
  {
    std::string GetAnswerKey()
    {
      /* gen code output: array size (8 x 4), 3 ways:
	 Unsigned Arg: 32
	 Unsigned Arg: 32
	 Unsigned Arg: 32
      */

      return std::string("Exit status: 0\nUe_B { Bool(7) b(false);  System s();  typedef Unsigned(8) Index;  typedef Unsigned(8) NIdx;  typedef Unsigned(8) IndArr[4];  typedef Unsigned(8) IArray[4];  Unsigned(8) arr[4](0,0,0,0);  Int(32) test() {  s ( 32u cast )print . s ( 32u cast )print . s ( 32u cast )print . 0 cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // with renames of typedefs, and arrays based on typedef
      bool rtn1 = fms->add("B.ulam","use System;\nelement B {\nSystem s;\nBool(7) b;\ntypedef Unsigned(8) Index;\ntypedef Index NIdx;\ntypedef NIdx IndArr[4];\ntypedef IndArr IArray;\n IArray arr;\n Int test(){ s.print((Unsigned) IndArr.sizeof);\n s.print((Unsigned) IArray.sizeof);\n s.print((Unsigned) arr.sizeof);\n return 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("B.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3291_test_compiler_typedefarray_minmaxsizeof)

} //end MFM



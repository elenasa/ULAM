#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3430_test_compiler_intermediatesaturation_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { typedef Int(9) Foo;  Int(9) f(254);  Int(32) test() {  f 255 = f f cast 1 +b 1 -b cast = f cast 254 cast == cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //aren't the same, but should be!!!

      bool rtn1 = fms->add("A.ulam"," element A {\ntypedef Int(9) Foo;\n Foo f;\n Int test(){\n f = f.maxof;\n f += 1;\n f-= 1;\n return f == f.maxof;\n }\n }\n");

      //bool rtn1 = fms->add("A.ulam","element A {\ntypedef Int(9) Foo;\n Foo f;\n Int test(){\nf = f.maxof;\n f = f + 1 - 1;\n return f == f.maxof;\n }\n }\n");



      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3430_test_compiler_intermediatesaturation_issue)

} //end MFM

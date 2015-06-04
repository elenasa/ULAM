#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3428_test_compiler_sum64bitint)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 64\nUe_A { Int(32) test() {  typedef Int(64) Long;  Int(64) k;  Int(64) m;  m k 9223372036854775807 +b = 64u cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam"," element A {\n Int test(){\ntypedef Int(64) Long;\n Long k,m;\n m = k + Long.maxof;\n return m.sizeof;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      //bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3428_test_compiler_sum64bitint)

} //end MFM

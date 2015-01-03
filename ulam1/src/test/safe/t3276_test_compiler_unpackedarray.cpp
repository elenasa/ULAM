#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3276_test_compiler_unpackedarray)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 0
	 Int Arg: 1
	 Int Arg: 2
	 Int Arg: 3
	 Int Arg: 4
	 Int Arg: 5
	 Int Arg: 6
	 Int Arg: 7
	 Int Arg: 8
	 Int Arg: 9
      */
      return std::string("Ue_A { System s();  typedef Int(32) BigSite[10];  Int(32) test() {  Int(32) site[10];  { Int(32) i;  i 0 cast = i 10 cast < cond { site i [] i = s ( site i [] )print . } _1: i 1 cast += while } 10 cast return } }\nExit status: 10");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\n element A {\nSystem s;\ntypedef Int BigSite[10];\nInt test(){ BigSite site; for(Int i = 0; i < 10; ++i){\n site[i] = i;\n s.print(site[i]);\n}\n return 10;\n }\n }\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3276_test_compiler_unpackedarray)

} //end MFM



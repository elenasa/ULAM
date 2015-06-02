#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3426_test_compiler_unpackedarray64)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 2147483647
	 Int Arg: -2147483648
	 Int Arg: -2147483647
	 Int Arg: -2147483646
	 Int Arg: -2147483645
	 Int Arg: -2147483644
	 Int Arg: -2147483643
	 Int Arg: -2147483642
	 Int Arg: -2147483641
	 Int Arg: -2147483640
      */
      return std::string("Exit status: 640\nUe_A { typedef Int(64) BigSite[10];  Int(32) test() {  System s;  Int(64) site[10];  { Int(32) i;  i 0 = i 10 < cond { Int(64) k;  k i 2147483647 +b cast = site i [] k = s ( site i [] cast )print . } _1: i 1 += while } 640u cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3276:
      bool rtn1 = fms->add("A.ulam","use System;\n element A {\ntypedef Int(64) BigSite[10];\nInt test(){System s;\n  BigSite site;  for(Int i = 0; i < 10; ++i){\nInt(64) k = i + Int.maxof;\n site[i] = k;\n s.print((Int) site[i]); \n }\n return site.sizeof;\n }\n }\n");

      //simplify for debugging
      //bool rtn1 = fms->add("A.ulam"," element A {\n Int test(){\nInt(64) k;\n k = 1 + Int.maxof;\n return k.sizeof;\n }\n }\n");

      //site[i] = i, k = 512, etc.. requires casting 32 to 64
      // ReadArrayLong and WriteArrayLong do not exist in BitField!!! do now!
      //bool rtn1 = fms->add("A.ulam"," element A {\ntypedef Int(64) BigSite[1];\nInt test(){ Int(64) k = 512;\n BigSite site; for(Int i = 0; i < 1; ++i){\n site[i] = k;\n }\n return site.sizeof;\n }\n }\n");

      // good for isolating bug!
      //bool rtn1 = fms->add("A.ulam"," element A {\ntypedef Int(64) BigSite[1];\nInt test(){ Int(64) k = 512;\n BigSite site; site[0] = k; return site.sizeof;\n }\n }\n");

      //./A.ulam:3:23: ERROR: Cast problem! Value type: Int(32) failed to be cast as type: Int(64).
      //bool rtn1 = fms->add("A.ulam"," element A {\ntypedef Int(32) BigSite[1];\nInt test(){ Int(64) k = 512;\n BigSite site; for(Int i = 0; i < 1; ++i){\n site[i] = k;\n }\n return site.sizeof;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3426_test_compiler_unpackedarray64)

} //end MFM

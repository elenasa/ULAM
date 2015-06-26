#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3425_test_compiler_packedarray64_asfuncarg)
  {
    std::string GetAnswerKey()
    {
      /* gencode output:
	 Int Arg: 9
      */
      return std::string("Exit status: 9\nUe_A { typedef Int(6) BigSite[10];  Int(32) test() {  System p;  Int(6) site[10];  site ( )func2 = Int(6) site2[10];  site2 site = p ( ( site )func )print . ( site )func return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by error/t3419, except Int(6) instead of Int(3)/
      //no binaryop+ even for packed array (site1 + site2).
      bool rtn1 = fms->add("A.ulam","use System;\n element A {\ntypedef Int(6) BigSite[10];\nInt func(BigSite sarr) {\n return sarr[9];\n}\n BigSite func2() {\n BigSite s;\nfor(Int i = 0; i < 10; ++i){\n s[i] = (Int(6)) i;\n}\n return s;\n}\n Int test(){\nSystem p;\n BigSite site = func2();\n BigSite site2 = site /*+ site2*/;\n p.print(func(site));\n return func(site);\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3425_test_compiler_packedarray64_asfuncarg)

} //end MFM

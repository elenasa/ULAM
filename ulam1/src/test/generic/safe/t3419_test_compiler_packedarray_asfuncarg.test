#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3419_test_compiler_packedarray_asfuncarg)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_A { typedef Int(3) BigSite[10];  Int(32) test() {  Int(3) site[10];  site ( )func2 = Int(3) site2[10];  site2 site = ( site )func return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by error/t3417_
      //no binaryop+ even for packed array (site1 + site2).
      //./A.ulam:15:23: ERROR: Incompatible (nonscalar) types, LHS: Int(3)[10], RHS: Int(3)[10] for binary operator+.
      bool rtn1 = fms->add("A.ulam","element A {\ntypedef Int(3) BigSite[10];\nInt func(BigSite sarr) {\n return sarr[9];\n}\n BigSite func2() {\n BigSite s;\nfor(Int i = 0; i < 10; ++i){\n s[i] = (Int(3)) i;\n}\n return s;\n}\n Int test(){\n BigSite site = func2();\n BigSite site2 = site /*+ site2*/;\n return func(site);\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3419_test_compiler_packedarray_asfuncarg)

} //end MFM

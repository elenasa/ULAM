#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3417_test_compiler_unpackedarray_asfuncarg_error)
  {
    std::string GetAnswerKey()
    {
      // avoid asserts! (with or without eval test -- see NodeBlockClass SKIP_EVAL)
      /*
	./A.ulam:3:10: ERROR: Function Definition parameter 1, type: Int(32)[10] (UTI12), requires UNPACKED array support.
	./A.ulam:6:2: ERROR: Function Definition <func2> return type: Int(32)[10] (UTI12), requires UNPACKED array support.
	./A.ulam:14:15: ERROR: Lefthand side of equals requires UNPACKED array support: <site>, type: Int(32)[10].
	./A.ulam:14:15: ERROR: Righthand side of equals requires UNPACKED array support: <func2>, type: Int(32)[10].
	./A.ulam:15:23: ERROR: Incompatible (nonscalar) types, LHS: Int(32)[10], RHS: Int(32)[10] for binary operator+.
	./A.ulam:16:14: ERROR: Function Definition parameter 1, type: Int(32)[10] (UTI12), requires UNPACKED array support.
	./A.ulam:3:10: ERROR: Function Definition parameter 1, type: Int(32)[10] (UTI12), requires UNPACKED array support.
	./A.ulam:6:2: ERROR: Function Definition <func2> return type: Int(32)[10] (UTI12), requires UNPACKED array support.
	./A.ulam:14:15: ERROR: Lefthand side of equals requires UNPACKED array support: <site>, type: Int(32)[10].
	./A.ulam:14:15: ERROR: Righthand side of equals requires UNPACKED array support: <func2>, type: Int(32)[10].
	./A.ulam:15:23: ERROR: Incompatible (nonscalar) types, LHS: Int(32)[10], RHS: Int(32)[10] for binary operator+.
	./A.ulam:16:14: ERROR: Function Definition parameter 1, type: Int(32)[10] (UTI12), requires UNPACKED array support.
      */
      return std::string("Exit status: 10\nUe_A { System s();  typedef Int(32) BigSite[10];  Int(32) test() {  Int(32) site[10];  { Int(32) i;  i 0 = i 10 < cond { site i [] i = s ( site i [] )print . } _1: i 1 += while } 10 return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\ntypedef Int BigSite[10];\nInt func(BigSite sarr) {\n return sarr[9];\n}\n BigSite func2() {\n BigSite s;\nfor(Int i = 0; i < 10; ++i){\n s[i] = i;\n}\n return s;\n}\n Int test(){\n BigSite site = func2();\n BigSite site2 = site + site2;\n return func(site);\n }\n }\n");

      //bool rtn1 = fms->add("A.ulam","element A {\ntypedef Int BigSite[10];\nInt test(){\n BigSite site, site2; site = site2;\n return 2;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3417_test_compiler_unpackedarray_asfuncarg_error)

} //end MFM

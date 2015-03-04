#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3332_test_compiler_elementwithclassparameters_deferredsizeofinquark)
  {
    //informed by t3331
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 6\nUe_T { Int(32) i(6);  Int(32) test() {  S(1,2) m;  i m ( )func . = i return } }\nUq_S { constant Int(CONSTANT) x = NONREADYCONST;  constant Int(CONSTANT) y = NONREADYCONST;  Int(UNKNOWN) i(0);  Int(UNKNOWN) j(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // here we set i using S(1,2).sizeof
      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse S;\n element T{\nInt i;\nInt test(){\nS(1,2) m;\ni = m.func();\n return i;\n}\n }\n");

      //infinite loop 'S(x+y,1) s;' with x+y as class arg!
      //note: quark self.sizeof returns 96 (an atom's size).
      //they all work now!!
      bool rtn1 = fms->add("S.ulam"," ulam 1;\nquark S(Int x, Int y){\nInt(x+y) i,j;\n Int func(){\nreturn /* (x + y) */  S(x,y).sizeof  /* Int(x+y).sizeof*/  /* i.sizeof */ ;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      //bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      //if(rtn1 && rtn2 && rtn3)
      if(rtn1 && rtn2)
	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3332_test_compiler_elementwithclassparameters_deferredsizeofinquark)

} //end MFM

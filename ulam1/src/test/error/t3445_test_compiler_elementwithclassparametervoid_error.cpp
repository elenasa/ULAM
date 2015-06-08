#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3445_test_compiler_elementwithclassparametervoid_error)
  {
    //informed by t3333
    std::string GetAnswerKey()
    {
      //./S.ulam:2:9: ERROR: Invalid constant-def Type <Void>.
      return std::string("Exit status: 7\nUe_T { Int(32) j(7);  System s();  Int(32) test() {  Int(4) b;  S(2,2) n;  j n ( b )func . = s ( j )print . j return } }\nUq_S { constant Int(CONSTANT) x = NONREADYCONST;  constant Int(CONSTANT) y = NONREADYCONST;  Int(UNKNOWN) i(0);  Int(UNKNOWN) j(0);  <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse S;\n element T{\n Int test(){\n /*S((Void) 2) n;\n*/ return 0;\n}\n }\n");

      bool rtn1 = fms->add("S.ulam"," ulam 1;\nquark S(Void v, Int i){\n }\n");

      if(rtn1 && rtn2)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3445_test_compiler_elementwithclassparametervoid_error)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3452_test_compiler_elementwithclassparameteratom_error)
  {
    //informed by t3333
    std::string GetAnswerKey()
    {
      /*
	./S.ulam:2:10: ERROR: Invalid constant-def Type <Atom>.
	./S.ulam:2:16: ERROR: Class Template has NO parameters: S(UNKNOWN)<14>.
	./T.ulam:5:10: ERROR: Invalid constant-def Type <Atom>.
	./T.ulam:5:1: ERROR: Invalid Statement (possible missing semicolon).
	./T.ulam:7:4: ERROR: Too many Class Arguments (1), class template: S expects 0.
      */
      return std::string("Exit status: 0\nUe_T { Int(32) test() {  Atom(96) a;  0 return } }\nUq_S { /* empty class block */ }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse S;\n element T{\n Int test(){\nconstant Atom aaa = 3;\n Atom a;\n S(a) s;\n return 0;\n}\n }\n");

      bool rtn1 = fms->add("S.ulam"," ulam 1;\n quark S(Atom v){\n }\n");

      if(rtn1 && rtn2)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3452_test_compiler_elementwithclassparameteratom_error)

} //end MFM

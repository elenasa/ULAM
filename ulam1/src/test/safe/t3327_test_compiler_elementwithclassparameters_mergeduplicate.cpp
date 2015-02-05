#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3327_test_compiler_elementwithclassparameters_mergeduplicate)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Foo { Bool(UNKNOWN) b(false);  constant Int(CONSTANT) x = NONREADYCONST;  constant Int(CONSTANT) y = NONREADYCONST;  Bool(UNKNOWN) sp(false);  Bool(UNKNOWN) c(false);  Int(32) test() {  Foo f;  Foo p;  0 return } }\nUe_Poo { Bool(1) valb[3](false,false,false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Poo;\nelement Foo(Int x, Int y) {\nBool(x) sp;\nBool(y) b,c;\nInt test() {\nFoo(1,3) f;\n Foo(1,Poo.sizeof) p;\n/*sp = (f.sizeof == p.sizeof);\n*/ return 0;\n }\n }\n");

      // try without decl list
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Poo;\nelement Foo(Int x, Int y) {\nBool(x) sp;\nBool(y) b;\nBool(y) c;\nInt test() {\nFoo(1,3) f;\n Foo(1,Poo.sizeof) p;\n/*sp = (f.sizeof == p.sizeof);\n*/ return 0;\n }\n }\n");

      bool rtn2 = fms->add("Poo.ulam"," ulam 1;\n element Poo {\n Bool valb[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3327_test_compiler_elementwithclassparameters_mergeduplicate)

} //end MFM



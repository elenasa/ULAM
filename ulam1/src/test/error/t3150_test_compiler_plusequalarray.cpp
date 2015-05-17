#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3150_test_compiler_plusequalarray)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:8:4: ERROR: Non-scalars require a loop for operator+= on LHS: <d>, type: Int(32)[2].
      return std::string("Ue_A { typedef Int(32) Foo[2];  Int(32) d[2](7,3);  Int(32) test() {  d 0 [] 1 cast = Int(32) e[2];  e 0 [] 6 cast = e 1 [] 3 cast = d e += d 0 [] return } }\nExit status: 7");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n typedef Int Foo [2];\n Int test() {\n d[0] = 1;\n Foo e;\n e[0] = 6;\n e[1] = 3;\n d += e;\n return d[0];\n /* match return type */}\nFoo d;\n }");  // want d[0] == 7. NOTE: requires 'operator+=' in c-code to add arrays


      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3150_test_compiler_plusequalarray)

} //end MFM

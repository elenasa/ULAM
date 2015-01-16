#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3127_test_compiler_typedef_scope)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_A { typedef Int(16) Bar[2];  Int(16) d[2](1,0);  Int(32) test() {  Int(16) myb[2];  myb 0 [] 1 cast = myb 1 [] 0 cast = d ( myb )bar = d 0 [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","element A { typedef Int Bar [2]; Bar bar(Bar b) { Bar m;  if(b[0]) m[0]=1; else m[0]=2; return m;} Int test() { Bar myb; myb[0] = 1; myb[1] = 0; d = bar(myb); return d[0]; /* match return type */ } Bar d; }");  // want d[0] == 1.

      //needs newlines
      bool rtn1 = fms->add("A.ulam","element A {\n typedef Int(16) Bar [2];\n Bar bar(Bar b) {\n Bar m;\n  if(b[0])\n m[0]=1;\n else\n m[0]=2;\n return m;\n}\n Int test() {\n Bar myb;\n myb[0] = 1;\n myb[1] = 0;\n d = bar(myb);\n return d[0];\n /* match return type */ }\n Bar d;\n }\n");  // want d[0] == 1.


      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3127_test_compiler_typedef_scope)

} //end MFM



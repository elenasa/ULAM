#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3252_test_compiler_typedef_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Bool(7) b(false);  typedef Unsigned(8) Byte;  Unsigned(8) arr[2](1,0);  Int(32) test() {  arr 0 [] 1 cast = arr 0 [] 0 cast == cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\nBool(7) b;\ntypedef Unsigned(8) Byte;\nByte arr[2];\n Int test(){ arr[0] = 1;\n return (Int) (arr[0] == 0);\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3252_test_compiler_typedef_issue)

} //end MFM

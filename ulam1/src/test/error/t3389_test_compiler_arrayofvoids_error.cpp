#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3389_test_compiler_arrayofvoids_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:6:2: ERROR: Invalid Void type array: Void(UNKNOWN)[UNKNOWN]> (UTI13).
      return std::string("Exit status: 0\nUe_A { Bool(1) a[5](false,false,false,false,true);  Int(32) test() {  a 4 [] true = 0 return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by3311
      bool rtn1 = fms->add("A.ulam","element A {\n Void(0) a[5];\n Int test() {\n return 1;\n}\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3389_test_compiler_arrayofvoids_error)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3400_test_compiler_arraysizeof_lengthzero)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Eltypo { typedef Empty EArr[4];  Int(32) test() {  Empty arr[4];  0u cast return } }\nUq_Empty { /* empty class block */ }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //arraysize zero ok, but not bitsize zero!
      // gcc balks at BitVector for immediate (Dave fixed it)
      bool rtn1 = fms->add("Empty.ulam", "ulam 1;\n quark Empty {\n }\n");
      bool rtn2 = fms->add("Eltypo.ulam", "ulam 1;\nuse Empty;\n element Eltypo {\ntypedef Empty EArr[4];\n Int test(){\nEArr arr;\n return arr.sizeof; \n}\n }\n");

      if(rtn1 && rtn2)
      return std::string("Eltypo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3400_test_compiler_arraysizeof_lengthzero)

} //end MFM

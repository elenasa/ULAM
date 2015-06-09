#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3442_test_compiler_typedefvoid_warn)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:2:13: Warning: Void bitsize expression disregarded; size is zero.
      return std::string("Exit status: 0\nUe_A { typedef Void(0) V0;  Int(32) test() {  ( )func 0u cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\ntypedef Void(300) V0;\n V0 func() {} Int test() {\n func();\n return V0.sizeof;\n }\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3442_test_compiler_typedefvoid_warn)

} //end MFM

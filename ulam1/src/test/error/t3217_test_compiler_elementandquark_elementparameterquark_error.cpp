#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3217_test_compiler_elementandquark_elementparameterquark_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:7:12: ERROR: Only primitive types may be an element parameter, not: <Bar>.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //can't be a quark
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBool(3) b;\n parameter Bar barchance;\nBool last;\nInt test() {\n b = barchance.valb[1];\ns.print(b);\nreturn b;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool valb[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3217_test_compiler_elementandquark_elementparameterquark_error)

} //end MFM

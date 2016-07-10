#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3460_test_compiler_elementandquark_namedconstantquark_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:5:15: ERROR: Named Constant 'barchance' cannot be based on a class type: Bar.
      //./Foo.ulam:5:15: ERROR: Invalid constant definition of Type: <Bar> and Name: <barchance>.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //based on 3217
      //constant can't be a quark or union
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBool(3) b;\nBar bar;\n constant Bar barchance = bar;\nBool last;\nInt test() {\n b = barchance.valb[1];\ns.print(b);\nreturn b;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool valb[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3460_test_compiler_elementandquark_namedconstantquark_error)

} //end MFM

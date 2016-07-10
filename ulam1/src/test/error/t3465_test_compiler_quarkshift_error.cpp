#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3465_test_compiler_quarkshift_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:5:4: ERROR: Cannot cast Int(32) (UTI2) to type: Bar (UTI14).
      //./A.ulam:5:4: ERROR: Cannot CAST type: Int(32) as a Bar(0)<14>.
      //./A.ulam:6:4: ERROR: Cannot cast Int(32) (UTI2) to type: Bar (UTI14).
      //./A.ulam:6:4: ERROR: Cannot CAST type: Int(32) as a Bar(0)<14>.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //using quark in a shift converts it toInt, t.f. no longer a 'Bar' type.
      bool rtn1 = fms->add("A.ulam","use Bar;\n element A {\nInt test() {\n Bar a, b;\n b = a << 1;\n b = 1 << a;\n return -1;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam","quark Bar {\n Int toInt() { return 1;\n}\n }\n");

      if(rtn1 && rtn2)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3465_test_compiler_quarkshift_error)

} //end MFM

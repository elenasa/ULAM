#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3121_test_compiler_arrays_add_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./D.ulam:1:56: ERROR: Incompatible (nonscalar) types, LHS: Int(16)[3], RHS: Int(16) for binary operator+.
	./D.ulam:1:46: ERROR: Function 'test''s Return type's: Int(16) does not match incomplete resulting type.
	./D.ulam:1:30: ERROR: By convention, Function 'test''s Return type must be 'Int', not Int(16).
      */
      return std::string(" D.ulam:1:35: ERROR: Incompatible (nonscalar) types, LHS: <Int.16.3>, RHS: <Int.16.-1> for binary operator+.\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","element D { Int(16) a[3], b; Int(16) test() {return (a + b);} }");

      if(rtn1)
	return std::string("D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3121_test_compiler_arrays_add_error)

} //end MFM

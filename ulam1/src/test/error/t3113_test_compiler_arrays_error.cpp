#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3113_test_compiler_arrays_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./D.ulam:1:38: ERROR: Array casts currently not supported.
	./D.ulam:1:38: ERROR: Consider implementing array casts: Cannot cast scalar into array.
	./D.ulam:1:38: ERROR: Cannot CAST type: Int(32) as a Int(32)[2].
      */
      return std::string(" D.ulam:1:33: ERROR: Not storeIntoAble: <a>, is type: <Int.32.2>.\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","element D { Int a[2]; Int test() { a = 1; return a[1]; } }");

      if(rtn1)
	return std::string("D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3113_test_compiler_arrays_error)

} //end MFM

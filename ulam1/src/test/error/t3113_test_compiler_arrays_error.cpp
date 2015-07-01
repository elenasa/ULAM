#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3113_test_compiler_arrays_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./D.ulam:5:6: Warning: Cannot check constant '1' fits into type: Int(32)[2].
	./D.ulam:5:4: ERROR: Converting Int(32) to Int(32)[2] requires explicit casting for operator=.
	./D.ulam:9:2: ERROR: Casting different Array sizes: Int(32) TO Int(32)[2].
	./D.ulam:6:7: ERROR: Cannot explicitly cast Int(32) to type: Int(32)[2].
      */
      return std::string(" D.ulam:1:33: ERROR: Not storeIntoAble: <a>, is type: <Int.32.2>.\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","element D {\n typedef Int IA[2];\n IA a;\n Int test() {\n a = 1;\n a = (IA) 1;\n return a[1];\n }\n }\n");

      if(rtn1)
	return std::string("D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3113_test_compiler_arrays_error)

} //end MFM

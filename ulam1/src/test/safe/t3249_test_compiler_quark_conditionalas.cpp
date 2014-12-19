#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3249_test_compiler_quark_conditionalas)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Counter4 { Int(32) d(0);  <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Counter4.ulam", "ulam 1;\nquark Counter4 {\nInt d;\nVoid incr(){\nd+=1;\n}\nInt get(){\nreturn d;\n}\n}\n");

      if(rtn2)
	return std::string("Counter4.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3249_test_compiler_quark_conditionalas)

} //end MFM

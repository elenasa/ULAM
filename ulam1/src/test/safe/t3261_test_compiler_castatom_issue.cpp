#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3261_test_compiler_castatom_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Bar { Foo valoo( Int(7) val(0); );  Int(32) test() {  1 cast return } }\nExit status: 1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\n element Tu {\n Int func(Atom a) {\n return 0;\n}\nInt test(){\n Tu t;\nAtom a = (Atom) t;\n return func((Atom) t);\n}\n}\n");

      if( rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3261_test_compiler_castatom_issue)

} //end MFM



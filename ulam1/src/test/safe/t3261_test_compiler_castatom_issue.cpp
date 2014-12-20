#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3261_test_compiler_castatom_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Tu { Int(32) test() {  Tu t;  Atom(96) a;  a t cast = ( t cast )func return } }\nExit status: 0");
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



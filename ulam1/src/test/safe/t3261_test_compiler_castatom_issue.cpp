#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3261_test_compiler_castatom_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Tu { Int(32) test() {  Tu t;  Atom(96) a;  a t cast = ( t cast )func return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\n element Tu {\n Int func(Atom a) {\n return 0;\n}\nInt test(){\n Tu t;\nAtom a = (Atom) t;\n return func((Atom) t);\n}\n}\n");

      // ok.
      //bool rtn1 = fms->add("Tu.ulam", "ulam 1;\n element Tu {\n Int func(Atom a) {\n return 0;\n}\nInt test(){\n Tu t;\nAtom a = (Atom) t;\n /*return func((Atom) t);*/ return 0;\n}\n}\n");

      // simplify for debugging
      //bool rtn1 = fms->add("Tu.ulam", "ulam 1;\n element Tu {\n Int func(Atom a) {\n return 0;\n}\nInt test(){\n Tu t;\nreturn func((Atom) t);\n}\n}\n");
      //bool rtn1 = fms->add("Tu.ulam", "ulam 1;\n element Tu {\n Int func(Atom a) {\n return 0;\n}\nInt test(){\n Tu t;\nInt r; r = func((Atom) t);\n return r;\n}\n}\n");

      if( rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3261_test_compiler_castatom_issue)

} //end MFM

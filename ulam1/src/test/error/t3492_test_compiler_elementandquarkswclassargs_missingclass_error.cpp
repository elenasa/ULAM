#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3492_test_compiler_elementandquarkswclassargs_missingclass_error)
  {
    std::string GetAnswerKey()
    {
      //./R.ulam:5:2: ERROR: Unresolved type <P> was never defined; Fails sizing.
      //./R.ulam:5:2: ERROR: Unresolved type <P> was never defined; Fails labeling.
      return std::string("Exit status: 3\nUe_R { Int(32) psize(3);  Int(32) test() {  P(3) pvar;  psize pvar ( )poo . = psize return } }\nUq_P { constant Int(32) a = NONREADYCONST;  Bool(UNKNOWN) b(false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn4 = fms->add("R.ulam","ulam 1;\nelement R {\nInt psize;\n Int test() {\n P(3) pvar; psize = pvar.poo();\nreturn psize;\n}\n}\nuse P;\n");

      // actual files tests a different code path which had an infinite loop Sat Jul 11 10:33:00 2015
      // see: home/elenas/WORK/ulam/issues/afterTypeSelect/missing
      //bool rtn1 = fms->add("P.ulam","ulam 1;\nquark P(Int a) {\nBool(a) b;\n Int poo() { return a;\n}\n}\n");

      // missing P!!!
      bool rtn1 = fms->add("P.ulam","");

      if(rtn1 && rtn4)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3492_test_compiler_elementandquarkswclassargs_missingclass_error)

} //end MFM

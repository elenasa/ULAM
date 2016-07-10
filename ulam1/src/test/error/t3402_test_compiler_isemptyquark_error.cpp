#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3402_test_compiler_isemptyquark_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:7:7: ERROR: Invalid righthand type of conditional operator 'is'; must be an element name, not type: Empty.

      //sh: 1: /home/elenas/WORK/ulam/repo/ULAM/ulam1/src/test/bin/main: not found

      // gen code issue: quark doesn't have internalCMethodImplementingIs!!
      //include/Ue_10163Foo0.tcc:28:57: error: ‘POS’ was not declared in this scope
      //        const u32 Uh_tmpreg_loadable_12 = Uq_10105Empty0<EC,POS>::internalCMethodImplementingIs(Uh_tmpval_unpacked_11);

      return std::string("Exit status: 0\nUe_Foo { Unary(6) energy(0);  Int(32) test() {  Atom(96) a;  a Empty is cond { energy cast 63u cast == cond { 1 return } if energy cast 0u cast == cond { 0 return } if else } if -1 return } }\nUq_Empty { /* empty class block */ }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Foo.ulam", "ulam 1;\n use Empty;\n element Foo {\nUnary(6) energy;\n Int test(){\nAtom a;\n if(a is Empty) {\n if(energy == energy.maxof) {\n return 1;\n }\n else if (energy == energy.minof) {\n return 0;\n }\n }\n return -1;\n }\n }\n");

      bool rtn1 = fms->add("Empty.ulam", "ulam 1;\n quark Empty {\n }\n");

      if(rtn1 && rtn2)
      return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3402_test_compiler_isemptyquark_error)

} //end MFM

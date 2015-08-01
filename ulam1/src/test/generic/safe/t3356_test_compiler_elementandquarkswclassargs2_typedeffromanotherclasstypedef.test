#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3356_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 2\nUe_P { constant Int(32) a = NONREADYCONST;  constant Unsigned(UNKNOWN) nvar = NONREADYCONST;  Int(UNKNOWN) arr[UNKNOWN](0);  Bool(1) b(false);  Int(32) test() {  P(1) pel;  pel arr 2 [] . 2 = pel arr 2 [] . return } }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) Foo;  <NOMAIN> }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3346:  recursive typedefs as named constant type; used for size of array
      // must already be parsed!
      bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Int a) {\nconstant V.Woof.Foo nvar = 3u;\nInt(4) arr[nvar];\nBool b;\nInt test() {\nP(1) pel;\n pel.arr[2] = 2;\n return pel.arr[2];\n}\n}\n");

      // unpack array doesn't work!! define SKIPEVAL in TestCase_EndToEndCompiler to skip eval step.
      // o.w. slotsNeeded assumes 1.
      //test: CallStack.cpp:82: MFM::UlamValue MFM::CallStack::loadUlamValueFromSlot(MFM::s32): Assertion `(m_currentFrame + slot < m_frames.size()) && (m_currentFrame + slot >= 0)' failed.
      //bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P {\nBool b;\nInt test() {\nconstant V.Woof.Foo nvar = 3u;\nInt arr[nvar];\n arr[2] = 2;\n return arr[2];\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q(Int s) {\ntypedef Unsigned(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("P.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3356_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)

} //end MFM

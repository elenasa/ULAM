#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3338_test_compiler_elementandquarkwargs_caarray_isparse)
  {
    std::string GetAnswerKey()
    {
      //Constants have explicit types; a cast changed
      return std::string("Exit status: -2\nUe_EventWindowTest { constant Int(32) v = -2;  EventWindow(-2) ew( constant Int(32) w = -2; );  Int(32) test() {  Atom(96) a;  EventWindowTest ewt;  ew 0 [] ewt cast = a ew 0 [] = a EventWindowTest is cond -2 return if 1 return } }\nUq_EventWindow { constant Int(32) w = NONREADYCONST;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3248
      // added named constant v; tests -2 (also used as "NONREADYCONST") as arg value.
      // 'a has System' doesn't appy to eval because atoms have no class declarations; but testable for gencode
      bool rtn1 = fms->add("EventWindowTest.ulam", "ulam 1;\nuse EventWindow;\nelement EventWindowTest {\nconstant Int v = -2;\n EventWindow(v) ew;\n Int test() {\n    Atom a;\nEventWindowTest ewt;\n ew[0] = ewt;\n   a = ew[0];  // This is me!\n    if (a is EventWindowTest)\n      return v;\n    return 1;\n  }\n}\n");

      bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow(Int w) {\n  Atom aref(Int index) native;\n  Void aset(Int index, Atom val) native;\n}\n");

      if(rtn1 && rtn2)
	return std::string("EventWindowTest.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3338_test_compiler_elementandquarkwargs_caarray_isparse)

} //end MFM

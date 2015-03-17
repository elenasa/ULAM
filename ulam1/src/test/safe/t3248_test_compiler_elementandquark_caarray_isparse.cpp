#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3248_test_compiler_elementandquark_caarray_isparse)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_EventWindowTest { EventWindow ew();  Int(32) test() {  Atom(96) a;  EventWindowTest ewt;  ew 0 [] ewt cast = a ew 0 [] = a EventWindowTest is cond 0 return if 1 return } }\nUq_EventWindow { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // 'a has System' doesn't appy to eval because atoms have no class declarations; but testable for gencode
      bool rtn1 = fms->add("EventWindowTest.ulam", "ulam 1;\nuse EventWindow;\nelement EventWindowTest {\nEventWindow ew;\n Int test() {\n    Atom a;\nEventWindowTest ewt;\n ew[0] = ewt;\n   a = ew[0];  // This is me!\n    if (a is EventWindowTest)\n      return 0;\n    return 1;\n  }\n}\n");

      bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow {\n  Atom aref(Int index) native;\n  Void aset(Int index, Atom val) native;\n}\n");

      // playing with errors..
      //bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow {\n  Atom aref(Int index) native;\n  Void aset(Int index, Int val) native;\n}\n");
      //bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow {\n  Atom aref(Int index) native;\n  Void aSet(Int index, Atom val) native;\n}\n");

      // test system quark with native overloaded print funcs; assert
      //bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2)
	return std::string("EventWindowTest.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3248_test_compiler_elementandquark_caarray_isparse)

} //end MFM

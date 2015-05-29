#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3413_test_compiler_elementandquark_caarray_funcreturn_error)
  {
    std::string GetAnswerKey()
    {
      //./EventWindowTest.ulam:14:9: ERROR: Lefthand side of equals is 'Not StoreIntoAble': <func>, type: Atom(96).
      return std::string("Exit status: 1\nUe_EventWindowTest { EventWindow ew();  Int(32) test() {  Atom(96) a;  a ( )func = 1 return } }\nUq_EventWindow { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // 'a has System' doesn't appy to eval because atoms have no class declarations; but testable for gencode
      //bool rtn1 = fms->add("EventWindowTest.ulam", "ulam 1;\nuse EventWindow;\nelement EventWindowTest {\nEventWindow ew;\n Atom func() {\n return ew[0];\n }\n Int test() {\n Atom a;\n EventWindowTest ewt;\n  ew[0] = ewt;\n  a = ew[0];  // This is me!\n    if (a is EventWindowTest)\n      return 0;\n    return 1;\n  }\n}\n");

      // here we try to use func() whereever ew[0] was used since that's its return value..but lhs is not storeintoable!
      bool rtn1 = fms->add("EventWindowTest.ulam", "ulam 1;\nuse EventWindow;\nelement EventWindowTest {\nEventWindow ew;\n /*EventWindow func() {\n return ew;\n }\n */ Atom func() {\n return ew[0];\n }\n Int test() {\n Atom a;\n EventWindowTest ewt;\n func() = ewt;\n /*ew[0] = ewt;\n*/   a = func(); /*a = ew[0]; */ // This is me!\n    if (a is EventWindowTest)\n      return 0;\n    return 1;\n  }\n}\n");

      //can't use a func to return ew[0] even on the rhs? OK now.
      //bool rtn1 = fms->add("EventWindowTest.ulam", "ulam 1;\nuse EventWindow;\nelement EventWindowTest {\nEventWindow ew;\n  Atom func() {\n return ew[0];\n }\n Int test() {\n Atom a;\n a = func();\n return 1;\n  }\n}\n");

      bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow {\n  Atom aref(Int index) native;\n  Void aset(Int index, Atom val) native;\n}\n");

      if(rtn1 && rtn2)
	return std::string("EventWindowTest.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3413_test_compiler_elementandquark_caarray_funcreturn_error)

} //end MFM

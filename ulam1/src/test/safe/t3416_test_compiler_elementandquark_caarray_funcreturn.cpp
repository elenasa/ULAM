#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3416_test_compiler_elementandquark_caarray_funcreturn)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_EventWindowTest { EventWindow ew();  Int(32) test() {  Atom(96) a;  a ( )func = 1 return } }\nUq_EventWindow { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //can't use a func to return ew[0] even on the rhs? OK now.
      // see t3413 for error as lhs.
      bool rtn1 = fms->add("EventWindowTest.ulam", "ulam 1;\nuse EventWindow;\nelement EventWindowTest {\nEventWindow ew;\n  Atom func() {\n return ew[0];\n }\n Int test() {\n Atom a;\n a = func();\n return 1;\n  }\n}\n");

      bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow {\n  Atom aref(Int index) native;\n  Void aset(Int index, Atom val) native;\n}\n");

      if(rtn1 && rtn2)
	return std::string("EventWindowTest.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3416_test_compiler_elementandquark_caarray_funcreturn)

} //end MFM

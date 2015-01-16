#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3248_test_compiler_quark_caarray_isparse)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_EventWindow { <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow {\n  Atom aref(Int index) native;\n  Void aset(Int index, Atom val) native;\n}\n");

      // playing with errors..
      //bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow {\n  Atom aref(Int index) native;\n  Void aset(Int index, Int val) native;\n}\n");
      //bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow {\n  Atom aref(Int index) native;\n  Void aSet(Int index, Atom val) native;\n}\n");

      if(rtn2)
	return std::string("EventWindow.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3248_test_compiler_quark_caarray_isparse)

} //end MFM

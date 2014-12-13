#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3248_test_compiler_quark_caarray_isparse)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_EventWindowTest { EventWindow ew();  Int(32) test() {  Atom(96) a;  EventWindowTest ewt;  ew 0 [] ewt cast = a ew 0 [] = a EventWindowTest is cast cond 0 cast return if 1 cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow {\n  Atom aRef(Int index) native;\n  Void aSet(Int index, Atom val) native;\n  Int size() native;\n  Void diffuse() native;\n}\n");

      if(rtn2)
	return std::string("EventWindow.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3248_test_compiler_quark_caarray_isparse)

} //end MFM

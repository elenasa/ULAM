#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3233_test_compiler_quarkarray_ew)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_EventWindow { <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      /*
     quark EventWindow {
	Atom aref(Int index) native;
	Void aset(Int index, Atom val) native;
	Int test() {
	  EventWindow ew;
	  Atom a;
	  a = ew[0];
	  return 0;
	}
      }
*/

      bool rtn2 = fms->add("EventWindow.ulam"," ulam 1;\n quark EventWindow {\nAtom aref(Int index) native;\nVoid aset(Int index, Atom v) native;\n}\n");

      //EventWindow.ulam:3:1: ERROR: Only elements may have element parameters: <EventWindow> is a quark.
      //bool rtn2 = fms->add("EventWindow.ulam"," ulam 1;\n quark EventWindow {\nelement Int nogood;\n Atom aref(Int index) native;\nVoid aset(Int index, Atom v) native;\nInt test() {\nEventWindow ew;\nAtom a;\na=ew[0];\nreturn 0;\n}\n }\n");

      if(rtn2)
	return std::string("EventWindow.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3233_test_compiler_quarkarray_ew)

} //end MFM



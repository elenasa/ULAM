#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3233_test_compiler_elementandquarkarray_ew)
  {
    std::string GetAnswerKey()
    {
      //Exit status: 0\nUe_EWE { Int(32) test() {  EventWindow ew;  Atom(96) a;  a ew 0 [] = 0 cast return } }\nUq_EventWindow { <NOMAIN> }
      return std::string("Exit status: 0\nUe_EWE { Int(32) test() {  EventWindow ew;  Atom(96) a;  a ew 0 [] = 0 return } }\nUq_EventWindow { <NOMAIN> }\n");
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
      //element to test EventWindow quark
      bool rtn1 = fms->add("EWE.ulam", "ulam 1;\nuse EventWindow;\nelement EWE{\nInt test() {\nEventWindow ew;\nAtom a;\na=ew[0];\nreturn 0;\n}\n}\n");

      bool rtn2 = fms->add("EventWindow.ulam"," ulam 1;\n quark EventWindow{\nAtom aref(Int index) native;\nVoid aset(Int index, Atom v) native;\n }\n");

      //EventWindow.ulam:3:1: ERROR: Only elements may have element parameters: <EventWindow> is a quark.
      //bool rtn2 = fms->add("EventWindow.ulam"," ulam 1;\n quark EventWindow {\nelement Int nogood;\n Atom aref(Int index) native;\nVoid aset(Int index, Atom v) native;\nInt test() {\nEventWindow ew;\nAtom a;\na=ew[0];\nreturn 0;\n}\n }\n");

      if(rtn1 && rtn2)
	return std::string("EWE.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3233_test_compiler_elementandquarkarray_ew)

} //end MFM

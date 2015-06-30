#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3484_test_compiler_elementandquark_caarray_ambiguousfunc_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_EventWindowTest { typedef Unsigned(6) SiteNum;  EventWindow ew( typedef Unsigned(6) SiteNum; );  Int(32) test() {  Unsigned(6) s;  s 6 cast = Atom(96) a;  a ew s [] = 1 return } }\nUq_EventWindow { typedef Unsigned(6) SiteNum;  <NOMAIN> }\nUq_C2D { Int(6) m_width(0);  Int(6) m_height(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3413, and issue/afterTypeSelect/combination.
      bool rtn1 = fms->add("EventWindowTest.ulam", "ulam 1;\nelement EventWindowTest {\ntypedef EventWindow.SiteNum SiteNum;\n EventWindow ew;\n Int test() {\n SiteNum s = 6;\n Atom a = ew[s];\n  return 1;\n  }\n}\nuse EventWindow;\n");

      bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nuse C2D;\n quark EventWindow {\ntypedef Unsigned(6) SiteNum;\n Atom aref(Int index) native;\n  Atom aref(SiteNum index){\n return aref((Int) index);\n}\n Atom aref(C2D coord){\nC2D c;\n return aref(c.getIndex(coord));}\n Void aset(Int index, Atom v) native;\n}\n");

      bool rtn3 = fms->add("C2D.ulam","quark C2D {\n Int(6) m_width;\n Int(6) m_height;\n  Void init(Int x, Int y) {\n m_width = (Int(6)) x;\n m_height = (Int(6)) y;\n return;\n }\n Void init() {\n m_width = 9;\n m_height = 4;\n return;\n /* event window overload */ }\n Int getIndex(Int a, Int b){\nreturn ((m_height-b) * m_width + (m_height-a));\n }\n Int getIndex(C2D coord){\nreturn getIndex(coord.m_width, coord.m_height);\n }\n }\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("EventWindowTest.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3484_test_compiler_elementandquark_caarray_ambiguousfunc_issue)

} //end MFM

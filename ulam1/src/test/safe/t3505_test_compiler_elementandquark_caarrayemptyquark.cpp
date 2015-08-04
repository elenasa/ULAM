#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3505_test_compiler_elementandquark_caarrayemptyquark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_Foo { typedef Unsigned(6) Sitenum;  Int(32) test() {  EventWindow ew;  ew 1 cast [] Empty is cast return } }\nUe_Empty { /* empty class block */ }Uq_EventWindow { typedef Unsigned(6) Sitenum;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // test nonInt custom array index subscript
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Empty;\nuse EventWindow;\n element Foo {\ntypedef EventWindow.Sitenum Sitenum;\n Int test(){\n EventWindow ew;\n return (Int) (ew[(Sitenum) 1] is Empty);\n}\n}\n");

      bool rtn2 = fms->add("Empty.ulam","ulam 1;\nelement Empty {\n /* empty element comment */ }\n");

      bool rtn3 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow {\ntypedef Unsigned(6) Sitenum;\n  Atom aref(Sitenum index){\n Atom a;\n return a;\n }\n  Void aset(Sitenum index, Atom val){\n return;\n}\n }\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3505_test_compiler_elementandquark_caarrayemptyquark)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3262_test_compiler_funccalltypedefarg_datamember_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_Ish { typedef Int(16) Coord;  Int(16) x(0);  Int(16) y(0);  Int(32) test() {  Ish f;  f ( 1 cast 3 cast )make = f y . cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // see t3256 for related test case
      /*
	ulam 1;
	element Ish {
	typedef Int(16) Coord;
	Coord x;
	Coord y;

	Ish make(Int x, Int y) {
	Ish ret;
	ret.x = x;
	ret.y = y;
	return ret;
	}
	Int test() {
	Ish f = make(1,3);
	return f.y;
	}
	}
      */

      bool rtn1 = fms->add("Ish.ulam","ulam 1;\nelement Ish {\ntypedef Int(16) Coord;\nCoord x;\nCoord y;\nIsh make(Int x, Int y) {\nIsh ret;\n ret.x = (Coord) x;\n ret.y = (Coord) y;\n return ret;\n }\n Int test() {\n Ish f = make(1,3);\n return f.y;\n }\n }\n");

      if(rtn1)
	return std::string("Ish.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3262_test_compiler_funccalltypedefarg_datamember_issue)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3189_test_compiler_elementandquark_memberselectedfunc_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:11:20: ERROR: Member selected must be a valid lefthand side, type: C2Dfunc requires a variable; may be a casted function call.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //member selection with a function call must be first saved to a
      //variable since we results are returned-by-value (see t3188)
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use C2D;\n element Foo {\n Int m_idx;\n C2D func(Int i, Int j) {\n C2D c;\n c.init(i,j);\n return c;\n }\n Int test() {\n m_idx = func(9,4).getIndex(0,0);\n  return m_idx;\n }\n }\n");

      bool rtn2 = fms->add("C2D.ulam","quark C2D {\n typedef Int(4) IF;\n IF m_width, m_height;\n  Void init(Int x, Int y) {\n m_width = (IF) x;\n m_height = (IF) y;\n return;\n }\n Void init() {\n m_width = IF.maxof;\n m_height = 4;\n return;\n /* event window overload */ }\n Int getIndex(Int a, Int b){\n return ((m_height-b) * m_width + (m_height-a));\n }\n }\n");


      if(rtn1 & rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3189_test_compiler_elementandquark_memberselectedfunc_error)

} //end MFM

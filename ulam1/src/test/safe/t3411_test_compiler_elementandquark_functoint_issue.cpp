#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3411_test_compiler_elementandquark_functoint_issue)
  {
    std::string GetAnswerKey()
    {
      // width 9 saturates at 7, then sum 11 also saturates at 7 for Int(4) aritmetic.
      // just got implemented!
      //Foo.ulam:1:121: (NodeFunctionCall.cpp:evalToStoreInto:269) ERROR: Use of function calls as lefthand values is not currently supported. Save the results of <func> to a variable, type: C2D.
      return std::string("Exit status: 7\nUe_Foo { Q q( C2D c( typedef Int(4) I;  Int(4) m_width(7);  Int(4) m_height(4); ); );  Int(32) test() {  q ( )toInt . cast cast return } }\nUq_Q { C2D c( typedef Int(4) I;  Int(4) m_width(7);  Int(4) m_height(4); );  <NOMAIN> }\nUq_C2D { typedef Int(4) I;  Int(4) m_width(7);  Int(4) m_height(4);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //member selection with a function call must be first saved to a
      //variable since we results are returned-by-value (see t3188)
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Q;\n element Foo {\n Q q;\n Int test() { return (Int) q;\n }\n }\n");

      bool rtn3 = fms->add("Q.ulam","ulam 1;\n use C2D;\n quark Q {\n C2D c;\n C2D func(){\n c.init();\n return c;\n }\n Int toInt(){\n return (Int) func();\n }\n }\n");
      //bool rtn3 = fms->add("Q.ulam","ulam 1;\n use C2D;\n quark Q {\n C2D c;\n C2D func(){\n c.init();\n return c;\n }\n Int toInt(){\n C2D r = func();\n return r;\n }\n }\n");

      // 9 doesn't fit in Int(4); made typedef for Int(4); cast constant. Int(5) changes the results.
      //./C2D.ulam:10:10: ERROR: Attempting to fit a constant <9> into a smaller bit size type, LHS: Int(4), for binary operator= .
      bool rtn2 = fms->add("C2D.ulam","quark C2D {\n typedef Int(4) I;\n I m_width;\n I m_height;\n  Void init(Int x, Int y) {\n m_width = (I) x;\n m_height = (I) y;\n return;\n }\n Void init() {\n m_width = (I) 9;\n m_height = 4;\n return; /* event window overload */\n }\n Int getIndex(Int a, Int b){\n return ((m_height-b) * m_width + (m_height-a));\n }\n Int toInt(){\n return (m_width + m_height);\n }\n }\n");


      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3411_test_compiler_elementandquark_functoint_issue)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3189_test_compiler_elementandquark_memberselectedfunc_error)
  {
    std::string GetAnswerKey()
    {
      //Foo.ulam:1:121: (NodeFunctionCall.cpp:evalToStoreInto:269) ERROR: Use of function calls as lefthand values is not currently supported. Save the results of <func> to a variable, type: C2D.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //member selection with a function call must be first saved to a
      //variable since we results are returned-by-value (see t3188)
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use C2D; element Foo { Int m_idx; C2D func(Int i, Int j) { C2D c; c.init(i,j); return c; } Int test() { m_idx = func(9,4).getIndex(0,0);  return m_idx; } }\n");

      bool rtn2 = fms->add("C2D.ulam","quark C2D { Int(4) m_width; Int(4) m_height;  Void init(Int x, Int y) { m_width = x; m_height = y; return; } Void init() { m_width = 9; m_height = 4; return; /* event window overload */ } Int getIndex(Int a, Int b){return ((m_height-b) * m_width + (m_height-a)); } }\n");


      if(rtn1 & rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3189_test_compiler_elementandquark_memberselectedfunc_error)

} //end MFM

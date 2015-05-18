#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3191_test_compiler_elementandquark_array2dplusone_quarkcasttoint)
  {
    std::string GetAnswerKey()
    {
      /*
	./Foo.ulam:1:163: Warning: Attempting to fit a constant <1> into a smaller bit size type, LHS: C2D(UNKNOWN)<11>, for binary operator+ .
	./Foo.ulam:1:32: ERROR: Data member <m_array> of type: Int(1)[71] (UTI15) total size: 71 MUST fit into 32 bits; Local variables do not have this restriction.
	Unrecoverable Program Type Label FAILURE.
      */
      return std::string("Ue_Foo { Int(1) m_array[71](0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);  Int(32) test() {  C2D c;  c ( 0 cast 0 cast )func = m_array c ( )toInt . cast 1 cast +b [] true cast = m_array c ( )toInt . cast [] cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //can we add [40 + 1] ?
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use C2D; element Foo { Int(1) m_array[71]; C2D func(Int i, Int j) { C2D c; c.init(); c.set(i,j); return c; } Int test() { C2D c; c = func(0,0); m_array[c + 1] = true; return m_array[c]; } }\n");

      bool rtn2 = fms->add("C2D.ulam","quark C2D { Int(8) m_width, m_height, m_x, m_y;  Void init(Int x, Int y) { m_width = x; m_height = y;} Void init() { m_width = 9; m_height = 4; /* event window overload */ } Void set(Int a, Int b) { m_x = a; m_y = b; } Int toInt(){return ((m_height-m_y) * m_width + (m_height-m_x)); } }\n");


      if(rtn1 & rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3191_test_compiler_elementandquark_array2dplusone_quarkcasttoint)

} //end MFM

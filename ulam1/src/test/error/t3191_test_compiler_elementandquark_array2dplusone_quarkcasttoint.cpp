#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3191_test_compiler_elementandquark_array2dplusone_quarkcasttoint)
  {
    std::string GetAnswerKey()
    {
      /*
	./Foo.ulam:4:2: ERROR: Data member <m_array> of type: Unsigned(1)[71], total size: 71 MUST fit into 64 bits; Local variables do not have this restriction.
	Unrecoverable Program Type Label FAILURE.
      */
      return std::string("Ue_Foo { Int(1) m_array[71](0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);  Int(32) test() {  C2D c;  c ( 0 cast 0 cast )func = m_array c ( )toInt . cast 1 cast +b [] true cast = m_array c ( )toInt . cast [] cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //can we add [40 + 1] ?
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use C2D;\n element Foo {\n Unsigned(1) m_array[71];\n C2D func(Int i, Int j) {\n C2D c;\n c.init();\n c.set(i,j);\n return c;\n }\n Int test() {\n C2D c;\n c = func(0,0);\n m_array[(Unsigned) c + 1] = 1;\n return (Int) m_array[c];\n }\n }\n");

      bool rtn2 = fms->add("C2D.ulam","quark C2D { typedef Int(8) IE;\n IE m_width, m_height, m_x, m_y;\n  Void init(Int x, Int y) {\n m_width = (IE) x;\n m_height = (IE) y;\n}\n Void init() {\n m_width = 9;\n m_height = 4;\n /* event window overload */ }\n Void set(Int a, Int b) {\n m_x = (IE) a;\n m_y = (IE) b;\n }\n Int toInt(){\nreturn ((m_height-m_y) * m_width + (m_height-m_x));\n }\n }\n");


      if(rtn1 & rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3191_test_compiler_elementandquark_array2dplusone_quarkcasttoint)

} //end MFM

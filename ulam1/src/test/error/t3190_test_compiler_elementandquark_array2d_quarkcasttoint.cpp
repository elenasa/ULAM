#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3190_test_compiler_elementandquark_array2d_quarkcasttoint)
  {
    std::string GetAnswerKey()
    {

      //Ue_Foo { Int(1) m_array[71](0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);  Int(32) test() {  C2D c;  c ( 0 cast 0 cast )func = m_array c ( )toInt . [] true cast = m_array c ( )toInt . [] cast return } }\nExit status: 1

      return std::string("Ue_Foo { Int(1) m_array[71](0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);  Int(32) test() {  C2D c;  c ( 0 cast 0 cast )func = m_array c ( )toInt . cast [] true cast = m_array c ( )toInt . cast [] cast return } }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // data members must fit within 32-bits (BitField limit)
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use C2D;\n element Foo {\n Int(1) m_array[71];\n C2D func(Int i, Int j) {\n C2D c;\n c.init();\n c.set(i,j);\n return c;\n }\n Int test() {\n C2D c;\n c = func(0,0);\n m_array[c] = true;\n return m_array[c];\n }\n }\n");

      // alternative: use as a local data member instead
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use C2D;\n element Foo {\nC2D func(Int i, Int j) {\n C2D c;\n c.init();\n c.set(i,j);\n return c;\n }\n Int test() {\n Int(1) m_array[71];\n C2D c;\n c = func(0,0);\n m_array[c] = true;\n return m_array[c];\n }\n }\n");

      bool rtn2 = fms->add("C2D.ulam","quark C2D {\n Int(8) m_width, m_height, m_x, m_y;\n  Void init(Int x, Int y) {\n m_width = x;\n m_height = y;\n}\n Void init() {\n m_width = 9;\n m_height = 4;\n /* event window overload */ }\n Void set(Int a, Int b) {\n m_x = a;\n m_y = b;\n }\n Int toInt(){\n return ((m_height-m_y) * m_width + (m_height-m_x));\n }\n }\n");


      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3190_test_compiler_elementandquark_array2d_quarkcasttoint)

} //end MFM



#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3190_test_compiler_elementandquark_array2d_quarkcasttoint)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(1) m_array[71](0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);  Int(32) test() {  C2D c;  c ( 0 cast 0 cast )func = m_array c ( )toInt . [] true cast = m_array c ( )toInt . [] cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use C2D; element Foo { Int(1) m_array[71]; C2D func(Int i, Int j) { C2D c; c.init(); c.set(i,j); return c; } Int test() { C2D c; c = func(0,0); m_array[c] = true; return m_array[c]; } }\n"); 

      bool rtn2 = fms->add("C2D.ulam","quark C2D { Int(8) m_width, m_height, m_x, m_y;  Void init(Int x, Int y) { m_width = x; m_height = y;} Void init() { m_width = 9; m_height = 4; /* event window overload */ } Void set(Int a, Int b) { m_x = a; m_y = b; } Int toInt(){return ((m_height-m_y) * m_width + (m_height-m_x)); } }\n");


      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3190_test_compiler_elementandquark_array2d_quarkcasttoint)
  
} //end MFM



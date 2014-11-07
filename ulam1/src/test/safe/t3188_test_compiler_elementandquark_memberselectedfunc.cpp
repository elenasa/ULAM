#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3188_test_compiler_elementandquark_memberselectedfunc)
  {
    std::string GetAnswerKey()
    {
      //note: 148 decimal == 0x94 hex

      //Ue_Foo { C2D m_coord( Int(4) m_width(9);  Int(4) m_height(4); );  Int(32) m_idx(40);  Bool(1) m_b(false);  Int(32) test() {  m_coord ( 9 cast 4 cast )func = m_idx m_coord ( 0 cast 0 cast )getIndex . = m_idx return } }\nExit status: 40

      return std::string("Ue_Foo { System s();  Bool(7) m_b(false);  Int(32) m_idx(40);  C2D m_coord( Int(6) m_width(9);  Int(6) m_height(4); );  Int(32) test() {  m_coord ( 9 cast 4 cast )func = m_idx m_coord ( 0 cast 0 cast )getIndex . = s ( m_idx )print . m_idx return } }\nExit status: 40");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; use C2D; element Foo { C2D m_coord; Int m_idx; Bool m_b; C2D func(Int i, Int j) { C2D c; c.init(i,j); return c; } Int test() { m_coord = func(9,4); m_idx = m_coord.getIndex(0,0);  return m_idx; } }\n"); 

      //bool rtn2 = fms->add("C2D.ulam","quark C2D { Int(4) m_width; Int(4) m_height;  Void init(Int x, Int y) { m_width = x; m_height = y; return; } Void init() { m_width = 9; m_height = 4; return; /* event window overload */ } Int getIndex(Int a, Int b){return ((m_height-b) * m_width + (m_height-a)); } }\n");

      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n use C2D;\n element Foo {\nSystem s;\n Bool(7) m_b;\nInt m_idx;\n C2D m_coord;\n C2D func(Int i, Int j) {\n C2D c;\n c.init(i,j);\n return c;\n }\n Int test() {\n m_coord = func(9,4);\n m_idx = m_coord.getIndex(0,0);\ns.print(m_idx);\n  return m_idx;\n }\n }\n");

      // increase bitsize of width and height to accomodate 9.
      bool rtn2 = fms->add("C2D.ulam","quark C2D {\n Int(6) m_width;\n Int(6) m_height;\n  Void init(Int x, Int y) {\n m_width = x;\n m_height = y;\n return;\n }\n Void init() {\n m_width = 9;\n m_height = 4;\n return;\n /* event window overload */ }\n Int getIndex(Int a, Int b){\nreturn ((m_height-b) * m_width + (m_height-a));\n }\n }\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3188_test_compiler_elementandquark_memberselectedfunc)
  
} //end MFM



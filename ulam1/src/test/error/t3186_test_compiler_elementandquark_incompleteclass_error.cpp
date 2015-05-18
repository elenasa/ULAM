#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3186_test_compiler_elementandquark_incompleteclass_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./Foo.ulam:1:18: ERROR: Only elements may have element parameters: <C2D> is a quark.
	./Foo.ulam:1:30: ERROR: Name of variable/function <{>: Identifier must begin with a lower-case letter.
	./C2D.ulam:1:1: Warning: Empty/Incomplete Class Definition: <C2D>; possible missing ending curly brace.
	./Foo.ulam:1:30: ERROR: Invalid Class Type: <{>; KEYWORD should be: 'element', 'quark', or 'union'.
	Unrecoverable Program Parse FAILURE.
      */
      return std::string("Ue_Foo { C2D m_coord(148);  Int(32) m_idx(40);  Bool(1) m_b(false);  Int(32) test() {  m_coord ( )init . m_idx m_coord ( 0 cast 0 cast )getIndex . = m_idx return } }\nExit status: 40");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use C2D; element Foo { C2D m_coord; Int m_idx; Bool m_b; Int test() { m_coord.init(); m_idx = m_coord.getIndex(0,0);  return m_idx; } }\n");

      bool rtn2 = fms->add("C2D.ulam","quark C2D { Int(4) m_width; Int(4) m_height;  Void init(Int x, Int y) { m_width = x; m_height = y; return; } Void init() { m_width = 9; m_height = 4; return; /* event window overload */ } Int getIndex(Int a, Int b){return ((m_height-b) * m_width + (m_height-a)); } \n");  //missing closing brace


      if(rtn1 & rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3186_test_compiler_elementandquark_incompleteclass_error)

} //end MFM

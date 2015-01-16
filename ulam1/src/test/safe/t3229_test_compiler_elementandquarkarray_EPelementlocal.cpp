#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3229_test_compiler_elementandquarkarray_EPelementlocal)
  {
    std::string GetAnswerKey()
    {

      return std::string("Exit status: 0\nUe_Foo { Bool(1) b[3](false,false,false);  System s();  Int(4) m_i(0);  Int(32) test() {  Foo f;  Bar boo;  boo f spoon mbar 0 [] . . = f spoon mbar 1 [] . . boo = f spoon sbar ( true cast )reset . . . 0 cast return } }\nUq_System { <NOMAIN> }\nUq_Bar { Bool(1) val_b[3](false,false,false);  <NOMAIN> }\nUe_Poo { Bool(3) b(false);  typedef Bar Pop[2];  System s();  Int(4) m_i(0);  Bar mbar[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,false,false); );  Bar sbar( Bool(1) val_b[3](false,false,false); );  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n use Bar;\nuse Poo;\n element Foo {\nSystem s;\nBool b[3];\n Int(4) m_i;\nelement Poo spoon;\nInt test() {\nFoo f;\nBar boo;\n boo = f.spoon.mbar[0];\nf.spoon.mbar[1] = boo;\nf.spoon.sbar.reset(true);\nreturn 0;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n Atom aref(Int index) native;\n Void aset(Int index, Atom v) native;\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      bool rtn4 = fms->add("Poo.ulam","ulam 1;\nuse Bar;\nelement Poo{\ntypedef Bar Pop[2];\nSystem s;\nBool(3) b;\n Int(4) m_i;\nPop mbar;\nBar sbar;\n }\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3229_test_compiler_elementandquarkarray_EPelementlocal)

} //end MFM



#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3227_test_compiler_elementandquarkarray_EPelement)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Foo { Bool(3) b(false);  Int(4) m_i(0);  typedef Bar Pop[2];  System s();  Bar m_bar2[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Atom(96) a;  a spoon sbar 0 [] . = spoon sbar 1 [] . a = 0 return } }\nUq_System { <NOMAIN> }\nUq_Bar { Bool(1) val_b[3](false,false,false);  <NOMAIN> }\nUe_Poo { Bool(3) b(false);  Int(4) m_i(0);  Bar sbar( Bool(1) val_b[3](false,false,false); );  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n use Bar;\nuse Poo;\n element Foo {\n typedef Bar Pop[2];\nSystem s;\nBool(3) b;\n Int(4) m_i;\n Pop m_bar2;\nelement Poo spoon;\nInt test() {\nAtom a;\n a = spoon.sbar[0];\nspoon.sbar[1] = a;\nreturn 0;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n Atom aref(Int index) native;\n Void aset(Int index, Atom v) native;\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      bool rtn4 = fms->add("Poo.ulam","ulam 1;\nuse Bar;\nelement Poo {\nBool(3) b;\n Int(4) m_i;\nBar sbar;\n}\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3227_test_compiler_elementandquarkarray_EPelement)

} //end MFM

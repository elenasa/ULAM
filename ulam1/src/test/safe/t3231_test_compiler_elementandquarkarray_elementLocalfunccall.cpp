#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3231_test_compiler_elementandquarkarray_elementLocalfunccall)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Bool(1) b[3](false,false,false);  System s();  Int(4) m_i(0);  Int(32) test() {  Poo spoon;  Bar boo;  boo spoon mbar 0 [] . = spoon mbar 1 [] . boo = Atom(96) a;  a spoon sbar 0 [] . = spoon sbar 1 [] . a = spoon sbar ( true cast )reset . . Foo f;  f ( 0 cast )check . cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n use Bar;\nuse Poo;\n element Foo {\nSystem s;\nBool b[3];\n Int(4) m_i;\nBool check(Int idx){\n return b[idx];\n}\n Int test() {\nPoo spoon;\nBar boo;\n boo = spoon.mbar[0];\nspoon.mbar[1] = boo;\nAtom a;\na = spoon.sbar[0];\nspoon.sbar[1] = a;\nspoon.sbar.reset(true);\n Foo f;\n return f.check(0);\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n Atom aref(Int index) native;\n Void aset(Int index, Atom v) native;\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      bool rtn4 = fms->add("Poo.ulam","ulam 1;\nuse Bar;\nelement Poo{\ntypedef Bar Pop[2];\nSystem s;\nBool(3) b;\n Int(4) m_i;\nPop mbar;\nBar sbar;\n }\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3231_test_compiler_elementandquarkarray_elementLocalfunccall)

} //end MFM



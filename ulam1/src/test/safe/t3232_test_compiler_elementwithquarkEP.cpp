#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3232_test_compiler_elementwithquarkEP)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Poo { Bool(3) b(false);  typedef Bar Pop[2];  Int(4) m_i(0);  Bar mbar[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,false,false); );  Bar tbar( Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Poo p;  p sbar ( true cast )reset . . sbar ( true cast )reset . p tbar ( false cast )reset . . tbar ( false cast )reset . 0 cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n Atom aref(Int index) native;\n Void aset(Int index, Atom v) native;\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      bool rtn4 = fms->add("Poo.ulam","ulam 1;\nuse Bar;\nelement Poo{\ntypedef Bar Pop[2];\nBool(3) b;\n Int(4) m_i;\nPop mbar;\nelement Bar sbar;\nBar tbar;\nInt test(){\nPoo p;\np.sbar.reset(true);\nsbar.reset(true);\np.tbar.reset(false);\ntbar.reset(false);\n return 0;\n}\n }\n");

      if(rtn2 && rtn3 && rtn4)
	return std::string("Poo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3232_test_compiler_elementwithquarkEP)

} //end MFM



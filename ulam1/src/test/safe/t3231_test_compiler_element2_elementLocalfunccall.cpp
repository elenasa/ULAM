#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3231_test_compiler_element2_elementLocalfunccall)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Poo { Bool(3) b(false);  typedef Bar Pop[2];  Int(4) m_i(0);  Bar mbar[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,false,false); );  Bar sbar( Bool(1) val_b[3](false,false,false); );  <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n Atom aref(Int index) native;\n Void aset(Int index, Atom v) native;\n }\n");

      bool rtn4 = fms->add("Poo.ulam","ulam 1;\nuse Bar;\nelement Poo{\ntypedef Bar Pop[2];\nBool(3) b;\n Int(4) m_i;\nPop mbar;\nBar sbar;\n }\n");

      if(rtn2 && rtn4)
	return std::string("Poo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3231_test_compiler_element2_elementLocalfunccall)

} //end MFM



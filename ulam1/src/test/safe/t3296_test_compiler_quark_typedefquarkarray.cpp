#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3296_test_compiler_quark_typedefquarkarray)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Bool(3) b(true);  typedef Bar Pop[2];  System s();  Int(4) m_i(0);  Bar m_bar2[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Foo f;  f m_i . 3 cast = b f ( 1 cast )check . cast = s ( f m_i . )print . s ( m_i )print . s ( b )print . m_i cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      if(rtn2)
	return std::string("Bar.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3296_test_compiler_quark_typedefquarkarray)

} //end MFM



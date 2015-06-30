#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3296_test_compiler_elementandquark_typedefquarkarray)
  {
    std::string GetAnswerKey()
    {
      //node terminal proxy
      return std::string("Exit status: 0\nUe_Foo { typedef Bar Pop[2];  Bar m_bar2[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,false,false); );  Int(32) test() {  0 return } }\nUq_Bar { Bool(1) val_b[3](false,false,false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\n element Foo {\n typedef Bar Pop[2];\n Pop m_bar2;\n Int test() {\n return 0;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = false;\n }\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3296_test_compiler_elementandquark_typedefquarkarray)

} //end MFM

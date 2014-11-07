#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3144_test_compiler_quark)
  {
    std::string GetAnswerKey()
    {
      //Uq_Bar { Foo valoo( Int(32) val(0); );  <NOMAIN> }\nExit status: -1
      return std::string("Uq_Bar { Foo valoo( Int(7) val(0); );  <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // notice the filename must match the class name including capitalization. 
      //bool rtn2 = fms->add("Bar.ulam","ulam 1; quark Foo { Int val; } quark Bar { Foo valoo; }\n"); ok!
      bool rtn2 = fms->add("Bar.ulam","ulam 1;\n use foo;\n quark Bar {\n Foo valoo;\n }\n");
      bool rtn1 = fms->add("foo.ulam","ulam 1;\n quark Foo {\nInt(7) val;\n }\n");
      
      if(rtn2 && rtn1)
	return std::string("Bar.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3144_test_compiler_quark)
  
} //end MFM



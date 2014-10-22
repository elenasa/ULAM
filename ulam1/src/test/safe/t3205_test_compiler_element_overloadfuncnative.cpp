#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3205_test_compiler_element_overloadfuncnative)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Bool(3) b(false);  Int(4) i(0);  Int(4) j(15);  Int(32) test() {  i 0 cast = j ( i cast 1 cast -b )update = j cast return } }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {      
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nelement Foo {\nBool(3) b;\nInt(4) i, j;\nInt(4) update(Int x)\n{\nreturn x;\n}\nInt test(){\ni = 0;\nj = update(i - 1);\n\nreturn j;\n}\n}\n");

      // test explicit cast in arg
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nelement Foo {\nBool(3) b;\nInt(4) i, j;\nInt(4) update(Int(4) x)\n{\nreturn x;\n}\nInt test(){\ni = 0;\nj = update((Int(4)) (i - 1));\nprint((Int) j);\nprint(j);\nreturn j;\n}\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\n}\n");

      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3205_test_compiler_element_overloadfuncnative)
  
} //end MFM



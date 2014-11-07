#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3141_test_compiler_castarg)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { typedef Int(32) Foo[2];  Int(32) d[2](1,0);  Int(32) test() {  Int(32) a;  a 1 cast = d ( a cast )foo = d 0 [] return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n typedef Int Foo [2];\n Foo foo(Bool b) {\n Foo m;\n if(b)\n m[0]=1;\n else\n m[0]=2;\n return m;\n}\n Int test() {\n Int a;\n a = 1;\nd = foo((Bool) a);\n return d[0];\n /* match return type */}\nFoo d;\n }\n");  // want d[0] == 1.

      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3141_test_compiler_castarg)
  
} //end MFM



#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3140_test_compiler_funcdef_overload_plusequalarray_returntype)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { typedef Int(32) Foo[2];  Int(32) d[2](7,3);  Int(32) test() {  Bool(1) mybool;  mybool true cast = d ( mybool )foo = d ( 6 cast )foo += d 0 [] return } }\nExit status: 7");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n typedef Int Foo [2];\n Foo foo(Bool b) {\n Foo m;\n if(b)\n m[0]=1;\n else\n m[0]=2;\n return m;\n}\n Foo foo(Int i) {\n Foo e;\n e[1] = 3;\n e[0] = i;\n return e;\n }\n Int test() {\n Bool mybool;\n mybool= true;\nd = foo(mybool);\n d += foo(6);\n return d[0];\n /* match return type */}\nFoo d;\n }\n");  // want d[0] == 7. NOTE: requires 'operator+=' in c-code to add arrays

      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3140_test_compiler_funcdef_overload_plusequalarray_returntype)
  
} //end MFM



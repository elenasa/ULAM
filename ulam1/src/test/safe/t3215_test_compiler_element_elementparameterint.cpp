#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3215_test_compiler_element_elementparameterint)
  {
    std::string GetAnswerKey()
    {
      // element parameter chance not stored (as static variable) for eval
      return std::string("Ue_Foo { System s();  Bool(7) sp(false);  Int(32) a(1);  Int(32) test() {  Foo f;  f chance . 1 cast = s ( chance )print . a f chance . = chance return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\nBool(7) sp;\nelement Int chance;\nInt a;\n Int test() {\n Foo f;\nf.chance = 1;\ns.print(chance);\na = f.chance;\nreturn chance;\n }\n }\n");
      
      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3215_test_compiler_element_elementparameterint)
  
} //end MFM



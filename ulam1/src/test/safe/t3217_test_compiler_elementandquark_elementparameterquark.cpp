#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3217_test_compiler_elementandquark_elementparameterquark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { System s();  Bool(7) sp(false);  Int(32) a(0);  Int(32) test() {  Foo f;  f chance . 1 cast = s ( chance )print . chance return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Bar;\nelement Foo {\nSystem s;\nBool(1) sp;\nBool(3) b,c;\n element Bar barchance;\nBool last;\nInt test() {\nbarchance.valb[1] = true;\nb = barchance.valb[1];\ns.print(b);\nreturn b;\n }\n }\n");
      
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool valb[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3217_test_compiler_elementandquark_elementparameterquark)
  
} //end MFM



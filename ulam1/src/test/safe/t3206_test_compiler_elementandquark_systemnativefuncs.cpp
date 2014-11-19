#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3206_test_compiler_elementandquark_systemnativefuncs)
  {
    std::string GetAnswerKey()
    {
      //Ue_Foo { System m_s();  Bool(3) b(false);  Int(4) i(0);  Int(4) j(15);  Int(32) test() {  i 0 cast = j ( i cast 1 cast -b cast )update = m_s ( j cast )print . m_s ( j )print . j cast return } }\nExit status: -1

      return std::string("Ue_Foo { Bool(3) b(false);  System m_s();  Int(4) i(0);  Int(4) j(15);  Int(32) test() {  i 0 cast = j ( i cast 1 cast -b cast )update = m_s ( j cast )print . m_s ( j )print . m_s ( b ! cast )assert . j cast return } }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {      
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nelement Foo {\nBool(3) b;\nInt(4) i, j;\nInt(4) update(Int x)\n{\nreturn x;\n}\nInt test(){\ni = 0;\nj = update(i - 1);\n\nreturn j;\n}\n}\n");

      // test system quark with native overloaded print funcs and assert; !! also works.
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nelement Foo {\nSystem m_s;\nBool(3) b;\nInt(4) i, j;\nInt(4) update(Int(4) x)\n{\nreturn x;\n}\nInt test(){\ni = 0;\nj = update((Int(4)) (i - 1));\nm_s.print((Int) j);\nm_s.print(j);\nm_s.assert((Bool) (!b));\nreturn j;\n}\n}\n");

      //bool rtn2 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid assert(Bool b) native;\n}\n");
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3206_test_compiler_elementandquark_systemnativefuncs)
  
} //end MFM



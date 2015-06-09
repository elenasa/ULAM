#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3220_test_compiler_element_elementparameterelement)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x0 (false)
	 Bool(3) Arg: 0x7 (true)
	 Bool(3) Arg: 0x0 (false)
	 Bool(3) Arg: 0x0 (false)
	 Bool(3) Arg: 0x7 (true)
	 Bool(3) Arg: 0x0 (false)
	 Int Arg: 1
      */
      return std::string("Exit status: 1\nUe_Foo { Bool(3) b(true);  System s();  Bool(1) sp(false);  Bool(3) c(false);  Bool(1) last(false);  Int(32) test() {  Poo p;  p ( sp )reset . poochance ( sp )reset . poochance valb 1 [] . true = b poochance valb 1 [] . cast = s ( b )print . b cast return } }\nUq_System { <NOMAIN> }\nUe_Poo { Bool(1) valb[3](false,true,true);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Poo;\nelement Foo {\nSystem s;\nBool(1) sp;\nBool(3) b,c;\n element Poo poochance;\nBool last;\n Int i;\n Int test() {\nPoo p;\n sp = true;\n p.reset(sp);\n c = p.valb[1];\n s.print(p.valb[0]);\n s.print(c);\n s.print(p.valb[2]);\n poochance.reset(sp);\n b = poochance.valb[1];\n s.print(poochance.valb[0]);\n s.print(b);\n s.print(poochance.valb[2]);\n i = c;\n s.print(i);\n return i;\n }\n }\n");

      bool rtn2 = fms->add("Poo.ulam"," ulam 1;\n element Poo {\n Bool(3) valb[3];\n  Void reset(Bool b) {\n valb[1] = b;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3220_test_compiler_element_elementparameterelement)

} //end MFM

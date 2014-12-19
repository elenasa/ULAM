#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3259_test_compiler_self_EP)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x7 (true)
	 Bool(3) Arg: 0x7 (true)
	 Bool(3) Arg: 0x0 (false)
      */
      //note: in eval, last case, uninitialized atom case is wrong!!
      return std::string("Ue_S { System s();  Bool(1) sp(false);  Bool(3) b1(true);  Bool(3) b2(true);  Int(32) test() {  Atom(96) a;  Atom(96) t;  a self cast = a S as cast cond { S a;  b2 a ( )func . cast = } if s ( b2 )print . b1 ( a )func cast = s ( b1 )print . b1 ( t )func cast = s ( b1 )print . b2 cast return } }\nExit status: 1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S{\nBool sp;\n Bool(3) b1, b2;\n element Bool(3) iep;\n Int test() {\nAtom a, t;\nself.iep = true;\n a = self;\n if(a as S) b2 = a.iep;\n return b2;\n }\n }\n");
      bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S{\nBool sp;\n Bool(3) b1, b2;\n element Bool(3) iep;\n Int test() {\n /*self.*/iep = false;\nreturn iep;\n }\n }\n");

      if(rtn1)
	return std::string("S.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3259_test_compiler_self_EP)

} //end MFM



#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3255_test_compiler_self)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x7 (true)
	 Bool(3) Arg: 0x7 (true)
	 Bool(3) Arg: 0x0 (false)
      */
      //note: in eval, last case, uninitialized atom case is wrong!!
      return std::string("Exit status: 1\nUe_S { System s();  Bool(1) sp(false);  Bool(3) b1(true);  Bool(3) b2(true);  Int(32) test() {  Atom(96) a;  Atom(96) t;  a self cast = a S as cond { S a;  b2 a ( )func . cast = } if s ( b2 )print . b1 ( a )func cast = s ( b1 )print . b1 ( t )func cast = s ( b1 )print . b2 cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // simple case:
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S{\nBool sp;\n Bool(3) b1, b2;\n Int test() {\nAtom a;\n a = self;\n self = a;\n return 0;\n }\n }\n");

      //over loading works here..
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S{\nBool sp;\n Bool(3) b1, b2;\n Bool func() {\n return (self is S);\n}\n  Bool func(Atom d) {\n return (d is S);\n}\n Int test() {\nAtom a;\n  b1 = func();\n  b2 = func(a);\n  return b2;\n }\n }\n");

      // works
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S{\nBool sp;\n Bool(3) b1, b2;\n Bool func() {\n return (self is S);\n}\n  Bool func(Atom d) {\n return (d is S);\n}\n Int test() {\nAtom a;\n b2 = self.func();\n b1 = self.func(a);\n return b2;\n }\n }\n");

      // here, only the first definition takes..then the m_funcSymbol for the second one is NULL in eval..
      // fixed bug in NodeMemberSelect (no ERR MSG).
      bool rtn1 = fms->add("S.ulam"," ulam 1;\nuse System;\n element S{\nSystem s;\nBool sp;\n Bool(3) b1, b2;\n Bool func() {\n return (self is S);\n}\n  Bool func(Atom d) {\n return (d is S);\n}\n Int test() {\nAtom a, t;\n a = self;\n if(a as S) b2 = a.func();\n s.print(b2);\n b1 = func(a);\n s.print(b1);\n b1 = func(t);\n s.print(b1);\n return (Int) b2;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("S.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3255_test_compiler_self)

} //end MFM

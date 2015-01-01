#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3249_test_compiler_elementandquark_conditionalas)
  {
    std::string GetAnswerKey()
    {
      //Foo.ulam:14:7: fyi: Invalid type for LHS of conditional operator 'as', <Atom.96.-1>; Passing through as UNFOUND for eval.

      // no cast before cond
      //Ue_Foo { Bool(1) sp(false);  Bool(3) bi(false);  Bool(3) bh(false);  Counter4 m_c4( Int(32) d(0); );  Int(32) e(0);  Int(32) test() {  Atom(96) a;  Foo f;  Bool(1) b;  a Foo as cast cond { Foo a;  a bi . true cast = } if a f cast = a Counter4 as cast cond { Counter4 a;  bh true cast = a ( )incr . e a ( )get . = } if { bh false cast = } else e return } }\nExit status: 0
      return std::string("Ue_Foo { Bool(1) sp(false);  Bool(3) bi(false);  Bool(3) bh(false);  Counter4 m_c4( Int(32) d(0); );  Int(32) e(0);  Int(32) test() {  Atom(96) a;  Foo f;  Bool(1) b;  a Foo as cond { Foo a;  a bi . true cast = } if a f cast = a Counter4 as cond { Counter4 a;  bh true cast = a ( )incr . e a ( )get . = } if { bh false cast = } else e return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // 'a has System' doesn't appy to eval because atoms have no class declarations; but testable for gencode
      //bool rtn1 = fms->add("Foo.ulam","use System;\nuse Counter4;\nelement Foo {\nSystem s;\nBool sp;\n Bool(3) bi, bh;\n Counter4 m_c4;\n Int e;\n Int test(){\n Atom a;\n Foo f;\n Bool b;\n a = f; //easy\n if(a as Counter4){\n bi = true;\n a.incr();\n e=a.get();\n }\n else {\n bh = true;\n}\n s.print(e);\n return e;\n }\n }\n");

      // ELSE
      bool rtn1 = fms->add("Foo.ulam","use Counter4;\nelement Foo {\nBool sp;\n Bool(3) bi, bh;\n Counter4 m_c4;\n Int e;\n Int test(){\n Atom a;\n Foo f;\n Bool b;\nif(a as Foo)\na.bi = true;\n a = f; //easy\n if(a as Counter4){\n bh = true;\n a.incr();\n e=a.get();\n }\n else {\n bh = false;\n}\nreturn e;\n}\n }\n");

      //no else case:
      //bool rtn1 = fms->add("Foo.ulam","use Counter4;\nelement Foo {\nBool sp;\n Bool(3) bi, bh;\n Counter4 m_c4;\n Int e;\n Int test(){\n Atom a;\n Foo f;\n Bool b;\n a = f; //easy\n if(a as Counter4){\n bi = true;\n a.incr();\n e=a.get();\n }\nreturn e;\n}\n }\n");

      //single statement case:
      //bool rtn1 = fms->add("Foo.ulam","use Counter4;\nelement Foo {\nBool sp;\n Bool(3) bi, bh;\n Counter4 m_c4;\n Int e;\n Int test(){\n Atom a;\n Foo f;\n Bool b;\n a = f; //easy\n if(a as Counter4)\na.incr();\nreturn e;\n}\n }\n");

      bool rtn2 = fms->add("Counter4.ulam", "ulam 1;\nquark Counter4 {\nInt d;\nVoid incr(){\nd+=1;\n}\nInt get(){\nreturn d;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      //      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3249_test_compiler_elementandquark_conditionalas)

} //end MFM

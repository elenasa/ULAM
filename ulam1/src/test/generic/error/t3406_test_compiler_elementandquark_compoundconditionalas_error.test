#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3406_test_compiler_elementandquark_compoundconditionalas_error)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Foo { Counter4 m_c4( Int(32) d(0); );  Int(32) e(0);  Bool(1) b(false);  Int(32) test() {  Atom(96) aq;  Foo f;  aq f cast = aq Counter4 as cond { Counter4 aq;  aq ( )incr . e aq ( )get . = } if e return } }\nUq_Counter4 { Int(32) d(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //point of this test; but invalid statement (double parens inside if condition)
      bool rtn1 = fms->add("Foo.ulam","use Counter4;\nelement Foo {\nCounter4 m_c4;\n Int e;\n Bool b;\nInt test(){\n Atom ae, aq;\n Foo f;\n ae = aq = f; //easy\n if((aq as Counter4) && (ae as Foo)){\naq.incr();\ne = aq.get();\n ae.b = true;\n}\nreturn e;\n}\n }\n");

      //simplified, works!
      //bool rtn1 = fms->add("Foo.ulam","use Counter4;\nelement Foo {\nCounter4 m_c4;\n Int e;\n Bool b;\n Int test(){\n Atom aq;\n Foo f;\n aq = f; //easy\n if(aq as Counter4){\naq.incr();\ne = aq.get();\n}\n return e;\n}\n }\n");

      //while too? yep
      //bool rtn1 = fms->add("Foo.ulam","use Counter4;\nelement Foo {\nCounter4 m_c4;\n Int e;\n Bool b;\n Int test(){\n Atom aq;\n Foo f;\n aq = f; //easy\n while(aq as Counter4){\naq.incr();\ne = aq.get();\n if(e > 2) break;}\n return e;\n}\n }\n");

      // not even with double parens instide single if condition
      //bool rtn1 = fms->add("Foo.ulam","use Counter4;\nelement Foo {\nCounter4 m_c4;\n Int e;\n Bool b;\n Int test(){\n Atom aq;\n Foo f;\n aq = f; //easy\n if((aq as Counter4)){\naq.incr();\ne = aq.get();\n}\n return e;\n}\n }\n");

      // not even with && b.
      //bool rtn1 = fms->add("Foo.ulam","use Counter4;\nelement Foo {\nCounter4 m_c4;\n Int e;\n Bool b;\n Int test(){\n Atom aq, ae;\n Foo f;\n ae = aq = f; //easy\n if(aq as Counter4 && b){\naq.incr();\ne = aq.get();\n}\n return e;\n}\n }\n");

      // parse okay with 'is' and 'has'
      //bool rtn1 = fms->add("Foo.ulam","use Counter4;\nelement Foo {\nCounter4 m_c4;\n Int e;\n Bool b;\nInt test(){\n Atom ae, aq;\n Foo f;\n ae = aq = f; //easy\n if((aq has Counter4) && (ae is Foo)){\n return 1;\n}\nreturn 0;\n}\n }\n");

      //simplified
      //bool rtn1 = fms->add("Foo.ulam","element Foo {\n Bool b,c;\n Int test(){\n b = true; if((b == true) && (c == false)){\n return 1;\n}\n return 0;\n}\n }\n");
      //bool rtn1 = fms->add("Foo.ulam","element Foo {\n Int b,c;\n Int test(){\n b = 100;\n c = 99;\n if((b <= 100) && (c < b)){\n return 1;\n}\n return 0;\n}\n }\n");

      bool rtn2 = fms->add("Counter4.ulam", "ulam 1;\nquark Counter4 {\nInt d;\nVoid incr(){\nd+=1;\n}\nInt get(){\nreturn d;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      //      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3406_test_compiler_elementandquark_compoundconditionalas_error)

} //end MFM

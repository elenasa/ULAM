#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3336_test_compiler_elementwithclassparameters_conditionalas)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 1
      */
      //eval can't test 'as' since atoms have no types.
      return std::string("Exit status: 7\nUe_Foo { System s();  Int(32) e(7);  constant Int(CONSTANT) v = 3;  Counter4(3) cv( constant Int(CONSTANT) x = 3;  Int(3) d(0); );  Int(32) test() {  Atom(96) a;  Foo f;  a f cast = a Counter4(3) as cond { Counter4(3) a;  a ( )incr . e a ( )get . = s ( e )print . } if e 7 cast = else e return } }\nUq_Counter4 { constant Int(CONSTANT) x = NONREADYCONST;  Int(UNKNOWN) d(0);  <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3249_test_compiler_elementandquark_conditionalas
      // 'a has System' doesn't appy to eval because atoms have no class declarations; but testable for gencode
      // enhanced by a 'named constant', v;
      bool rtn1 = fms->add("Foo.ulam","use Counter4;\nuse System;\n element Foo {\nSystem s;\nInt e;\nconstant Int v = 3;\nCounter4(v) cv;\n Int test(){\nAtom a;\nFoo f;\n a = f;\nif(a as Counter4(v))\n {\n a.incr();\ne = a.get();\n s.print(e);\n}\nelse\n e=7;\n return e;\n}\n }\n");

      // without named constant..returns 1 (yay!)
      //bool rtn1 = fms->add("Foo.ulam","use Counter4;\nuse System;\n element Foo {\nSystem s;\nInt e;\nCounter4(3) cv;\n Int test(){\nAtom a;\nFoo f;\n a = f;\nif(a as Counter4(3))\n {\n a.incr();\ne = a.get();\n s.print(e);\n}\nelse\n e=7;\n return e;\n}\n }\n");

      // most simple case: check for stub before trying to calc size during recursive calcVariableSymbolTypeSize
      //bool rtn1 = fms->add("Foo.ulam","use Counter4;\n element Foo {\nInt e;\nCounter4(3) cv;\n Int test(){\n return cv.sizeof;\n}\n }\n");

      bool rtn2 = fms->add("Counter4.ulam", "ulam 1;\nquark Counter4(Int x) {\nInt(x) d;\nVoid incr(){\nd+=1;\n}\nInt get(){\nreturn d;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	//if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3336_test_compiler_elementwithclassparameters_conditionalas)

} //end MFM

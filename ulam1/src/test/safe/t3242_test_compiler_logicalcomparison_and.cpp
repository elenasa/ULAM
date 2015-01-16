#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3242_test_compiler_logicalcomparison_and)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x0 (false)
	 Bool(3) Arg: 0x0 (false)
	 Bool(3) Arg: 0x0 (false)
	 Bool(3) Arg: 0x7 (true)
      */
      return std::string("Exit status: 1\nUe_A { System s();  Bool(1) sp(false);  Bool(3) d(false);  Bool(3) e(false);  Bool(3) f(false);  Bool(3) g(true);  Int(32) test() {  Bool(3) a;  Bool(3) b;  a false cast = b false cast = d a b && = s ( d )print . b true cast = e a b && = s ( e )print . f b a && = s ( f )print . a true cast = g a b && = s ( g )print . b cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool sp;\nBool(3) d;\nInt test(){Bool(3) a,b;\n a = true;\nb = true;\nd = a && b;\ns.print(d);\nreturn d;\n }\n }\n");
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool sp;\nBool(3) d,e,f,g;\nInt test(){Bool(3) a,b;\na = false;\nb = false;\nd = a && b;\ns.print(d);\nb = true;\ne = a && b;\ns.print(e);\nf = b && a;\ns.print(f);\na = true;\ng = a && b;\ns.print(g);\nreturn b;\n }\n }\n");


      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3242_test_compiler_logicalcomparison_and)

} //end MFM



#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3245_test_compiler_funcdefwlogicalcomparison)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 20
      */
      return std::string("Exit status: 20\nUe_A { Bool(7) b(false);  Int(32) d(20);  System s();  Int(32) test() {  d ( 4 5 )times = s ( d )print . d return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // explicit cast for &&:
      //./A.ulam:5:10: ERROR: Bool is the supported type for logical operator&&; Suggest casting Int(32) to Bool(32).
      //copied from t3244
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nInt times(Int m, Int n) {\nInt d;\n while(n!=0 && m!=0){\nd += n;\n m-=1;\n}\n return d;\n}\n\nSystem s;\nBool(7) b;\nInt d;\nInt test(){ d = times(4,5);\ns.print(d);\nreturn d;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3245_test_compiler_funcdefwlogicalcomparison)

} //end MFM

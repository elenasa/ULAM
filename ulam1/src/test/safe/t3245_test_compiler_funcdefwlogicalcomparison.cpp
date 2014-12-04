#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3245_test_compiler_funcdefwlogicalcomparison)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 20
      */
      return std::string("Ue_A { Bool(7) b(false);  Int(32) d(20);  System s();  Int(32) test() {  d ( 4 cast 5 cast )times = s ( d )print . d return } }\nExit status: 20");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //copied from t3244
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nInt times(Int m, Int n) {\nInt d;\n while(n && m){\nd += n;\n m-=1;\n}\n return d;\n}\n\nSystem s;\nBool(7) b;\nInt d;\nInt test(){ d = times(4,5);\ns.print(d);\nreturn d;\n }\n }\n");
      
      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3245_test_compiler_funcdefwlogicalcomparison)
  
} //end MFM



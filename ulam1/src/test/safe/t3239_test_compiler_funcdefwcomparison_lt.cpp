#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3239_test_compiler_funcdefwcomparison_lt)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) d(15);  Int(32) test() {  d ( 4 cast 5 cast )times = d return } }\nExit status: 15
      return std::string("Ue_A { Bool(7) b(false);  Int(32) d(20);  System s();  Int(32) test() {  d ( 4 cast 5 cast )times = s ( d )print . d return } }\nExit status: 20");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //      bool rtn1 = fms->add("A.ulam","element A { Int times(Int m, Int n) { Int d; while( m-=1 ) d += n; return d; } Int d; Int test(){ d = times(4,5); return d; } }");
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\n Int times(Int m, Int n) {\nInt d,e;\n while(e < m)\n{ d += n;\n e+=1;\ns.print(d);\n}\n return d;\n }\nSystem s;\nBool(7) b;\nInt d;\nInt test(){ d = times(4,5);\ns.print(d);\nreturn d;\n }\n }\n");
      
      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3239_test_compiler_funcdefwcomparison_lt)
  
} //end MFM



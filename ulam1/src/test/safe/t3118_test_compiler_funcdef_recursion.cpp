#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3118_test_compiler_funcdef_recursion)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) a(24);  Int(32) test() {  a ( 4 cast )fact = return } }\nExit status: 24
      return std::string("Ue_A { System s();  Bool(7) d(false);  Int(32) a(24);  Int(32) test() {  a ( 4 cast )fact = s ( a )print . a return } }\nExit status: 24");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","element A { Int test() { return a = fact(4); } Int a; Int fact(Int n) { if(n) return (n * fact(n-1)); else return 1; }  }");  
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool(7) d;\nInt test() {\na = fact(4);\ns.print(a);\nreturn a;\n}\nInt a;\nInt fact(Int n) {\nif(n)\nreturn (n * fact(n-1));\nelse\nreturn 1;\n}\n}\n");  
      
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3118_test_compiler_funcdef_recursion)
  
} //end MFM



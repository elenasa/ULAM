#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3222_test_compiler_localatom)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(3) b(3);  System s();  Bool(1) d(true);  Int(3) a(3);  Int(3) c(3);  Int(32) test() {  a 3 cast = b 4 cast = c a cast b cast +b cast = s ( c )print . a b += s ( a )print . d a cast = s ( d )assert . a cast return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool d;\nInt(3) a, b, c;\nInt test() {\nAtom tm;\nreturn 0;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn3)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3222_test_compiler_localatom)
  
} //end MFM



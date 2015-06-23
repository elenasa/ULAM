
#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3136_test_compiler_twofuncdefs)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Bool(7) b(false);  System s();  Unsigned(32) x(15);  Unsigned(32) y(0);  Int(32) test() {  y x ( 4 5 )times cast = = s ( x )print . y ( x x )max = s ( y )print . y cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // try overload 'max' with Unsigned args and return Unsigned; let x and y be Unsigned
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nInt times(Int m, Int n) {\nInt e;\nwhile( m-=1 )\n e += n;\n return e;\n }\nInt max(Int a, Int b) {\n return (a - b);\n}\nUnsigned max(Unsigned a, Unsigned b){\n return a - b;\n}\nBool(7) b;\nUnsigned x, y;\nInt test(){\ny = x = (Unsigned) times(4,5);\ns.print(x);\ny = max(x,x);\ns.print(y);\nreturn (Int) y;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3136_test_compiler_twofuncdefs)

} //end MFM

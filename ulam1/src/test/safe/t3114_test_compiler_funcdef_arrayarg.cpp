#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3114_test_compiler_funcdef_arrayarg)
  {
    std::string GetAnswerKey()
    {

      return std::string("Exit status: 1\nUe_A { System s();  Int(32) d(1);  Bool(7) sp(false);  Int(32) test() {  Bool(1) mybool[3];  mybool 0 [] true cast = mybool 1 [] false cast = mybool 2 [] false cast = d ( 7 cast mybool )m = s ( d )print . d return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","element A { Int m(Int m, Bool b[3]) { if(b[0]) m=1; else m=2; return m;} Int test() { Bool mybool[3]; mybool[0] = true; mybool[1] = false; mybool[2]= false; d = m(7, mybool); return d; } Int d; }");  // want d == 1.

      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\n Int m(Int m, Bool b[3]) {\n if(b[0])\n m=1;\n else m=2;\n return m;\n}\n Int test() {\n Bool mybool[3];\n mybool[0] = true;\n mybool[1] = false;\n mybool[2]= false;\n d = m(7, mybool);\ns.print(d);\n return d;\n }\nBool(7) sp;\n Int d;\n }\n");  // want d == 1.

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3114_test_compiler_funcdef_arrayarg)

} //end MFM



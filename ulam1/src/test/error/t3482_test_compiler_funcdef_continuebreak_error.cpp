#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3482_test_compiler_funcdef_continuebreak_error)
  {
    std::string GetAnswerKey()
    {
      /*
	A.ulam:8:2: ERROR: Continue statement not within loop.
	A.ulam:9:2: ERROR: Unexpected input!! Token: <else>.
	A.ulam:10:2: ERROR: Break statement not within loop or switch.
	A.ulam:2:11: fyi: 3 warnings during parsing.
	A.ulam:2:11: fyi: 3 TOO MANY PARSE ERRORS.
	Unrecoverable Program Parse FAILURE.
      */

      return std::string("Ue_A { Bool(7) b(false);  Int(32) d(20);  System s();  Int(32) test() {  d ( 4 cast 5 cast )times = s ( d )print . d return } }\nExit status: 20");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nInt times(Int m, Int n) {\nInt d;\nd += n;\n m-=1;\n if(m !=0 )\n continue;\n else\n break;\n return d;\n}\n\nSystem s;\nBool(7) b;\nInt d;\nInt test(){ d = times(4,5);\ns.print(d);\nreturn d;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3482_test_compiler_funcdef_continuebreak_error)

} //end MFM

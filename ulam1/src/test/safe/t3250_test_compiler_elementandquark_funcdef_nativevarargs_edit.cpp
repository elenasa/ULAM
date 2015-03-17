#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3250_test_compiler_elementandquark_funcdef_nativevarargs_edit)
  {
    std::string GetAnswerKey()
    {
      /* gen code: to test 'maxof' edit A.tcc and set evalling Bool to false;
	 because eval is unable to return a non-void native function in an empty block
	 Int Arg: 3  (without edit)
	 Int Arg: 4  (after edit)
      */
      return std::string("Exit status: 3\nUe_A { System s();  Math math();  Int(32) test() {  Int(32) m;  Bool(1) evalling;  evalling true = evalling cond m 3 = if m math ( 1 4 2 )max . = else s ( m )print . m return } }\nUq_System { <NOMAIN> }\nUq_Math { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {

      bool rtn1 = fms->add("A.ulam","use System;\nuse Math;\nelement A {\nSystem s;\nMath math;\nInt test(){\nInt m;\nBool evalling;\nevalling = true;\n if(evalling)\n m = 3;\nelse\n m = math.max(1, 4, 2);\n s.print(m);\nreturn m;\n}\n }\n");

      // max with variable number of args...
      //with final void * 0 instead of nargs as first arg.
      bool rtn2 = fms->add("Math.ulam", "ulam 1;\nquark Math {\nInt max(...) native;\n}\n");

      // test system quark with native overloaded print funcs; assert (see t3207 quarksystem)
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3250_test_compiler_elementandquark_funcdef_nativevarargs_edit)

} //end MFM

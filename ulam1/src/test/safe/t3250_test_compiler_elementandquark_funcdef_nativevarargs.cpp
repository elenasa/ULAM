#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3250_test_compiler_elementandquark_funcdef_nativevarargs)
  {
    std::string GetAnswerKey()
    {
      /* gen code: to test 'maxof' edit A.tcc and set evalling Bool to false;
	 because eval is unable to return a non-void native function in an empty block
      */
      return std::string("Ue_A { System s();  Math math();  Int(32) test() {  Int(32) m;  Bool(1) evalling;  evalling true cast = evalling cast cond m 3 cast = if m math ( 1 cast 4 cast 2 cast )maxof . = else s ( m )print . m return } }\nExit status: 3");
    }

    std::string PresetTest(FileManagerString * fms)
    {

      bool rtn1 = fms->add("A.ulam","use System;\nuse Math;\nelement A {\nSystem s;\nMath math;\nInt test(){\nInt m;\nBool evalling;\nevalling = true;\n if(evalling)\n m = 3;\nelse\n m = math.maxof(1, 4, 2);\n s.print(m);\nreturn m;\n}\n }\n");

      // maxof with variable number of args...
      //with final void * 0 instead of nargs as first arg.
      bool rtn2 = fms->add("Math.ulam", "ulam 1;\nquark Math {\nInt maxof(...) native;\n}\n");

      // test system quark with native overloaded print funcs; assert (see t3207 quarksystem)
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3250_test_compiler_elementandquark_funcdef_nativevarargs)

} //end MFM



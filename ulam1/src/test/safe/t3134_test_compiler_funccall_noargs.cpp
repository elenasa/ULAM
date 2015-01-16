#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3134_test_compiler_funccall_noargs)
  {
    std::string GetAnswerKey()
    {
      //updated to even word boundary
      //updated to print a in gencode
      return std::string("Exit status: 1\nUe_A { Bool(7) b(false);  System s();  Int(32) a(1);  Int(32) test() {  a ( )foo = s ( a )print . a return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","element A {\nBool(7) b;\nInt a;\nInt foo() {\n return 1;\n }\nInt test() {\nreturn foo();\n}\n}\n");
      bool rtn1 = fms->add("A.ulam","use System; element A {\nSystem s;\nBool(7) b;\nInt a;\nInt foo() {\n return 1;\n }\nInt test() {\na = foo();\ns.print(a);\nreturn a;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3134_test_compiler_funccall_noargs)

} //end MFM



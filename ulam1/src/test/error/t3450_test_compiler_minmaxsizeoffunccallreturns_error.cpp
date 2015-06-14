#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3450_test_compiler_minmaxsizeoffunccallreturns_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./T.ulam:12:14: ERROR: Unexpected token <TOK_KW_MINOF>; Expected <IDENT>.
	./T.ulam:12:2: ERROR: Invalid Statement (possible missing semicolon).
	./T.ulam:13:14: ERROR: Unexpected token <TOK_KW_MAXOF>; Expected <IDENT>.
	./T.ulam:13:2: ERROR: Invalid Statement (possible missing semicolon).
	./T.ulam:14:17: ERROR: Unexpected token <TOK_KW_SIZEOF>; Expected <IDENT>.
	./T.ulam:14:2: ERROR: Invalid Statement (possible missing semicolon).
	Unrecoverable Program Parse FAILURE.
      */
      return std::string("Exit status: 5\nUe_T { typedef Unary(7) B;  Unary(7) m(5);  Unary(7) n(2);  Unary(7) r(5);  Unary(7) a(2);  Unary(7) h(3);  Int(32) test() {  m ( )func1 = n ( )func2 = r ( )func1 ( )func2 | = a ( )func1 ( )func2 & = h ( )func1 ( )func2 ^ = r cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("T.ulam"," ulam 1;\n element T{\ntypedef Int(7) B;\n B m,n;\n B func1() {\n return 5;\n}\n B func2() {\n return 7;\n}\n Int test(){\n m = func1().minof;\n n = func2().maxof;\n return func1().sizeof;\n}\n }\n");

      if(rtn2)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3450_test_compiler_minmaxsizeoffunccallreturns_error)

} //end MFM

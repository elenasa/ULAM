#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3248_test_compiler_isparse_error)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { System s();  Bool(1) sp(false);  Bool(3) bi(true);  Bool(3) bh(true);  Int(32) d(3);  Int(32) test() {  Atom(96) a;  Foo f;  Bool(1) b;  a f cast = a Foo is cast cond bi true cast = if f a cast = f System has cast cond bh true cast = if b a System has = s ( b ! )assert . d a System has cast 3 cast +b = d return } }\nExit status: 3");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // 'a has System' doesn't appy to eval because atoms have no class declarations; but testable for gencode
      bool rtn1 = fms->add("EventWindowTest.ulam", "ulam 1;\nuse EventWindow;\nelement EventWindowTest {\nEventWindow ew;\n Int test() {\n    Atom a;\n    a = ew[0];  // This is me!\n    if (a is EventWindowTest)\n      return 0;\n    return 1;\n  }\n}\n");
      bool rtn2 = fms->add("EventWindow.ulam", "ulam 1;\nquark EventWindow {\n  Atom aRef(Int index) native;\n  Void aSet(Int index, Atom val) native;\n  Int size() native;\n  Void diffuse() native;\n}\n");

      // test system quark with native overloaded print funcs; assert
      //bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2)
	return std::string("EventWindowTest.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3248_test_compiler_isparse_error)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3328_test_compiler_elementwithclassparameters_self)
  {
    //informed by t3255
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_S { constant Int(CONSTANT) x = NONREADYCONST;  constant Int(CONSTANT) y = NONREADYCONST;  Bool(UNKNOWN) b2(false);  Int(32) test() {  S s12;  s12 b2 . s12 ( )func . = s12 b2 . return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // simple case: OK!  "Bool(x+y) b1, b2;" works!!
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S(Int x, Int y){\nBool sp;\n Bool(x+y) b1, b2;\n Int test() {\nS(1,2) s;\n/*Atom a;\n a = self;\n self = a;\n*/ return 0;\n }\n }\n");

      //"(self is S(x,y)" works
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S(Int x, Int y){\nBool sp;\n Bool func() {\n return (self is S(x,y));\n}\n Int test() {\nS(1,2) s12;\n s12.sp = s12.func();\n return 0;\n }\n }\n");

      // works, BUT "return (self is S(x,y))" doesn't work!
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\nuse System;\n element S(Int x, Int y){\nSystem s;\nBool(x+y) b1, b2;\n Bool func() {\n return (self is S(x,y));\n}\n  Bool func(Atom d) {\n return (d is S(x,y));\n}\n Int test() {\nAtom a;\nS(1,2) s12;\n s12.b2 = s12.func();\n s.print(s12.b2);\ns12.b1 = s12.func(a);\n s.print(s12.b1);\nreturn s12.b2;\n }\n }\n");

      //one type of func
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\nuse System;\n element S(Int x, Int y){\nSystem s;\nBool(x+y) b2;\n Bool func() {\n return (self is S(x,y));\n}\n Int test() {\nS(1,2) s12;\n s12.b2 = s12.func();\n s.print(s12.b2);\nreturn s12.b2;\n }\n }\n");

      // the other type of func
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\nuse System;\n element S(Int x, Int y){\nSystem s;\nBool(x+y) b1;\n Bool func(Atom d) {\n return (d is S(x,y));\n}\n Int test() {\nAtom a;\nS(1,2) s12;\n s12.b1 = s12.func(a);\n s.print(s12.b1);\nreturn s12.b1;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      //bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      bool rtn1 = fms->add("S.ulam"," ulam 1;\nelement S(Int x, Int y){\nBool(x+y) b2;\n Bool func() {\n return (self is S(x,y));\n}\n Int test() {\nS(1,2) s12;\n s12.b2 = s12.func();\nreturn s12.b2;\n }\n }\n");

      //if(rtn1 && rtn3)
      if(rtn1)
	return std::string("S.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3328_test_compiler_elementwithclassparameters_self)

} //end MFM



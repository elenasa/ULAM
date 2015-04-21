#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3337_test_compiler_elementwargs_elementparameterelement)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x7 (true)
      */
      //Constants have explicit types
      return std::string("Exit status: 1\nUe_Foo { Bool(3) b(true);  System s();  Bool(1) sp(false);  Bool(3) c(false);  Bool(1) last(false);  Int(32) test() {  Poo(1,2,0) p;  p ( sp )reset . poochance ( sp )reset . poochance valb 1 [] . true = b poochance valb 1 [] . cast = s ( b )print . b cast return } }\nUq_System { <NOMAIN> }\nUe_Poo { constant Int(32) x = NONREADYCONST;  constant Int(32) y = NONREADYCONST;  constant Int(32) z = NONREADYCONST;  Bool(UNKNOWN) valb[UNKNOWN](false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3220
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Poo;\nelement Foo {\nSystem s;\nBool(1) sp;\nBool(3) b,c;\n element Poo(1,2,0) poochance;\nBool last;\nInt test() {\nPoo(1,2,0) p;\np.reset(sp);\npoochance.reset(sp);\npoochance.valb[1] = true;\nb = poochance.valb[1];\ns.print(b);\nreturn b;\n }\n }\n");

      //only EP
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Poo;\nelement Foo {\nSystem s;\nBool(1) sp;\nBool(3) b,c;\n element Poo(1,2,0) poochance;\nBool last;\nInt test() {\npoochance.reset(sp);\npoochance.valb[1] = true;\nb = poochance.valb[1];\ns.print(b);\nreturn b;\n }\n }\n");

      // only EP, 2nd ='s has problems trying to c&l and incomplete class:
      //Foo.ulam:6:11: ERROR: Member selected is incomplete class: Poo(1,2,0), check and label fails this time around.
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Poo;\nelement Foo {\nelement Poo(1,2,0) poochance;\nInt test() {\npoochance.valb[1] = true;\nreturn 0;\n }\n }\n");

      // no EP, local poo:  eventually resolved
      //Foo.ulam:12:3: ERROR: Casting 'incomplete' types: Bool(UNKNOWN)(UTI23) to be Bool(3)(UTI15) in class: Foo.
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Poo;\nelement Foo {\nSystem s;\nBool(1) sp;\nBool(3) b,c;\nBool last;\nInt test() {\nPoo(1,2,0) p;\np.reset(sp);\nb = p.valb[1];\ns.print(b);\nreturn b;\n }\n }\n");

      // poo as DM is an ERROR:
      //Foo.ulam:8:1: ERROR: Data member <p> of type: Poo(1,2,0) (UTI21) is an element, and is NOT permitted; Local variables, quarks, and Element Parameters do not have this restriction.
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Poo;\nelement Foo {\nSystem s;\nBool(1) sp;\nBool(3) b,c;\nPoo(1,2,0) p;\nBool last;\nInt test() {\np.reset(sp);\nb = p.valb[1];\ns.print(b);\nreturn b;\n }\n }\n");

      // as data member, without class parameters? ERROR!
      //Foo.ulam:8:1: ERROR: Data member <p> of type: Poo (UTI16) is an element, and is NOT permitted; Local variables, quarks, and Element Parameters do not have this restriction.
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Poo;\nelement Foo {\nSystem s;\nBool(1) sp;\nBool(3) b,c;\nPoo p;\nBool last;\nInt test() {\np.reset(sp);\nb = p.valb[1];\ns.print(b);\nreturn b;\n }\n }\n");
      // poo without parameters
      //bool rtn2 = fms->add("Poo.ulam"," ulam 1;\n element Poo {\n Bool valb[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      // poo with 3 parameters
      bool rtn2 = fms->add("Poo.ulam"," ulam 1;\n element Poo(Int x, Int y, Int z) {\n Bool(x+z) valb[y+z];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	//if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3337_test_compiler_elementwargs_elementparameterelement)

} //end MFM

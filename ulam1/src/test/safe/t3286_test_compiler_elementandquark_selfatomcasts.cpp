#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3286_test_compiler_elementandquark_selfatomcasts)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 assert: arg is 1
	 after assert's abort: arg is 1
	 assert: arg is 1
	 after assert's abort: arg is 1
      */
      // no cast cond
      //Ue_Foo { System s();  Bool(1) sp(false);  Bool(3) bi(true);  Bool(3) bh(true);  Int(32) d(3);  Int(32) test() {  Atom(96) a;  Foo f;  Bool(1) b;  b a System has = s ( b ! )assert . a f cast = a Foo is cast cond bi true cast = if f a cast = f System has cast cond bh true cast = if d a System has cast 3 cast +b = d return } }\nExit status: 3
      return std::string("Ue_Foo { System s();  Bool(1) sp(false);  Bool(3) bi(true);  Bool(3) bh(true);  Int(32) d(3);  Int(32) test() {  Atom(96) a;  Foo f;  Bool(1) b;  b a System has = s ( b ! )assert . a f cast = a Foo is cond bi true cast = if b a System has = s ( b )assert . f a cast = f System has cond bh true cast = if d a System has cast 3 cast +b = d return } }\nExit status: 3");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // in element, 'self' is an element: cast self to an atom arg, and as RHS.
      bool rtn1 = fms->add("Foo.ulam","use Bar;\nelement Foo {\nInt test(){Atom a;\nBar b;\nb.func((Atom) self);\n a = (Atom) self;\n return 0;\n }\n }\n");

      // in quark, 'self' is an atom: cast self to an an atom (rhs)
      //bool rtn2 = fms->add("Bar.ulam","quark Bar {\nVoid func(Atom a){Atom t = (Atom) self;\n}\n}\n");
      bool rtn2 = fms->add("Bar.ulam","quark Bar {\nVoid func(Atom a){Atom t = self;\n}\n}\n");


      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3286_test_compiler_elementandquark_selfatomcasts)

} //end MFM

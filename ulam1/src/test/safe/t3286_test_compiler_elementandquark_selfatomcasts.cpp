#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3286_test_compiler_elementandquark_selfatomcasts)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
      */
      // no cast cond
      return std::string("Exit status: 0\nUe_Foo { Int(32) test() {  Atom(96) a;  Bar b;  b ( self cast )func . a self cast = 0 cast return } }\nUq_Bar { <NOMAIN> }\n");
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

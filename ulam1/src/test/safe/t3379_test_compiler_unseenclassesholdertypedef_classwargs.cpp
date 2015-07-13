#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3379_test_compiler_unseenclassesholdertypedef_classwargs)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUq_A { Bool(UNKNOWN) y(unknown);  constant Int(32) a = NONREADYCONST;  <NOMAIN> }\nUe_F { typedef A(3) Foo;  Int(32) test() {  A(3) f;  f y . true cast = f y . cond f y . cast return if 0 cast return } }\nUe_E { typedef A(3) X;  <NOMAIN> }\nUe_D { typedef A(3) X;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use F;\n use E;\n use D;\n quark A(Int a){Bool(a) y;\n }\n");

      bool rtn2 = fms->add("F.ulam", "element F{\n typedef E.X Foo;\n Int test(){\n Foo f;\n f.y = true;\n if(f.y)\n return f.y;\n return 0;\n}\n}\n");

      bool rtn3 = fms->add("E.ulam", "element E{\n typedef D.X X;\n }\n");

      bool rtn4 = fms->add("D.ulam", "element D{\n typedef A(3) X;\n }\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3379_test_compiler_unseenclassesholdertypedef_classwargs)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3373_test_compiler_unseenclassesholdertypedefs)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUq_A { /* empty class block */ }Ue_F { typedef Bool(1) Foo;  Int(32) test() {  Bool(1) f;  f true = f cond f cast return if 0 return } }\nUe_E { typedef Bool(1) X;  <NOMAIN> }\nUe_D { typedef Bool(1) X;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use F;\n use E;\n use D;\n quark A{ }\n");

      bool rtn2 = fms->add("F.ulam", "element F{\n typedef E.X Foo;\n Int test(){\n Foo f;\n f = true;\n if(f)\n return f;\n return 0;\n}\n}\n");
      // simplify for testing
      //bool rtn2 = fms->add("F.ulam", "element F{\n typedef E.X Foo;\n Int test(){\n Foo f;\n return 0;\n}\n}\n");

      bool rtn3 = fms->add("E.ulam", "element E{\n typedef D.X X;\n }\n");

      bool rtn4 = fms->add("D.ulam", "element D{\n typedef Bool X;\n }\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3373_test_compiler_unseenclassesholdertypedefs)

} //end MFM

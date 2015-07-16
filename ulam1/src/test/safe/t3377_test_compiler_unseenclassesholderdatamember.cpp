#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3377_test_compiler_unseenclassesholderdatamember)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUq_A { /* empty class block */ }Ue_F { Bool(3) f[2](true,false);  Int(32) test() {  f 0 [] true cast = f 0 [] cond f 0 [] cast return if 0 cast return } }\nUe_E { typedef Bool(3) X;  <NOMAIN> }\nUe_D { typedef Bool(3) X;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use F;\n use E;\n use D;\n quark A{ }\n");

      // array is not part of E.X typedef
      bool rtn2 = fms->add("F.ulam", "element F{\nE.X f[2];\n Int test(){\n f[0] = true;\n if(f[0])\n return f[0];\n return 0;\n}\n}\n");

      bool rtn3 = fms->add("E.ulam", "element E{\n typedef D.X X;\n }\n");

      //test constant expression bitwise
      bool rtn4 = fms->add("D.ulam", "element D{\n typedef Bool(3) X;\n }\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3377_test_compiler_unseenclassesholderdatamember)

} //end MFM

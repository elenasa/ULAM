#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3258_test_compiler_elementandquark_funccalltypedefarg_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_TypedefIssue { typedef Unsigned(3) Symmetry;  Bool(1) b(true);  Vector t( typedef Unsigned(3) Symmetry;  Unsigned(3) m(1);  Unsigned(3) n(1); );  Int(32) test() {  t ( 0 cast 1 cast )set . cond b true = if t ( 1 cast 1 cast )set . cond b true = if t m . cast return } }\nUq_Vector { typedef Unsigned(3) Symmetry;  Unsigned(3) m(4);  Unsigned(3) n(4);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // Unsigned(3) version with if, Vector as data member
      //bool rtn1 = fms->add("TypedefIssue.ulam","ulam 1;\nuse Vector;\n element TypedefIssue {\n // Types\n typedef Unsigned(3) Symmetry;\nBool b;\nVector t;\n Int test() {\nif(t.set((Symmetry) 0))\n b=true;\n  if(t.set((Unsigned(3)) 1))\n b=true;\n return t.m;\n}\n}\n");

      // Unsigned(3) version with if, Vector as local variable
      bool rtn1 = fms->add("TypedefIssue.ulam","ulam 1;\nuse Vector;\n element TypedefIssue {\n // Types\n typedef Unsigned(3) Symmetry;\nBool b;\nVector t;\n Int test() {\nif(t.set((Symmetry) 0, (Symmetry) 1))\n b=true;\n  if(t.set((Unsigned(3)) 1, (Unsigned(3)) 1))\n b=true;\n return t.m;\n}\n}\n");

      bool rtn2 = fms->add("Vector.ulam","ulam 1;\nquark Vector {\ntypedef Unsigned(3) Symmetry;\n Symmetry m;\nSymmetry n;\n Bool set(Symmetry vector, Symmetry index) {\nm=vector;\nn=index;\n return ((Bool) m && (Bool) n);\n }\n}\n");

      if(rtn1 && rtn2)
	return std::string("TypedefIssue.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3258_test_compiler_elementandquark_funccalltypedefarg_issue)

} //end MFM

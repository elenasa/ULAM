#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3270_test_compiler_elementandquark_arraytypedeffromanotherclass)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_TypedefIssue { typedef Unsigned(3) Symmetry[2];  Unsigned(3) x[2](2,1);  Bool(1) b(true);  Int(32) test() {  Vector t;  x 0 [] 2 cast = x 1 [] 1 cast = t ( x )set . cast cond b true cast = if t m 1 [] . cast return } }\nExit status: 1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // informed by t3268: array typedef 'Symmetry' from quark used
      // as typedef, data member array, and casted arg in element.
      // must already be parsed! (e.g. couldn't use element yet! because its Class Block doesn't exist).
      bool rtn1 = fms->add("TypedefIssue.ulam","ulam 1;\nuse Vector;\n element TypedefIssue {\ntypedef Vector.Symmetry Symmetry;\n Symmetry x;\n Bool b;\n Int test() {\n Vector t;\nx[0] = 2;\n x[1] = 1;\nif(t.set(x))\n b=true;\nreturn t.m[1];\n}\n}\n");

      bool rtn2 = fms->add("Vector.ulam","ulam 1;\nquark Vector {\ntypedef Unsigned(3) Symmetry[2];\n Symmetry m;\nBool set(Symmetry vector) {\nm[0]=vector[0];\n m[1]=vector[1];\n return (m[0] && m[1]);\n }\n}\n");

      if(rtn1 && rtn2)
	return std::string("TypedefIssue.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3270_test_compiler_elementandquark_arraytypedeffromanotherclass)

} //end MFM



#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3337_test_compiler_elementandquark_arraytypedeffromanotherclass_unseen)
  {
    std::string GetAnswerKey()
    {
      //No error about zero bitsize!!

      //Exit status: 1\nUe_TypedefIssue { Unsigned(3) x[2](2,1);  Bool(1) b(true);  Vector t( typedef Unsigned(3) Symmetry[2];  Unsigned(3) m[2](2,1); );  Int(32) test() {  x 0 [] 2 cast = x 1 [] 1 cast = t ( x )set . cond b true = if t m 1 [] . cast return } }\nUq_Vector { typedef Unsigned(3) Symmetry[2];  Unsigned(3) m[2](2,1);  <NOMAIN> }
      return std::string("Exit status: 1\nUe_R { Int(32) test() {  TypedefIssue ish;  ish ( )foo . return } }\nUe_TypedefIssue { typedef Unsigned(3) Symmetry[2];  Unsigned(3) x[2](0,0);  Bool(1) b(false);  Vector t( typedef Unsigned(3) Symmetry[2];  Unsigned(3) m[2](0,0);  typedef Unsigned(3) Channel; );  <NOMAIN> }\nUq_Vector { typedef Unsigned(3) Symmetry[2];  Unsigned(3) m[2](0,0);  typedef Unsigned(3) Channel;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3269, UNSEEN..

      // informed by t3268: array typedef 'Symmetry' from quark used
      // as typedef, data member array, and casted arg in element.
      // must already be parsed! (e.g. couldn't use element yet! because its Class Block doesn't exist).
      bool rtn3 = fms->add("R.ulam", "ulam 1;\n use TypedefIssue;\n use Vector;\n element R {\n Int test(){\nTypedefIssue ish;\n return ish.foo();\n}\n }\n");

      bool rtn1 = fms->add("TypedefIssue.ulam","ulam 1;\nelement TypedefIssue {\ntypedef Vector.Symmetry Symmetry;\n Symmetry x;\n Bool b;\nVector t;\n Int foo() {\nx[0] = 2;\n x[1] = 1;\nif(t.set(x))\n b=true;\nreturn t.m[1];\n}\n}\n");

      bool rtn2 = fms->add("Vector.ulam","ulam 1;\nquark Vector {\ntypedef Unsigned(3) Channel;\n typedef Channel Symmetry[2];\n Symmetry m;\nBool set(Symmetry vector) {\nm[0]=vector[0];\n m[1]=vector[1];\n return (m[0]!=0 && m[1]!=0);\n }\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3337_test_compiler_elementandquark_arraytypedeffromanotherclass_unseen)

} //end MFM

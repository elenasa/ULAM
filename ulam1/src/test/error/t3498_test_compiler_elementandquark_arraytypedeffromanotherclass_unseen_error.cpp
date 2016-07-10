#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3498_test_compiler_elementandquark_arraytypedeffromanotherclass_unseen_error)
  {
    std::string GetAnswerKey()
    {
      //./R.ulam:9:2: ERROR: Invalid zero sizes to set for nonClass: Unsigned(UNKNOWN).
      //No error about zero bitsize!!
      return std::string("Exit status: 12\nUe_R { Int(32) test() {  TypedefIssue(2u) ish;  ish ( )foo2 . return } }\nUe_TypedefIssue { constant Unsigned(32) tif = NONREADYCONST;  typedef 0Holder(UNKNOWN) Symmetry;  0Holder(UNKNOWN) x(0Holder(UNKNOWN));  Bool(1) b(false);  Vector(vec) t( constant Unsigned(32) vec = NONREADYCONST; );  <NOMAIN> }\nUq_Vector { typedef Unsigned(UNKNOWN) Symmetry[UNKNOWN];  Unsigned(UNKNOWN) m[UNKNOWN](0);  constant Unsigned(32) vec = NONREADYCONST;  typedef Unsigned(UNKNOWN) Channel;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3498, here arraysize has divide by zero
      //./Vector.ulam:4:35: ERROR: Possible Divide By Zero Attempt.

      //informed by 3337, array subscript delayed.
      //informed by 3269, UNSEEN..

      // informed by t3268: array typedef 'Symmetry' from quark used
      // as typedef, data member array, and casted arg in element.
      bool rtn3 = fms->add("R.ulam", "ulam 1;\n use TypedefIssue;\n use Vector;\n element R {\n Int test(){\nTypedefIssue(2u) ish;\n return ish.foo2();\n}\n }\n");

      bool rtn1 = fms->add("TypedefIssue.ulam","ulam 1;\nelement TypedefIssue(Unsigned tif) {\ntypedef Vector(tif).Symmetry Symmetry;\n Symmetry x;\n Bool b;\nVector(tif) t;\n Int foo() {\nx[0] = 2;\n x[1] = 1;\nif(t.set(x))\n b=true;\nreturn t.m[1];\n} Int foo2() {\n return x.sizeof;\n}\n}\n");

      //Divide by Zero can also be detected in Channel typedef bitsize
      bool rtn2 = fms->add("Vector.ulam","ulam 1;\nquark Vector(Unsigned vec) {\ntypedef Unsigned((2u*vec)) Channel;\n typedef Channel Symmetry[(vec+1u)/vec.minof];\n Symmetry m;\nBool set(Symmetry vector) {\nm[0]=vector[0];\n m[1]=vector[1];\n return (m[0]!=0 && m[1]!=0);\n }\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3498_test_compiler_elementandquark_arraytypedeffromanotherclass_unseen_error)

} //end MFM

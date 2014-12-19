#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3258_test_compiler_quark_funccalltypedefarg_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Vector { typedef Unsigned(3) Symmetry;  Unsigned(3) m(0);  Unsigned(3) n(0);  <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Vector.ulam","ulam 1;\nquark Vector {\ntypedef Unsigned(3) Symmetry;\n Symmetry m;\nSymmetry n;\n Bool set(Symmetry vector, Symmetry index) {\nm=vector;\nn=index;\n return (m && n);\n }\n}\n");

      if(rtn2)
	return std::string("Vector.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3258_test_compiler_quark_funccalltypedefarg_issue)

} //end MFM



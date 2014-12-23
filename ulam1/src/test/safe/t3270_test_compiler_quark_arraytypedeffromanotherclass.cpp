#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3270_test_compiler_quark_arraytypedeffromanotherclass)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Vector { typedef Unsigned(3) Symmetry[2];  Unsigned(3) m[2](0,0);  <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Vector.ulam","ulam 1;\nquark Vector {\ntypedef Unsigned(3) Symmetry[2];\n Symmetry m;\nBool set(Symmetry vector) {\nm[0]=vector[0];\n m[1]=vector[1];\n return (m[0] && m[1]);\n }\n}\n");

      if(rtn2)
	return std::string("Vector.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3270_test_compiler_quark_arraytypedeffromanotherclass)

} //end MFM



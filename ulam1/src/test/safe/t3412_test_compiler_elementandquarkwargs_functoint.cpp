#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3412_test_compiler_elementandquarkwargs_functoint)
  {
    std::string GetAnswerKey()
    {
      // repeat of func() doesn't make a new toIntHelper.
      return std::string("Exit status: 7\nUe_Foo { Q q( C2D(4,7) c( constant Int(4) m_width = 4;  constant Int(4) m_height = 7; );  C2D(2,5) d( constant Int(4) m_width = 2;  constant Int(4) m_height = 5; ); );  Int(8) i(7);  Int(8) j(7);  Int(8) k(7);  Int(32) test() {  i ( q ( )func . )_toIntHelper cast cast = j ( q ( )func2 . )_toIntHelper cast cast = k ( q ( )func . )_toIntHelper cast cast = ( q ( )func2 . )_toIntHelper cast return } }\nUq_Q { C2D(4,7) c( constant Int(4) m_width = 4;  constant Int(4) m_height = 7; );  C2D(2,5) d( constant Int(4) m_width = 2;  constant Int(4) m_height = 5; );  <NOMAIN> }\nUq_C2D { constant Int(4) m_width = NONREADYCONST;  constant Int(4) m_height = NONREADYCONST;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //member selection with a function call must be first saved to a
      //variable since we results are returned-by-value (see t3188)
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Q;\n element Foo {\n Q q;\n Int(8) i,j,k;\n Int test() { i = (Int(8)) q.func();\n j = (Int(8)) q.func2();\n k = (Int(8)) q.func();\n return q.func2();\n}\n }\n");

      bool rtn3 = fms->add("Q.ulam","ulam 1;\n use C2D;\n quark Q {\n C2D(4,7) c;\n C2D(2,5) d;\n C2D(4,7) func(){\n return c;\n }\n C2D(2,5) func2(){\n return d;\n }\n }\n");

      // order nor typedefs change the missing file:
      // include/Uq_10101Q10.tcc:5:43: fatal error: Uq_10103C2D1210141i1410141i17.h: No such file or directory
      //bool rtn3 = fms->add("Q.ulam","ulam 1;\n use C2D;\n quark Q {\ntypedef C2D(2,5) C2D25;\n typedef C2D(4,7) C2D47;\n   C2D25 d;\n C2D47 c;\n C2D47 func(){\n return c;\n }\n C2D25 func2(){\n return d;\n }\n }\n");

      bool rtn2 = fms->add("C2D.ulam","quark C2D(Int(4) m_width, Int(4) m_height) {\n Int getIndex(Int a, Int b){\n return ((m_height-b) * m_width + (m_height-a));\n }\n Int toInt(){\n return (Int) (m_width + m_height);\n }\n }\n");


      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3412_test_compiler_elementandquarkwargs_functoint)

} //end MFM

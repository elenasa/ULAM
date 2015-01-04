#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3286_test_compiler_quark_selfatomcasts)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Bar { <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // in quark, 'self' is an atom: cast self to an an atom (rhs)
      // prblem gcc'ing the gencode:
      /*
	include/Uq_10103Bar.tcc: In static member function ‘static void MFM::Uq_10103Bar<CC, POS>::Uf_4func(MFM::UlamCon \
	text<CC>&, MFM::Uq_10103Bar<CC, POS>::T&, MFM::Ui_Ut_102964Atom<CC>)’:
	include/Uq_10103Bar.tcc:19:9: error: ‘Ut_102964Atom’ was not declared in this scope
	include/Uq_10103Bar.tcc:19:25: error: expected primary-expression before ‘>’ token
	include/Uq_10103Bar.tcc:19:26: error: ‘::THE_INSTANCE’ has not been declared
      */

      bool rtn2 = fms->add("Bar.ulam","quark Bar {\nVoid func(Atom a){\nAtom t = (Atom) self;\n}\n }\n");

      //ok! without the cast.
      //bool rtn2 = fms->add("Bar.ulam","quark Bar {\nVoid func(Atom a){\nAtom t = self;\n}\n }\n");


      if(rtn2)
	return std::string("Bar.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3286_test_compiler_quark_selfatomcasts)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3256_test_compiler_funccalltypedefarg_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_TypedefIssue { typedef Int(3) Symmetry;  Int(32) test() {  TypedefIssue t;  t ( 0 cast )set . 0 cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {

      /*
      ulam 1;
      use Empty;

      element TypedefIssue {
	// Types
	typedef Int(3) Symmetry;

	Void set(Symmetry vector) { }
	Int test() {
	  TypedefIssue t;
	  t.set((Symmetry) 0);  // line 11
	  return 0;
	}
      }
*/

      bool rtn1 = fms->add("TypedefIssue.ulam","ulam 1;\n element TypedefIssue {\n // Types\n typedef Int(3) Symmetry;\nVoid set(Symmetry vector) { }\nInt test() {\nTypedefIssue t;\n t.set((Symmetry) 0);  // line 11\n return 0;\n}\n}\n");


      if(rtn1)
	return std::string("TypedefIssue.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3256_test_compiler_funccalltypedefarg_issue)

} //end MFM



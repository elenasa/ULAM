#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3501_test_compiler_elementandquarkcaarray_unsignedindex_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Foo { Bool(3) b(false);  typedef Bar Pop[2];  Int(4) m_i(0);  Bar m_bar2[2]( typedef Unsigned(6) Index;  Bool(1) val_b[3](false,false,false);  typedef Unsigned(6) Index;  Bool(1) val_b[3](false,false,false); );  Bar sbar( typedef Unsigned(6) Index;  Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Atom(96) a;  a sbar 4 cast [] = sbar 5 cast [] a = a sbar 2 cast [] = 0 cast return } }\nUq_Bar { typedef Unsigned(6) Index;  Bool(1) val_b[3](false,false,false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //./Foo.ulam:16:10: ERROR: Ambiguous matches (2) of custom array get function for argument type Int(3); Explicit casting required.
      //informed by 3223
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Bar;\n element Foo {\n typedef Bar Pop[2];\nBool(3) b;\n Int(4) m_i;\n Pop m_bar2;\nBar sbar;\n Bool check(Int v) {\n return true;\n }\n Int test() {\nAtom a;\n a = sbar[4];\nsbar[5] = a;\n a = sbar[(Unary(2))2];\nreturn 0;\n }\n }\n"); //tests offsets, but too complicated data members in Foo for memberselect

      //./Foo.ulam:16:10: ERROR: Ambiguous matches (2) of custom array get function for argument type Int(2); Explicit casting required.
      //./Foo.ulam:17:5: ERROR: Ambiguous matches (2) of custom array get function for argument type Int(2); Explicit casting required.
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n typedef Unsigned(6) Index;\n Bool val_b[3];\n  Void reset(Bool b) {\n b = false;\n }\n Atom aref(Index index){ Atom a; return a;\n}\n Atom aref(Unary(2) index){ Atom a; return a;\n}\n Void aset(Index index, Atom v) {\n return;\n}\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3501_test_compiler_elementandquarkcaarray_unsignedindex_issue)

} //end MFM

#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3148_test_compiler_elementmemberselect_returnarray)
  {
    std::string GetAnswerKey()
    {
      //code gen output:
      //Bool(3) Arg: 0x0 (false)
      //Bool(3) Arg: 0x0 (false)
      //Bool(3) Arg: 0x7 (true)
      //Bool(3) Arg: 0x7 (true)

      return std::string("Exit status: 0\nUe_Foo { typedef Bool(1) BA[6];  System s();  Bool(1) m_ba[6](false,true,false,true,false,true);  Int(32) test() {  Foo f;  f m_ba 0 [] . true cast = f m_ba . f ( 1 cast )check . = m_ba f m_ba . = s ( f m_ba 0 [] . cast )print . s ( m_ba 0 [] cast )print . s ( f m_ba 5 [] . cast )print . s ( m_ba 5 [] cast )print . 0 cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\n typedef Bool BA[6];\n BA m_ba;\n BA check(Int v) {\n BA rba;\n rba[1] = rba[3] = rba[5] = true;\n return rba;\n }\n Int test() {\n Foo f;\n f.m_ba[0] = true;\n f.m_ba = f.check(1);\n m_ba = f.m_ba;\n return 0;\n }\n }\n"); //2 basic member select tests: data member, func call

      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\n typedef Bool BA[6];\nSystem s;\n BA m_ba;\n BA check(Int v) {\n BA rba;\n rba[1] = rba[3] = rba[5] = true;\n return rba;\n }\n Int test() {\n Foo f;\n f.m_ba[0] = true;\n f.m_ba = f.check(1);\n m_ba = f.m_ba;\ns.print( (Bool(3)) f.m_ba[0]);\ns.print( (Bool(3)) m_ba[0]);\ns.print( (Bool(3)) f.m_ba[5]);\ns.print( (Bool(3)) m_ba[5]);\n return 0;\n }\n }\n"); //2 basic member select tests: data member, func call

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3148_test_compiler_elementmemberselect_returnarray)

} //end MFM



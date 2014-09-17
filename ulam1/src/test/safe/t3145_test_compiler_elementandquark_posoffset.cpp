#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3145_test_compiler_elementandquark_posoffset)
  {
    std::string GetAnswerKey()
    {
      //note: order of appearance of 'Bool b' first; not in parse-tree order to print their values.
      return std::string("Ue_Foo { Bool(1) b(false);  typedef Bar Pop[2];  Bool(1) a(false);  Bar m_bar1(0);  Int(32) m_i(7);  Bar m_bar2[2](0,0);  Bar m_bar3(0);  Int(32) test() {  Foo f;  f ( 1 )check . m_i 7 = m_i return } }\nExit status: 7");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      /* in Ud_0024613Foo.h:     
	 Ut_001114Bool Um_11a;
	 Uq_001313Bar<1> Um_16m_bar1;
	 Ut_0023213Int Um_13m_i;
	 Ut_121313Bar<36> Um_16m_bar2;
	 Ut_001114Bool Um_11b;
	 Uq_001313Bar<43> Um_16m_bar3;
      */

      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { typedef Bar Pop[2]; Bool a; Bar m_bar1; Int m_i; Pop m_bar2[2]; Bool b; Bar m_bar3; Bool check(Int v) { return true; } Int test() { Foo f; f.check(1); m_i = 7; return m_i; } }\n"); //tests offsets, but too complicated data members in Foo for memberselect
      
      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset(Bool b) { b = 0; } }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3145_test_compiler_elementandquark_posoffset)
  
} //end MFM



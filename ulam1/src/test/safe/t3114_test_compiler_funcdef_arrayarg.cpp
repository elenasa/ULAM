#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3114_test_compiler_funcdef_arrayarg)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) d(1);  Int(32) test() {  Bool(1) mybool[3];  mybool 0 [] true cast = mybool 1 [] false cast = mybool 2 [] false cast = d ( 7 cast mybool )m = d return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int m(Int m, Bool b[3]) { if(b[0]) m=1; else m=2; return m;} Int test() { Bool mybool[3]; mybool[0] = true; mybool[1] = false; mybool[2]= false; d = m(7, mybool); return d; } Int d; }");  // want d == 1.
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3114_test_compiler_funcdef_arrayarg)
  
} //end MFM



#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3124_test_compiler_typedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { typedef Int(32) Bar[2];  Int(32) d[2](1,0);  Int(32) test() {  Bool(1) mybool;  mybool true cast = d ( mybool )bar = d 0 [] return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { typedef Int Bar [2]; Bar bar(Bool b) { Bar m; if(b) m[0]=1; else m[0]=2; return m;} Int test() { Bool mybool; mybool= true; d = bar(mybool); return d[0]; /* match return type */} Bar d; }");  // want d[0] == 1.
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3124_test_compiler_typedef)
  
} //end MFM



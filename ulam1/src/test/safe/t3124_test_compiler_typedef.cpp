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

      //bool rtn1 = fms->add("A.ulam","element A { typedef Int Bar [2]; Bar bar(Bool b) { Bar m; if(b) m[0]=1; else m[0]=2; return m;} Int test() { Bool mybool; mybool= true; d = bar(mybool); return d[0]; /* match return type */} Bar d; }");  // want d[0] == 1.

      // needs newlines
      bool rtn1 = fms->add("A.ulam","element A {\n typedef Int Bar [2];\n Bar bar(Bool b) {\n Bar m;\n if(b)\n m[0]=1;\n else\n m[0]=2;\n return m;\n}\n Int test() {\n Bool mybool;\n mybool= true;\n d = bar(mybool);\n return d[0];\n /* match return type */}\n Bar d;\n }\n");  // want d[0] == 1.

      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3124_test_compiler_typedef)
  
} //end MFM



#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3200_test_compiler_quark_twoquarkscasttoint_compilable)
  {
    std::string GetAnswerKey()
    {
      //developed by Dave (10092014) in c++ and then converted to Ulam.
      return std::string("Ue_Foo { Bar bar1( Unsigned(2) x(1);  Unsigned(2) y(2); );  Bar bar2( Unsigned(2) x(3);  Unsigned(2) y(0); );  Int(4) i[2](6,12);  Int(32) test() {  bar1 x . 1 cast = bar1 y . 2 cast = bar2 x . 3 cast = bar2 y . 0 cast = i 0 [] bar1 ( )toInt . cast = i 1 [] bar2 ( )toInt . cast = 0 cast return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //return expression
      //note: don't have <<2, so substituted *4
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nUnsigned(2) x;\nUnsigned(2) y;\nInt toInt(){\nreturn (x * 4) + y;\n}\n}\n"); 

      //bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nUnsigned(2) x;\nUnsigned(2) y;\nInt toInt(){\nUnsigned u;\n u = (x * 4) + y;\nreturn u;\n}\n}\n"); //use variable

      // temporary! use a constant return value until binop + * are gen codeed.
      //bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nUnsigned(2) x;\nUnsigned(2) y;\nInt toInt(){\nreturn 4;\n}\n}\n");
      
      if(rtn2)
	return std::string("Bar.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3200_test_compiler_quark_twoquarkscasttoint_compilable)
  
} //end MFM



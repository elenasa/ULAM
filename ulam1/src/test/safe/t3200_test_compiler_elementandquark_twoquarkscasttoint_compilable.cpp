#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3200_test_compiler_elementandquark_twoquarkscasttoint_compilable)
  {
    std::string GetAnswerKey()
    {
      //developed by Dave (10092014) in c++ and then converted to Ulam.
      return std::string("Ue_Foo { Bar bar1( Unsigned(2) x(1);  Unsigned(2) y(2); );  Bar bar2( Unsigned(2) x(3);  Unsigned(2) y(0); );  Int(4) i[2](6,12);  Int(32) test() {  bar1 x . 1 cast = bar1 y . 2 cast = bar2 x . 3 cast = bar2 y . 0 cast = i 0 [] bar1 ( )toInt . cast = i 1 [] bar2 ( )toInt . cast = 0 cast return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //note: don't have printf's yet so comments for now
      // printf(\"%d\n\", (Int) bar1);\nprintf(\"%d\n\", (Int) bar2);\n STARS AND SLASHES OH MY!!!

      // fixed a bug that didn't address different int bit sizes automatically during casting
      // no support for arrays in simulator code yet..change i[2] to i and j.
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBar bar1;\nBar bar2;\nInt(4) i;\nInt(4) j; Int test(){\nbar1.x = 1;\nbar1.y = 2;\nbar2.x = 3;\nbar2.y = 0;\ni = bar1;\nj = bar2;\n\nreturn 0;}\n}\n");

      // now that we believe toInt works, let's simplify Foo (remove the Int data member) for code gen testing
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBar bar1;\nBar bar2;\nInt test(){\nbar1.x = 1;\nbar1.y = 2;\nbar2.x = 3;\nbar2.y = 0;\nreturn 0;}\n}\n");

      //note: don't have <<2, so substituted *4
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nUnsigned(2) x;\nUnsigned(2) y;\nInt toInt(){\nreturn (x * 4) + y;\n}\n}\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3200_test_compiler_elementandquark_twoquarkscasttoint_compilable)
  
} //end MFM



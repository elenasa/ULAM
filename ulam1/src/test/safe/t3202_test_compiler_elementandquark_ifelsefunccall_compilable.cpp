#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3202_test_compiler_elementandquark_ifelsefunccall_compilable)
  {
    std::string GetAnswerKey()
    {
      //developed by Dave (10092014) in c++ and then converted to Ulam.
      // with printf in NodePrograms enabled for this test:
      // Bar1 toInt = 2
      // Bar2 toInt = 0

      return std::string("Ue_Foo { Bar bar1( Bool(1) b(true);  Unsigned(3) x(1);  Unsigned(3) y(2); );  Bar bar2( Bool(1) b(false);  Unsigned(3) x(3);  Unsigned(3) y(0); );  Int(4) i(2);  Int(4) j(0);  Int(32) test() {  Int(32) d;  d 1 cast = bar1 ( d 2 cast )set . bar2 ( 3 cast 0 cast )set . i bar1 ( )toInt . cast = j bar2 ( )toInt . cast = 0 cast return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //note: don't have printf's yet so comments for now
      // printf(\"%d\n\", (Int) bar1);\nprintf(\"%d\n\", (Int) bar2);\n STARS AND SLASHES OH MY!!!

      // fixed a bug that didn't address different int bit sizes automatically during casting

      // now that we believe toInt works, let's simplify Foo (remove the Int data member) for code gen testing
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBar bar1;\nBar bar2;\nInt test(){\nbar1.x = 1;\nbar1.y = 2;\nbar2.x = 3;\nbar2.y = 0;\nreturn 0;}\n}\n");

      // use a variable for rhs of bar1
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBar bar1;\nBar bar2;\nInt(4) i, j;\nInt test(){\nInt(2) d;\nd = 1;\nbar1.x = d;\nbar1.y = 2;\nbar2.x = 3;\nbar2.y = 0;\ni = bar1;\nj = bar2;\n\nreturn 0;}\n}\n");
      
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBar bar1;\nBar bar2;\nInt(4) i, j;\nInt test(){\nInt d;\nd = 1;\nbar1.set(d,2);\nbar2.set(3,0);\ni = bar1;\nj = bar2;\n\nreturn 0;}\n}\n");

      // use set function; test bool to avoid divide by zero
      //note: don't have <<2, so substituted *4; use 3 bits so not to cross word boundary
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool b;\nUnsigned(3) x, y;\nInt toInt(){\nif(b)\nreturn (x * 4) / y;\nelse\nreturn 0;\n}\nVoid set(Int xarg, Int yarg){\nx=xarg;\ny=yarg;\nif(yarg){\nb=true;\n}\nelse{\nb=false;\n}\n}\n}\n");

      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3202_test_compiler_elementandquark_ifelsefunccall_compilable)
  
} //end MFM



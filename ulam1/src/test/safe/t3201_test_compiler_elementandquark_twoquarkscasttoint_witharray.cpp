#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3201_test_compiler_elementandquark_twoquarkscasttoint_witharray)
  {
    std::string GetAnswerKey()
    {
      //developed by Dave (10092014) in c++ and then converted to Ulam.
      // 7 is max that Int(4) can be positively, so 12 becomes 7.
      //Ue_Foo { Bar bar1( Unsigned(2) x(1);  Unsigned(2) y(2); );  Bar bar2( Unsigned(2) x(3);  Unsigned(2) y(0); );  Int(4) i[2](6,7);  Int(32) test() {  Int(2) d;  d 1 cast = bar1 x . d cast = bar1 y . 2 cast = bar2 x . 3 cast = bar2 y . 0 cast = i 0 [] bar1 ( )toInt . cast = i 1 [] bar2 ( )toInt . cast = i 1 [] cast return } }\nExit status: 7
      return std::string("Ue_Foo { System s();  Bar bar1( Bool(1) b(false);  Unsigned(3) x(1);  Unsigned(3) y(2); );  Bar bar2( Bool(1) b(false);  Unsigned(3) x(3);  Unsigned(3) y(0); );  Int(4) i[2](6,7);  Int(32) test() {  Int(2) d;  d 1 cast = bar1 x . d cast = bar1 y . 2 cast = bar2 x . 3 cast = bar2 y . 0 cast = i 0 [] bar1 ( )toInt . cast = i 1 [] bar2 ( )toInt . cast = s ( bar1 x . cast )print . s ( bar1 y . cast )print . s ( bar2 x . cast )print . s ( bar2 y . cast )print . s ( i 0 [] )print . s ( i 1 [] )print . i 1 [] cast return } }\nExit status: 7");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //note: don't have printf's yet so comments for now
      // printf(\"%d\n\", (Int) bar1);\nprintf(\"%d\n\", (Int) bar2);\n STARS AND SLASHES OH MY!!!

      // fixed a bug that didn't address different int bit sizes automatically during casting

      // test with array data member (lhs), and return array item[1]. [currently not supported by simulator]
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Bar;\nelement Foo {\nSystem s;\nBar bar1;\nBar bar2;\nInt(4) i[2];\nInt test(){\nInt(2) d;\nd = 1;\nbar1.x = d;\nbar1.y = 2;\nbar2.x = 3;\nbar2.y = 0;\ni[0] = bar1;\ni[1] = bar2;\ns.print((Unsigned) bar1.x);\ns.print((Unsigned) bar1.y);\ns.print((Unsigned) bar2.x);\ns.print((Unsigned) bar2.y);\n\ns.print(i[0]);\ns.print(i[1]);\n\nreturn i[1];}\n}\n");

      //note: don't have <<2, so substituted *4
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool b;\nUnsigned(3) x, y;\nInt toInt(){\nreturn (x * 4) + y;\n}\n}\n"); 

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      
      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3201_test_compiler_elementandquark_twoquarkscasttoint_witharray)
  
} //end MFM



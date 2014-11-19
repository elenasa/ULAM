#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3203_test_compiler_elementandquark_funccallargexpression)
  {
    std::string GetAnswerKey()
    {
      //Ue_Foo { Bar bar1( Bool(1) b(true);  Unsigned(3) x(1);  Unsigned(3) y(2); );  Bar bar2( Bool(1) b(false);  Unsigned(3) x(3);  Unsigned(3) y(0); );  Int(4) i(2);  Int(4) j(7);  Int(32) test() {  Int(32) d;  d 1 cast = bar1 ( d 2 cast )set . bar2 ( 3 cast 0 cast )set . i bar1 ( )toInt . cast = j bar2 ( )toInt . cast = j ( i cast 7 cast +b )update cast = 0 cast return } }\nExit status: 0

      //developed by Dave (10092014) in c++ and then converted to Ulam.
      return std::string("Ue_Foo { System s();  Bar bar1( Bool(1) b(true);  Unsigned(3) x(1);  Unsigned(3) y(2); );  Bar bar2( Bool(1) b(false);  Unsigned(3) x(3);  Unsigned(3) y(0); );  Int(4) i(2);  Int(4) j(7);  Int(32) test() {  Int(32) d;  d 1 cast = bar1 ( d 2 cast )set . bar2 ( 3 cast 0 cast )set . i bar1 ( )toInt . cast = s ( i )print . j bar2 ( )toInt . cast = s ( j )print . j ( i cast 7 cast +b )update = s ( j )print . 0 cast return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {      
      //   bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBar bar1;\nBar bar2;\nInt(4) i, j;\nInt update(Int x)\n{\nreturn x;\n}\nInt test(){\nInt d;\nd = 1;\nbar1.set(d,2);\nbar2.set(3,0);\ni = bar1;\nj = bar2;\nj = update(i + 7);\n\nreturn 0;}\n}\n");

   bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Bar;\nelement Foo {\nSystem s;\nBar bar1;\nBar bar2;\nInt(4) i, j;\nInt(4) update(Int x)\n{\nreturn x;\n}\nInt test(){\nInt d;\nd = 1;\nbar1.set(d,2);\nbar2.set(3,0);\ni = bar1;\ns.print(i);\nj = bar2;\ns.print(j);\nj = update(i + 7);\ns.print(j);\nreturn 0;}\n}\n");

      // use set function; test bool to avoid divide by zero
      //note: don't have <<2, so substituted *4; use 3 bits so not to cross word boundary
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool b;\nUnsigned(3) x, y;\nInt toInt(){\nif(b)\nreturn (x * 4) / y;\nelse\nreturn 0;\n}\nVoid set(Int xarg, Int yarg){\nx=xarg;\ny=yarg;\nif(yarg){\nb=true;\n}\nelse{\nb=false;\n}\n}\n}\n");


      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     


      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3203_test_compiler_elementandquark_funccallargexpression)
  
} //end MFM



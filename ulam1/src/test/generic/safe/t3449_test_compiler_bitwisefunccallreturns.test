#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3449_test_compiler_bitwisefunccallreturns)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 5\nUe_T { typedef Bits(7) T;  typedef Unary(7) B;  Unary(7) m(5);  Unary(7) n(2);  Unary(7) r(5);  Unary(7) a(2);  Unary(7) h(3);  Int(32) test() {  m ( )func1 = n ( )func2 = r ( )func1 cast ( )func2 cast | cast = a ( )func1 cast ( )func2 cast & cast = h ( )func1 cast ( )func2 cast ^ cast = r cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //explicit cast to Bits(7), T.
      //./T.ulam:15:14: ERROR: Bits is the supported type for bitwise operator&; Suggest casting Unary(7) and Unary(7) to Bits(7).
      // unary: bitwise-or is the max, bitwise-and is the min, bitwise-xor is the (max - min)
      // trying funcs return octal (couldn't do bit values).
      bool rtn2 = fms->add("T.ulam"," ulam 1;\n element T{\ntypedef Unary(7) B;\n typedef Bits(7) T;\n B m,n,r,a,h;\n B func1() {\n return '\\5';\n}\n B func2() {\n return '\\2';\n}\n Int test(){\nm = func1();\n n = func2();\n r =  (B) ((T) func1() | (T) func2());\n a =  (B) ((T)func1() & (T)func2());\nh = (B) ((T) func1() ^ (T) func2());\n return r;\n}\n }\n");

      if(rtn2)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3449_test_compiler_bitwisefunccallreturns)

} //end MFM

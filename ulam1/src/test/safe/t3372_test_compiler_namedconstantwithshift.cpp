#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3372_test_compiler_namedconstantwithshift)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x7 (true)
       */
      //simplified case answer
      //Exit status: 4\nUe_A { Bool(3) b(false);  System s();  Int(32) test() {  Counter(3u) c;  c ( )thanWhat . cast return } }\nUq_System { <NOMAIN> }\nUq_Counter { constant Unsigned(32) bits = NONREADYCONST;  typedef Unsigned(UNKNOWN) Count;  <NOMAIN> }

      //different casts since Constants have explicit types
      return std::string("Exit status: 4\nUe_A { Bool(3) b(true);  System s();  Int(32) test() {  Counter(3u) c;  b c ( 0u cast 1u cast )isEarlier . = s ( b )print . c ( )thanWhat . cast return } }\nUq_System { <NOMAIN> }\nUq_Counter { constant Unsigned(32) bits = NONREADYCONST;  typedef Unsigned(UNKNOWN) Count;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nuse Counter;\nelement A{\nSystem s;\nBool(3) b;\nInt test () {\nCounter(3u) c;\n b = c.isEarlier(0u, 1u);\ns.print(b);\n return  c.thanWhat();\n}\n}\n");

      bool rtn2 = fms->add("Counter.ulam", "quark Counter(Unsigned bits) {\n  typedef Unsigned(bits) Count;\nBool(3) isEarlier(Count a, Count b) {\n if (a < b)\n return (b - a) < (Count) ((Bits(bits)) 1u<<(bits-1));\n return (a - b) > (Count) ((Bits(bits)) 1u<<(bits-1));\n  }\n Count thanWhat() {\nreturn (Count) ((Bits(bits)) 1u<<(bits-1));\n  }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
      	return std::string("A.ulam");


      //simplify #1 for testing: is it the namedconstantwithshiftbits?
      //bool rtn1 = fms->add("A.ulam","use Counter;\nelement A{\nBool(3) b;\nInt test () {\nCounter(3u) c;\n return  c.thanWhat();\n}\n}\n");
      //bool rtn2 = fms->add("Counter.ulam", "quark Counter(Unsigned bits) {\n  typedef Unsigned(bits) Count;\nCount thanWhat() {\nreturn (1u<<(bits-1));\n  }\n }\n");


      //simplify #2 for testing: compare with RHS constant fold, missing parentnode.
      //bool rtn1 = fms->add("A.ulam","use Counter;\nelement A{\nBool(3) b;\nInt test () {\nCounter(3u) c;\n b = c.isEarlier(0u, 1u);\n return  0;\n}\n}\n");
      //bool rtn2 = fms->add("Counter.ulam", "quark Counter(Unsigned bits) {\n  typedef Unsigned(bits) Count;\nBool(3) isEarlier(Count a, Count b) {\n /*if (a < b)\n return (b - a) < (1u<<(bits-1));\n*/ return /*(a - b) > (1u<<(bits-1))*/ a > (bits - 1);\n  }\n }\n");

      //if(rtn1 && rtn2)
      //	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3372_test_compiler_namedconstantwithshift)

} //end MFM

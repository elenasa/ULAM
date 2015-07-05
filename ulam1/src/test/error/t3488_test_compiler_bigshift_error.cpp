#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3488_test_compiler_bigshift_error)
  {
    std::string GetAnswerKey()
    {
      //debug output for TARGETMAP and PARAMETERMAP:
      //Size of target map is 3
      //ULAM INFO: TARGET ./A.ulam:4:10: A Ue_102321A10 32 test element /**%20This%20Is%20A%20Structured%20Comment.%20*/
      //ULAM INFO: TARGET ./B.ulam:1:30: B Uq_10101B11102321i11 0 notest quark /**%20This%20is%20quark%20B%20*/
      //ULAM INFO: TARGET ./B.ulam:1:30: B Uq_10101B11102321i12 0 notest quark /**%20This%20is%20quark%20B%20*/
      //Size of model parameter map is 2
      //ULAM INFO: PARAMETER ./A.ulam:10:38: Ut_10131i Uc_Ue_102321A10_Up_3m_p 0xffffffffffffffff /**This%20is%20a%20%0amodel%20parameter:%20*/
      //ULAM INFO: PARAMETER ./A.ulam:12:51: Ut_10131u Uc_Ue_102321A10_Up_4m_p2 0x0 /**2nd%20model%20parameter:%20*/

      /*
	./A.ulam:5:22: Warning: Shift distance greater than data width, operator<<.
	./A.ulam:6:18: Warning: Shift distance greater than data width, operator<<.
	./A.ulam:7:23: Warning: Shift distance greater than data width, operator>>.
	./A.ulam:8:18: Warning: Shift distance greater than data width, operator>>.
      */

      // regardless signed or unsigned
      //note: g++ shift == 32 produces 1 << 32 = 1, 1 >> 32 = 0; for ulam: 1 << 32 = 0; 1 >> 32 = 0; DIFFERENT!
      //note: g++ shift == 33 produces 1 << 33 = 2, 1 >> 33 = 0; for ulam: 1 << 33 = 0; 1 >> 33 = 0; DIFFERENT!
      return std::string("Exit status: 0\nUe_A { Unsigned(8) d(0);  Unsigned(8) f(0);  Int(8) e(0);  Int(8) g(0);  Int(32) test() {  32u = shift const d 0u cast = e 0u cast = f 1u cast = g 1u cast = 0 return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3480
      bool rtn1 = fms->add("A.ulam","ulam 1;\n use B;\n /** This Is A Structured Comment. */\n element A {\n B(1) b;\n B(2) b2;\n Unsigned(8) d,f;\nInt(8) e,g;\n /**This is a \nmodel parameter: */ parameter Int(3) m_p = -1;\n /**fake out */\n /**2nd model parameter: */ parameter Unsigned(3) m_p2 = 0;\n Int test(){constant Unsigned shift = 32;\nd = (Unsigned(8)) (1 << shift);\n e = (Int(8)) (1 << shift);\n f = (Unsigned(8)) (1 >> shift);\n g = (Int(8)) (1 >> shift);\n return 0;\n }\n }\n");

      bool rtn2 = fms->add("B.ulam", "/** This is quark B */ quark B(Int a){\n }\n");

      if(rtn1 && rtn2)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3488_test_compiler_bigshift_error)

} //end MFM

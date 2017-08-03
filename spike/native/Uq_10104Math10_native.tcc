/**                                      -*- mode:C++ -*- */

#include <stdio.h>
#include <stdlib.h>
#include "Util.h"
#include "UlamTypeInfo.h"
namespace MFM{

  // returns immediate 32-bit Int, value of max of var args; no args returns S32_MIN;
  // casts and/or sign extends as required; fails upon illegal input.
  // note: uses UlamTypeInfo; see tests t3250, t41099
  template<class CC>
  Ui_Ut_102321i<CC> Uq_10104Math10<CC>::Uf_3max(const UlamContext<CC>& uc, UlamRef<CC>& ur, ...) const //native
  {
    va_list ap;
    va_start(ap, ur);
    s32 max = S32_MIN;
    u32 argnum = 1;
    while(true)
      {
	BitStorage<CC> * stgptr = va_arg(ap, BitStorage<CC> *);
	if(!stgptr) break;

	u32 width = stgptr->GetBitSize();
	if(width > 32)
	  FAIL(UNSUPPORTED_OPERATION);

	//depending on type, extend or cast raw value
	const char * amangledtypename = stgptr->GetUlamTypeMangledName();
	printf("arg %d type name: <%s>\n", argnum, amangledtypename);

	UlamTypeInfo utin;
        if (!utin.InitFrom(amangledtypename)) FAIL(ILLEGAL_STATE);

	if (utin.m_category != UlamTypeInfo::PRIM) FAIL(ILLEGAL_ARGUMENT);

	if(utin.GetBitSize() != width) FAIL(ILLEGAL_STATE);

	if(utin.GetArrayLength() > 0) FAIL(ILLEGAL_ARGUMENT);

	//for sanity checking
	u32 namelen = strlen(amangledtypename);
	if(namelen == 0) FAIL(ILLEGAL_STATE);
	char atype = amangledtypename[namelen-1];

	s32 cval = 0;
	UlamTypeInfoPrimitive::PrimType primType = utin.m_utip.GetPrimType();

	if(atype != UlamTypeInfoPrimitive::CharFromPrimType(primType))
	  FAIL(ILLEGAL_STATE); //sanity

	u32 val = stgptr->Read(0, width); //raw value


	switch (primType)
	  {
	  case UlamTypeInfoPrimitive::INT:
	    {
	      cval = _SignExtend32(val, width);
	      break;
	    }
	  case UlamTypeInfoPrimitive::UNSIGNED:
	    {
	      cval = _Unsigned32ToCs32(val, width);
	      break;
	    }
	  case UlamTypeInfoPrimitive::UNARY:
	    {
	      cval = _Unary32ToInt32(val, width, 32);
	      break;
	    }
	  case UlamTypeInfoPrimitive::BOOL:
	  case UlamTypeInfoPrimitive::BITS:
	  case UlamTypeInfoPrimitive::VOID:
	  case UlamTypeInfoPrimitive::STRING:
	  default:
	    FAIL(ILLEGAL_STATE);
	  }

	if(cval > max) max = cval;
	argnum++;
      } //end while

    va_end(ap);
    return Ui_Ut_102321i<CC>(max); //default Int size is still 32 (see ULAM's UlamType.inc)
  }

} //MFM

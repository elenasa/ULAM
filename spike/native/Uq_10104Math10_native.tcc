/**                                      -*- mode:C++ -*- */

#include <stdio.h>
#include <stdlib.h>
#include "Util.h"
namespace MFM{

  template<class CC>
  Ui_Ut_102321i<CC> Uq_10104Math10<CC>::Uf_3max(const UlamContext<CC>& uc, UlamRef<CC>& ur, ...) const //native
  {
    va_list ap;
    va_start(ap, ur);
    s32 max = S32_MIN;
    u32 argnum = 1;
    while(true)
      {
	BitStorage<CC> * aptr = va_arg(ap, BitStorage<CC> *);
	if(!aptr) break;

	u32 width = aptr->GetBitSize();
	if(width > 32)
	  FAIL(UNSUPPORTED_OPERATION);

	u32 au = aptr->Read(0, width); //raw value

	//depending on type, extend or cast raw value
	const char * amangledname = aptr->GetUlamTypeMangledName();
	printf("arg %d type name: <%s>\n", argnum, amangledname);
	u32 namelen = strlen(amangledname);
	if(namelen == 0)
	  FAIL(ILLEGAL_STATE);
	char atype = amangledname[namelen-1];

	s32 a = 0;
	switch(atype)
	  {
	  case 'i':
	    a = _SignExtend32(au, width);
	    break;
	  case 'u':
	    a = _Unsigned32ToCs32(au, width);
	    break;
	  case 'y':
	    a = au;
	    break;
	  default:
	    FAIL(ILLEGAL_ARGUMENT);
	  };

	if(a > max) max = a;
	argnum++;
      } //end while

    va_end(ap);
    return Ui_Ut_102321i<CC>(max);
  }

} //MFM

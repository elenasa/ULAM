#include <assert.h>
#include <sstream>
#include "StringPoolUser.h"


namespace MFM {

  StringPoolUser::StringPoolUser() : StringPool(""), m_runningIndex(1) {}

  StringPoolUser::~StringPoolUser() {}

  u32 StringPoolUser::getIndexForDataString(std::string str)
  {
    u32 idx;   // index to data in maps
    std::map<std::string,u32>::iterator it = m_stringToDataIndex.find(str);

    if(it != m_stringToDataIndex.end())
      {
	idx = it->second;  //reuse existing identifier index
      }
    else
      {
	u32 slen = str.length();
	assert(slen < 256); //or return 0?
	idx = m_runningIndex;

	std::ostringstream newstr;
	if(slen == 0)
	  {
	    m_runningIndex += 1; //null string, len only
	    newstr << 0; //for null string
	  }
	else
	  {
	    m_runningIndex += (slen + 2); //add byte for len at start; end with null byte;
	    newstr << slen << str << 0; //for new string
	  }

	m_stringToDataIndex.insert(std::pair<std::string,u32> (newstr.str(), idx));
	m_dataAsString.insert(std::pair<u32, std::string> (idx, newstr.str()));
      }
    return idx;
  } //getIndexForDataString

  const std::string & StringPoolUser::getDataAsString(u32 dataindex)
  {
    std::map<u32,std::string>::iterator it = m_dataAsString.find(dataindex);
    assert(it != m_dataAsString.end());
    return it->second;
  }


} //MFM

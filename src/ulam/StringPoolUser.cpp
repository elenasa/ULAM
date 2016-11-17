#include <assert.h>
#include <sstream>
#include "StringPoolUser.h"
#include "CompilerState.h"

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
	u32 slen = str[0]; //len in first byte set by lexer
	idx = m_runningIndex;

	if(slen == 0)
	  m_runningIndex += 1; //null string, len only
	else
	  m_runningIndex += slen + 2; //add byte for len at start; end with null byte;

	m_stringToDataIndex.insert(std::pair<std::string,u32> (str, idx));
	m_dataAsString.insert(std::pair<u32, std::string> (idx, str));
      }
    return idx;
  } //getIndexForDataString

  u32 StringPoolUser::getIndexForNumberAsString(u32 num)
  {
    std::ostringstream nstr;
    nstr << num;
    std::string numstr = nstr.str();

    u32 nlen = numstr.length();
    assert(nlen < 256);

    std::ostringstream newnstr;
    newnstr << nlen << numstr << 0;
    return getIndexForDataString(newnstr.str());
  }

  u32 StringPoolUser::getIndexForDataAsFormattedString(u32 dataindex, CompilerState * state)
  {
    std::map<u32,std::string>::iterator it = m_dataAsString.find(dataindex);
    assert(it != m_dataAsString.end());
    assert(state);
    return formatDoubleQuotedString(it->second, state);
  }

  const std::string & StringPoolUser::getDataAsString(u32 dataindex)
  {
    std::map<u32,std::string>::iterator it = m_dataAsString.find(dataindex);
    assert(it != m_dataAsString.end());
    return it->second;
  }

  const std::string & StringPoolUser::getDataAsFormattedString(u32 dataindex, CompilerState * state)
  {
    u32 sindex = getIndexForDataAsFormattedString(dataindex, state);
    return state->m_pool.getDataAsString(sindex);
  }

  u32 StringPoolUser::getStringLength(u32 dataindex)
  {
    const std::string & strref = getDataAsString(dataindex);
    if(strref.length() == 0)
      return 0;
    return strref[0] - '0';
  }

  u32 StringPoolUser::formatDoubleQuotedString(const std::string& str, CompilerState * state)
  {
    if(str.length() == 0)
      return state->m_pool.getIndexForDataString("");

    std::ostringstream newstr;
    u32 slen = str[0] - '0';
    newstr << "\""; //open quote (no escape)
    for(u32 i = 1; i <= slen; i++)
      {
	u8 c = str[i];
	if((c == '"') || (c == '\\')) //embedded dbl quote, or backslash
	  newstr << "\\" << c; //requires escape
	else if(isgraph(c)) //any printable char except space
	  newstr << c; //raw
	else
	  newstr << "\\" << c; //e.g. bell '\a', requires escape
      }
    newstr << "\""; //close quote (no escape)
    assert(state);
    return state->m_pool.getIndexForDataString(newstr.str());
  }

} //MFM

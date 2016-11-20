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
	u32 slen = str[0] - '0'; //len in first byte set by lexer
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

  u8 StringPoolUser::getByteOf(u32 dataindex, u32 offset)
  {
    const std::string & strref = getDataAsString(dataindex);
    u32 slen = strref.length();
    assert(slen > offset);
    return strref[offset+1]; //skip len
  }

  s32 StringPoolUser::getUserStringPoolCount()
  {
    return m_dataAsString.size() - 1;
  }

  void StringPoolUser::generateUserStringPoolEntries(File * fp, CompilerState * state)
  {
    assert(state);

    //note: not using C++ String because that uses malloc;
    // double quoted strings next to each other get merged
    state->indent(fp);
    fp->write("const unsigned char ");
    fp->write(state->getMangledNameForUserStringPool());
    fp->write("[] = "); GCNL;

    state->m_currentIndentLevel++;

    std::map<u32,std::string>::iterator it = m_dataAsString.begin(); //ascending order by default
    while(it != m_dataAsString.end())
      {
	state->indent(fp);

	writeDoubleQuotedString(fp, it->second);
	fp->write("\n");
	it++;
      }
    fp->write(";"); GCNL;

    state->m_currentIndentLevel--;

    fp->write("\n");

    state->indent(fp);
    fp->write("#define ");
    fp->write(state->getDefineNameForUserStringPoolCount());
    fp->write(" ");
    fp->write_decimal(getUserStringPoolCount()); GCNL;
    fp->write("\n");

    return;
  } //generateUserStringPoolEntries

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

  void StringPoolUser::writeDoubleQuotedString(File * fp, const std::string& str)
  {
    if(str.length() == 0)
      {
	writeOpenCloseDblQuote(fp);
	writeNullByte(fp);
	writeOpenCloseDblQuote(fp);
	return;
      }

    u32 slen = str[0] - '0';

    if(slen == 0)
      {
	writeOpenCloseDblQuote(fp);
	writeNullByte(fp);
	writeOpenCloseDblQuote(fp);
	return;
      }

    writeOpenCloseDblQuote(fp);
    writeDblQuotedChar(fp, slen);
    for(u32 i = 1; i <= slen; i++)
      {
	u8 c = str[i];
	writeDblQuotedChar(fp, c);
      }
    writeNullByte(fp);
    writeOpenCloseDblQuote(fp);
  }

  void StringPoolUser::writeDblQuotedChar(File * fp, u8 c)
  {
    if(c == '"' || c == '\\')
      {
	char tmp[8*2+1];
	sprintf(tmp,"\\%c",c);
	fp->write(tmp);
      }
    else
      writeEscaped(fp, c);
  }

  void StringPoolUser::writeRawChar(File * fp, u8 c)
  {
    char tmp[8+1];
    sprintf(tmp, "%c", c);
    fp->write(tmp);
  }

  void StringPoolUser::writeEscaped(File * fp, u8 c)
  {
    if(!isgraph(c))
      {
	char tmp[8*3+1];
	sprintf(tmp,"\\%03o",c);
	fp->write(tmp);
      }
    else
      writeRawChar(fp, c);
  }

  void StringPoolUser::writeOpenCloseDblQuote(File * fp)
  {
    fp->write("\""); //no printed escape
  }

  void StringPoolUser::writeNullByte(File * fp)
  {
    writeEscaped(fp, '\0');
  }

} //MFM

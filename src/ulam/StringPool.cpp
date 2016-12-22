#include <assert.h>
#include <sstream>
#include "StringPool.h"


namespace MFM {

  StringPool::StringPool()
  {
    std::string dummy = "dummy data";
    m_dataAsString.insert(std::pair<u32, std::string> (0, dummy)); //one is taken.
  }

  StringPool::StringPool(std::string str)
  {
    //index 0 is unitialized; not in m_stringToDataIndex
    m_dataAsString.insert(std::pair<u32, std::string> (0, str)); //first one is now taken.
  }

  StringPool::StringPool(const StringPool& spref) : m_dataAsString(spref.m_dataAsString), m_stringToDataIndex(spref.m_stringToDataIndex) {}

  StringPool::~StringPool()
  {
    m_dataAsString.clear();
    m_stringToDataIndex.clear(); //both? strings already destructed.
  }

  u32 StringPool::getIndexForDataString(std::string str)
  {
    u32 idx; //index to data in maps
    std::map<std::string,u32>::iterator it = m_stringToDataIndex.find(str);

    if(it != m_stringToDataIndex.end())
      {
	idx = it->second; //reuse existing identifier index
      }
    else
      {
	idx = m_stringToDataIndex.size() + 1;
	assert(idx == m_dataAsString.size());
	m_stringToDataIndex.insert(std::pair<std::string,u32> (str, idx));
	m_dataAsString.insert(std::pair<u32, std::string> (idx, str));
      }
    return idx;
  } //getIndexForDataString

  u32 StringPool::getIndexForNumberAsString(u32 num)
  {
    std::ostringstream nstr;
    nstr << num;
    return getIndexForDataString(nstr.str());
  }

  const std::string & StringPool::getDataAsString(u32 dataindex)
  {
    assert(m_dataAsString.size() > dataindex && dataindex > 0);
    std::map<u32,std::string>::iterator it = m_dataAsString.find(dataindex);

    assert(it != m_dataAsString.end());
    return it->second;
  }

} //MFM

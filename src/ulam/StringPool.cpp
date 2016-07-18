#include <assert.h>
#include <sstream>
#include "StringPool.h"


namespace MFM {

  StringPool::StringPool()
  {
    std::string dummy = "dummy data";
    m_dataAsString.push_back(dummy); //one is taken.
  }

  StringPool::~StringPool()
  {
    m_dataAsString.clear();
    m_stringToDataIndex.clear(); //both? strings already destructed.
  }

  u32 StringPool::getIndexForDataString(std::string str)
  {
    u32 idx;   // index to data in map, vector
    std::map<std::string,u32>::iterator it = m_stringToDataIndex.find(str);

    if(it != m_stringToDataIndex.end())
      {
	idx = it->second;  //reuse existing identifier index
      }
    else
      {
	idx = m_stringToDataIndex.size() + 1;
	assert(idx == m_dataAsString.size());
	m_stringToDataIndex.insert(std::pair<std::string,u32> (str,idx));
	m_dataAsString.push_back(str);
      }
    return idx;
  } //getIndexForDataString

  u32 StringPool::getIndexForNumberAsString(u32 num)
  {
    std::ostringstream nstr;
    nstr << num;
    return getIndexForDataString(nstr.str());
  }

  std::string StringPool::getDataAsString(u32 dataindex)
  {
    assert(m_dataAsString.size() > dataindex && dataindex > 0);
    return m_dataAsString[dataindex];
  }


} //MFM

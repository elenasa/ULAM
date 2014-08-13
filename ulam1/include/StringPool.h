/* -*- c++ -*- */

#ifndef STRINGPOOL_H
#define STRINGPOOL_H

#include <stdio.h>
#include <string>
#include <string.h>
#include <map>
#include <vector>
#include "itype.h"


namespace MFM
{

  class StringPool
  {
  public:

    StringPool();

    ~StringPool();

    u32 getIndexForDataString(std::string str);    //< makes a new entry in map, vector if nonexistent
    std::string getDataAsString(u32 dataindex);

  private: 

    std::vector<std::string> m_dataAsString;       //< string indexed by dataindex
    std::map<std::string,u32> m_stringToDataIndex; //< value indexes into m_dataAsString; avoid duplicates


  };
}

#endif  /* STRINGPOOL_H */

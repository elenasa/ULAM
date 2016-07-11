#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <map>
#include <iterator>

#define MAX_TYPENAME_LEN 15

struct UlamTypeKey
{
  char m_typeName[MAX_TYPENAME_LEN + 1];
  int  m_bits;
  int m_arraySize;
  
  void init(const char * name, int bitsize, int arraysize)
  {
    m_bits = bitsize;
    m_arraySize = arraysize; 
    strncpy(m_typeName, name, MAX_TYPENAME_LEN);
    m_typeName[MAX_TYPENAME_LEN] = '\0';
  }
};

  
struct less_than_key
{
  inline bool operator() (const UlamTypeKey key1, const UlamTypeKey key2)
  {    
    if(strcmp(key1.m_typeName, key2.m_typeName) < 0) return true;
    if(key1.m_bits < key2.m_bits) return true;
    if(key1.m_arraySize < key2.m_arraySize) return true;
    return false;
  }
};




int main()
{

  UlamTypeKey key1;
  key1.init("Int", 32, 0);
  UlamTypeKey key2;
  key2.init("Int", 32, 1);


  std::map<UlamTypeKey, int, less_than_key> m_map;   //key -> address of ulamtype 
  //std::map<UlamTypeKey&, int> m_map;   //key -> address of ulamtype 

  //m_map.insert(key1,1);
  //m_map.insert(key2,2);
  m_map[key1] = 10;
  m_map[key2] = 1;

  //std::map<UlamTypeKey, int>::key_compare keycomp = m_map.key_comp();
  //UlamTypeKey highest = m_map.rbegin()->first;     // key value of last element
  
  std::map<UlamTypeKey,int, less_than_key >::iterator it = m_map.find(key2);
    
  if(it != m_map.end())
    {
      std::cout << " found it! " << it->second << std::endl;  
    }
  else
    {
      std::cout << " NOT found! "  << std::endl;  
    }

  return 0;
}

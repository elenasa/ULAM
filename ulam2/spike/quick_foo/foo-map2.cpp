#include <algorithm>
#include <iostream>
#include <map>
#include <string.h>
 
using namespace std;

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
  

 
template<class T>
struct PairsOutputter
{
void operator()( const T& p) {
  cout << "{" << p.first <<"=>" << p.second << "} ";
}
};

template<class T>
struct KeysOutputter
{
void operator()( const T& p) {
  cout << "{" << p.first.m_typeName <<"=>" << p.second << "} ";
}
};

 
bool KeyDescComparator (char key1, char key2) {return key1>key2;}
 
  bool less_than_key(const UlamTypeKey key1, const UlamTypeKey key2)
  {    
    if(strcmp(key1.m_typeName, key2.m_typeName) < 0) return true;
    if(key1.m_bits < key2.m_bits) return true;
    if(key1.m_arraySize < key2.m_arraySize) return true;
    return false;
  }


int main()
{
  UlamTypeKey key1;
  key1.init("Int", 32, 10);
  UlamTypeKey key2;
  key2.init("Int", 32, 1);
  UlamTypeKey key3;
  key3.init("Bool", 8, 0);

  PairsOutputter< pair<char,int> > outputter;
  KeysOutputter< pair<UlamTypeKey,int> > keyoutputter;
 
  bool(*fn_pt)(char,char) = KeyDescComparator; 
  map<char,int,bool(*)(char,char)> m1(fn_pt);

  bool(*fn_pt2)(const UlamTypeKey, const UlamTypeKey) = less_than_key;
  std::map<UlamTypeKey, int, bool(*) (const UlamTypeKey, const UlamTypeKey)> m_map(fn_pt2);   //key -> address of ulamtype 



 m1['a'] = 1;
 m1['k'] = 2;
 m1['p'] = 3;
 
 m_map[key1] = 10;
 m_map[key2] = 30;
 m_map[key3] = 1;

 for_each(m1.begin(), m1.end(), outputter);
 cout << endl;

 for_each(m_map.begin(), m_map.end(), keyoutputter); 
 

  std::map<UlamTypeKey, int, bool(*) (const UlamTypeKey, const UlamTypeKey) >::iterator it = m_map.find(key3);

  if(it != m_map.end())
    {
      std::cout << " found it! " << it->second << std::endl;  
    }
  else
    {
      std::cout << " NOT found! " << std::endl;  
    }


return 0;
}

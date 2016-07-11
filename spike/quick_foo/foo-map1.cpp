#include <algorithm>
#include <iostream>
#include <map>
 
using namespace std;
 
template<class T>
struct PairsOutputter
{
void operator()( const T& p) {
  cout << "{" << p.first <<"=>" << p.second << "} ";
}
};
 
bool KeyDescComparator (char key1, char key2) {return key1>key2;}
 
int main()
{
  PairsOutputter< pair<char,int> > outputter;
 
  bool(*fn_pt)(char,char) = KeyDescComparator;
 
  map<char,int,bool(*)(char,char)> m1(fn_pt);
 m1['a'] = 1;
 m1['k'] = 2;
 m1['p'] = 3;
 
 for_each(m1.begin(), m1.end(), outputter);
 cout << endl;
 
return 0;
}

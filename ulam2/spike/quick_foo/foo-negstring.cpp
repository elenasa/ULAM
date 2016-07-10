//Thu May 29 08:38:56 2014 
#include <iostream>       // std::cout
#include <string>         // std::string


int atoi(char c)
{
  return c - '0';
}


int main ()
{
  std::string str1 ("foo\377oops");

  char c = (unsigned char) str1[3];  //no diff with cast
  int i = str1[3];
  int m = (char) str1[3] & 0xFF;
  int u = (unsigned char) str1[3];

  if(-1 == u)

    std::cout << " i = " << i << " and c = " << c << " and masked i is " << m << ", as unsighed " << u << std::endl;

  char d = -1;
  char e = (unsigned char) -1;  //no diff i
  int j =  (unsigned char) e; 
  int k = d;
  std::cout << "char of " << i << " == " << d << " and casted as unsigned is " << e << " and back again " << j << " or no cast " << k << std::endl;

  // conclusion:
  // so that a \377 isn't returned as EOF (-1) we cast to unsigned char before returning on the read.
  // nothing to do about it on the write.

  return 0;
}

// string::find_last_of
#include <iostream>       // std::cout
#include <string>         // std::string

//void SplitFilename (const std::string& str)
void SplitFilename (std::string& str)
{
  std::cout << "Splitting: " << str << '\n';
  size_t found = str.find_last_of("/\\");
  if(found == std::string::npos)  // -1
    {
    std::cout << " found = " << found << std::endl;
    }

  std::cout << " path: <" << str.substr(0,found) << ">\n";
  std::cout << " file: " << str.substr(found+1) << '\n';
  std::cout << " full: " << str.substr(0,found) << '/' << str.substr(found+1) << '\n';
  std::string newfile;
  str = str.substr(found+1);
}

int main ()
{
  std::string str1 ("/usr/bin/man");
  std::string str2 ("c:\\windows\\winhelp.exe");
  std::string str3 ("woman");
  std::string str4 ("/hello");

  SplitFilename (str1);
  std::cout << " newfile " << str1 << '\n';

  SplitFilename (str2);
  std::cout << " newfile " << str2 << '\n';

  SplitFilename (str3);
  std::cout << " newfile " << str3 << '\n';

  SplitFilename (str4);
  std::cout << " newfile " << str4 << '\n';

  return 0;
}

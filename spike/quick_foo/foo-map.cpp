// string::find_last_of
#include <iostream>       // std::cout
#include <string>         // std::string
#include <map>
#include <iterator>

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
  std::string str ("");
  std::cout << "size: " << str.size() << "\n";
  std::cout << "length: " << str.length() << "\n";
  std::cout << "capacity: " << str.capacity() << "\n";
  std::cout << "max_size: " << str.max_size() << "\n";
  str.append(1, 'a');
  std::cout << "str: <" << str << ">\n";
  std::cout << "size: " << str.size() << "\n";

  std::map <std::string,std::string> mymap;
  std::string str1 ("/usr/bin/man");

  SplitFilename (str1);
  std::cout << " newfile " << str1 << '\n';

  std::map<std::string,std::string>::iterator sit = mymap.find(str1);

  if(sit == mymap.end())
    {
      std::cout << "add to map" << std::endl;
      //      std::string * p = new std::string(str1);
      //std::string * c = new std::string("hello world");
      //mymap[*p] = *c;
      mymap[str1] = "hello word";
    }
  else
    {
      std::cout << "found! key <" << sit->first << ">" << std::endl;
      std::cout << "found! value <" << sit->second << ">" << std::endl;
    }

  std::map<std::string,std::string>::iterator sit2 = mymap.find(str1);
  if(sit2 == mymap.end())
    {
      std::cerr << "not found after adding to map" << std::endl;
    }
  else
    {
      std::cout << "found! key <" << sit2->first << ">" << std::endl;
      std::cout << "found! value <" << sit2->second << ">" << std::endl;

      std::cout << "found! value at 3 <" << sit2->second[3] << ">" << std::endl;
      sit2->second[3] = 'L';
      std::cout << "found! value <" << sit2->second << ">" << std::endl;

      sit2->second.clear();
      std::cout << "cleared value <" << sit2->second << ">" << std::endl;
    }


  return 0;
}

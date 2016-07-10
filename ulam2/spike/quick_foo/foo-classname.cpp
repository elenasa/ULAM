// string::find_last_of
#include <iostream>       // std::cout
#include <string>         // std::string


const std::string nodeName(const std::string& prettyFunction)
{
  std::cerr << "elena! nodeName got: <" << prettyFunction << ">" << std::endl;
  size_t colons = prettyFunction.find("::");
  if (colons == std::string::npos)
    return "::";
  size_t begin = colons + 2;
  size_t colons2 = prettyFunction.find("::", begin);
  size_t end = colons2 - colons - 2;
  
  std::string nodename = prettyFunction.substr(begin,end);
  std::cerr << "elena! returning: <" << nodename << ">" << std::endl;
  return nodename;
}


int main ()
{
  std::string str1 ("virtual void MFM::NodeStatements::print(MFM::File*)");

  std::cout << nodeName(str1).c_str() << '\n';

  return 0;
}

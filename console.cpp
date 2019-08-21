#include <iostream>
#include <string>

std::ostream& console(std::cout);

void fatal(std::string s)
{
    std::cerr << s << '\n';
    exit(-1);
}





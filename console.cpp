#include <stdlib.h>
#include <iostream>
#include <sstream>

std::ostream& console(std::cout);
std::ostream& debug_console(std::cout);
//std::ostream& debug_console(std::cerr);

void fatal(std::string s)
{
	std::cerr << s << std::endl;
	exit(-1);
}

std::string Str(int d)
{
	std::ostringstream s;
	s << d;
	return s.str();
}



#include "net.h"
#include "console.h"

int main(int argc, char *argv[])
{
//    BgNet *neural = BgNet::readFile("net.w");
    BgNet *neural = readFile("net.w");
    neural->writeFile("copy.w");
}

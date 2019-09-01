#include "net.h"
#include "console.h"

int main(int argc, char *argv[])
{
    BgNet *neural = net::readFile("net.w");
    neural->writeFile("copy.w");
}

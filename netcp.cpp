#include "net.h"

int main(int argc, char *argv[])
{
    BgNet *neural = readFile("net.w");
    writeFile(*neural, "copy.w");
}

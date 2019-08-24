#include "net.h"

int main(int argc, char *argv[])
{
    net *neural = net::read_network("net.w");
    neural->dump_network("copy.w");
}

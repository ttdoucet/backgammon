#include "net.h"

struct foo
{
    constexpr static int stride = 8;
    char foobar[stride];
};


int main(int argc, char *argv[])
{

    foo f;
    console << "sizeof(f): " << sizeof(f) << "\n";

    net *neural = net::read_network("net.w");
    neural->dump_network("copy.w");
}

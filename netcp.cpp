#include "net.h"

#include "stopwatch.h"

stopwatch timer;

int main(int argc, char *argv[])
{


    timer.start();
    net *neural = net::read_network("net.w");
    neural->dump_network("copy.w");
    timer.stop();
    console << "elapsed: " << timer.elapsed() << "\n";    
}

#include "net.h"
#include "console.h"


#include "stopwatch.h"

stopwatch timer;

int main(int argc, char *argv[])
{
    timer.start();
    net *neural = net::readFile("net.w");
    neural->writeFile("copy.w");
    timer.stop();
    console << "elapsed: " << timer.elapsed() << "\n";    
}

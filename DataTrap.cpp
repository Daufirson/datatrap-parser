#include "System.h"

int main(int argc, char* argv[])
{
    System sys;

    sys.SayWelcome(argv[0]);
    sys.HandleArgs(argc, argv);
    sys.SayBye();

    return 0;
}

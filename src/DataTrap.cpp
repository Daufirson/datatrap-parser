/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

#include "System.h"

int main(int argc, char* argv[])
{
    System sys;

    sys.SayWelcome(argv[0]);
    sys.HandleArgs(argc, argv);
    sys.SayBye();

    return 0;
}

/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

#pragma once

#include "Defines.h"

class Logfile
{
public:
    Logfile(void);
    ~Logfile(void);

    // Logbook entry
    void Log(const char* _Format, ...);
    // Write line
    void Logline();
};

/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

#pragma once

#ifndef SYSTEM_H
#define SYSTEM_H

#include "IO.h"
#include "DatabaseUpdate.h"

class DatabaseUpdate;

class System : private IO
{
public:
    System(void);
    ~System(void);

    void SayWelcome(char* str);
    void HandleArgs(int argc, char* argv[]);
    void SayBye();

private:
    DatabaseUpdate DU;

    void ShowUsage();
    void ShowCFG();

    void HandleImportCommand();
    void HandleDumpCommand();

    void ExtractMPQDBCFiles();

    void HandleDatabaseUpdateParams(int argc, char* argv[]);
};

#endif

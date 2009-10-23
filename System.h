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
    void ShowUsage();
    void ShowCFG();

    void HandleImportCommand();
    void HandleDumpCommand();

    void ExtractMPQDBCFiles();

    void HandleDatabaseUpdateParams(int argc, char* argv[]);
};

#endif

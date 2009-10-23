#pragma once

#include "Defines.h"

class Logfile
{
public:
    Logfile(void);
    ~Logfile(void);

    // Logbuch-Eintrag schreiben
    void Log(const char* _Format, ...);
    // Trennlinie schreiben
    void Logline();
};

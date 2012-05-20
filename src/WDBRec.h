/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

#pragma once

#include "WDBDef.h"

class WDBRec
{
public:
    WDBRec(void);
    ~WDBRec(void);

    wdbfield_t m_fields[WDB_MAX_FIELDS];

public:
    wdbfield_t* getField(int32 index) { return &m_fields[index-1]; }
    void setField(wdbfield_t field, int32 index) { m_fields[index-1] = field; }
};

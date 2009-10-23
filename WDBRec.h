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

#include "WDBDef.h"

WDBDef::WDBDef(uint32 fields, char* name)
{
	m_fields = fields;
	m_wdbname = name;
}
 
WDBDef::WDBDef()
{
	m_fields = 0;
	m_index = 0;
}
 
WDBDef::~WDBDef()
{
}

/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

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

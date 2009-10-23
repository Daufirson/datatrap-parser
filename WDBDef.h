#pragma once

#include "Defines.h"

#define WDB_MAX_FIELDS  256
/* Eigentlich 9 mit wowcache.wdb, aber das Format ist unbekannt... :-(
   Um auch diese Datei zu lesen, einfach auf 9 setzen hier (crasht imo!). ;) */
#define WDB_MAX_FILES   8

#define WDB_FILE_CREATURE   "creaturecache.wdb"
#define WDB_FILE_GAMEOBJECT "gameobjectcache.wdb"
#define WDB_FILE_ITEM       "itemcache.wdb"
#define WDB_FILE_ITEM_NAME  "itemnamecache.wdb"
#define WDB_FILE_ITEM_TEXT  "itemtextcache.wdb"
#define WDB_FILE_NPC        "npccache.wdb"
#define WDB_FILE_PAGETEXT   "pagetextcache.wdb"
#define WDB_FILE_QUEST      "questcache.wdb"
#define WDB_FILE_WOW        "wowcache.wdb"

#define WDB_ENTRY_FIELD         "entry"
#define WDB_SIZEOFENTRY_FIELD   "esize"

#define CREATURE_SIG    "BOMW"
#define GAMEOBJECT_SIG  "BOGW"
#define ITEM_SIG        "BDIW"
#define ITEMNAME_SIG    "BDNW"
#define ITEMTEXT_SIG    "XTIW"
#define NPC_SIG         "CPNW"
#define PAGETEXT_SIG    "XTPW"
#define QUEST_SIG       "TSQW"
#define WOW_SIG         "NDRW"

enum WDB_FILES
{
    CREATURECACHE = 0,
    GAMEOBJECTCACHE,
    ITEMCACHE,
    ITEMNAMECACHE,
    ITEMTEXTCACHE,
    NPCCACHE,
    PAGETEXTCACHE,
    QUESTCACHE,
    WOWCACHE,
};

struct wdbheader_t
{
    char    signature[4];
    uint32  ver;
    char    lang[4];
    uint32  fields;
    uint32  unknown1;
    uint32  unknown2;
};

// wowcache.wdb - hat einen anderen header!
struct wdbheaderwow_t
{/*
    char    signature[4];
    uint32  ver;
    char    lang[4];
    uint32  fields;
    uint32  unknown;*/
};

union wdbfield_t
{
    uint32  uint32val;
    uint16  uint16val;
    uint8   uint8val;
    int32   int32val;
    int16   int16val;
    int8    int8val;
    float   fval;
    char*   strval;
};

enum wdbfieldtypes
{
    t_undef = 1,
    t_uint32,
    t_uint16,
    t_uint8,
    t_int32,
    t_int16,
    t_int8,
    t_float,
    t_bitmask16,
    t_bitmask32,
    t_string,
};

class WDBDef
{
    uint16 m_fieldtypes[WDB_MAX_FIELDS];
    uint16 m_index;
	uint32 m_fields;

    bool isReadable;
    char signature[4];

    std::string m_fieldnames[WDB_MAX_FIELDS];
    std::string m_wdbname;

public:
    WDBDef(uint32 fields, char* name);
    WDBDef();
    ~WDBDef();

	char* getSignature() { return &signature[0]; }
	char* getWDBName() { return (char*)m_wdbname.c_str(); }
	uint16 getFieldType(uint32 index) {	return m_fieldtypes[index-1]; }
	uint32 getNumFields() {	return m_fields; }
	void setNumFields(uint32 nfields) { m_fields = nfields; }
	void setFieldType(uint32 index, uint8 fieldtype) { m_fieldtypes[index-1] = fieldtype; }
	void setFieldName(uint32 index, char* name) { m_fieldnames[index-1] = name;	}
	void setWDBName(char* name) { m_wdbname = name; }

	char* getFieldName(uint32 index)
	{ 
		if (index > m_fields) return NULL;
		if (!m_fieldnames[index-1].c_str()) return "";
		return (char*)m_fieldnames[index-1].c_str(); 
	}
 
	void addField(uint8 fieldtype, char* name)
	{
		m_fieldtypes[m_index] = fieldtype;
		m_fieldnames[m_index] = name;
		m_index++;
		m_fields++;
	}
 
	void setSignature(char* sig)
	{ 
		strcpy_s(&signature[0],strlen(signature)+strlen(sig)+1,sig); 
		m_index = 0;    // HACK: This is terrible, move this...
		m_fields = 0;   // HACK: ughkmsdsdfsd
	}
};

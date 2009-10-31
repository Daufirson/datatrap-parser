#pragma once

#ifndef DATABASEUPDATE_H
#define DATABASEUPDATE_H

#include "Defines.h"
#include "Database.h"
#include "IO.h"

#define CREATURE_INSERT_FILE    "insert_into_creature_template.sql"
#define GAMEOBJECT_INSERT_FILE  "insert_into_gameobject_template.sql"
#define ITEM_INSERT_FILE        "insert_into_item_template.sql"
#define NPCTEXT_INSERT_FILE     "insert_into_npc_text.sql"
#define PAGETEXT_INSERT_FILE    "insert_into_page_text.sql"
#define QUEST_INSERT_FILE       "insert_into_quest_template.sql"

#define CREATURE_UPDATE_FILE    "update_creature_template.sql"
#define GAMEOBJECT_UPDATE_FILE  "update_gameobject_template.sql"
#define ITEM_UPDATE_FILE        "update_item_template.sql"
#define NPCTEXT_UPDATE_FILE     "update_npc_text.sql"
#define PAGETEXT_UPDATE_FILE    "update_page_text.sql"
#define QUEST_UPDATE_FILE       "update_quest_template.sql"

#if PLATFORM == PLATFORM_WINDOWS
#define APPLY_SQL_FILE  "executesql.cmd"
#else
#define APPLY_SQL_FILE  "executesql"
#endif

#define DATATRAP_FILE_HEADER        "##############################################\n# Created with DataTrap - http://www.uwom.de #\n##############################################\n\n"
#define REM_DATATRAP_FILE_HEADER    "@ECHO OFF\n\nREM ##############################################\nREM # Created with DataTrap - http://www.uwom.de #\nREM ##############################################\n\n"

#define MAX_INSERTS 100
                                 
class Database;
class IO;

enum UPDATE_TABLES
{
    CREATURE    = 1,
    GAMEOBJECT  = 2,
    QUEST       = 4,
    ITEM        = 8,
    NPCTEXT     = 16,
    PAGETEXT    = 32,
    OWN_GOS     = 64,
    COLUMN      = 128,
};

class DatabaseUpdate
{
public:
    DatabaseUpdate(void);
    ~DatabaseUpdate(void);

    void WriteInsertSQL(const char* user, const char* pw, const char* host, const char* wdbdb,
        const char* worlddb, uint32 todo, const char* home);

    void WriteUpdateSQL(const char* user, const char* pw, const char* host, const char* wdbdb,
        const char* worlddb, uint32 todo, const char* home, bool column);

    void CreateApplySQLFile(const char* home, const char* user, const char* pw,
        const char* host, const char* worlddb, const char* mysqlpath);

private:
    IO io;

    bool ColumnExists(Tokens toks, const char* search);

    void CreateCreatureInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home);
    void CreateGameobjectInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home, bool own_gos = false);
    void CreateQuestInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home);
    void CreateItemInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home);
    void CreateNPCTextInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home);
    void CreatePageTextInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home);

    void CreateCreatureUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, Tokens columns, bool docolumn);
    void CreateGameobjectUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, bool own_gos, Tokens columns, bool docolumn);
    void CreateQuestUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, Tokens columns, bool docolumn);
    void CreateItemUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, Tokens columns, bool docolumn);
    void CreateNPCTextUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, Tokens columns, bool docolumn);
    void CreatePageTextUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, Tokens columns, bool docolumn);
};

#endif

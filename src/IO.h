/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

#pragma once

#ifndef IO_H
#define IO_H

#include "Defines.h"
#include "Database.h"
#include "MPQDBCFile.h"
#include "DBCFile.h"
#include "DBCFormat.h"
#include "WDBFile.h"
#include "IO.h"

static char* const langs[]={"enGB", "enUS", "deDE", "esES", "frFR", "koKR", "zhCN", "zhTW", "enCN", "enTW", "esMX", "ruRU" };

class Database;
class MPQFile;
class WDBFile;

typedef std::vector<std::string> Tokens;

class IO
{
public:
    IO(void);
    ~IO(void);

    std::string Homepath;

    bool usegetch;

    uint8 extract,
        import,
        dump;

    enum what_todo
    {
        IMPORT_EXTRACTED_DBCS = 1,
        IMPORT_DBCS,
        IMPORT_WDBS,
        DUMP_DCBS,
        DUMP_WDBS,
        DUMP_ALL,
        MAX_TODO // AT THE END!
    };

    typedef struct Configuration // DataTrap.cfg
    {
        uint32 dummy; // old resolution (resolution) in the map extract
        std::string db_user,
            db_pw,
            db_host,
            dbc_db,
            wdb_db,
            mysqlpath,
            wowpath,
            world_db;
    } config;

    config* cfg;

    typedef struct CHeaderEntry // Entries of ConfigHeader
    {
        uint16 pos;
        uint16 size;
    } cheaderentry;

    typedef struct ConfigHeader // Header of DataTrap.cfg
    {
        cheaderentry dummy, // old resolution (resolution) in the map extract
            db_user,
            db_pw,
            db_host,
            dbc_db,
            wdb_db,
            mysqlpath, // Because mysqldump under win necessary if MySQL is not in the system path
            wowpath,
            world_db;
    } cfgheader;

    cfgheader* cfg_header;

    void SetHomepath(char* str);
    void LoadCFG();
    void WriteCFG();

    // Looks whether _search in _array exists, and returns a pointer to
    uint32* BinSearch(uint32 _array[], size_t l, uint32 _search);

    // Is the smaller value of val1 + val2 returns
    uint32 SmallerOne(uint32 val1, uint32 val2);

    // Set bit (nr) in value?
    bool IsBitSet(uint32 value, uint32 nr);

    // Check whether param in _argv passed
    bool ParamExists(uint8 argc, char* _argv[], char* param);

    // Returns the rounded value
    uint32 Round(float value);

    // Makes a string unreadable
    void Encode(char* str);

    // Makes a string again readable (usable)
    void Decode(char* str);

    // Deletes \ or / at the end of a path
    void FixSlashes(char* str);

    // Divides a string "sep"
    Tokens StrSplit(const std::string &src, const std::string &sep);

    // Converts a string in WCHAR (UTF8)
    WCHAR* A2WStr (const char* str);

    // Converts a WCHAR string (UTF8)
    const char* W2AStr(WCHAR* wstr);

    // Terminated characters for SQL
    const char* Terminator(std::string &str);

    void SayError(const char* _Format, ...);

    void FixPathes();
    void FixDirs();
    void FixFiles();
    bool DirExists(const char* pzPath);
    bool FileExists(const char* FileName);
    std::string GetColumnFileData(const char* columnfile);

    void _ExtractMPQDBCFiles(int locale, bool basicLocale);
    void LoadLocaleMPQFiles(int const locale);
    void LoadCommonMPQFiles();
    void CloseMPQFiles();

    void GetDBCDatasAndWriteToDatabase(const char* db, const char* user, const char* pw, const char* host);
    void GetWDBDatasAndWriteToDatabase(const char* db, const char* user, const char* pw, const char* host);
    void DumpDBDatas(const char* mysqlpath, const char* db, const char* user, const char* pw, const char* host);

private:
    void FillDBCColumnNames();

    void CreateDir(const std::string &Path);

    void UpdateCfgHeader();
    void EncodeCfg();
    void DecodeCfg();
};

#endif

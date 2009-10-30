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
        MAX_TODO // ENDE!!!
    };

    typedef struct Configuration // DataTrap.cfg
    {
        uint32 dummy; // alte auflösung (resolution) beim map extract
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
        cheaderentry dummy, // alte auflösung (resolution) beim map extract
            db_user,
            db_pw,
            db_host,
            dbc_db,
            wdb_db,
            mysqlpath, // Wegen mysqldump unter Win nötig, wenn MySQL nicht im System-Pfad steht
            wowpath,
            world_db;
    } cfgheader;

    cfgheader* cfg_header;

    void SetHomepath(char* str);
    void LoadCFG();
    void WriteCFG();

    // Schaut ob _search in _array vorhanden ist, und gibt einen Zeiger darauf zurück
    uint32* BinSearch(uint32 _array[], size_t l, uint32 _search);

    // Gibt den kleineren Wert von val1 + val2 zurück
    uint32 SmallerOne(uint32 val1, uint32 val2);

    // Ist Bit (nr) in value gesetzt?
    bool IsBitSet(uint32 value, uint32 nr);

    // Schaut ob param in _argv übergeben wurde
    bool ParamExists(uint8 argc, char* _argv[], char* param);

    // Gibt gerundeten Wert zurück
    uint32 Round(float value);

    // Macht einen String unleserlich
    void Encode(char* str);

    // Macht einen String wieder lesbar (nutzbar)
    void Decode(char* str);

    // Löscht \ oder / am Ende eines Pfades
    void FixSlashes(char* str);

    // Teilt einen String bei "sep"
    Tokens StrSplit(const std::string &src, const std::string &sep);

    // Konvertiert einen String in WCHAR (UTF8)
    WCHAR* A2WStr (const char* str);

    // Konvertiert einen WCHAR in String (UTF8)
    const char* W2AStr(WCHAR* wstr);

    // Terminiert Zeichen für SQL
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

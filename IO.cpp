#include "IO.h"
#include "MPQArchive.h"

extern ArchiveSet gOpenArchives;

IO::IO(void)
{
    extract = 0;
    import = 0;
    dump = 0;

    usegetch = false;

    cfg = new Configuration;
    cfg->dummy      = 256; // alte auflösung (resolution) beim map extract
    cfg->db_user    = "";
    cfg->db_pw      = "";
    cfg->db_host    = "127.0.0.1";
    cfg->dbc_db     = "dbcdata";
    cfg->wdb_db     = "wdbdata";
    cfg->wowpath    = "";
    cfg->mysqlpath  = ""; // Wegen mysqldump unter Win nötig, wenn MySQL nicht im System-Pfad steht
    cfg->world_db   = "mangos";

    cfg_header = new cfgheader;

    UpdateCfgHeader();
}

IO::~IO(void)
{
}

void IO::SetHomepath(char* str)
{
#if PLATFORM == PLATFORM_WINDOWS
    Tokens toks = StrSplit(str, "\\");
#else
    Tokens toks = StrSplit(str, "/");
#endif
    Tokens::iterator iter = toks.begin();
    while(iter != toks.end())
    {
        if (iter+1 != toks.end())
        {
            Homepath.append((*iter).c_str());
            if (iter+2 != toks.end())
#if PLATFORM == PLATFORM_WINDOWS
                Homepath.append("\\");
#else
                Homepath.append("/");
#endif
        }
        *iter++;
    }
    if (!Homepath.size()) Homepath = ".";
}

void IO::CreateDir(const std::string& Path)
{
#if PLATFORM == PLATFORM_WINDOWS 
    _mkdir(Path.c_str());
#else
    mkdir(Path.c_str(), 0777);
#endif
}

bool IO::DirExists(const char* pzPath)
{
    DIR* pDir = opendir(pzPath);
    if (pDir) { closedir(pDir); return true; }
    return false;
}

bool IO::FileExists(const char* FileName)
{
    if (FILE* fp = fopen(FileName, "rb")) { fclose(fp); return true; }
    return false;
}

void IO::FixPathes()
{
    if (cfg->mysqlpath.size() && (cfg->mysqlpath.at(0) == '\\' ||
        cfg->mysqlpath.at(0) == '/')) cfg->mysqlpath.insert(0,".");

    if (cfg->wowpath.size() && (cfg->wowpath.at(0) == '\\' ||
        cfg->wowpath.at(0) == '/')) cfg->wowpath.insert(0,".");
    else if (!cfg->wowpath.size()) cfg->wowpath.append(".");

    FixSlashes((char*)cfg->mysqlpath.c_str());
    FixSlashes((char*)cfg->wowpath.c_str());
}

void IO::FixDirs()
{
    std::string dbcpath     = Homepath.c_str();
    std::string wdbpath     = Homepath.c_str();
    std::string dumppath    = Homepath.c_str();
    std::string sqlpath     = Homepath.c_str();
    std::string miscpath    = Homepath.c_str();

#if PLATFORM == PLATFORM_WINDOWS 
    dbcpath.append("\\dbc");
    wdbpath.append("\\wdb");
    dumppath.append("\\dump");
    sqlpath.append("\\sql");
    miscpath.append("\\misc");
#else
    dbcpath.append("/dbc");
    wdbpath.append("/wdb");
    dumppath.append("/dump");
    sqlpath.append("/sql");
    miscpath.append("/misc");
#endif
    if (!DirExists(dbcpath.c_str()))    CreateDir(dbcpath.c_str());
    if (!DirExists(wdbpath.c_str()))    CreateDir(wdbpath.c_str());
    if (!DirExists(dumppath.c_str()))   CreateDir(dumppath.c_str());
    if (!DirExists(sqlpath.c_str()))    CreateDir(sqlpath.c_str());
    if (!DirExists(miscpath.c_str()))   CreateDir(miscpath.c_str());
}

void IO::FixFiles()
{
    std::string cr_file = Homepath.c_str();
    std::string go_file = Homepath.c_str();
    std::string qu_file = Homepath.c_str();
    std::string it_file = Homepath.c_str();
    std::string np_file = Homepath.c_str();
    std::string pa_file = Homepath.c_str();

#if PLATFORM == PLATFORM_WINDOWS 
    cr_file.append("\\misc\\");
    go_file.append("\\misc\\");
    qu_file.append("\\misc\\");
    it_file.append("\\misc\\");
    np_file.append("\\misc\\");
    pa_file.append("\\misc\\");
#else
    cr_file.append("/misc/");
    go_file.append("/misc/");
    qu_file.append("/misc/");
    it_file.append("/misc/");
    np_file.append("/misc/");
    pa_file.append("/misc/");
#endif

    cr_file.append(_CREATURE_COLUMNS);
    go_file.append(_GAMEOBJECT_COLUMNS);
    qu_file.append(_QUEST_COLUMNS);
    it_file.append(_ITEM_COLUMNS);
    np_file.append(_NPCTEXT_COLUMNS);
    pa_file.append(_PAGETEXT_COLUMNS);

    if (!FileExists(cr_file.c_str()))
    {
        FILE* file = fopen(cr_file.c_str(), "a+");
        if (!file) SayError("Can't create: %s", cr_file.c_str());
        fclose(file);
    }
    if (!FileExists(go_file.c_str()))
    {
        FILE* file = fopen(go_file.c_str(), "a+");
        if (!file) SayError("Can't create: %s", go_file.c_str());
        fclose(file);
    }
    if (!FileExists(qu_file.c_str()))
    {
        FILE* file = fopen(qu_file.c_str(), "a+");
        if (!file) SayError("Can't create: %s", qu_file.c_str());
        fclose(file);
    }
    if (!FileExists(it_file.c_str()))
    {
        FILE* file = fopen(it_file.c_str(), "a+");
        if (!file) SayError("Can't create: %s", it_file.c_str());
        fclose(file);
    }
    if (!FileExists(np_file.c_str()))
    {
        FILE* file = fopen(np_file.c_str(), "a+");
        if (!file) SayError("Can't create: %s", np_file.c_str());
        fclose(file);
    }
    if (!FileExists(pa_file.c_str()))
    {
        FILE* file = fopen(pa_file.c_str(), "a+");
        if (!file) SayError("Can't create: %s", pa_file.c_str());
        fclose(file);
    }
}

void IO::UpdateCfgHeader()
{   // Längen der Daten setzen
    cfg_header->dummy.size = sizeof(cfg->dummy); // alte auflösung (resolution) beim map extract
    cfg_header->db_user.size    = strlen(cfg->db_user.c_str())+1;
    cfg_header->db_pw.size      = strlen(cfg->db_pw.c_str())+1;
    cfg_header->db_host.size    = strlen(cfg->db_host.c_str())+1;
    cfg_header->dbc_db.size     = strlen(cfg->dbc_db.c_str())+1;
    cfg_header->wdb_db.size     = strlen(cfg->wdb_db.c_str())+1;
    cfg_header->mysqlpath.size  = strlen(cfg->mysqlpath.c_str())+1;
    cfg_header->wowpath.size    = strlen(cfg->wowpath.c_str())+1;
    cfg_header->world_db.size   = strlen(cfg->world_db.c_str())+1;
    // Anfangspositionen der Daten im File setzen
    cfg_header->dummy.pos  = sizeof(cfgheader); // alte auflösung (resolution) beim map extract
    cfg_header->db_user.pos     = cfg_header->dummy.pos         + cfg_header->dummy.size;
    cfg_header->db_pw.pos       = cfg_header->db_user.pos       + cfg_header->db_user.size;
    cfg_header->db_host.pos     = cfg_header->db_pw.pos         + cfg_header->db_pw.size;
    cfg_header->dbc_db.pos      = cfg_header->db_host.pos       + cfg_header->db_host.size;
    cfg_header->wdb_db.pos      = cfg_header->dbc_db.pos        + cfg_header->dbc_db.size;
    cfg_header->mysqlpath.pos   = cfg_header->wdb_db.pos        + cfg_header->wdb_db.size;
    cfg_header->wowpath.pos     = cfg_header->mysqlpath.pos     + cfg_header->mysqlpath.size;
    cfg_header->world_db.pos    = cfg_header->wowpath.pos       + cfg_header->wowpath.size;
}

// Macht die CFG-Daten unleserlich
void IO::EncodeCfg()
{
    Encode((char*)cfg->db_user.c_str());
    Encode((char*)cfg->db_pw.c_str());
    Encode((char*)cfg->db_host.c_str());
    Encode((char*)cfg->dbc_db.c_str());
    Encode((char*)cfg->wdb_db.c_str());
    Encode((char*)cfg->mysqlpath.c_str());
    Encode((char*)cfg->wowpath.c_str());
    Encode((char*)cfg->world_db.c_str());
}

// Macht CFG-Daten wieder lesbar (nutzbar)
void IO::DecodeCfg()
{
    Decode((char*)cfg->db_user.c_str());
    Decode((char*)cfg->db_pw.c_str());
    Decode((char*)cfg->db_host.c_str());
    Decode((char*)cfg->dbc_db.c_str());
    Decode((char*)cfg->wdb_db.c_str());
    Decode((char*)cfg->mysqlpath.c_str());
    Decode((char*)cfg->wowpath.c_str());
    Decode((char*)cfg->world_db.c_str());
}

void IO::LoadCFG()
{
    char* tmp;
    std::string fstr = Homepath.c_str();
#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\").append(_CONFIGFILE);
#else
    fstr.append("/").append(_CONFIGFILE);
#endif
    FILE* cfgfile = fopen(fstr.c_str(), "rb");
    if (!cfgfile)
    {
        cfgfile = fopen(fstr.c_str(), "wb");
        if (!cfgfile) SayError("Can't create: %s", fstr.c_str());

        EncodeCfg();

        fwrite(cfg_header, sizeof(cfgheader), 1, cfgfile);
        fwrite(&cfg->dummy, cfg_header->dummy.size, 1, cfgfile); // alte auflösung (resolution) beim map extract
        fwrite(cfg->db_user.c_str(), cfg_header->db_user.size, 1, cfgfile);
        fwrite(cfg->db_pw.c_str(), cfg_header->db_pw.size, 1, cfgfile);
        fwrite(cfg->db_host.c_str(), cfg_header->db_host.size, 1, cfgfile);
        fwrite(cfg->dbc_db.c_str(), cfg_header->dbc_db.size, 1, cfgfile);
        fwrite(cfg->wdb_db.c_str(), cfg_header->wdb_db.size, 1, cfgfile);
        fwrite(cfg->mysqlpath.c_str(), cfg_header->mysqlpath.size, 1, cfgfile);
        fwrite(cfg->wowpath.c_str(), cfg_header->wowpath.size, 1, cfgfile);
        fwrite(cfg->world_db.c_str(), cfg_header->world_db.size, 1, cfgfile);

        DecodeCfg();
        fclose(cfgfile);
        return;
    }

    fseek(cfgfile, 0L, SEEK_SET);
    fread(cfg_header, sizeof(cfgheader), 1, cfgfile);

    fread(&cfg->dummy, cfg_header->dummy.size, 1, cfgfile); // alte auflösung (resolution) beim map extract

    tmp=(char*)malloc(1024); fread(tmp, cfg_header->db_user.size, 1, cfgfile);    cfg->db_user = tmp;     free(tmp);
    tmp=(char*)malloc(1024); fread(tmp, cfg_header->db_pw.size, 1, cfgfile);      cfg->db_pw = tmp;       free(tmp);
    tmp=(char*)malloc(1024); fread(tmp, cfg_header->db_host.size, 1, cfgfile);    cfg->db_host = tmp;     free(tmp);
    tmp=(char*)malloc(1024); fread(tmp, cfg_header->dbc_db.size, 1, cfgfile);     cfg->dbc_db = tmp;      free(tmp);
    tmp=(char*)malloc(1024); fread(tmp, cfg_header->wdb_db.size, 1, cfgfile);     cfg->wdb_db = tmp;      free(tmp);
    tmp=(char*)malloc(1024); fread(tmp, cfg_header->mysqlpath.size, 1, cfgfile);  cfg->mysqlpath = tmp;   free(tmp);
    tmp=(char*)malloc(1024); fread(tmp, cfg_header->wowpath.size, 1, cfgfile);    cfg->wowpath = tmp;     free(tmp);
    tmp=(char*)malloc(1024); fread(tmp, cfg_header->world_db.size, 1, cfgfile);   cfg->world_db = tmp;    free(tmp);

    DecodeCfg();
    fclose(cfgfile);

#if PLATFORM == PLATFORM_WINDOWS 
    // MySQL-Pfad ermitteln (nur unter windows notwendig)
    if (!cfg->mysqlpath.size())
    {
        if (strstr(_strupr(getenv("PATH")), "MYSQL"))
        {
            std::string path = _strupr(getenv("PATH"));
            Tokens toks = StrSplit(path, ";");
            Tokens::iterator iter = toks.begin();
            while(iter != toks.end())
            {
                if (strstr((char*)(*iter).c_str(), "MYSQL"))
                {
                    cfg->mysqlpath = (*iter).c_str();
                    FixSlashes((char*)cfg->mysqlpath.c_str());
                    UpdateCfgHeader();
                    break;
                } else if (iter != toks.end()) *iter++;
            }
        }
        // MySQL-Pfad nicht gefunden
        if (!cfg->mysqlpath.size())
        {
            printf("WARNING: MySQL not found in the system path!\n"
                "WARNING: You have to set the path in my config to use the dump functions!\n");
        }
    }
#endif
}

void IO::WriteCFG()
{
    UpdateCfgHeader();

    std::string fstr = Homepath.c_str();
#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\").append(_CONFIGFILE);
#else
    fstr.append("/").append(_CONFIGFILE);
#endif
    FILE* cfgfile = fopen(fstr.c_str(), "wb");
    if (!cfgfile) SayError("Can't create: %s", fstr.c_str());

    EncodeCfg();

    fwrite(cfg_header, sizeof(cfgheader), 1, cfgfile);
    fwrite(&cfg->dummy, cfg_header->dummy.size, 1, cfgfile);
    fwrite(cfg->db_user.c_str(), cfg_header->db_user.size, 1, cfgfile);
    fwrite(cfg->db_pw.c_str(), cfg_header->db_pw.size, 1, cfgfile);
    fwrite(cfg->db_host.c_str(), cfg_header->db_host.size, 1, cfgfile);
    fwrite(cfg->dbc_db.c_str(), cfg_header->dbc_db.size, 1, cfgfile);
    fwrite(cfg->wdb_db.c_str(), cfg_header->wdb_db.size, 1, cfgfile);
    fwrite(cfg->mysqlpath.c_str(), cfg_header->mysqlpath.size, 1, cfgfile);
    fwrite(cfg->wowpath.c_str(), cfg_header->wowpath.size, 1, cfgfile);
    fwrite(cfg->world_db.c_str(), cfg_header->world_db.size, 1, cfgfile);

    DecodeCfg();
    fclose(cfgfile);
}

void IO::SayError(const char* _Format, ...)
{
    va_list ap;
    char szStr [1024];
    szStr[0]='\0';
    va_start(ap, _Format);
    vsnprintf(szStr, 1024, _Format, ap);
    va_end(ap);

#if PLATFORM == PLATFORM_WINDOWS
    printf("\nERROR: %s!\nERROR: Shutdown. Press any key to exit...", szStr);
    _getch();
#else
    printf("\nERROR: %s! Shutdown.", szStr);
#endif
    exit(1);
}

// Schaut ob _search in _array vorhanden ist, und gibt einen Zeiger darauf zurück
uint32* IO::BinSearch(uint32 _array[], size_t l, uint32 _search)
{
    if (l<1) { return NULL; }
    else if (l==1) { return (_array[0]==_search ? &_array[0] : NULL); }
    else
    {
        uint32 index = l/2;
        if (_array[index]==_search) { return &_array[index]; }
        else if (_array[index] > _search) { return BinSearch(_array, index, _search); }
        else { return BinSearch(&_array[index+1], l - index - 1, _search); }
    }
}

// Gibt den kleineren Wert von val1 + val2 zurück
uint32 IO::SmallerOne(uint32 val1, uint32 val2) { return (val1<=val2)?val1:val2; }

// Ist Bit (nr) in value gesetzt?
bool IO::IsBitSet(uint32 value, uint32 nr) { value>>=nr; return (value&1); }

// Schaut ob param in _argv übergeben wurde
bool IO::ParamExists(uint8 _argc, char* _argv[], char* param)
{
    char* _param = (char*)malloc(MAX_PATH);
    strcpy(_param, param);
    std::string tmpstr = _strupr(_param);

    for (uint8 i=0; i<_argc; i++)
    {
        char* tmp = (char*)malloc(MAX_PATH);
        strcpy(tmp, _argv[i]);
        if (strcmp(_strupr(tmp), tmpstr.c_str())==0)
        {
            free(tmp);
            free(_param);
            return true;
        }
        free(tmp);
    }
    free(_param);
    return false;
}

// Gibt gerundeten Wert zurück
uint32 IO::Round(float value) { return (uint32)value; }

// Macht einen String unleserlich
void IO::Encode(char* str)
{
    for (uint8 i=0; i<=strlen(str); i++)
    {
        if (str[i] == '\0') return;
        str[i] = str[i]-0xFACA;
    }
}

// Macht einen String wieder lesbar (nutzbar)
void IO::Decode(char* str)
{
    for (uint8 i=0; i<=strlen(str); i++)
    {
        if (str[i] == '\0') return;
        str[i] = str[i]+0xFACA;
    }
}

// Löscht \ oder / oder " am Ende eines Pfades
void IO::FixSlashes(char* str)
{
    for (uint8 i=0; i<=strlen(str); i++)
    {
        if ((str[i] == '\\' || str[i] == '/') && str[i+1] == '\0')
        {
            str[i] = '\0';
            return;
        }
        if (str[i] == '\0') return;
    }
}

// Teilt einen String bei "sep"
Tokens IO::StrSplit(const std::string &src, const std::string &sep)
{
    Tokens r;
    std::string s;
    for (std::string::const_iterator i = src.begin(); i != src.end(); i++)
    {
        if (sep.find(*i) != std::string::npos)
        {
            if (s.length()) r.push_back(s);
            s = "";
        } else s += *i;
    }
    if (s.length()) r.push_back(s);
    return r;
}

// Konvertiert einen String in WCHAR (UTF8)
WCHAR* IO::A2WStr(const char* str)
{
    static WCHAR wstr[1024];
    MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str, (int16)strlen(str), wstr, (int16)strlen(str));
    return wstr;
}

// Konvertiert einen WCHAR in String (UTF8)
const char* IO::W2AStr(WCHAR* wstr)
{
    static char str[1024];
    WideCharToMultiByte(CP_UTF8, MB_PRECOMPOSED, wstr, -1, str, (int16)strlen(str), NULL, NULL);
    return (const char*)str;
}

// Terminiert Zeichen für SQL
const char* IO::Terminator(std::string &str)
{
    if (!&str) return NULL;
    if (!str.size()) return str.c_str();

    size_t found = str.find("\\");
    while (found != str.npos)
    {
        str.insert(found, "\\");
        found = str.find("\\", found+2);
    }
    found = str.find("'");
    while (found != str.npos)
    {
        str.insert(found, "\\");
        found = str.find("'", found+2);
    }
    found = str.find("\"");
    while (found != str.npos)
    {
        str.insert(found, "\\");
        found = str.find("\"", found+2);
    }
    found = str.find("\r\n");
    while (found != str.npos)
    {
        str.replace(found, 2, "\\r\\n");
        found = str.find("\r\n", found+1);
    }
    return str.c_str();
}

void IO::_ExtractMPQDBCFiles(int locale, bool basicLocale)
{
    printf("Extracting dbc files... ");

    set <string> dbcfiles;

    // get DBC file list
    for (ArchiveSet::iterator i = gOpenArchives.begin(); i != gOpenArchives.end(); ++i)
    {
        vector<string> files;
        (*i)->GetFileListTo(files);
        for (vector<string>::iterator iter = files.begin(); iter != files.end(); ++iter)
            if (iter->rfind(".dbc") == iter->length() - strlen(".dbc")) dbcfiles.insert(*iter);
    }

    string path = Homepath.c_str();
#if PLATFORM == PLATFORM_WINDOWS
    path += "\\dbc\\";
#else
    path += "/dbc/";
#endif
    if (!basicLocale)
    {
        path += langs[locale];
#if PLATFORM == PLATFORM_WINDOWS
        path += "\\";
#else
        path += "/";
#endif
    }
    // extract DBCs
    uint32 count = 0;
    for (set<string>::iterator iter = dbcfiles.begin(); iter != dbcfiles.end(); ++iter)
    {
        string filename = path;
        filename += (iter->c_str() + strlen("DBFilesClient\\"));
        FILE *output=fopen(filename.c_str(),"wb");
        if (!output)
        {
            printf("\nCan't create the output file '%s'\n",filename.c_str());
            continue;
        }
        MPQFile m(iter->c_str());
        
        if (!m.isEof()) fwrite(m.getPointer(),1,m.getSize(),output);

        fclose(output);
        ++count;
    }
    printf("%u files extracted.\n", count);
}

void IO::LoadLocaleMPQFiles(int const locale)
{
    char filename[512];

#if PLATFORM == PLATFORM_WINDOWS
    sprintf(filename,"%s\\Data\\%s\\locale-%s.MPQ",cfg->wowpath.c_str(),langs[locale],langs[locale]);
#else
    sprintf(filename,"%s/Data/%s/locale-%s.MPQ",cfg->wowpath.c_str(),langs[locale],langs[locale]);
#endif
    new MPQArchive(filename);

    for (uint8 i=1; i < 5; ++i)
    {
        char ext[3] = "";
        if (i > 1) sprintf(ext, "-%i", i);

#if PLATFORM == PLATFORM_WINDOWS
        sprintf(filename,"%s\\Data\\%s\\patch-%s%s.MPQ",cfg->wowpath.c_str(),langs[locale],langs[locale],ext);
#else
        sprintf(filename,"%s/Data/%s/patch-%s%s.MPQ",cfg->wowpath.c_str(),langs[locale],langs[locale],ext);
#endif
        if (FileExists(filename)) new MPQArchive(filename);
    }
}

void IO::LoadCommonMPQFiles()
{
    char filename[512];

#if PLATFORM == PLATFORM_WINDOWS
    sprintf(filename,"%s\\Data\\common-2.MPQ",cfg->wowpath.c_str());
    new MPQArchive(filename);
    sprintf(filename,"%s\\Data\\lichking.MPQ",cfg->wowpath.c_str());
    new MPQArchive(filename);
    sprintf(filename,"%s\\Data\\expansion.MPQ",cfg->wowpath.c_str());
    new MPQArchive(filename);
#else
    sprintf(filename,"%s/Data/common-2.MPQ",cfg->wowpath.c_str());
    new MPQArchive(filename);
    sprintf(filename,"%s/Data/lichking.MPQ",cfg->wowpath.c_str());
    new MPQArchive(filename);
    sprintf(filename,"%s/Data/expansion.MPQ",cfg->wowpath.c_str());
    new MPQArchive(filename);
#endif
    for (uint8 i=1; i < 5; ++i)
    {
        char ext[3] = "";
        if (i > 1) sprintf(ext, "-%i", i);
#if PLATFORM == PLATFORM_WINDOWS
        sprintf(filename,"%s\\Data\\patch%s.MPQ",cfg->wowpath.c_str(),ext);
#else
        sprintf(filename,"%s/Data/patch%s.MPQ",cfg->wowpath.c_str(),ext);
#endif
        if (FileExists(filename)) new MPQArchive(filename);
    }
}

void IO::CloseMPQFiles()
{
    for(ArchiveSet::iterator j = gOpenArchives.begin(); j != gOpenArchives.end();++j) (*j)->close();
        gOpenArchives.clear();
}

void IO::GetDBCDatasAndWriteToDatabase(const char* db, const char* user, const char* pw, const char* host)
{
    DBCFile file;
    Database DB;

    uint16 DBCFilesCount = 0;

    DIR* dir = opendir(Homepath.c_str());

    bool structdone = false;
    bool dbdropped = false;
    bool dbcreated = false;
    bool dbinuse = false;
    bool mysqlready = false;
    bool noprimarykey = false;
    bool structcreated = false;
    bool norows = true;

    std::string dropcommand = "DROP DATABASE IF EXISTS `";
    dropcommand += db;
    dropcommand += "`";

    std::string createcommand = "CREATE DATABASE IF NOT EXISTS `";
    createcommand += db;
    createcommand += "` CHARACTER SET utf8";

    std::string usecommand = "USE `";
    usecommand += db;
    usecommand += "`";

    // MySQL Initialisieren und DBs vorbereiten
    if (!mysqlready)
    {
        if (DB.Initialize(user, pw, NULL, host))
        {
            if (!dbdropped)
            {
                if (!DB.DirectExecute(dropcommand.c_str())) SayError("Can't execute drop database command");
                dbdropped = true;
            }
            if (!dbcreated)
            {
                if (!DB.DirectExecute(createcommand.c_str())) SayError("Can't execute create database command");
                dbcreated = true;
            }
            if (!dbinuse)
            {
                if (!DB.DirectExecute(usecommand.c_str())) SayError("Can't execute use database command");
                dbinuse = true;
            }
        } else SayError("Can't connect to mysql server");

        mysqlready = true;
    }
    // Anzahl aller vorhandenen *.dbc Dateien ermitteln
    struct dirent* ent;
    if (dir != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
            if (strstr(ent->d_name, ".dbc")) DBCFilesCount++;
        closedir(dir);
    }

    // Spaltennamen und Beschreibungen initialisieren
    FillDBCColumnNames();

    // Über alle DBC Dateien gehen
    for (uint16 i=0; i<MAX_DBCS; i++)
    {
        norows = true;
        structcreated = false;
        noprimarykey = false;

        // Datei hat 0 Byte @ 3.0.3.9183
        if (DBCFileList[i].filename == "CharVariations.dbc") continue;

        // Spezialfall ohne PRIMARY KEY!
        if (DBCFileList[i].filename == "CharacterFacialHairStyles.dbc" ||
            DBCFileList[i].filename == "CharBaseInfo.dbc" ||
            DBCFileList[i].filename == "GameTables.dbc" ||
            DBCFileList[i].filename == "gtBarberShopCostBase.dbc" ||
            DBCFileList[i].filename == "gtChanceToMeleeCrit.dbc" ||
            DBCFileList[i].filename == "gtChanceToMeleeCritBase.dbc" ||
            DBCFileList[i].filename == "gtChanceToSpellCrit.dbc" ||
            DBCFileList[i].filename == "gtChanceToSpellCritBase.dbc" ||
            DBCFileList[i].filename == "gtCombatRatings.dbc" ||
            DBCFileList[i].filename == "gtNPCManaCostScaler.dbc" ||
            DBCFileList[i].filename == "gtOCTRegenHP.dbc" ||
            DBCFileList[i].filename == "gtOCTRegenMP.dbc" ||
            DBCFileList[i].filename == "gtRegenHPPerSpt.dbc" ||
            DBCFileList[i].filename == "gtRegenMPPerSpt.dbc" ||
            DBCFileList[i].filename == "ItemSubClass.dbc" ||
            DBCFileList[i].filename == "ItemSubClassMask.dbc" ||
            DBCFileList[i].filename == "PaperDollItemFrame.dbc" ||
            DBCFileList[i].filename == "WorldStateZoneSounds.dbc")
            noprimarykey = true;

        uint16 formatsize = strlen(DBCFileList[i].format);
        std::string fpath = Homepath.c_str();
#if PLATFORM == PLATFORM_WINDOWS 
        fpath += "\\dbc\\";
#else
        fpath += "/dbc/";
#endif
        fpath += DBCFileList[i].filename;

        // Format dieser DBC ist noch nicht gegeben (DBCFormat.h)
        if (DBCFileList[i].format == "")
        {
            printf("INFO: Format of '%s' isn't set internal - skipping it.\n", DBCFileList[i].filename);
            continue;
        }
        // DBCFile kann die Datei nicht öffnen
        if (!file.Load(fpath.c_str(), DBCFileList[i].format))
        {
            printf("WARNING: Can't load '%s' - skipping it!\n", DBCFileList[i].filename);
            continue;
        }

        std::string sqldroptable = "DROP TABLE IF EXISTS `";
        std::string sqlstruct = "";
        std::string sqlinsert = "INSERT INTO `";
        std::string sortcommand = "ALTER TABLE `";

        if (file.IsLoaded())
        {   // DBCFile hat weniger oder mehr Spalten
            if (formatsize && file.GetCols() != formatsize)
            {
                SayError("The number of columns of your '%s' differs!\n"
                    "ERROR: It should have '%u' but it has '%u'.\n"
                    "ERROR: Please use client version '%s' dbcfiles!\n"
                    "ERORR: If you have a higher client version\n"
                    "ERROR: contact the author to update me, thanks",
                    DBCFileList[i].filename, formatsize, file.GetCols(), _CLIENT_VERSION);
            }
            printf("Working on '%s'... this may take some time...\n", DBCFileList[i].filename);

            structdone = false;

            std::string tmptablename = DBCFileList[i].filename;
            tmptablename.resize(tmptablename.size()-4);

            // SQL Drop Table
            sqldroptable += tmptablename.c_str();
            sqldroptable += "`";

            // SQL Struct
            sqlstruct += "CREATE TABLE `";
            sqlstruct += tmptablename.c_str();
            sqlstruct += "` (\n";

            // SQL Insert
            sqlinsert += tmptablename.c_str();
            sqlinsert += "`  VALUES\n";

            sortcommand += tmptablename.c_str();
            sortcommand += "` ORDER BY `id`";

            // Über alle Datensätze gehen
            for (uint32 j=0; j<file.GetNumRows(); j++)
            {
                norows = false;

                if (structdone && (sqlinsert.size()+1024 > MAX_QUERY_LEN) && (j+1 < file.GetNumRows()))
                {
                    if (noprimarykey && !structcreated)
                    {
                        sqlstruct.resize(sqlstruct.size()-2);
                        sqlstruct += "\n) ENGINE=MyISAM DEFAULT CHARSET=utf8";
                    } else sqlstruct += "  PRIMARY KEY (`id`)\n) ENGINE=MyISAM DEFAULT CHARSET=utf8";

                    if (!structcreated)
                    {
                        // SQL Drop Query senden
                        std::string sql = sqldroptable.c_str();
                        sqldroptable.clear();
                        if (!DB.DirectExecute(sql.c_str())) SayError("Can't drop table");
                        sql.clear();

                        // SQL Struktur Query senden
                        sql = sqlstruct.c_str();
                        sqlstruct.clear();
                        if (!DB.DirectExecute(sql.c_str())) SayError("Can't create table struct");
                        sql.clear();

                        structcreated = true;
                    }
                    sqlinsert.resize(sqlinsert.size()-1);
                    if (!DB.DirectExecute(sqlinsert.c_str())) SayError("Can't insert table data");
                    sqlinsert.clear();
                    sqlinsert = "INSERT INTO `";
                    sqlinsert += tmptablename.c_str();
                    sqlinsert += "`  VALUES\n(";
                } else sqlinsert += "(";

                // Über alle Spalten (ein Datensatz) gehen
                for (int32 k=0; k<formatsize; k++)
                {
                    char* tmp;
                    std::string tmpstr = "";

                    // SQL Struct
                    if (!structdone && DBCFileList[i].col_name[k].length())
                    {
                        sqlstruct += "  `";
                        sqlstruct += DBCFileList[i].col_name[k];
                        sqlstruct += "` ";
                    }
                    else if (!structdone)
                    {
                        tmp = (char*)malloc(4);
                        sprintf(tmp, "%u", k);
                        sqlstruct += "  `field_";
                        sqlstruct += tmp;
                        sqlstruct += "` ";
                        free(tmp);
                    }
                    // SQL Insert
                    sqlinsert += "'";

                    // Abfragen einzelner Spaltendaten für einen Datensatz
                    switch(DBCFileList[i].format[k])
                    {
                        case FT_IND:
                        case FT_INT:
                            if (!structdone && DBCFileList[i].col_desc[k].empty()) sqlstruct += "bigint(20) NOT NULL,\n";
                            else if (!structdone)
                            {
                                sqlstruct += "bigint(20) NOT NULL COMMENT '";
                                sqlstruct += DBCFileList[i].col_desc[k];
                                sqlstruct += "',\n";
                            }
                            tmp = (char*)malloc(256);
                            sprintf(tmp, "%i", (int32)file.getRecord(j).getUInt(k));
                            sqlinsert += tmp;
                            free(tmp);
                            break;
                        case FT_FLOAT:
                            if (!structdone && DBCFileList[i].col_desc[k].empty()) sqlstruct += "float NOT NULL,\n";
                            else if (!structdone)
                            {
                                sqlstruct += "float NOT NULL COMMENT '";
                                sqlstruct += DBCFileList[i].col_desc[k];
                                sqlstruct += "',\n";
                            }
                            tmp = (char*)malloc(256);
                            sprintf(tmp, "%f", file.getRecord(j).getFloat(k));
                            sqlinsert += tmp;                            
                            free(tmp);
                            break;
                        case FT_STRING:
                            if (!structdone && DBCFileList[i].col_desc[k].empty()) sqlstruct += "text NOT NULL,\n";
                            else if (!structdone)
                            {
                                sqlstruct += "text NOT NULL COMMENT '";
                                sqlstruct += DBCFileList[i].col_desc[k];
                                sqlstruct += "',\n";
                            }
                            tmpstr += file.getRecord(j).getString(k);
                            sqlinsert += Terminator(tmpstr);
                            tmpstr.clear();
                            break;
                        case FT_BYTE:
                            if (!structdone && DBCFileList[i].col_desc[k].empty()) sqlstruct += "tinyint(3) NOT NULL,\n";
                            else if (!structdone)
                            {
                                sqlstruct += "tinyint(3) NOT NULL COMMENT '";
                                sqlstruct += DBCFileList[i].col_desc[k];
                                sqlstruct += "',\n";
                            }
                            tmp = (char*)malloc(256);
                            sprintf(tmp, "%i", (int8)file.getRecord(j).getUInt8(k));
                            sqlinsert += tmp;                            
                            free(tmp);
                            break;
                    }
                    // Kommen noch mehr Datensätze, oder müssen wir den INSERT beenden?
                    if ((j+1) == file.GetNumRows() && (k+1) == formatsize) sqlinsert += "')";
                    else if ((k+1) == formatsize) sqlinsert += "'),";
                    else sqlinsert += "',";
                }
                // SQL Struktur nur 1x erstellen lassen
                structdone = true;
            }
            if (noprimarykey && !structcreated)
            {
                sqlstruct.resize(sqlstruct.size()-2);
                sqlstruct += "\n) ENGINE=MyISAM DEFAULT CHARSET=utf8";
            } else sqlstruct += "  PRIMARY KEY (`id`)\n) ENGINE=MyISAM DEFAULT CHARSET=utf8";
        }
        else
        {
            // DBCFile kann die Datei nicht lesen
            printf("WARNING: Can't read '%s' - skipping it!\n", DBCFileList[i].filename);
        }

        if (!structcreated && !norows)
        {
            // SQL Drop Query senden
            std::string sql = sqldroptable.c_str();
            sqldroptable.clear();
            if (!DB.DirectExecute(sql.c_str())) SayError("Can't drop table");
            sql.clear();
        
            // SQL Struktur Query senden
            sql = sqlstruct.c_str();
            sqlstruct.clear();
            if (!DB.DirectExecute(sql.c_str())) SayError("Can't create table struct");
            sql.clear();
        }

        if (!norows)
        {
            // SQL Insert Query senden
            std::string sql = sqlinsert.c_str();
            sqlinsert.clear();
            if (!DB.DirectExecute(sql.c_str())) SayError("Can't insert table data");
            sql.clear();

            // SQL Sort Query
            if (!noprimarykey)
                if (!DB.DirectExecute(sortcommand.c_str())) SayError("Can't sort table data");
            sortcommand.clear();
        }
    }
    // Anzahl der gefundenen DBC Dateien entspricht nicht unserer Anzahl!
/*    if (DBCFilesCount != MAX_DBCS)
    {
        printf("WARNING: DBC filenumber differs!\n"
            "WARNING: You should have '%u' *.dbc files but you have '%u'!\n", MAX_DBCS, DBCFilesCount);
    }*/
}

void IO::GetWDBDatasAndWriteToDatabase(const char* db, const char* user, const char* pw, const char* host)
{
    Database DB;

    // SQL Create Database
    std::string createcmd = "CREATE DATABASE IF NOT EXISTS `";
    createcmd += db;
    createcmd += "` CHARACTER SET `utf8`";

    // SQL Use Database
    std::string usecmd = "USE `";
    usecmd += db;
    usecmd += "`";

    // MySQL Initialisieren und DB vorbereiten
    if (DB.Initialize(user, pw, NULL, host))
    {
        if (!DB.DirectExecute(createcmd.c_str())) SayError("Can't execute create database command");
        if (!DB.DirectExecute(usecmd.c_str())) SayError("Can't execute use database command");
    } else SayError("Can't connect to mysql server");

    // WDB_MAX_FILES ist in Defines.h mit 8 definiert, weil das wowcache.wdb Format nicht bekannt ist!
    for (uint8 i=0; i<WDB_MAX_FILES; i++)
    {
    	WDBFile wdb;
        wdb.LoadDefinitions(i);

        std::string tmptablename;
        
        // Pfad + WDB-Datei
        std::string fpath = Homepath.c_str();
#if PLATFORM == PLATFORM_WINDOWS 
        fpath += "\\wdb\\";
#else
        fpath += "/wdb/";
#endif
        // SQL Struct
        std::string sqlstruct = "CREATE TABLE IF NOT EXISTS `";

        // SQL Insert
        std::string sqlinsert = "REPLACE INTO `";

        // SQL Sort
        std::string sortcmd = "ALTER TABLE `";

        switch(i)
        {
            case CREATURECACHE:
                {
                    tmptablename = WDB_FILE_CREATURE;
                    tmptablename.resize(tmptablename.size()-4);
                    fpath       += WDB_FILE_CREATURE;
                    sqlstruct   += tmptablename.c_str();
                    sqlinsert   += tmptablename.c_str();
                    sortcmd     += tmptablename.c_str();
                }
                break;
            case GAMEOBJECTCACHE:
                {
                    tmptablename = WDB_FILE_GAMEOBJECT;
                    tmptablename.resize(tmptablename.size()-4);
                    fpath       += WDB_FILE_GAMEOBJECT;
                    sqlstruct   += tmptablename.c_str();
                    sqlinsert   += tmptablename.c_str();
                    sortcmd     += tmptablename.c_str();
                }
                break;
            case ITEMCACHE:
                {
                    tmptablename = WDB_FILE_ITEM;
                    tmptablename.resize(tmptablename.size()-4);
                    fpath       += WDB_FILE_ITEM;
                    sqlstruct   += tmptablename.c_str();
                    sqlinsert   += tmptablename.c_str();
                    sortcmd     += tmptablename.c_str();
                }
                break;
            case ITEMNAMECACHE:
                {
                    tmptablename = WDB_FILE_ITEM_NAME;
                    tmptablename.resize(tmptablename.size()-4);
                    fpath       += WDB_FILE_ITEM_NAME;
                    sqlstruct   += tmptablename.c_str();
                    sqlinsert   += tmptablename.c_str();
                    sortcmd     += tmptablename.c_str();
                }
                break;
            case ITEMTEXTCACHE:
                {
                    tmptablename = WDB_FILE_ITEM_TEXT;
                    tmptablename.resize(tmptablename.size()-4);
                    fpath       += WDB_FILE_ITEM_TEXT;
                    sqlstruct   += tmptablename.c_str();
                    sqlinsert   += tmptablename.c_str();
                    sortcmd     += tmptablename.c_str();
                }
                break;
            case NPCCACHE:
                {
                    tmptablename = WDB_FILE_NPC;
                    tmptablename.resize(tmptablename.size()-4);
                    fpath       += WDB_FILE_NPC;
                    sqlstruct   += tmptablename.c_str();
                    sqlinsert   += tmptablename.c_str();
                    sortcmd     += tmptablename.c_str();
                }
                break;
            case PAGETEXTCACHE:
                {
                    tmptablename = WDB_FILE_PAGETEXT;
                    tmptablename.resize(tmptablename.size()-4);
                    fpath       += WDB_FILE_PAGETEXT;
                    sqlstruct   += tmptablename.c_str();
                    sqlinsert   += tmptablename.c_str();
                    sortcmd     += tmptablename.c_str();
                }
                break;
            case QUESTCACHE:
                {
                    tmptablename = WDB_FILE_QUEST;
                    tmptablename.resize(tmptablename.size()-4);
                    fpath       += WDB_FILE_QUEST;
                    sqlstruct   += tmptablename.c_str();
                    sqlinsert   += tmptablename.c_str();
                    sortcmd     += tmptablename.c_str();
                }
                break;
            case WOWCACHE:
                {
                    tmptablename = WDB_FILE_WOW;
                    tmptablename.resize(tmptablename.size()-4);
                    fpath       += WDB_FILE_WOW;
                    sqlstruct   += tmptablename.c_str();
                    sqlinsert   += tmptablename.c_str();
                    sortcmd     += tmptablename.c_str();
                }
                break;
        }
        // SQL Insert
        sqlinsert += "` VALUES\n";

        // WDB-Datei laden
        if (wdb.loadWDB(fpath.c_str()))
        {
            // WDB-Datei lesen
            wdb.parseWDB();

            if (wdb.isLoaded)
            {
                printf("Writing to DB...");

                bool structcreated = false;
                bool empty = true;
                uint32 nFields = wdb.m_def->getNumFields();

                // SQL Struct
                sqlstruct.append("` (\n");
                for (uint32 k=1; k<=nFields; k++)
                {
                    sqlstruct.append("`");

                    if (strlen(wdb.getDef()->getFieldName(k)) > 0) sqlstruct.append(wdb.getDef()->getFieldName(k));
                    else
                    {
                        char* tmp = (char*)malloc(256);
                        sprintf(tmp, "%u", k-1);
                        sqlstruct += "field";
                        sqlstruct += tmp;
                        free(tmp);
                    }

                    switch(wdb.m_def->getFieldType(k))
                    {
                        case t_undef:       sqlstruct.append("` int(12) signed NOT NULL DEFAULT '0',\n"); break;
                        case t_uint32:      sqlstruct.append("` int(11) unsigned NOT NULL DEFAULT '0',\n"); break;
                        case t_uint16:      sqlstruct.append("` int(6) unsigned NOT NULL DEFAULT '0',\n"); break;
                        case t_uint8:       sqlstruct.append("` int(3) unsigned NOT NULL DEFAULT '0',\n"); break;
                        case t_int32:       sqlstruct.append("` int(12) signed NOT NULL DEFAULT '0',\n"); break;
                        case t_int16:       sqlstruct.append("` int(7) signed NOT NULL DEFAULT '0',\n"); break;
                        case t_int8:        sqlstruct.append("` int(4) signed NOT NULL DEFAULT '0',\n"); break;
                        case t_float:       sqlstruct.append("` float NOT NULL DEFAULT '0',\n"); break;
                        case t_bitmask16:   sqlstruct.append("` text NOT NULL,\n"); break;
                        case t_bitmask32:   sqlstruct.append("` text NOT NULL,\n"); break;
                        case t_string:      sqlstruct.append("` text NOT NULL,\n"); break;
                    }
                }
                // SQL Sortcmd
                sortcmd.append("` ORDER BY `").append(wdb.getDef()->getFieldName(1)).append("`");
                // SQL Struct
                sqlstruct.append(" PRIMARY KEY (`").append(wdb.getDef()->getFieldName(1)).append("`)\n");
                sqlstruct.append(") ENGINE=MyISAM DEFAULT CHARSET=utf8");

                if (!structcreated)
                {
                    if (!DB.DirectExecute(sqlstruct.c_str())) SayError("Can't create table struct");
                    structcreated = true;
                }

                // Daten in den sqlinsert einlesen
                for (uint32 j=0; j<wdb.m_records.size(); j++)
                {
                    char* tmp;
                    std::string tmpstr;
                    empty = false;

                    sqlinsert += "(";

                    for (uint32 k=1; k<=nFields; k++)
                    {
                        sqlinsert += "'";

                        switch(wdb.m_def->getFieldType(k))
                        {
                            case t_undef:
                                tmp = (char*)malloc(256);
                                sprintf(tmp, "%i", wdb.getField(j, k)->int32val);
                                sqlinsert += tmp; free(tmp); break;
                            case t_uint32:
                                tmp = (char*)malloc(256);
                                sprintf(tmp, "%u", wdb.getField(j, k)->uint32val);
                                sqlinsert += tmp; free(tmp); break;
                            case t_uint16:
                                tmp = (char*)malloc(256);
                                sprintf(tmp, "%u", wdb.getField(j, k)->uint16val);
                                sqlinsert += tmp; free(tmp); break;
                            case t_uint8:
                                tmp = (char*)malloc(256);
                                sprintf(tmp, "%u", wdb.getField(j, k)->uint8val);
                                sqlinsert += tmp; free(tmp); break;
                            case t_int32:
                                tmp = (char*)malloc(256);
                                sprintf(tmp, "%i", wdb.getField(j, k)->int32val);
                                sqlinsert += tmp; free(tmp); break;
                            case t_int16:
                                tmp = (char*)malloc(256);
                                sprintf(tmp, "%i", wdb.getField(j, k)->int16val);
                                sqlinsert += tmp; free(tmp); break;
                            case t_int8:
                                tmp = (char*)malloc(256);
                                sprintf(tmp, "%i", wdb.getField(j, k)->int8val);
                                sqlinsert += tmp; free(tmp); break;
                            case t_float:
                                tmp = (char*)malloc(256);
                                sprintf(tmp, "%f", wdb.getField(j, k)->fval);
                                sqlinsert += tmp; free(tmp); break;
                            case t_bitmask16:
                                tmpstr = wdb.BM16toString(wdb.getField(j, k)->uint16val);
                                sqlinsert += Terminator(tmpstr); tmpstr.clear(); break;
                            case t_bitmask32:
                                tmpstr = wdb.BM32toString(wdb.getField(j, k)->uint32val);
                                sqlinsert += Terminator(tmpstr); tmpstr.clear(); break;
                            case t_string:
                                tmpstr = wdb.getField(j, k)->strval;
                                sqlinsert += Terminator(tmpstr); tmpstr.clear(); break;
                        }
                        if ((j+1) == wdb.m_records.size() && k == nFields) sqlinsert += "')";
                        else if (k == nFields) sqlinsert += "'),";
                        else sqlinsert += "',";

                        // Haben wir MAX_QUERY_LEN fast erreicht und Daten übrig? Dann sqlinsert senden und löschen!
                        if (k == nFields && (sqlinsert.size()+1024) > MAX_QUERY_LEN && (j+1) < wdb.m_records.size())
                        {
                            sqlinsert.resize(sqlinsert.size()-1);
                            if (!DB.DirectExecute(sqlinsert.c_str())) SayError("Can't insert table data");
                            sqlinsert.clear();
                            sqlinsert.append("REPLACE INTO `").append(tmptablename.c_str()).append("` VALUES\n");
                        }
                    }
                }
                printf("\n");

                if (!empty)
                    if (!DB.DirectExecute(sqlinsert.c_str())) SayError("Can't insert table data");
                // SQL Sortcmd
                if (!DB.DirectExecute(sortcmd.c_str())) SayError("Can't sort table data");

            } else SayError("Can't load '%s'", fpath.c_str());

            wdb.gDefList.empty();

        } else SayError("Can't open '%s'", fpath.c_str());
    }
}

void IO::DumpDBDatas(const char* mysqlpath, const char* db, const char* user, const char* pw, const char* host)
{
    Database DB;
    std::string cmd;

#if PLATFORM == PLATFORM_WINDOWS
    if (strlen(mysqlpath)) cmd.append(mysqlpath).append("/mysqldump --user=");
    else cmd = "mysqldump --user=";
#else
    cmd = "mysqldump --user=";
#endif
    cmd.append(user).append(" --password=").append(pw).append(" --host=").append(host);
    cmd.append(" --opt --complete-insert --compress --quote-names --allow-keywords --add-drop-database --databases --result-file=");

#if PLATFORM == PLATFORM_WINDOWS
    cmd.append("\"").append(Homepath.c_str()).append("\\dump\\").append(db).append(".sql\" ").append(db);
    printf("Dumping DB '%s' into '%s\\dump\\%s.sql'...\n", db, Homepath.c_str(), db);
#else
    cmd.append("\"").append(Homepath.c_str()).append("/dump/").append(db).append(".sql\" ").append(db);
    printf("Dumping DB '%s' into '%s/dump/%s.sql'...\n", db, Homepath.c_str(), db);
#endif

    if (system(cmd.c_str())) SayError("Can't dump database");
}

void IO::FillDBCColumnNames()
{   // Alle Leeren initialisieren
    for (uint16 i=0; i<MAX_DBCS; i++)
        for (uint32 j=0; j<=strlen(DBCFileList[0].format); j++)
        {
            DBCFileList[i].col_name[j].clear();
            DBCFileList[i].col_desc[j].clear();
        }
// Da viel zu groß, wurde dieser Block exportiert (Spaltennamen und Beschreibungen initialisieren)
#include "DBCColumnNames.inc"
}

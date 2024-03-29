/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

#include "System.h"

System::System(void) { }
System::~System(void) { }

void System::SayWelcome(char* str)
{
#if PLATFORM == PLATFORM_WINDOWS
    system("cls");
#else
    system("clear");
#endif
    printf("--------------------------------------------------------------------------\n"
        "    DataTrap v%s - http://www.projectskyfire.org - Open Source \n"
        " Written and Copyrighted � %s by Project-SkyFire Team - 4.x parser \n"
        "--------------------------------------------------------------------------\n\n",
           _VERSION, _YEAR);

    // IMPORTANT: initialize home path!
    SetHomepath(str);
    // Create the necessary directories, if necessary
    FixDirs();
    // Create the necessary files, if necessary
    FixFiles();
    // Load config
    LoadCFG();
}

void System::ShowUsage()
{
    usegetch = true;

    /* ATTENTION: IF "INSERT" AND "FIX" IS THERE STILL THE PARAMETERS "OWNGOS"!
                 "OWNGOS" set in type 3 +25 Data1 to the ID of the GO.*/
    printf("Usage: DataTrap -switch value (more as one pair is possible)\n\n"
        "'-h'                   - this commandline help\n"
        "'-c'                   - the current configuration data\n\n"

        "'-a tdb'               - the db name of your world db (default: tdb)\n"
        "'-d dbcdata'           - the db name for dbc data (default: dbcdata)\n"
        "'-w wdbdata'           - the db name for wdb data (default: wdbdata)\n\n"

        "'-p password'          - the database password\n"
        "'-u user'              - the database username\n"
        "'-s 127.0.0.1'         - the database server ip (default: 127.0.0.1)\n\n"

        "'-m c:\\mysql\\bin'      - path to your mysql binaries\n"
        "                         it's only important for windows and '-k'\n"
        "                         if mysql isn't in the system path environment.\n"
        "'-f c:\\path\\wow'       - path to your root wow directory\n\n"

        "'-e'                   - extract the dbc files\n\n"

        "'-i 1'                 - import dbc files into your database\n"
        "                         1 = extract and import dbcs into your database\n"
        "                         2 = import dbcs into your database\n"
        "                         3 = import wdbs into your database\n\n"

        "'-k 4'                 - dump dbc database\n"
        "                         4 = dump dbc database\n"
        "                         5 = dump wdb database\n"
        "                         6 = dump both\n\n"

        "'-insert quest item'   - created sql insert files for new quests + items\n"
        "                         possible values: creature, gameobject, quest,\n"
        "                                          item, npctext, pagetext\n\n"

        "'-update quest item'   - created sql update files for new quest + item data\n"
        "                         possible values: creature, gameobject, quest,\n"
        "                                          item, npctext, pagetext\n\n"

        "'-column quest item'   - created sql update files for new quest and\n"
        "                         item information. possible values: creature,\n"
        "                         game object, quest, item, npctext, pagetext.\n"
        "                         Unlike '-update' this must be in the\n"
        "                         appropriate files in the directory 'misc',\n"
        "                         the column update the database are\n"
        "                         specified. the specification of the columns\n"
        "                         in these files must be with a space\n"
        "                         separated from each other, and it is pay\n"
        "                         attention to the spelling (case sensitive).\n"
        "                         the columns must all be listed in the first\n"
        "                         row behind the other.\n\n"

        "NOTE1: You can use -insert/-update combined but without other switches!\n"
        "NOTE2: You have to write pathnames with spaces like this: 'c:\\my space path\\'!\n");
}

void System::ShowCFG()
{
    usegetch = true;

    if (cfg->db_pw.size())
        printf("DB User     : %s\n"
            "DB Password : (not displayed)\n"
            "DB Host     : %s\n"
            "DBC DB      : %s\n"
            "WDB DB      : %s\n"
            "World DB    : %s\n"
            "WoW Path    : %s\n"
            "MySQL Path  : %s\n",
            cfg->db_user.c_str(),
            cfg->db_host.c_str(),
            cfg->dbc_db.c_str(),
            cfg->wdb_db.c_str(),
            cfg->world_db.c_str(),
            cfg->wowpath.c_str(),
            cfg->mysqlpath.c_str());
    else
        printf("DB User     : %s\n"
            "DB Password : %s\n"
            "DB Host     : %s\n"
            "DBC DB      : %s\n"
            "WDB DB      : %s\n"
            "World DB    : %s\n"
            "WoW Path    : %s\n"
            "MySQL Path  : %s\n",
            cfg->db_user.c_str(),
            cfg->db_pw.c_str(),
            cfg->db_host.c_str(),
            cfg->dbc_db.c_str(),
            cfg->wdb_db.c_str(),
            cfg->world_db.c_str(),
            cfg->wowpath.c_str(),
            cfg->mysqlpath.c_str());
}

void System::SayBye()
{
#if PLATFORM == PLATFORM_WINDOWS
    if (usegetch) _getch();
#endif
    WriteCFG();
}

void System::HandleArgs(int argc, char* argv[])
{
    bool onlycfg = true;
    bool onlyshowhelp = false;
    bool onlyshowcfg = false;

    for (uint8 i=1; i<argc; i++)
    {
        if (argv[i][0] != '-') ShowUsage();
        {
            if (!argv[i+1] && argv[i][1] != 'h' && argv[i][1] != 'c' && argv[i][1] != 'e')
            {
                onlyshowhelp = true;
                ShowUsage();
                break;
            }

            if (ParamExists(argc, argv, "-insert") || ParamExists(argc, argv, "-update") || ParamExists(argc, argv, "-column"))
            {
                HandleDatabaseUpdateParams(argc, argv);
                return;
            }

            switch (argv[i][1])
            {
                case 'a': cfg->world_db = argv[i+1];                            break;
                case 'd': cfg->dbc_db = argv[i+1];                              break;
                case 'e': extract = MAX_TODO;           onlycfg = false;        break;
                case 'f':
                    {
                        cfg->wowpath = "";

                        if (argv[i+1][0] == '\'')
                        {
                            i++;
                            char* tmp = (char*)malloc(strlen(argv[i]));
                            for (uint8 j=1; j<=strlen(argv[i]); j++) tmp[j-1] = argv[i][j];
                            cfg->wowpath.append(tmp);
                            free(tmp);
                            i++;

                            for (uint8 j=i; j<argc; j++)
                            {
                                char* tmp = (char*)malloc(strlen(argv[j])+1);
                                if (argv[j][strlen(argv[j])-1] == '\'')
                                {
                                    cfg->wowpath.append(" ");
                                    for (uint8 k=0; k<=strlen(argv[j]); k++) tmp[k] = argv[j][k];
                                    cfg->wowpath.append(tmp);
                                    cfg->wowpath.resize(cfg->wowpath.length()-1);
                                    free(tmp);
                                    break;
                                } else cfg->wowpath.append(" ").append(argv[j]);
                            }
                        } else cfg->wowpath = argv[i+1];
                    }
                    break;
                case 'h': ShowUsage();                  onlyshowhelp = true;    break;
                case 'i': import = atoi(argv[i+1]);     onlycfg = false;        break;
                case 'c': ShowCFG();                    onlyshowcfg = true;     break;
                case 'k': dump = atoi(argv[i+1]);       onlycfg = false;        break;
                case 'm':
                    {
                        cfg->mysqlpath = "";

                        if (argv[i+1][0] == '\'')
                        {
                            i++;
                            char* tmp = (char*)malloc(strlen(argv[i]));
                            for (uint8 j=1; j<=strlen(argv[i]); j++) tmp[j-1] = argv[i][j];
                            cfg->mysqlpath.append(tmp);
                            free(tmp);
                            i++;

                            for (uint8 j=i; j<argc; j++)
                            {
                                char* tmp = (char*)malloc(strlen(argv[j])+1);
                                if (argv[j][strlen(argv[j])-1] == '\'')
                                {
                                    cfg->mysqlpath.append(" ");
                                    for (uint8 k=0; k<=strlen(argv[j]); k++) tmp[k] = argv[j][k];
                                    cfg->mysqlpath.append(tmp);
                                    cfg->mysqlpath.resize(cfg->mysqlpath.length()-1);
                                    free(tmp);
                                    break;
                                } else cfg->mysqlpath.append(" ").append(argv[j]);
                            }
                        } else cfg->mysqlpath = argv[i+1];
                    }
                    break;
                case 'p': cfg->db_pw = argv[i+1];                               break;
                case 'u': cfg->db_user = argv[i+1];                             break;
                case 's': cfg->db_host = argv[i+1];                             break;
                case 'w': cfg->wdb_db = argv[i+1];                              break;
            }
            i++;
        }
    }

    FixPathes();

    if (!onlycfg && extract == MAX_TODO) ExtractMPQDBCFiles();

    if (!onlycfg && (import < IMPORT_EXTRACTED_DBCS || import > IMPORT_WDBS))
    {
        import = 0;
        if (!extract && !dump) ShowUsage();
    } else if (!onlycfg && import) HandleImportCommand();

    if (!onlycfg && (dump < DUMP_DCBS || dump > DUMP_ALL))
    {
        dump = 0;
        if (!extract && !import) ShowUsage();
    } else if (!onlycfg && dump) HandleDumpCommand();

    if (!onlyshowhelp && !onlyshowcfg && !extract && !import && argc > 1 && onlycfg)
    {
        printf("Writing configuration. The new config:\n\n");
        ShowCFG();
    }
    else if (argc == 1)
    {
        printf("Enter 'DataTrap -h' to see my help!\n");
        usegetch = true;
    }
}

void System::HandleDatabaseUpdateParams(int argc, char* argv[])
{
    if (!cfg->db_user.size() || !cfg->db_pw.size())
        SayError("No user and/or password specified for the mysql access");

    uint32 todo = 0;

    for (uint8 i=1; i<argc; i++)
    {
        char* tmp = (char*)malloc(MAX_PATH);
        strcpy(tmp, argv[i]);

        if (strcmp(_strupr(tmp), "CREATURE")==0)    todo = todo | CREATURE;
        if (strcmp(_strupr(tmp), "GAMEOBJECT")==0)  todo = todo | GAMEOBJECT;
        if (strcmp(_strupr(tmp), "QUEST")==0)       todo = todo | QUEST;
        if (strcmp(_strupr(tmp), "ITEM")==0)        todo = todo | ITEM;
        if (strcmp(_strupr(tmp), "NPCTEXT")==0)     todo = todo | NPCTEXT;
        if (strcmp(_strupr(tmp), "PAGETEXT")==0)    todo = todo | PAGETEXT;
        if (strcmp(_strupr(tmp), "-OWNGOS")==0)     todo = todo | OWN_GOS;
        if (strcmp(_strupr(tmp), "COLUMN")==0)      todo = todo | COLUMN;

        free(tmp);
    }

    if (ParamExists(argc, argv, "-insert"))
    {
        DU.WriteInsertSQL(cfg->db_user.c_str(), cfg->db_pw.c_str(), cfg->db_host.c_str(),
            cfg->wdb_db.c_str(), cfg->world_db.c_str(), todo, Homepath.c_str());

        if (ParamExists(argc, argv, "-update") || ParamExists(argc, argv, "-column")) printf("\n");
    }

    if (ParamExists(argc, argv, "-update") && ParamExists(argc, argv, "-column"))
        SayError("You can't combine the parameter '-update' and '-column'");

    if (ParamExists(argc, argv, "-update") || ParamExists(argc, argv, "-column"))
    {
        DU.WriteUpdateSQL(cfg->db_user.c_str(), cfg->db_pw.c_str(), cfg->db_host.c_str(),
            cfg->wdb_db.c_str(), cfg->world_db.c_str(), todo, Homepath.c_str(), ParamExists(argc, argv, "-column"));
    }

    DU.CreateApplySQLFile(Homepath.c_str(), cfg->db_user.c_str(), cfg->db_pw.c_str(),
        cfg->db_host.c_str(), cfg->world_db.c_str(), cfg->mysqlpath.c_str());
}

void System::HandleImportCommand()
{
    if (!cfg->db_user.size() || !cfg->db_pw.size())
        SayError("No user and/or password specified to import data");

    switch (import)
    {
        case IMPORT_EXTRACTED_DBCS:
            ExtractMPQDBCFiles();
            GetDBCDatasAndWriteToDatabase(cfg->dbc_db.c_str(), cfg->db_user.c_str(),
                cfg->db_pw.c_str(), cfg->db_host.c_str());
            break;
        case IMPORT_DBCS:
            GetDBCDatasAndWriteToDatabase(cfg->dbc_db.c_str(), cfg->db_user.c_str(),
                cfg->db_pw.c_str(), cfg->db_host.c_str());
            break;
        case IMPORT_WDBS:
            GetWDBDatasAndWriteToDatabase(cfg->wdb_db.c_str(), cfg->db_user.c_str(),
                cfg->db_pw.c_str(), cfg->db_host.c_str());
            break;
    }
}

void System::HandleDumpCommand()
{
    if (!cfg->db_user.size() || !cfg->db_pw.size())
        SayError("No user and/or password specified to dump data");

#if PLATFORM == PLATFORM_WINDOWS
    if (!cfg->mysqlpath.size())
        SayError("To dump data you have to set the MySQL path");
#endif

    switch (dump)
    {
        case DUMP_DCBS:
            DumpDBDatas(cfg->mysqlpath.c_str(), cfg->dbc_db.c_str(), cfg->db_user.c_str(),
                cfg->db_pw.c_str(), cfg->db_host.c_str());
            break;
        case DUMP_WDBS:
            DumpDBDatas(cfg->mysqlpath.c_str(), cfg->wdb_db.c_str(), cfg->db_user.c_str(),
                cfg->db_pw.c_str(), cfg->db_host.c_str());
            break;
        case DUMP_ALL:
            DumpDBDatas(cfg->mysqlpath.c_str(), cfg->dbc_db.c_str(), cfg->db_user.c_str(),
                cfg->db_pw.c_str(), cfg->db_host.c_str());
            DumpDBDatas(cfg->mysqlpath.c_str(), cfg->wdb_db.c_str(), cfg->db_user.c_str(),
                cfg->db_pw.c_str(), cfg->db_host.c_str());
            break;
    }
}

void System::ExtractMPQDBCFiles()
{
    int FirstLocale = -1;

    for (uint8 i=0; i<MAX_LOCALE; i++)
    {
        char tmp[MAX_PATH];
        sprintf(tmp, "%s/Data/%s/locale-%s.MPQ", cfg->wowpath.c_str(), langs[i], langs[i]);

        if (FileExists(tmp))
        {
            printf("Detected locale: %s.\n", langs[i]);

            //Open MPQs
            LoadLocaleMPQFiles(i);

            if (((extract & MAX_TODO) == 0) && ((import & IMPORT_EXTRACTED_DBCS) == 0))
            {
                FirstLocale=i;
                break;
            }

            //Extract DBC files
            if (FirstLocale<0)
            {
                _ExtractMPQDBCFiles(i, true);
                FirstLocale = i;
            } else _ExtractMPQDBCFiles(i, false);

            //Close MPQs
            CloseMPQFiles();
        }
    }

    if (FirstLocale<0)
    {
        if (!DirExists(cfg->wowpath.c_str())) SayError("Wrong WoW path: '%s'", cfg->wowpath.c_str());
        else SayError("No locales detected");
    }
}

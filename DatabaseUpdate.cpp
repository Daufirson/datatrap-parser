#include "DatabaseUpdate.h"

DatabaseUpdate::DatabaseUpdate(void) { }
DatabaseUpdate::~DatabaseUpdate(void) { }

bool DatabaseUpdate::ColumnExists(Tokens toks, const char* search)
{
    if (!toks.empty())
    {
        Tokens::iterator iter = toks.begin();
        while(iter != toks.end())
        {
            if (!iter->compare(search))
                return true;

            ++iter;
        }
    }
    else
        return false;

    return false;
}

void DatabaseUpdate::WriteInsertSQL(const char* user, const char* pw, const char* host, const char* wdbdb,
                                    const char* worlddb, uint32 todo, const char* home)
{
    Database* DB = new Database;

    // MySQL Initialisieren
    if (!DB->Initialize(user, pw, NULL, host))
        io.SayError("Can't connect to mysql server");

    bool own_gos = false;

    if (todo & OWN_GOS)
        own_gos = true;

    if (todo & CREATURE)
        CreateCreatureInsert(wdbdb, worlddb, DB, home);

    if (todo & GAMEOBJECT)
        CreateGameobjectInsert(wdbdb, worlddb, DB, home, own_gos); // own_gos -> lootid

    if (todo & ITEM)
        CreateItemInsert(wdbdb, worlddb, DB, home);

    if (todo & NPCTEXT)
        CreateNPCTextInsert(wdbdb, worlddb, DB, home);

    if (todo & PAGETEXT)
        CreatePageTextInsert(wdbdb, worlddb, DB, home);

    if (todo & QUEST)
        CreateQuestInsert(wdbdb, worlddb, DB, home);

    delete DB;
}

void DatabaseUpdate::WriteUpdateSQL(const char* user, const char* pw, const char* host, const char* wdbdb,
                                    const char* worlddb, uint32 todo, const char* home, bool column)
{
    Database* DB = new Database;
    bool owngos = false;
    Tokens toks;

    // MySQL Initialisieren
    if (!DB->Initialize(user, pw, NULL, host))
        io.SayError("Can't connect to mysql server");

    if (todo & OWN_GOS)
        owngos = true;

    if (todo & CREATURE)
    {
        if (column)
            toks = io.StrSplit(io.GetColumnFileData(_CREATURE_COLUMNS), " ");

        CreateCreatureUpdate(wdbdb, worlddb, DB, home, toks, column);
    }
    if (todo & GAMEOBJECT)
    {
        if (column)
            toks = io.StrSplit(io.GetColumnFileData(_GAMEOBJECT_COLUMNS), " ");

        CreateGameobjectUpdate(wdbdb, worlddb, DB, home, owngos, toks, column); // owngos -> lootid
    }
    if (todo & ITEM)
    {
        if (column)
            toks = io.StrSplit(io.GetColumnFileData(_ITEM_COLUMNS), " ");

        CreateItemUpdate(wdbdb, worlddb, DB, home, toks, column);
    }
    if (todo & NPCTEXT)
    {
        if (column)
            toks = io.StrSplit(io.GetColumnFileData(_NPCTEXT_COLUMNS), " ");

        CreateNPCTextUpdate(wdbdb, worlddb, DB, home, toks, column);
    }
    if (todo & PAGETEXT)
    {
        if (column)
            toks = io.StrSplit(io.GetColumnFileData(_PAGETEXT_COLUMNS), " ");

        CreatePageTextUpdate(wdbdb, worlddb, DB, home, toks, column);
    }
    if (todo & QUEST)
    {
        if (column)
            toks = io.StrSplit(io.GetColumnFileData(_QUEST_COLUMNS), " ");

        CreateQuestUpdate(wdbdb, worlddb, DB, home, toks, column);
    }
    delete DB;
}

void DatabaseUpdate::CreateCreatureInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string insertsql = "";
    std::string query = "";
    std::string sortsql = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(CREATURE_INSERT_FILE);
#else
    fstr.append("/sql/").append(CREATURE_INSERT_FILE);
#endif
    insertsql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\nSET CHARACTER SET `utf8`;\n\n"

    "INSERT INTO `creature_template` "
    "(`entry`,`name`,`subname`,`IconName`,`type_flags`,`type`,`family`,`rank`,`KillCredit1`,`KillCredit2`,"
    "`modelid1`,`modelid2`,`modelid3`,`modelid4`,`Health_mod`,`Mana_mod`,`RacialLeader`,"
    "`questItem1`,`questItem2`,`questItem3`,`questItem4`,`questItem5`,`questItem6`,`movementId`,"
    "`minlevel`,`maxlevel`,`faction_A`,`faction_H`,`scale`,`unit_class`,`WDBVerified`) VALUES\n('");

    query.append("SELECT `entry`,`name`,`subname`,`IconName`,`type_flags`,`type`,`family`,`rank`,`KillCredit1`,`KillCredit2`,"
        "`modelid1`,`modelid2`,`modelid3`,`modelid4`,`Health_mod`,`Mana_mod`,`RacialLeader`,"
        "`questItem1`,`questItem2`,`questItem3`,`questItem4`,`questItem5`,`questItem6`,`movementId`"
        " FROM `").append(wdbdb).append("`.`creaturecache` WHERE `entry` NOT IN "
        "(SELECT `entry` FROM `").append(worlddb).append("`.`creature_template`)");

    printf("Searching for new creatures...\t\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        bool first = true;
        uint32 count = 0;
        uint32 counttotal = 0;

        fputs(insertsql.c_str(), sqlfile);

        do
        {
            count++;

            insertsql.erase();

            if (!first) fputs(",\n('", sqlfile);

            fields = result->Fetch();
            if (fields)
            {
                for (uint8 i=0; i<24; i++)
                {
                    char* tmp = (char*)malloc(32);

                    if (i == 1 || i == 2 || i == 3)
                    {
                        // name + subname + IconName
                        insertsql.append(io.Terminator(fields[i].GetCppString())).append("','");
                    }
                    else if (i == 14 || i == 15)
                    {
                        sprintf(tmp, "%f", fields[i].GetFloat());
                        insertsql.append(tmp).append("','");
                    }
                    else
                    {
                        sprintf(tmp, "%u", fields[i].GetUInt32());
                        insertsql.append(tmp).append("','");
                    }
                    free(tmp);
                }

                // Müssen wegen der Core gesetzt werden beim Insert
                insertsql.append("1','1','35','35','1','1','").append(_WDB_VERIFIED).append("')");

                // Haben wir MAX_INSERTS erreicht und Daten übrig? Dann insert senden und löschen!
                if (count >= MAX_INSERTS && (count+counttotal+1) < result->GetRowCount())
                {
                    insertsql.append(";\n");
                    fputs(insertsql.c_str(), sqlfile);
                    insertsql.clear();
                    counttotal += count;
                    count = 0;

                    insertsql.append("INSERT INTO `creature_template` "
                    "(`entry`,`name`,`subname`,`IconName`,`type_flags`,`type`,`family`,`rank`,`KillCredit1`,`KillCredit2`,"
                    "`modelid1`,`modelid2`,`modelid3`,`modelid4`,`Health_mod`,`Mana_mod`,`RacialLeader`,"
                    "`questItem1`,`questItem2`,`questItem3`,`questItem4`,`questItem5`,`questItem6`,`movementId`,"
                    "`minlevel`,`maxlevel`,`faction_A`,`faction_H`,`scale`,`unit_class`,`WDBVerified`) VALUES\n('");

                    first = true;

                } else first = false;
            }
            fputs(insertsql.c_str(), sqlfile);
            
        } while (result->NextRow());

        char* tmp = (char*)malloc(32);
        sprintf(tmp, "%u", result->GetRowCount());
        sortsql.append(";\n\n# ").append(tmp).append(" new creatures found.\n\nALTER TABLE `creature_template` ORDER BY `entry`;");

        printf("%s new creatures found.\n", tmp);

        fputs(sortsql.c_str(), sqlfile);
        fclose(sqlfile);

        delete result;

        free(tmp);

    } else printf("no new creatures found.\n");
}

void DatabaseUpdate::CreateGameobjectInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home, bool own_style)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string insertsql = "";
    std::string query = "";
    std::string sortsql = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(GAMEOBJECT_INSERT_FILE);
#else
    fstr.append("/sql/").append(GAMEOBJECT_INSERT_FILE);
#endif
    insertsql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\n"
        "SET CHARACTER SET `utf8`;\n\nINSERT INTO `gameobject_template` "
        "(`entry`,`type`,`displayId`,`name`,`IconName`,`castBarCaption`,`unk1`,`data0`,`data1`,`data2`,`data3`,`data4`,`data5`,"
        "`data6`,`data7`,`data8`,`data9`,`data10`,`data11`,`data12`,`data13`,`data14`,`data15`,`data16`,`data17`,"
        "`data18`,`data19`,`data20`,`data21`,`data22`,`data23`,`size`,"
        "`questItem1`,`questItem2`,`questItem3`,`questItem4`,`questItem5`,`questItem6`," // NEU!!!
        "`WDBVerified`) VALUES\n('");

    query.append("SELECT `entry`,`type`,`displayId`,`Name1`,`IconName`,`castBarCaption`,`unk1`,`data0`,`data1`,`data2`,`data3`,`data4`,"
        "`data5`,`data6`,`data7`,`data8`,`data9`,`data10`,`data11`,`data12`,`data13`,`data14`,`data15`,`data16`,"
        "`data17`,`data18`,`data19`,`data20`,`data21`,`data22`,`data23`,`size`,"
        "`questItem1`,`questItem2`,`questItem3`,`questItem4`,`questItem5`,`questItem6`" // NEU!!!
        " FROM `");
    query.append(wdbdb).append("`.`gameobjectcache` WHERE `entry` NOT IN "
        "(SELECT `entry` FROM `").append(worlddb).append("`.`gameobject_template`)");

    printf("Searching for new gameobjects...\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        bool first = true;
        uint32 count = 0;
        uint32 counttotal = 0;

        fputs(insertsql.c_str(), sqlfile);

        do
        {
            count++;

            insertsql.erase();

            if (!first) fputs(",\n('", sqlfile);

            fields = result->Fetch();
            if (fields)
            {
                for (int8 i=0; i<31; ++i)
                {
                    char* tmp = (char*)malloc(32);

                    if (i > 2 && i < 7)
                    {
                        // name1 + IconName + castBarCaption + unk1
                        insertsql.append(io.Terminator(fields[i].GetCppString())).append("','");
                    }
                    else
                    {
                        if (own_style)
                        {   // Kisten und Angelplätze
                            if (i == 8 && (fields[i].GetUInt32() == 3 || fields[i].GetUInt32() == 25))
                            {
                                sprintf(tmp, "%u", fields[0].GetUInt32()); // entry als lootid setzen
                                insertsql.append(tmp).append("','");
                            }
                            else
                            {
                                sprintf(tmp, "%i", fields[i].GetInt32());
                                insertsql.append(tmp).append("','");
                            }
                        }
                        else
                        {
                            sprintf(tmp, "%i", fields[i].GetInt32());
                            insertsql.append(tmp).append("','");
                        }
                    }
                    free(tmp);
                }
                // size
                char* tmp = (char*)malloc(32);
                sprintf(tmp, "%f", fields[31].GetFloat());
                insertsql.append(tmp).append("','");
                free(tmp);

                // questItem1
                tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[32].GetUInt32());
                insertsql.append(tmp).append("','");
                free(tmp);
                // questItem2
                tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[33].GetUInt32());
                insertsql.append(tmp).append("','");
                free(tmp);
                // questItem3
                tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[34].GetUInt32());
                insertsql.append(tmp).append("','");
                free(tmp);
                // questItem4
                tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[35].GetUInt32());
                insertsql.append(tmp).append("','");
                free(tmp);
                // questItem5
                tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[36].GetUInt32());
                insertsql.append(tmp).append("','");
                free(tmp);
                // questItem6
                tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[37].GetUInt32());
                insertsql.append(tmp).append("','").append(_WDB_VERIFIED).append("')");
                free(tmp);

                // Haben wir MAX_INSERTS erreicht und Daten übrig? Dann insert senden und löschen!
                if (count >= MAX_INSERTS && (count+counttotal+1) < result->GetRowCount())
                {
                    insertsql.append(";\n");
                    fputs(insertsql.c_str(), sqlfile);
                    insertsql.clear();
                    counttotal += count;
                    count = 0;

                    insertsql.append("INSERT INTO `gameobject_template` "
                        "(`entry`,`type`,`displayId`,`name`,`IconName`,`castBarCaption`,`unk1`,`data0`,`data1`,`data2`,`data3`,`data4`,`data5`,"
                        "`data6`,`data7`,`data8`,`data9`,`data10`,`data11`,`data12`,`data13`,`data14`,`data15`,`data16`,`data17`,"
                        "`data18`,`data19`,`data20`,`data21`,`data22`,`data23`,`size`,"
                        "`questItem1`,`questItem2`,`questItem3`,`questItem4`,`questItem5`,`questItem6`," // NEU!!!
                        "`WDBVerified`) VALUES\n('");

                    first = true;

                } else first = false;
            }
            fputs(insertsql.c_str(), sqlfile);
            
        } while (result->NextRow());

        char* tmp = (char*)malloc(32);
        sprintf(tmp, "%u", result->GetRowCount());
        sortsql.append(";\n\n# ").append(tmp).append(" new gameobjects found.\n\nALTER TABLE `gameobject_template` ORDER BY `entry`;");

        printf("%s new gameobjects found.\n", tmp);

        fputs(sortsql.c_str(), sqlfile);
        fclose(sqlfile);

        delete result;

        free(tmp);

    } else printf("no new gameobjects found.\n");
}

void DatabaseUpdate::CreateItemInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string insertsql = "";
    std::string query = "";
    std::string sortsql = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(ITEM_INSERT_FILE);
#else
    fstr.append("/sql/").append(ITEM_INSERT_FILE);
#endif
    insertsql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\n"
        "SET CHARACTER SET `utf8`;\n\nINSERT INTO `item_template` "
        "(`entry`,`class`,`subclass`,`unk0`,`name`,`displayid`,`Quality`,`Flags`,`BuyPrice`,`SellPrice`,`InventoryType`,"
        "`AllowableClass`,`AllowableRace`,`ItemLevel`,`RequiredLevel`,`RequiredSkill`,`RequiredSkillRank`,"
        "`requiredspell`,`requiredhonorrank`,`RequiredCityRank`,`RequiredReputationFaction`,"
        "`RequiredReputationRank`,`maxcount`,`stackable`,`ContainerSlots`,`StatsCount`,`stat_type1`,`stat_value1`,"
        "`stat_type2`,`stat_value2`,`stat_type3`,`stat_value3`,`stat_type4`,`stat_value4`,`stat_type5`,`stat_value5`,"
        "`stat_type6`,`stat_value6`,`stat_type7`,`stat_value7`,`stat_type8`,`stat_value8`,`stat_type9`,"
        "`stat_value9`,`stat_type10`,`stat_value10`,`ScalingStatDistribution`,`ScalingStatValue`,"
        "`dmg_min1`,`dmg_max1`,`dmg_type1`,`dmg_min2`,`dmg_max2`,`dmg_type2`,"
        "`armor`,`holy_res`,`fire_res`,`nature_res`,`frost_res`,`shadow_res`,`arcane_res`,`delay`,"
        "`ammo_type`,`RangedModRange`,`spellid_1`,`spelltrigger_1`,`spellcharges_1`,`spellcooldown_1`,"
        "`spellcategory_1`,`spellcategorycooldown_1`,`spellid_2`,`spelltrigger_2`,`spellcharges_2`,"
        "`spellcooldown_2`,`spellcategory_2`,`spellcategorycooldown_2`,`spellid_3`,`spelltrigger_3`,"
        "`spellcharges_3`,`spellcooldown_3`,`spellcategory_3`,`spellcategorycooldown_3`,`spellid_4`,"
        "`spelltrigger_4`,`spellcharges_4`,`spellcooldown_4`,`spellcategory_4`,`spellcategorycooldown_4`,"
        "`spellid_5`,`spelltrigger_5`,`spellcharges_5`,`spellcooldown_5`,`spellcategory_5`,"
        "`spellcategorycooldown_5`,`bonding`,`description`,`PageText`,`LanguageID`,`PageMaterial`,`startquest`,"
        "`lockid`,`Material`,`sheath`,`RandomProperty`,`RandomSuffix`,`block`,`itemset`,`MaxDurability`,`area`,"
        "`Map`,`BagFamily`,`TotemCategory`,`socketColor_1`,`socketContent_1`,`socketColor_2`,`socketContent_2`,"
        "`socketColor_3`,`socketContent_3`,`socketBonus`,`GemProperties`,`RequiredDisenchantSkill`,"
        "`ArmorDamageModifier`,`Duration`,`ItemLimitCategory`,`HolidayId`,`WDBVerified`) VALUES\n('");

    //                      0
    query.append("SELECT `entry`,`Class`,`SubClass1`,`unk0`,`Name1`,`ItemDisplayID`,`Quality`,`Flags`,"
        "`BuyPrice`,`SellPrice`,`InventorySlot`,`AllowableClass`,`AllowableRace`,`ItemLevel`,`RequiredLevel`,`RequiredSkill`,"
        "`RequiredSkillRank`,`RequiredSpell`,`RequiredHonorRank`,`RequiredCityRank`,`RequiredReputationFaction`,`RequiredReputationRank`,`maxcount`,`stackable`,"
        "`ContainerSlots`,`StatsCount`,"
        // 26
        "`Stat1`,`Stat1Val`,`Stat2`,`Stat2Val`,`Stat3`,`Stat3Val`,`Stat4`,`Stat4Val`,`Stat5`,`Stat5Val`,`Stat6`,"
        "`Stat6Val`,`Stat7`,`Stat7Val`,`Stat8`,`Stat8Val`,`Stat9`,`Stat9Val`,`Stat10`,`Stat10Val`,"
        "`Stat10`,`Stat10Val`," // Doppel wegen `ScalingStatDistribution`,`ScalingStatValue` im Insert! Damit es paßt!
        // 48
        "`Damage1Min`,`Damage1Max`,`Damage1Type`,`Damage2Min`,`Damage2Max`,`Damage2Type`,"
        // 54
        "`Armor`,`HolyResist`,`FireResist`,`NatureResist`,`FrostResist`,`ShadowResist`,`ArcaneResist`,`Speed`,"
        "`AmmoType`,`RangedModRange`,"
        // 64
        "`Spell1ID`,`Spell1Trigger`,`Spell1Charges`,`Spell1Cooldown`,`Spell1Category`,`Spell1CategoryCooldown`,"
        "`Spell2ID`,`Spell2Trigger`,`Spell2Charges`,`Spell2Cooldown`,`Spell2Category`,`Spell2CategoryCooldown`,"
        "`Spell3ID`,`Spell3Trigger`,`Spell3Charges`,`Spell3Cooldown`,`Spell3Category`,`Spell3CategoryCooldown`,"
        "`Spell4ID`,`Spell4Trigger`,`Spell4Charges`,`Spell4Cooldown`,`Spell4Category`,`Spell4CategoryCooldown`,"
        "`Spell5ID`,`Spell5Trigger`,`Spell5Charges`,`Spell5Cooldown`,`Spell5Category`,`Spell5CategoryCooldown`,"
        // 94
        "`Bonding`,`Description`,`BookTextID`,`LanguageID`,`PageMaterial`,`BeginQuestID`,`LockID`,`Material`,"
        "`Sheath`,`RandomProperty`,`RandomSuffix`,`BlockValue`,`ItemSetID`,`Durability`,`AreaID`,"
        "`ItemMapID`,`BagFamily`,`TotemCategory`,`SocketColor1`,`SocketContent1`,`SocketColor2`,`SocketContent2`,"
        "`SocketColor3`,`SocketContent3`,`socketBonus`,`GemProperties`,`requiredDisenchantSkill`,"
        "`armorDamageModifier`,`Duration`,`ItemLimitCategory`,`HolidayId` FROM `");

        //int felder 3,7,8,11,12,22,23,26-47,64-93,94,98,101,102,103,120,122
        //float felder 48,49,51,52,63,121
    
    query.append(wdbdb).append("`.`itemcache` WHERE `entry` NOT IN (SELECT `entry` FROM `").append(worlddb).append("`.`item_template`)");

    printf("Searching for new items...\t\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        bool first = true;
        uint32 count = 0;
        uint32 counttotal = 0;

        fputs(insertsql.c_str(), sqlfile);

        do
        {
            count++;

            insertsql.erase();

            if (!first) fputs(",\n('", sqlfile);

            fields = result->Fetch();
            if (fields)
            {
                for (uint8 i=0; i<125; i++)
                {
                    char* tmp = (char*)malloc(32);

                    switch(i)
                    {
                        // string felder
                        case 4: case 95:
                            {   // Texte terminieren
                                if (i+1 < 125) insertsql.append(io.Terminator(fields[i].GetCppString())).append("','");
                                else insertsql.append(io.Terminator(fields[i].GetCppString()));
                            }
                            break;
                        // float felder
                        case 48: case 49: case 51: case 52: case 63: case 121:
                            {
                                sprintf(tmp, "%f", fields[i].GetFloat());
                                if (i+1 < 125) insertsql.append(tmp).append("','");
                                else insertsql.append(tmp);
                            }
                            break;
                        // int32 felder
                        case 3: case 7: case 8: case 11: case 12: case 22: case 23: case 26: case 27: case 28: case 29: case 30: case 31: case 32: case 33:
                        case 34: case 35: case 36: case 37: case 38: case 39: case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47:

                        // All Spell related fields (int32)
                        case 64: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78:
                        case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 91: case 92:  case 93:

                        // int32 felder
                        case 94: case 98: case 101: case 102: case 103: case 120: case 122:
                            {
                                int32 tmpint = fields[i].GetInt32();

                                // Account gebundene Items mit veränderlichen Stats!
                                if (i == 44 || i == 45)// StatType10 + StatValue10
                                {                      // StatCount
                                    if (tmpint != 0 && fields[25].GetInt32() == 0)
                                    {   // Bei `ScalingStatDistribution`,`ScalingStatValue` den Wert neu setzen
                                        char* tmpchar = (char*)malloc(32);
                                        fields[i+2].SetValue(_itoa(tmpint, tmpchar, 10));
                                        free(tmpchar);
                                    }
                                }
                                // TODO: REMOVE IF CORE SUPPORTS INT VALUES!
                                // RandomProperty - Workaround until Trinitycore accept -1 values
                                if (i == 103 && tmpint < 0)
                                    tmpint = 0;

                                sprintf(tmp, "%i", tmpint);
                                if (i+1 < 125) insertsql.append(tmp).append("','");
                                else insertsql.append(tmp);
                            }
                            break;
                        // uint32 felder
                        default:
                            {
                                uint32 tmpuint = fields[i].GetUInt32(); /*
                                ########################################################################################################
                                # Faction-"Korrektur" wegen core! :-( UPDATE `itemcache` SET `reqfactionlvl`='0' WHERE `reqfaction`='0';
                                ######################################################################################################## */
                                if (i == 21 && tmpuint > 0)
                                    if (fields[20].GetUInt32() == 0) tmpuint = 0;

                                if (tmpuint == 4294967295) tmpuint = 0;

                                sprintf(tmp, "%u", tmpuint);
                                if (i+1 < 125) insertsql.append(tmp).append("','");
                                else insertsql.append(tmp);
                            }
                            break;
                    }
                    free(tmp);
                }
                insertsql.append("','").append(_WDB_VERIFIED).append("')");

                // Haben wir MAX_INSERTS erreicht und Daten übrig? Dann insert senden und löschen!
                if (count >= MAX_INSERTS && (count+counttotal+1) < result->GetRowCount())
                {
                    insertsql.append(";\n");
                    fputs(insertsql.c_str(), sqlfile);
                    insertsql.clear();
                    counttotal += count;
                    count = 0;

                    insertsql.append("INSERT INTO `item_template` "
                        "(`entry`,`class`,`subclass`,`unk0`,`name`,`displayid`,`Quality`,`Flags`,`BuyPrice`,`SellPrice`,`InventoryType`,"
                        "`AllowableClass`,`AllowableRace`,`ItemLevel`,`RequiredLevel`,`RequiredSkill`,`RequiredSkillRank`,"
                        "`requiredspell`,`requiredhonorrank`,`RequiredCityRank`,`RequiredReputationFaction`,"
                        "`RequiredReputationRank`,`maxcount`,`stackable`,`ContainerSlots`,`StatsCount`,`stat_type1`,`stat_value1`,"
                        "`stat_type2`,`stat_value2`,`stat_type3`,`stat_value3`,`stat_type4`,`stat_value4`,`stat_type5`,`stat_value5`,"
                        "`stat_type6`,`stat_value6`,`stat_type7`,`stat_value7`,`stat_type8`,`stat_value8`,`stat_type9`,"
                        "`stat_value9`,`stat_type10`,`stat_value10`,`ScalingStatDistribution`,`ScalingStatValue`,"
                        "`dmg_min1`,`dmg_max1`,`dmg_type1`,`dmg_min2`,`dmg_max2`,`dmg_type2`,"
                        "`armor`,`holy_res`,`fire_res`,`nature_res`,`frost_res`,`shadow_res`,`arcane_res`,`delay`,"
                        "`ammo_type`,`RangedModRange`,`spellid_1`,`spelltrigger_1`,`spellcharges_1`,`spellcooldown_1`,"
                        "`spellcategory_1`,`spellcategorycooldown_1`,`spellid_2`,`spelltrigger_2`,`spellcharges_2`,"
                        "`spellcooldown_2`,`spellcategory_2`,`spellcategorycooldown_2`,`spellid_3`,`spelltrigger_3`,"
                        "`spellcharges_3`,`spellcooldown_3`,`spellcategory_3`,`spellcategorycooldown_3`,`spellid_4`,"
                        "`spelltrigger_4`,`spellcharges_4`,`spellcooldown_4`,`spellcategory_4`,`spellcategorycooldown_4`,"
                        "`spellid_5`,`spelltrigger_5`,`spellcharges_5`,`spellcooldown_5`,`spellcategory_5`,"
                        "`spellcategorycooldown_5`,`bonding`,`description`,`PageText`,`LanguageID`,`PageMaterial`,`startquest`,"
                        "`lockid`,`Material`,`sheath`,`RandomProperty`,`RandomSuffix`,`block`,`itemset`,`MaxDurability`,`area`,"
                        "`Map`,`BagFamily`,`TotemCategory`,`socketColor_1`,`socketContent_1`,`socketColor_2`,`socketContent_2`,"
                        "`socketColor_3`,`socketContent_3`,`socketBonus`,`GemProperties`,`RequiredDisenchantSkill`,"
                        "`ArmorDamageModifier`,`Duration`,`ItemLimitCategory`,`HolidayId`,`WDBVerified`) VALUES\n('");

                    first = true;

                } else first = false;
            }
            fputs(insertsql.c_str(), sqlfile);
            
        } while (result->NextRow());

        char* tmp = (char*)malloc(32);
        sprintf(tmp, "%u", result->GetRowCount());
        sortsql.append(";\n\n# ").append(tmp).append(" new items found.\n\nALTER TABLE `item_template` ORDER BY `entry`;");

        printf("%s new items found.\n", tmp);

        fputs(sortsql.c_str(), sqlfile);
        fclose(sqlfile);

        delete result;

        free(tmp);

    } else printf("no new items found.\n");
}

void DatabaseUpdate::CreateNPCTextInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string insertsql = "";
    std::string query = "";
    std::string sortsql = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(NPCTEXT_INSERT_FILE);
#else
    fstr.append("/sql/").append(NPCTEXT_INSERT_FILE);
#endif
    insertsql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\n"
        "SET CHARACTER SET `utf8`;\n\nINSERT INTO `npc_text` (`ID`,"
        "`text0_0`,`text0_1`,`lang0`,`prob0`,`em0_0`,`em0_1`,`em0_2`,`em0_3`,`em0_4`,"
        "`text1_0`,`text1_1`,`lang1`,`prob1`,`em1_0`,`em1_1`,`em1_2`,`em1_3`,`em1_4`,"
        "`text2_0`,`text2_1`,`lang2`,`prob2`,`em2_0`,`em2_1`,`em2_2`,`em2_3`,`em2_4`,"
        "`text3_0`,`text3_1`,`lang3`,`prob3`,`em3_0`,`em3_1`,`em3_2`,`em3_3`,`em3_4`,"
        "`text4_0`,`text4_1`,`lang4`,`prob4`,`em4_0`,`em4_1`,`em4_2`,`em4_3`,`em4_4`,"
        "`text5_0`,`text5_1`,`lang5`,`prob5`,`em5_0`,`em5_1`,`em5_2`,`em5_3`,`em5_4`,"
        "`text6_0`,`text6_1`,`lang6`,`prob6`,`em6_0`,`em6_1`,`em6_2`,`em6_3`,`em6_4`,"
        "`text7_0`,`text7_1`,`lang7`,`prob7`,`em7_0`,`em7_1`,`em7_2`,`em7_3`,`em7_4`,"
        "`WDBVerified`) VALUES\n('");

    query.append("SELECT `entry`,"
        "`text0_0`,`text0_1`,`lang0`,`prob0`,`em0_0`,`em0_1`,`em0_2`,`em0_3`,`em0_4`,"
        "`text1_0`,`text1_1`,`lang1`,`prob1`,`em1_0`,`em1_1`,`em1_2`,`em1_3`,`em1_4`,"
        "`text2_0`,`text2_1`,`lang2`,`prob2`,`em2_0`,`em2_1`,`em2_2`,`em2_3`,`em2_4`,"
        "`text3_0`,`text3_1`,`lang3`,`prob3`,`em3_0`,`em3_1`,`em3_2`,`em3_3`,`em3_4`,"
        "`text4_0`,`text4_1`,`lang4`,`prob4`,`em4_0`,`em4_1`,`em4_2`,`em4_3`,`em4_4`,"
        "`text5_0`,`text5_1`,`lang5`,`prob5`,`em5_0`,`em5_1`,`em5_2`,`em5_3`,`em5_4`,"
        "`text6_0`,`text6_1`,`lang6`,`prob6`,`em6_0`,`em6_1`,`em6_2`,`em6_3`,`em6_4`,"
        "`text7_0`,`text7_1`,`lang7`,`prob7`,`em7_0`,`em7_1`,`em7_2`,`em7_3`,`em7_4` FROM `");

    query.append(wdbdb).append("`.`npccache` WHERE `entry` NOT IN "
        "(SELECT `ID` FROM `").append(worlddb).append("`.`npc_text`)");

    printf("Searching for new npc texts...\t\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        bool first = true;
        uint32 count = 0;
        uint32 counttotal = 0;

        fputs(insertsql.c_str(), sqlfile);

        do
        {
            count++;

            insertsql.erase();

            if (!first) fputs(",\n('", sqlfile);

            fields = result->Fetch();
            if (fields)
            {
                for (uint8 i=0; i<73; i++)
                {
                    if (i == 1 || i == 2 || i == 10 || i == 11 || i == 19 || i == 20 || i == 28 || i == 29 ||
                        i == 37 || i == 38 || i == 46 || i == 47 || i == 55 || i == 56 || i == 64 || i == 65)
                    {
                        // Texte terminieren
                        if (i+1 < 73) insertsql.append(io.Terminator(fields[i].GetCppString())).append("','");
                        else insertsql.append(io.Terminator(fields[i].GetCppString()));
                    }
                    else if (i == 4 || i == 13 || i == 22 || i == 31 || i == 40 || i == 49 || i == 58 || i == 67)
                    {
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%f", fields[i].GetFloat());
                        if (i+1 < 73) insertsql.append(tmp).append("','");
                        else insertsql.append(tmp);
                        free(tmp);
                    }
                    else
                    {
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[i].GetUInt32());
                        if (i+1 < 73) insertsql.append(tmp).append("','");
                        else insertsql.append(tmp);
                        free(tmp);
                    }
                }
                insertsql.append("','").append(_WDB_VERIFIED).append("')");

                // Haben wir MAX_INSERTS erreicht und Daten übrig? Dann insert senden und löschen!
                if (count >= MAX_INSERTS && (count+counttotal+1) < result->GetRowCount())
                {
                    insertsql.append(";\n");
                    fputs(insertsql.c_str(), sqlfile);
                    insertsql.clear();
                    counttotal += count;
                    count = 0;

                    insertsql.append("INSERT INTO `npc_text` (`ID`,"
                        "`text0_0`,`text0_1`,`lang0`,`prob0`,`em0_0`,`em0_1`,`em0_2`,`em0_3`,`em0_4`,"
                        "`text1_0`,`text1_1`,`lang1`,`prob1`,`em1_0`,`em1_1`,`em1_2`,`em1_3`,`em1_4`,"
                        "`text2_0`,`text2_1`,`lang2`,`prob2`,`em2_0`,`em2_1`,`em2_2`,`em2_3`,`em2_4`,"
                        "`text3_0`,`text3_1`,`lang3`,`prob3`,`em3_0`,`em3_1`,`em3_2`,`em3_3`,`em3_4`,"
                        "`text4_0`,`text4_1`,`lang4`,`prob4`,`em4_0`,`em4_1`,`em4_2`,`em4_3`,`em4_4`,"
                        "`text5_0`,`text5_1`,`lang5`,`prob5`,`em5_0`,`em5_1`,`em5_2`,`em5_3`,`em5_4`,"
                        "`text6_0`,`text6_1`,`lang6`,`prob6`,`em6_0`,`em6_1`,`em6_2`,`em6_3`,`em6_4`,"
                        "`text7_0`,`text7_1`,`lang7`,`prob7`,`em7_0`,`em7_1`,`em7_2`,`em7_3`,`em7_4`,"
                        "`WDBVerified`) VALUES\n('");

                    first = true;

                } else first = false;
            }
            fputs(insertsql.c_str(), sqlfile);
            
        } while (result->NextRow());

        char* tmp = (char*)malloc(32);
        sprintf(tmp, "%u", result->GetRowCount());
        sortsql.append(";\n\n# ").append(tmp).append(" new npc texts found.\n\nALTER TABLE `npc_text` ORDER BY `ID`;");

        printf("%s new npc texts found.\n", tmp);

        fputs(sortsql.c_str(), sqlfile);
        fclose(sqlfile);

        delete result;

        free(tmp);

    } else printf("no new npc texts found.\n");
}

void DatabaseUpdate::CreatePageTextInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string insertsql = "";
    std::string query = "";
    std::string sortsql = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(PAGETEXT_INSERT_FILE);
#else
    fstr.append("/sql/").append(PAGETEXT_INSERT_FILE);
#endif
    insertsql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\n"
        "SET CHARACTER SET `utf8`;\n\nINSERT INTO `page_text` "
        "(`entry`,`text`,`next_page`,`WDBVerified`) VALUES\n('");

    query.append("SELECT `entry`,`text`,`next_page` FROM `");

    query.append(wdbdb).append("`.`pagetextcache` WHERE `entry` NOT IN "
        "(SELECT `entry` FROM `").append(worlddb).append("`.`page_text`)");

    printf("Searching for new page texts...\t\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        bool first = true;
        uint32 count = 0;
        uint32 counttotal = 0;

        fputs(insertsql.c_str(), sqlfile);

        do
        {
            count++;

            insertsql.erase();

            if (!first) fputs(",\n('", sqlfile);

            fields = result->Fetch();
            if (fields)
            {
                // entry
                char* tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[0].GetUInt32());
                insertsql.append(tmp).append("','");
                free(tmp);

                // text
                insertsql.append(io.Terminator(fields[1].GetCppString())).append("','");

                // next_page
                tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[2].GetUInt32());
                insertsql.append(tmp).append("','").append(_WDB_VERIFIED).append("')");
                free(tmp);

                // Haben wir MAX_INSERTS erreicht und Daten übrig? Dann insert senden und löschen!
                if (count >= MAX_INSERTS && (count+counttotal+1) < result->GetRowCount())
                {
                    insertsql.append(";\n");
                    fputs(insertsql.c_str(), sqlfile);
                    insertsql.clear();
                    counttotal += count;
                    count = 0;

                    insertsql.append("INSERT INTO `page_text` "
                        "(`entry`,`text`,`next_page`,`WDBVerified`) VALUES\n('");

                    first = true;

                } else first = false;
            }
            fputs(insertsql.c_str(), sqlfile);
            
        } while (result->NextRow());

        char* tmp = (char*)malloc(32);
        sprintf(tmp, "%u", result->GetRowCount());
        sortsql.append(";\n\n# ").append(tmp).append(" new page texts found.\n\nALTER TABLE `page_text` ORDER BY `entry`;");

        printf("%s new page texts found.\n", tmp);

        fputs(sortsql.c_str(), sqlfile);
        fclose(sqlfile);

        delete result;

        free(tmp);

    } else printf("no new page texts found.\n");
}

void DatabaseUpdate::CreateQuestInsert(const char* wdbdb, const char* worlddb, Database* DB, const char* home)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string insertsql = "";
    std::string query = "";
    std::string sortsql = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(QUEST_INSERT_FILE);
#else
    fstr.append("/sql/").append(QUEST_INSERT_FILE);
#endif
    insertsql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\n"
        "SET CHARACTER SET `utf8`;\n\nINSERT INTO `quest_template` "
        "(`entry`,`Method`,`QuestLevel`,`MinLevel`,`ZoneOrSort`,`Type`,`SuggestedPlayers`,"
        // 7
        "`RepObjectiveFaction`,`RepObjectiveValue`,`RepObjectiveFaction2`,`RepObjectiveValue2`,`NextQuestInChain`,"
        // 12
        "`RewXPId`,`RewOrReqMoney`,`RewMoneyMaxLevel`,`RewSpell`,`RewSpellCast`,`RewHonorAddition`,`RewHonorMultiplier`,"
        // 19
        "`SrcItemId`,`QuestFlags`,`CharTitleId`,`PlayersSlain`,`BonusTalents`,`RewardArenaPoints`,`unk0`,"
        // 26
        "`RewItemId1`,`RewItemCount1`,`RewItemId2`,`RewItemCount2`,`RewItemId3`,`RewItemCount3`,`RewItemId4`,`RewItemCount4`,"
        // 34
        "`RewChoiceItemId1`,`RewChoiceItemCount1`,`RewChoiceItemId2`,`RewChoiceItemCount2`,`RewChoiceItemId3`,`RewChoiceItemCount3`,"
        // 40
        "`RewChoiceItemId4`,`RewChoiceItemCount4`,`RewChoiceItemId5`,`RewChoiceItemCount5`,`RewChoiceItemId6`,`RewChoiceItemCount6`,"
        // 46
        "`RewRepFaction1`,`RewRepFaction2`,`RewRepFaction3`,`RewRepFaction4`,`RewRepFaction5`,"
        // 51
        "`RewRepValueId1`,`RewRepValueId2`,`RewRepValueId3`,`RewRepValueId4`,`RewRepValueId5`,"
        // 56
        "`RewRepValue1`,`RewRepValue2`,`RewRepValue3`,`RewRepValue4`,`RewRepValue5`,"
        // 61
        "`PointMapId`,`PointX`,`PointY`,`PointOpt`,"
        // 65
        "`Title`,`Objectives`,`Details`,`EndText`,`CompletedText`,"
        // 70
        "`ReqCreatureOrGOId1`,`ReqCreatureOrGOCount1`,`ReqSourceId1`,`ReqSourceCount1`,"
        // 74
        "`ReqCreatureOrGOId2`,`ReqCreatureOrGOCount2`,`ReqSourceId2`,`ReqSourceCount2`,"
        // 78
        "`ReqCreatureOrGOId3`,`ReqCreatureOrGOCount3`,`ReqSourceId3`,`ReqSourceCount3`,"
        // 82
        "`ReqCreatureOrGOId4`,`ReqCreatureOrGOCount4`,`ReqSourceId4`,`ReqSourceCount4`,"
        // 86
        "`ReqItemId1`,`ReqItemCount1`,`ReqItemId2`,`ReqItemCount2`,`ReqItemId3`,`ReqItemCount3`,"
        // 92
        "`ReqItemId4`,`ReqItemCount4`,`ReqItemId5`,`ReqItemCount5`,`ReqItemId6`,`ReqItemCount6`,"
        // 98                                                    101
        "`ObjectiveText1`,`ObjectiveText2`,`ObjectiveText3`,`ObjectiveText4`,"
        // Set SrcItemCount to 1 if SrcItemId exists
        "`SrcItemCount`,"
        "`WDBVerified`) VALUES\n('");

    query.append("SELECT `entry`,`Method`,`QuestLevel`,`MinLevel`,`ZoneOrSort`,`Type`,`SuggestedPlayers`,"
        "`RepObjectiveFaction`,`RepObjectiveValue`,`RepObjectiveFaction2`,`RepObjectiveValue2`,`NextQuestInChain`,"
        "`RewXPId`,`RewOrReqMoney`,`RewMoneyMaxLevel`,`RewSpell`,`RewSpellCast`,`RewHonorAddition`,`RewHonorMultiplier`,"
        "`SrcItemId`,`QuestFlags`,`CharTitleId`,`PlayersSlain`,`BonusTalents`,`RewardArenaPoints`,`unk0`,"
        "`RewItemId1`,`RewItemCount1`,`RewItemId2`,`RewItemCount2`,`RewItemId3`,`RewItemCount3`,`RewItemId4`,`RewItemCount4`,"
        "`RewChoiceItemId1`,`RewChoiceItemCount1`,`RewChoiceItemId2`,`RewChoiceItemCount2`,`RewChoiceItemId3`,`RewChoiceItemCount3`,"
        "`RewChoiceItemId4`,`RewChoiceItemCount4`,`RewChoiceItemId5`,`RewChoiceItemCount5`,`RewChoiceItemId6`,`RewChoiceItemCount6`,"
        "`RewRepFaction1`,`RewRepFaction2`,`RewRepFaction3`,`RewRepFaction4`,`RewRepFaction5`,"
        "`RewRepValueId1`,`RewRepValueId2`,`RewRepValueId3`,`RewRepValueId4`,`RewRepValueId5`,"
        "`RewRepValue1`,`RewRepValue2`,`RewRepValue3`,`RewRepValue4`,`RewRepValue5`,"
        "`PointMapId`,`PointX`,`PointY`,`PointOpt`,"
        "`Title`,`Objectives`,`Details`,`EndText`,`CompletedText`,"
        "`ReqCreatureOrGOId1`,`ReqCreatureOrGOCount1`,`ReqSourceId1`,`ReqSourceCount1`,"
        "`ReqCreatureOrGOId2`,`ReqCreatureOrGOCount2`,`ReqSourceId2`,`ReqSourceCount2`,"
        "`ReqCreatureOrGOId3`,`ReqCreatureOrGOCount3`,`ReqSourceId3`,`ReqSourceCount3`,"
        "`ReqCreatureOrGOId4`,`ReqCreatureOrGOCount4`,`ReqSourceId4`,`ReqSourceCount4`,"
        "`ReqItemId1`,`ReqItemCount1`,`ReqItemId2`,`ReqItemCount2`,`ReqItemId3`,`ReqItemCount3`,"
        "`ReqItemId4`,`ReqItemCount4`,`ReqItemId5`,`ReqItemCount5`,`ReqItemId6`,`ReqItemCount6`,"
        "`ObjectiveText1`,`ObjectiveText2`,`ObjectiveText3`,`ObjectiveText4` FROM `");

    query.append(wdbdb).append("`.`questcache` WHERE `entry` NOT IN "
        "(SELECT `entry` FROM `").append(worlddb).append("`.`quest_template`)");

    printf("Searching for new quests...\t\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        bool first = true;
        bool sourceitem = false;
        uint32 count = 0;
        uint32 counttotal = 0;
        uint8 MAX_FIELDS = 102; // Max query values

        fputs(insertsql.c_str(), sqlfile);

        do
        {
            count++;

            insertsql.erase();

            if (!first) fputs(",\n('", sqlfile);

            fields = result->Fetch();
            if (fields)
            {
                for (uint8 i=0; i<MAX_FIELDS; ++i)
                {
                    char* tmp = (char*)malloc(32);

                    if ((i > 0 && i < 18) ||
                        (i > 18 && i < 62) ||
                        (i > 69 && i < 98) ||
                        i == 64)
                    {
                        int32 tmpint = fields[i].GetInt32();

                        if (i == 19 && tmpint > 0)
                            sourceitem = true;

                        // Mask QuestFlags
                        if (i == 20)
                            tmpint = tmpint&0xFFFF;

                        // GOs umrechnen für den Core
                        if ((i == 70 || i == 74 || i == 78 || i == 82) && tmpint < 0)
                            sprintf(tmp, "%i", (tmpint + 2147483648)*-1);
                        else
                            sprintf(tmp, "%i", tmpint);

                        if (i+1 < MAX_FIELDS)
                            insertsql.append(tmp).append("','");
                        else
                            insertsql.append(tmp);
                    }
                    else if ((i > 64 && i < 70) || (i > 97 && i < MAX_FIELDS))
                    {
                        // Texte terminieren
                        if (i+1 < MAX_FIELDS)
                            insertsql.append(io.Terminator(fields[i].GetCppString())).append("','");
                        else
                            insertsql.append(io.Terminator(fields[i].GetCppString()));
                    }
                    else if (i == 18 || i == 62 || i == 63)
                    {
                        // RewHonorMultiplier + PointX + PointY
                        sprintf(tmp, "%f", fields[i].GetFloat());
                        if (i+1 < MAX_FIELDS)
                            insertsql.append(tmp).append("','");
                        else
                            insertsql.append(tmp);
                    }
                    else
                    {
                        sprintf(tmp, "%u", fields[i].GetUInt32());
                        if (i+1 < MAX_FIELDS)
                            insertsql.append(tmp).append("','");
                        else
                            insertsql.append(tmp);
                    }
                    free(tmp);
                }
                // Set SrcItemCount to 1 if SrcItemId exists
                if (sourceitem)
                    insertsql.append("','1','").append(_WDB_VERIFIED).append("')");
                else
                    insertsql.append("','0','").append(_WDB_VERIFIED).append("')");

                sourceitem = false;

                // Haben wir MAX_INSERTS erreicht und Daten übrig, dann Insert senden und löschen?!
                if (count >= MAX_INSERTS && (count+counttotal+1) < result->GetRowCount())
                {
                    insertsql.append(";\n");
                    fputs(insertsql.c_str(), sqlfile);
                    insertsql.clear();
                    counttotal += count;
                    count = 0;

                    insertsql.append("INSERT INTO `quest_template` "
                        "(`entry`,`Method`,`QuestLevel`,`MinLevel`,`ZoneOrSort`,`Type`,`SuggestedPlayers`,"
                        "`RepObjectiveFaction`,`RepObjectiveValue`,`RepObjectiveFaction2`,`RepObjectiveValue2`,`NextQuestInChain`,"
                        "`RewXPId`,`RewOrReqMoney`,`RewMoneyMaxLevel`,`RewSpell`,`RewSpellCast`,`RewHonorAddition`,`RewHonorMultiplier`,"
                        "`SrcItemId`,`QuestFlags`,`CharTitleId`,`PlayersSlain`,`BonusTalents`,`RewardArenaPoints`,`unk0`,"
                        "`RewItemId1`,`RewItemCount1`,`RewItemId2`,`RewItemCount2`,`RewItemId3`,`RewItemCount3`,`RewItemId4`,`RewItemCount4`,"
                        "`RewChoiceItemId1`,`RewChoiceItemCount1`,`RewChoiceItemId2`,`RewChoiceItemCount2`,`RewChoiceItemId3`,`RewChoiceItemCount3`,"
                        "`RewChoiceItemId4`,`RewChoiceItemCount4`,`RewChoiceItemId5`,`RewChoiceItemCount5`,`RewChoiceItemId6`,`RewChoiceItemCount6`,"
                        "`RewRepFaction1`,`RewRepFaction2`,`RewRepFaction3`,`RewRepFaction4`,`RewRepFaction5`,"
                        "`RewRepValueId1`,`RewRepValueId2`,`RewRepValueId3`,`RewRepValueId4`,`RewRepValueId5`,"
                        "`RewRepValue1`,`RewRepValue2`,`RewRepValue3`,`RewRepValue4`,`RewRepValue5`,"
                        "`PointMapId`,`PointX`,`PointY`,`PointOpt`,"
                        "`Title`,`Objectives`,`Details`,`EndText`,`CompletedText`,"
                        "`ReqCreatureOrGOId1`,`ReqCreatureOrGOCount1`,`ReqSourceId1`,`ReqSourceCount1`,"
                        "`ReqCreatureOrGOId2`,`ReqCreatureOrGOCount2`,`ReqSourceId2`,`ReqSourceCount2`,"
                        "`ReqCreatureOrGOId3`,`ReqCreatureOrGOCount3`,`ReqSourceId3`,`ReqSourceCount3`,"
                        "`ReqCreatureOrGOId4`,`ReqCreatureOrGOCount4`,`ReqSourceId4`,`ReqSourceCount4`,"
                        "`ReqItemId1`,`ReqItemCount1`,`ReqItemId2`,`ReqItemCount2`,`ReqItemId3`,`ReqItemCount3`,"
                        "`ReqItemId4`,`ReqItemCount4`,`ReqItemId5`,`ReqItemCount5`,`ReqItemId6`,`ReqItemCount6`,"
                        "`ObjectiveText1`,`ObjectiveText2`,`ObjectiveText3`,`ObjectiveText4`,"
                        // Set SrcItemCount to 1 if SrcItemId exists
                        "`SrcItemCount`,"
                        "`WDBVerified`) VALUES\n('");

                    first = true;

                } else first = false;
            }
            fputs(insertsql.c_str(), sqlfile);
            
        } while (result->NextRow());

        char* tmp = (char*)malloc(32);
        sprintf(tmp, "%u", result->GetRowCount());
        sortsql.append(";\n\n# ").append(tmp).append(" new quests found.\n\n"
            "ALTER TABLE `quest_template` ORDER BY `entry`;");

        printf("%s new quests found.\n", tmp);

        fputs(sortsql.c_str(), sqlfile);
        fclose(sqlfile);

        delete result;

        free(tmp);

    } else printf("no new quests found.\n");
}

void DatabaseUpdate::CreateCreatureUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, Tokens columns, bool docolumn)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string updatesql = "";
    std::string query = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(CREATURE_UPDATE_FILE);
#else
    fstr.append("/sql/").append(CREATURE_UPDATE_FILE);
#endif
    updatesql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\nSET CHARACTER SET `utf8`;\n\n"

        "UPDATE `creature_template` ");

    query.append("SELECT WDB.entry, WDB.name, WORLD.name, WDB.subname, WORLD.subname, WDB.IconName, WORLD.IconName, "
        "WDB.type_flags, WORLD.type_flags, WDB.`type`, WORLD.`type`, WDB.family, "
        "WORLD.family, WDB.rank, WORLD.rank, WDB.KillCredit1, WORLD.KillCredit1, WDB.KillCredit2, WORLD.KillCredit2, "
        "WDB.modelid1, WORLD.modelid1, WDB.modelid2, WORLD.modelid2, WDB.modelid3, WORLD.modelid3, WDB.modelid4, "
        "WORLD.modelid4, WDB.Health_mod, WORLD.Health_mod, WDB.Mana_mod, WORLD.Mana_mod, WDB.RacialLeader, WORLD.RacialLeader, "
        "WDB.questItem1, WORLD.questItem1, WDB.questItem2, WORLD.questItem2, WDB.questItem3, WORLD.questItem3, WDB.questItem4, WORLD.questItem4, "
        "WDB.questItem5, WORLD.questItem5, WDB.questItem6, WORLD.questItem6, WDB.movementId, WORLD.movementId"
        " FROM `");
    query.append(wdbdb).append("`.`creaturecache` AS WDB, `");
    query.append(worlddb).append("`.`creature_template` AS WORLD WHERE WDB.entry = WORLD.entry AND "
        "(WDB.name != WORLD.name || WDB.subname != WORLD.subname || WDB.IconName != WORLD.IconName || "
        "WDB.type_flags != WORLD.type_flags || WDB.`type` != WORLD.`type` || WDB.family != WORLD.family || WDB.rank != WORLD.rank || "
        "WDB.KillCredit1 != WORLD.KillCredit1 || WDB.KillCredit2 != WORLD.KillCredit2 || "
        "WDB.modelid1 != WORLD.modelid1 || WDB.modelid2 != WORLD.modelid2 || WDB.modelid3 != WORLD.modelid3 || "
        "WDB.modelid4 != WORLD.modelid4 || WDB.Health_mod != WORLD.Health_mod || WDB.Mana_mod != WORLD.Mana_mod || WDB.RacialLeader != WORLD.RacialLeader || "
        "WDB.questItem1 != WORLD.questItem1 || WDB.questItem2 != WORLD.questItem2 || WDB.questItem3 != WORLD.questItem3 || WDB.questItem4 != WORLD.questItem4 || "
        "WDB.questItem5 != WORLD.questItem5 || WDB.questItem6 != WORLD.questItem6 || "
        "WDB.movementId != WORLD.movementId)");

    printf("Searching for different creatures...\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        uint64 count = 0;

        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        const char* column1[10] = {"type_flags", "type", "family", "rank", "KillCredit1", "KillCredit2", "modelid1", "modelid2", "modelid3", "modelid4"};
        const char* column2[8] = {"RacialLeader", "questItem1", "questItem2", "questItem3", "questItem4", "questItem5", "questItem6", "movementId"};

        do
        {
            bool first = true;
            fields = result->Fetch();

            if (fields)
            {
                if ((docolumn && ColumnExists(columns, "name")) || !docolumn)
                {
                    if (strcmp(fields[1].GetCppString().c_str(), fields[2].GetCppString().c_str()) != 0)
                    {
                        if (first) updatesql.append("SET `name`='").append(io.Terminator(fields[1].GetCppString())).append("',");
                        else updatesql.append("`name`='").append(io.Terminator(fields[1].GetCppString())).append("',");
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "subname")) || !docolumn)
                {
                    if (strcmp(fields[3].GetCppString().c_str(), fields[4].GetCppString().c_str()) != 0)
                    {
                        if (first) updatesql.append("SET `subname`='").append(io.Terminator(fields[3].GetCppString())).append("',");
                        else updatesql.append("`subname`='").append(io.Terminator(fields[3].GetCppString())).append("',");
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "IconName")) || !docolumn)
                {
                    if (strcmp(fields[5].GetCppString().c_str(), fields[6].GetCppString().c_str()) != 0)
                    {
                        if (first) updatesql.append("SET `IconName`='").append(io.Terminator(fields[5].GetCppString())).append("',");
                        else updatesql.append("`IconName`='").append(io.Terminator(fields[5].GetCppString())).append("',");
                        first = false;
                    }
                }
                for (uint8 i=0; i<10; ++i)
                {
                    if ((docolumn && ColumnExists(columns, column1[i])) || !docolumn)
                    {
                        if (fields[7+i*2].GetUInt32() != fields[8+i*2].GetUInt32())
                        {
                            uint32 tmpuint = fields[7+i*2].GetUInt32();
                            uint32 tmpentry = fields[0].GetUInt32();

                            // KEIN MODELUPDATE FÜR DIESE ENTRIES, WEIL FALSCHE DATEN IN DEN WDBS STEHEN!!!
                            if (tmpentry == 24938 || tmpentry == 25115 || tmpentry == 25001 || tmpentry == 26477) continue;
                            else
                            {
                                if (first) updatesql.append("SET `").append(column1[i]).append("`='");
                                else updatesql.append("`").append(column1[i]).append("`='");
                                char* tmp = (char*)malloc(32);
                                sprintf(tmp, "%u", tmpuint);
                                updatesql.append(tmp).append("',");
                                first = false;
                                free(tmp);
                            }
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "Health_mod")) || !docolumn)
                {
                    // Health_mod
                    if (fields[27].GetFloat() != fields[28].GetFloat())
                    {
                        if (first) updatesql.append("SET `Health_mod`='");
                        else updatesql.append("`Health_mod`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%f", fields[27].GetFloat());
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
                if ((docolumn && ColumnExists(columns, "Mana_mod")) || !docolumn)
                {
                    // Mana_mod
                    if (fields[29].GetFloat() != fields[30].GetFloat())
                    {
                        if (first) updatesql.append("SET `Mana_mod`='");
                        else updatesql.append("`Mana_mod`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%f", fields[29].GetFloat());
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
                for (uint8 i=0; i<8; ++i)
                {
                    if ((docolumn && ColumnExists(columns, column2[i])) || !docolumn)
                    {
                        if (fields[31+i*2].GetUInt32() != fields[32+i*2].GetUInt32())
                        {
                            if (first) updatesql.append("SET `").append(column2[i]).append("`='");
                            else updatesql.append("`").append(column2[i]).append("`='");
                            char* tmp = (char*)malloc(32);
                            sprintf(tmp, "%u", fields[31+i*2].GetUInt32());
                            updatesql.append(tmp).append("',");
                            first = false;
                            free(tmp);
                        }
                    }
                }
            }
            if (!first)
            {
                count++;
                updatesql.resize(updatesql.length()-1);
                char* tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[0].GetUInt32());
                updatesql.append(",`WDBVerified`='").append(_WDB_VERIFIED).append("' WHERE `entry`='").append(tmp).append("' LIMIT 1;\n");
                fputs(updatesql.c_str(), sqlfile);
                updatesql.clear();
                free(tmp);

                updatesql.append("UPDATE `creature_template` ");
            }
        } while(result->NextRow());

        if (count)
        {
            char* tmp = (char*)malloc(32);
            sprintf(tmp, "%u", count);
            updatesql.clear();
            updatesql.append("\n# Differences in ").append(tmp).append(" entries found.");
            fputs(updatesql.c_str(), sqlfile);
            printf("%u different entries found.\n", count);
            free(tmp);
            fclose(sqlfile);
        }
        else
        {
            fclose(sqlfile);
            remove(fstr.c_str());
            printf("no differences found.\n");
        }
        delete result;
    }
}

void DatabaseUpdate::CreateGameobjectUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, bool own_style, Tokens columns, bool docolumn)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string updatesql = "";
    std::string query = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(GAMEOBJECT_UPDATE_FILE);
#else
    fstr.append("/sql/").append(GAMEOBJECT_UPDATE_FILE);
#endif
    updatesql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\n"
        "SET CHARACTER SET `utf8`;\n\n"
        "UPDATE `gameobject_template` ");

    query.append("SELECT WDB.entry, WDB.type, WORLD.type, WDB.displayId, WORLD.displayId, WDB.Name1, WORLD.name,"
        " WDB.IconName, WORLD.IconName, WDB.castBarCaption, WORLD.castBarCaption, WDB.unk1, WORLD.unk1,"
        " WDB.data0, WORLD.data0, WDB.data1, WORLD.data1, WDB.data2, WORLD.data2, WDB.data3, WORLD.data3,"
        " WDB.data4, WORLD.data4, WDB.data5, WORLD.data5, WDB.data6, WORLD.data6, WDB.data7, WORLD.data7,"
        " WDB.data8, WORLD.data8, WDB.data9, WORLD.data9, WDB.data10, WORLD.data10, WDB.data11, WORLD.data11,"
        " WDB.data12, WORLD.data12, WDB.data13, WORLD.data13, WDB.data14, WORLD.data14,"
        " WDB.data15, WORLD.data15, WDB.data16, WORLD.data16, WDB.data17, WORLD.data17,"
        " WDB.data18, WORLD.data18, WDB.data19, WORLD.data19, WDB.data20, WORLD.data20,"
        " WDB.data21, WORLD.data21, WDB.data22, WORLD.data22, WDB.data23, WORLD.data23,"
        " WDB.size, WORLD.size,"
        " WDB.questItem1, WORLD.questItem1, WDB.questItem2, WORLD.questItem2, WDB.questItem3, WORLD.questItem3, WDB.questItem4, WORLD.questItem4,"
        " WDB.questItem5, WORLD.questItem5, WDB.questItem6, WORLD.questItem6" // NEU!!!
        " FROM `");

    query.append(wdbdb).append("`.`gameobjectcache` AS WDB, `");

    query.append(worlddb).append("`.`gameobject_template` AS WORLD WHERE WDB.entry = WORLD.entry AND "
        "(WDB.type != WORLD.type || WDB.displayId != WORLD.displayId || WDB.Name1 != WORLD.name ||"
        " WDB.IconName != WORLD.IconName || WDB.castBarCaption != WORLD.castBarCaption || WDB.unk1 != WORLD.unk1 ||"
        " WDB.data0 != WORLD.data0 ||"
        " WDB.data1 != WORLD.data1 || WDB.data2 != WORLD.data2 || WDB.data3 != WORLD.data3 ||"
        " WDB.data4 != WORLD.data4 || WDB.data5 != WORLD.data5 || WDB.data6 != WORLD.data6 ||"
        " WDB.data7 != WORLD.data7 || WDB.data8 != WORLD.data8 || WDB.data9 != WORLD.data9 ||"
        " WDB.data10 != WORLD.data10 || WDB.data11 != WORLD.data11 || WDB.data12 != WORLD.data12 ||"
        " WDB.data13 != WORLD.data13 || WDB.data14 != WORLD.data14 || WDB.data15 != WORLD.data15 ||"
        " WDB.data16 != WORLD.data16 || WDB.data17 != WORLD.data17 || WDB.data18 != WORLD.data18 ||"
        " WDB.data19 != WORLD.data19 || WDB.data20 != WORLD.data20 || WDB.data21 != WORLD.data21 ||"
        " WDB.data22 != WORLD.data22 || WDB.data23 != WORLD.data23 || WDB.size != WORLD.size ||"
        " WDB.questItem1 != WORLD.questItem1 || WDB.questItem2 != WORLD.questItem2 || WDB.questItem3 != WORLD.questItem3 || WDB.questItem4 != WORLD.questItem4 ||"
        " WDB.questItem5 != WORLD.questItem5 || WDB.questItem6 != WORLD.questItem6)"); // NEU!!!

    printf("Searching for different gameobjects...\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        uint64 count = 0;

        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        const char* column[24] = {"data0","data1","data2","data3","data4","data5","data6","data7","data8","data9",
            "data10","data11","data12","data13","data14","data15","data16","data17","data18","data19","data20",
            "data21","data22","data23"};

        do
        {
            bool first = true;
            fields = result->Fetch();

            if (fields)
            {
                char* tmp;

                if ((docolumn && ColumnExists(columns, "type")) || !docolumn)
                {
                    // type
                    if (fields[1].GetUInt32() != fields[2].GetUInt32())
                    {
                        if (first) updatesql.append("SET `type`='");
                        else updatesql.append("`type`='");

                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[1].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "displayId")) || !docolumn)
                {
                    // displayId
                    if (fields[3].GetUInt32() != fields[4].GetUInt32())
                    {
                        if (first) updatesql.append("SET `displayId`='");
                        else updatesql.append("`displayId`='");

                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[3].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "name")) || !docolumn)
                {
                    // name
                    if (strcmp(fields[5].GetCppString().c_str(), fields[6].GetCppString().c_str()) != 0)
                    {
                        if (first) updatesql.append("SET `name`='").append(io.Terminator(fields[5].GetCppString())).append("',");
                        else updatesql.append("`name`='").append(io.Terminator(fields[5].GetCppString())).append("',");
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "IconName")) || !docolumn)
                {
                    // IconName
                    if (strcmp(fields[7].GetCppString().c_str(), fields[8].GetCppString().c_str()) != 0)
                    {
                        if (first) updatesql.append("SET `IconName`='").append(io.Terminator(fields[7].GetCppString())).append("',");
                        else updatesql.append("`IconName`='").append(io.Terminator(fields[7].GetCppString())).append("',");
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "castBarCaption")) || !docolumn)
                {
                    // castBarCaption
                    if (strcmp(fields[9].GetCppString().c_str(), fields[10].GetCppString().c_str()) != 0)
                    {
                        if (first) updatesql.append("SET `castBarCaption`='").append(io.Terminator(fields[9].GetCppString())).append("',");
                        else updatesql.append("`castBarCaption`='").append(io.Terminator(fields[9].GetCppString())).append("',");
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "unk1")) || !docolumn)
                {
                    // unk1
                    if (strcmp(fields[11].GetCppString().c_str(), fields[12].GetCppString().c_str()) != 0)
                    {
                        if (first) updatesql.append("SET `unk1`='").append(io.Terminator(fields[11].GetCppString())).append("',");
                        else updatesql.append("`unk1`='").append(io.Terminator(fields[11].GetCppString())).append("',");
                        first = false;
                    }
                }
                for (int8 i=0; i<24; ++i)
                {
                    if ((docolumn && ColumnExists(columns, column[i])) || !docolumn)
                    {
                        // TODO: Must be removed if Wintergrasp was fixed by Trinity!
                        // Exception for the Wintergrasp Transporter because a update would stop the work of them atm
                        // Atm this works well: UPDATE `gameobject_template` SET `data0`='54643',`ScriptName`='' WHERE `entry` IN ('190763','192819');
                        if (fields[0].GetUInt32() == 192819 || fields[0].GetUInt32() == 190763)
                            continue;

                        // lootid auf entry setzen
                        if (own_style && i == 1 && fields[0].GetUInt32() != fields[14+i*2].GetInt32() &&
                            (fields[1].GetUInt32() == 3 || fields[1].GetUInt32() == 25))
                        {
                            if (first) updatesql.append("SET `").append(column[i]).append("`='");
                            else updatesql.append("`").append(column[i]).append("`='");

                            tmp = (char*)malloc(32);
                            sprintf(tmp, "%u", fields[0].GetUInt32());
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                        else
                        {
                            if (fields[13+i*2].GetInt32() != fields[14+i*2].GetInt32() && !(own_style && i == 1))
                            {
                                if (first) updatesql.append("SET `").append(column[i]).append("`='");
                                else updatesql.append("`").append(column[i]).append("`='");

                                tmp = (char*)malloc(32);
                                sprintf(tmp, "%i", fields[13+i*2].GetInt32());
                                updatesql.append(tmp).append("',");
                                free(tmp);
                                first = false;
                            }
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "size")) || !docolumn)
                {
                    // size
                    if (fields[61].GetFloat() != fields[62].GetFloat())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `size`='");
                        else updatesql.append("`size`='");
                        sprintf(tmp, "%f", fields[61].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem1")) || !docolumn)
                {
                    // questItem1
                    if (fields[63].GetUInt32() != fields[64].GetUInt32())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `questItem1`='");
                        else updatesql.append("`questItem1`='");
                        sprintf(tmp, "%u", fields[63].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem2")) || !docolumn)
                {
                    // questItem2
                    if (fields[65].GetUInt32() != fields[66].GetUInt32())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `questItem2`='");
                        else updatesql.append("`questItem2`='");
                        sprintf(tmp, "%u", fields[65].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem3")) || !docolumn)
                {
                    // questItem3
                    if (fields[67].GetUInt32() != fields[68].GetUInt32())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `questItem3`='");
                        else updatesql.append("`questItem3`='");
                        sprintf(tmp, "%u", fields[67].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem4")) || !docolumn)
                {
                    // questItem4
                    if (fields[69].GetUInt32() != fields[70].GetUInt32())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `questItem4`='");
                        else updatesql.append("`questItem4`='");
                        sprintf(tmp, "%u", fields[69].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem5")) || !docolumn)
                {
                    // questItem5
                    if (fields[71].GetUInt32() != fields[72].GetUInt32())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `questItem5`='");
                        else updatesql.append("`questItem5`='");
                        sprintf(tmp, "%u", fields[71].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem6")) || !docolumn)
                {
                    // questItem6
                    if (fields[73].GetUInt32() != fields[74].GetUInt32())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `questItem6`='");
                        else updatesql.append("`questItem6`='");
                        sprintf(tmp, "%u", fields[73].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
            }
            if (!first)
            {
                ++count;
                updatesql.resize(updatesql.length()-1);
                char* tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[0].GetUInt32());
                updatesql.append(",`WDBVerified`='").append(_WDB_VERIFIED).append("' WHERE `entry`='").append(tmp).append("' LIMIT 1;\n");
                fputs(updatesql.c_str(), sqlfile);
                updatesql.clear();
                free(tmp);

                updatesql.append("UPDATE `gameobject_template` ");
            }
        } while(result->NextRow());

        if (count)
        {
            char* tmp = (char*)malloc(32);
            sprintf(tmp, "%u", count);
            updatesql.clear();
            updatesql.append("\n# Differences in ").append(tmp).append(" entries found.");
            fputs(updatesql.c_str(), sqlfile);
            printf("%u different entries found.\n", count);
            free(tmp);
            fclose(sqlfile);
        }
        else
        {
            fclose(sqlfile);
            remove(fstr.c_str());
            printf("no differences found.\n");
        }
        delete result;
    }
}

void DatabaseUpdate::CreateItemUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, Tokens columns, bool docolumn)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string updatesql = "";
    std::string query = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(ITEM_UPDATE_FILE);
#else
    fstr.append("/sql/").append(ITEM_UPDATE_FILE);
#endif
    updatesql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\n"
        "SET CHARACTER SET `utf8`;\n\n"
        "UPDATE `item_template` ");

    //                         0
    query.append("SELECT WDB.entry,WDB.Class,WORLD.class,WDB.SubClass1,WORLD.subclass,WDB.unk0,WORLD.unk0,"
        "WDB.Name1,WORLD.name,WDB.ItemDisplayID,WORLD.displayid,WDB.Quality,WORLD.Quality,WDB.Flags,"
        "WORLD.Flags,WDB.BuyPrice,WORLD.BuyPrice,WDB.SellPrice,WORLD.SellPrice,WDB.InventorySlot,"
        "WORLD.InventoryType,WDB.AllowableClass,WORLD.AllowableClass,WDB.AllowableRace,WORLD.AllowableRace,WDB.ItemLevel,"
        "WORLD.ItemLevel,WDB.RequiredLevel,WORLD.RequiredLevel,WDB.RequiredSkill,WORLD.RequiredSkill,WDB.RequiredSkillRank,"
        "WORLD.RequiredSkillRank,WDB.RequiredSpell,WORLD.RequiredSpell,WDB.RequiredHonorRank,WORLD.RequiredHonorRank,"
        "WDB.RequiredCityRank,WORLD.RequiredCityRank,WDB.RequiredReputationFaction,WORLD.RequiredReputationFaction,WDB.RequiredReputationRank,"
        "WORLD.RequiredReputationRank,WDB.maxcount,WORLD.maxcount,WDB.stackable,WORLD.stackable,"
        "WDB.ContainerSlots,WORLD.ContainerSlots,WDB.StatsCount,WORLD.StatsCount,"

        //    51
        "WDB.Stat1,WORLD.stat_type1,WDB.Stat1Val,WORLD.stat_value1,"
        "WDB.Stat2,WORLD.stat_type2,WDB.Stat2Val,WORLD.stat_value2,"
        "WDB.Stat3,WORLD.stat_type3,WDB.Stat3Val,WORLD.stat_value3,"
        "WDB.Stat4,WORLD.stat_type4,WDB.Stat4Val,WORLD.stat_value4,"
        "WDB.Stat5,WORLD.stat_type5,WDB.Stat5Val,WORLD.stat_value5,"
        "WDB.Stat6,WORLD.stat_type6,WDB.Stat6Val,WORLD.stat_value6,"
        "WDB.Stat7,WORLD.stat_type7,WDB.Stat7Val,WORLD.stat_value7,"
        "WDB.Stat8,WORLD.stat_type8,WDB.Stat8Val,WORLD.stat_value8,"
        "WDB.Stat9,WORLD.stat_type9,WDB.Stat9Val,WORLD.stat_value9,"
        "WDB.Stat10,WORLD.stat_type10,WDB.Stat10Val,WORLD.stat_value10,"

        //     91
        "WDB.Stat10,WORLD.ScalingStatDistribution,WDB.Stat10Val,WORLD.ScalingStatValue,"

        //      95
        "WDB.Damage1Min,WORLD.dmg_min1,WDB.Damage1Max,WORLD.dmg_max1,WDB.Damage1Type,WORLD.dmg_type1,"
        "WDB.Damage2Min,WORLD.dmg_min2,WDB.Damage2Max,WORLD.dmg_max2,WDB.Damage2Type,WORLD.dmg_type2,"

        //    107
        "WDB.Armor,WORLD.armor,WDB.HolyResist,WORLD.holy_res,WDB.FireResist,WORLD.fire_res,WDB.NatureResist,"
        "WORLD.nature_res,WDB.FrostResist,WORLD.frost_res,WDB.ShadowResist,WORLD.shadow_res,WDB.ArcaneResist,"
        "WORLD.arcane_res,WDB.Speed,WORLD.delay,WDB.AmmoType,WORLD.ammo_type,WDB.RangedModRange,WORLD.RangedModRange,"

        //      127
        "WDB.Spell1ID,WORLD.spellid_1,WDB.Spell1Trigger,WORLD.spelltrigger_1,WDB.Spell1Charges,"
        "WORLD.spellcharges_1,WDB.Spell1Cooldown,WORLD.spellcooldown_1,WDB.Spell1Category,"
        "WORLD.spellcategory_1,WDB.Spell1CategoryCooldown,WORLD.spellcategorycooldown_1,"

        //      139
        "WDB.Spell2ID,WORLD.spellid_2,WDB.Spell2Trigger,WORLD.spelltrigger_2,WDB.Spell2Charges,"
        "WORLD.spellcharges_2,WDB.Spell2Cooldown,WORLD.spellcooldown_2,WDB.Spell2Category,"
        "WORLD.spellcategory_2,WDB.Spell2CategoryCooldown,WORLD.spellcategorycooldown_2,"

        //      151
        "WDB.Spell3ID,WORLD.spellid_3,WDB.Spell3Trigger,WORLD.spelltrigger_3,WDB.Spell3Charges,"
        "WORLD.spellcharges_3,WDB.Spell3Cooldown,WORLD.spellcooldown_3,WDB.Spell3Category,"
        "WORLD.spellcategory_3,WDB.Spell3CategoryCooldown,WORLD.spellcategorycooldown_3,"

        //      163
        "WDB.Spell4ID,WORLD.spellid_4,WDB.Spell4Trigger,WORLD.spelltrigger_4,WDB.Spell4Charges,"
        "WORLD.spellcharges_4,WDB.Spell4Cooldown,WORLD.spellcooldown_4,WDB.Spell4Category,"
        "WORLD.spellcategory_4,WDB.Spell4CategoryCooldown,WORLD.spellcategorycooldown_4,"

        //      175
        "WDB.Spell5ID,WORLD.spellid_5,WDB.Spell5Trigger,WORLD.spelltrigger_5,WDB.Spell5Charges,"
        "WORLD.spellcharges_5,WDB.Spell5Cooldown,WORLD.spellcooldown_5,WDB.Spell5Category,"
        "WORLD.spellcategory_5,WDB.Spell5CategoryCooldown,WORLD.spellcategorycooldown_5,"

        //     187
        "WDB.Bonding,WORLD.bonding,WDB.Description,WORLD.description,WDB.BookTextID,WORLD.PageText,WDB.LanguageID,"
        "WORLD.LanguageID,WDB.PageMaterial,WORLD.PageMaterial,WDB.BeginQuestID,WORLD.startquest,WDB.LockID,"
        "WORLD.lockid,WDB.Material,WORLD.Material,WDB.Sheath,WORLD.sheath,WDB.RandomProperty,"
        "WORLD.RandomProperty,WDB.RandomSuffix,WORLD.RandomSuffix,WDB.BlockValue,WORLD.block,WDB.ItemSetID,"
        "WORLD.itemset,WDB.Durability,WORLD.MaxDurability,WDB.AreaID,WORLD.area,WDB.ItemMapID,WORLD.Map,"
        "WDB.BagFamily,WORLD.BagFamily,WDB.TotemCategory,WORLD.TotemCategory,WDB.SocketColor1,WORLD.socketColor_1,"
        "WDB.SocketContent1,WORLD.socketContent_1,WDB.SocketColor2,WORLD.socketColor_2,WDB.SocketContent2,"
        "WORLD.socketContent_2,WDB.SocketColor3,WORLD.socketColor_3,WDB.SocketContent3,WORLD.socketContent_3,"
        "WDB.socketBonus,WORLD.socketBonus,WDB.GemProperties,WORLD.GemProperties,WDB.requiredDisenchantSkill,"
        "WORLD.RequiredDisenchantSkill,WDB.armorDamageModifier,WORLD.ArmorDamageModifier,WDB.Duration,WORLD.Duration,"
        "WDB.ItemLimitCategory,WORLD.ItemLimitCategory,WDB.HolidayId,WORLD.HolidayId FROM `");

    query.append(wdbdb).append("`.`itemcache` AS WDB, `");

    query.append(worlddb).append("`.`item_template` AS WORLD WHERE WDB.entry = WORLD.entry AND "
        "(WDB.Class != WORLD.class || WDB.SubClass1 != WORLD.subclass || WDB.unk0 != WORLD.unk0 ||"
        " WDB.Name1 != WORLD.name || WDB.ItemDisplayID != WORLD.displayid || WDB.Quality != WORLD.Quality ||"
        " WDB.Flags != WORLD.Flags || WDB.BuyPrice != WORLD.BuyPrice || WDB.SellPrice != WORLD.SellPrice ||"
        " WDB.InventorySlot != WORLD.InventoryType || WDB.AllowableClass != WORLD.AllowableClass ||"
        " WDB.AllowableRace != WORLD.AllowableRace || WDB.ItemLevel != WORLD.ItemLevel ||"
        " WDB.RequiredLevel != WORLD.RequiredLevel || WDB.RequiredSkill != WORLD.RequiredSkill ||"
        " WDB.RequiredSkillRank != WORLD.RequiredSkillRank || WDB.RequiredSpell != WORLD.RequiredSpell ||"
        " WDB.RequiredHonorRank != WORLD.RequiredHonorRank || WDB.RequiredCityRank != WORLD.RequiredCityRank ||"
        " WDB.RequiredReputationFaction != WORLD.RequiredReputationFaction || WDB.RequiredReputationRank != WORLD.RequiredReputationRank ||"
        " WDB.maxcount != WORLD.maxcount || WDB.stackable != WORLD.stackable ||"
        " WDB.ContainerSlots != WORLD.ContainerSlots || WDB.StatsCount != WORLD.StatsCount ||"

        "WDB.Stat1 != WORLD.stat_type1 || WDB.Stat1Val != WORLD.stat_value1 || "
        "WDB.Stat2 != WORLD.stat_type2 || WDB.Stat2Val != WORLD.stat_value2 || "
        "WDB.Stat3 != WORLD.stat_type3 || WDB.Stat3Val != WORLD.stat_value3 || "
        "WDB.Stat4 != WORLD.stat_type4 || WDB.Stat4Val != WORLD.stat_value4 || "
        "WDB.Stat5 != WORLD.stat_type5 || WDB.Stat5Val != WORLD.stat_value5 || "
        "WDB.Stat6 != WORLD.stat_type6 || WDB.Stat6Val != WORLD.stat_value6 || "
        "WDB.Stat7 != WORLD.stat_type7 || WDB.Stat7Val != WORLD.stat_value7 || "
        "WDB.Stat8 != WORLD.stat_type8 || WDB.Stat8Val != WORLD.stat_value8 || "
        "WDB.Stat9 != WORLD.stat_type9 || WDB.Stat9Val != WORLD.stat_value9 || "
        "WDB.Stat10 != WORLD.stat_type10 || WDB.Stat10Val != WORLD.stat_value10 || "

        "WDB.Stat10 != WORLD.ScalingStatDistribution || WDB.Stat10Val != WORLD.ScalingStatValue || "

        "WDB.Damage1Min != WORLD.dmg_min1 || WDB.Damage1Max != WORLD.dmg_max1 || WDB.Damage1Type != WORLD.dmg_type1 || "
        "WDB.Damage2Min != WORLD.dmg_min2 || WDB.Damage2Max != WORLD.dmg_max2 || WDB.Damage2Type != WORLD.dmg_type2 || "

        " WDB.Armor != WORLD.armor || WDB.HolyResist != WORLD.holy_res || WDB.FireResist != WORLD.fire_res ||"
        " WDB.NatureResist != WORLD.nature_res || WDB.FrostResist != WORLD.frost_res ||"
        " WDB.ShadowResist != WORLD.shadow_res || WDB.ArcaneResist != WORLD.arcane_res || WDB.Speed != WORLD.delay ||"
        " WDB.AmmoType != WORLD.ammo_type || WDB.RangedModRange != WORLD.RangedModRange || "

        "WDB.Spell1ID != WORLD.spellid_1 || WDB.Spell1Trigger != WORLD.spelltrigger_1 || "
        "WDB.Spell1Charges != WORLD.spellcharges_1 || WDB.Spell1Cooldown != WORLD.spellcooldown_1 || "
        "WDB.Spell1Category != WORLD.spellcategory_1 || WDB.Spell1CategoryCooldown != WORLD.spellcategorycooldown_1 || "

        "WDB.Spell2ID != WORLD.spellid_2 || WDB.Spell2Trigger != WORLD.spelltrigger_2 || "
        "WDB.Spell2Charges != WORLD.spellcharges_2 || WDB.Spell2Cooldown != WORLD.spellcooldown_2 || "
        "WDB.Spell2Category != WORLD.spellcategory_2 || WDB.Spell2CategoryCooldown != WORLD.spellcategorycooldown_2 || "

        "WDB.Spell3ID != WORLD.spellid_3 || WDB.Spell3Trigger != WORLD.spelltrigger_3 || "
        "WDB.Spell3Charges != WORLD.spellcharges_3 || WDB.Spell3Cooldown != WORLD.spellcooldown_3 || "
        "WDB.Spell3Category != WORLD.spellcategory_3 || WDB.Spell3CategoryCooldown != WORLD.spellcategorycooldown_3 || "

        "WDB.Spell4ID != WORLD.spellid_4 || WDB.Spell4Trigger != WORLD.spelltrigger_4 || "
        "WDB.Spell4Charges != WORLD.spellcharges_4 || WDB.Spell4Cooldown != WORLD.spellcooldown_4 || "
        "WDB.Spell4Category != WORLD.spellcategory_4 || WDB.Spell4CategoryCooldown != WORLD.spellcategorycooldown_4 || "

        "WDB.Spell5ID != WORLD.spellid_5 || WDB.Spell5Trigger != WORLD.spelltrigger_5 || "
        "WDB.Spell5Charges != WORLD.spellcharges_5 || WDB.Spell5Cooldown != WORLD.spellcooldown_5 || "
        "WDB.Spell5Category != WORLD.spellcategory_5 || WDB.Spell5CategoryCooldown != WORLD.spellcategorycooldown_5 || "

        "WDB.Bonding != WORLD.bonding || WDB.Description != WORLD.description || WDB.BookTextID != WORLD.PageText || "
        "WDB.LanguageID != WORLD.LanguageID || WDB.PageMaterial != WORLD.PageMaterial || "
        "WDB.BeginQuestID != WORLD.startquest || WDB.LockID != WORLD.lockid || WDB.Material != WORLD.Material || "
        "WDB.Sheath != WORLD.sheath || WDB.RandomProperty != WORLD.RandomProperty || "
        "WDB.RandomSuffix != WORLD.RandomSuffix || WDB.BlockValue != WORLD.block || "
        "WDB.ItemSetID != WORLD.itemset || WDB.Durability != WORLD.MaxDurability || WDB.AreaID != WORLD.area || "
        "WDB.ItemMapID != WORLD.Map || WDB.BagFamily != WORLD.BagFamily || WDB.TotemCategory != WORLD.TotemCategory || "
        "WDB.SocketColor1 != WORLD.socketColor_1 || WDB.SocketContent1 != WORLD.socketContent_1 || "
        "WDB.SocketColor2 != WORLD.socketColor_2 || WDB.SocketContent2 != WORLD.socketContent_2 || "
        "WDB.SocketColor3 != WORLD.socketColor_3 || WDB.SocketContent3 != WORLD.socketContent_3 || "
        "WDB.socketBonus != WORLD.socketBonus || WDB.GemProperties != WORLD.GemProperties || "
        "WDB.requiredDisenchantSkill != WORLD.RequiredDisenchantSkill || "
        "WDB.armorDamageModifier != WORLD.ArmorDamageModifier || WDB.Duration != WORLD.Duration || "
        "WDB.ItemLimitCategory != WORLD.ItemLimitCategory || WDB.HolidayId != WORLD.HolidayId)");

    printf("Searching for different items...\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        uint64 count = 0;

        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        // uint32
        const char* column1[9] = {"ItemLevel","RequiredLevel","RequiredSkill","RequiredSkillRank","requiredspell",
            "requiredhonorrank","RequiredCityRank","RequiredReputationFaction","RequiredReputationRank"};
        // int32
        const char* column2[22] = {"stat_type1","stat_value1","stat_type2","stat_value2","stat_type3","stat_value3",
            "stat_type4","stat_value4","stat_type5","stat_value5","stat_type6","stat_value6","stat_type7",
            "stat_value7","stat_type8","stat_value8","stat_type9","stat_value9","stat_type10","stat_value10",
            "ScalingStatDistribution","ScalingStatValue"};
        // uint32
        const char* column3[9] = {"armor","holy_res","fire_res","nature_res","frost_res","shadow_res",
            "arcane_res","delay","ammo_type"};
        const char* column4[30] = {"spellid_1","spelltrigger_1","spellcharges_1","spellcooldown_1","spellcategory_1","spellcategorycooldown_1",
            "spellid_2","spelltrigger_2","spellcharges_2","spellcooldown_2","spellcategory_2","spellcategorycooldown_2",
            "spellid_3","spelltrigger_3","spellcharges_3","spellcooldown_3","spellcategory_3","spellcategorycooldown_3",
            "spellid_4","spelltrigger_4","spellcharges_4","spellcooldown_4","spellcategory_4","spellcategorycooldown_4",
            "spellid_5","spelltrigger_5","spellcharges_5","spellcooldown_5","spellcategory_5","spellcategorycooldown_5"};
        const char* column5[16] = {"RandomSuffix","block","itemset","MaxDurability","area","Map","BagFamily","TotemCategory",
            "socketColor_1","socketContent_1","socketColor_2","socketContent_2","socketColor_3","socketContent_3","socketBonus","GemProperties"};

        do
        {
            bool first = true;
            fields = result->Fetch();

            if (fields)
            {
                char* tmp;

                if ((docolumn && ColumnExists(columns, "class")) || !docolumn)
                {
                    // class
                    if (fields[1].GetUInt32() != fields[2].GetUInt32())
                    {
                        if (first) updatesql.append("SET `class`='");
                        else updatesql.append("`class`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[1].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "subclass")) || !docolumn)
                {
                    // subclass
                    if (fields[3].GetUInt32() != fields[4].GetUInt32())
                    {
                        if (first) updatesql.append("SET `subclass`='");
                        else updatesql.append("`subclass`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[3].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "unk0")) || !docolumn)
                {
                    // unk0
                    if (fields[5].GetInt32() != fields[6].GetInt32())
                    {
                        if (first) updatesql.append("SET `unk0`='");
                        else updatesql.append("`unk0`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[5].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "name")) || !docolumn)
                {
                    // name
                    if (strcmp(fields[7].GetCppString().c_str(), fields[8].GetCppString().c_str()) != 0)
                    {
                        if (first) updatesql.append("SET `name`='").append(io.Terminator(fields[7].GetCppString())).append("',");
                        else updatesql.append("`name`='").append(io.Terminator(fields[7].GetCppString())).append("',");
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "displayid")) || !docolumn)
                {
                    // displayid
                    if (fields[9].GetUInt32() != fields[10].GetUInt32())
                    {
                        if (first) updatesql.append("SET `displayid`='");
                        else updatesql.append("`displayid`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[9].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "Quality")) || !docolumn)
                {
                    // Quality
                    if (fields[11].GetUInt32() != fields[12].GetUInt32())
                    {
                        if (first) updatesql.append("SET `Quality`='");
                        else updatesql.append("`Quality`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[11].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "Flags")) || !docolumn)
                {
                    // Flags
                    if (fields[13].GetInt32() != fields[14].GetInt32())
                    {
                        if (first) updatesql.append("SET `Flags`='");
                        else updatesql.append("`Flags`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[13].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "BuyPrice")) || !docolumn)
                {
                    // BuyPrice
                    if (fields[15].GetInt32() != fields[16].GetInt32())
                    {
                        if (first) updatesql.append("SET `BuyPrice`='");
                        else updatesql.append("`BuyPrice`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[15].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "SellPrice")) || !docolumn)
                {
                    // SellPrice
                    if (fields[17].GetUInt32() != fields[18].GetUInt32())
                    {
                        if (first) updatesql.append("SET `SellPrice`='");
                        else updatesql.append("`SellPrice`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[17].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "InventoryType")) || !docolumn)
                {
                    // InventoryType
                    if (fields[19].GetUInt32() != fields[20].GetUInt32())
                    {
                        if (first) updatesql.append("SET `InventoryType`='");
                        else updatesql.append("`InventoryType`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[19].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "AllowableClass")) || !docolumn)
                {
                    // AllowableClass
                    if (fields[21].GetInt32() != fields[22].GetInt32())
                    {
                        if (first) updatesql.append("SET `AllowableClass`='");
                        else updatesql.append("`AllowableClass`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[21].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "AllowableRace")) || !docolumn)
                {
                    // AllowableRace
                    if (fields[23].GetInt32() != fields[24].GetInt32())
                    {
                        if (first) updatesql.append("SET `AllowableRace`='");
                        else updatesql.append("`AllowableRace`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[23].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // column1
                for (uint8 i=0; i<9; i++)
                {
                    if ((docolumn && ColumnExists(columns, column1[i])) || !docolumn)
                    {
                        if (fields[25+i*2].GetUInt32() != fields[26+i*2].GetUInt32())
                        {
                            uint32 tmpuint = fields[25+i*2].GetUInt32();

                            // Faction-"Korrektur" wegen core! :-(
                            if ((25+i*2) == 41 && tmpuint > 0)
                                if (fields[39].GetUInt32() == 0) tmpuint = 0;

                            if (tmpuint != fields[26+i*2].GetUInt32())
                            {
                                if (first) updatesql.append("SET `").append(column1[i]).append("`='");
                                else updatesql.append("`").append(column1[i]).append("`='");
                                tmp = (char*)malloc(32);
                                sprintf(tmp, "%u", tmpuint);
                                updatesql.append(tmp).append("',");
                                free(tmp);
                                first = false;
                            }
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "maxcount")) || !docolumn)
                {
                    // maxcount
                    if (fields[43].GetInt32() != fields[44].GetInt32())
                    {
                        if (first) updatesql.append("SET `maxcount`='");
                        else updatesql.append("`maxcount`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[43].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "stackable")) || !docolumn)
                {
                    // stackable
                    if (fields[45].GetInt32() != fields[46].GetInt32())
                    {
                        if (first) updatesql.append("SET `stackable`='");
                        else updatesql.append("`stackable`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[45].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "ContainerSlots")) || !docolumn)
                {
                    // ContainerSlots
                    if (fields[47].GetUInt32() != fields[48].GetUInt32())
                    {
                        if (first) updatesql.append("SET `ContainerSlots`='");
                        else updatesql.append("`ContainerSlots`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[47].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "StatsCount")) || !docolumn)
                {
                    // StatsCount
                    if (fields[49].GetUInt32() != fields[50].GetUInt32())
                    {
                        if (first) updatesql.append("SET `StatsCount`='");
                        else updatesql.append("`StatsCount`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[49].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // column2
                for (uint8 i=0; i<22; i++)
                {
                    if ((docolumn && ColumnExists(columns, column2[i])) || !docolumn)
                    {
                        uint32 index = (51+i*2);

                        //  stattype10     statvalue10
                        if (index == 87 || index == 89) fields[index].SetValue("0");

                        //  stattype10     statvalue10
                        if (index == 91 || index == 93)
                        {
                            char* tmpchar = (char*)malloc(32);
                            if (fields[index].GetInt32() != fields[52+i*2].GetInt32())
                            {
                                fields[52+i*2].SetValue(_itoa(fields[index].GetInt32(), tmpchar, 10));

                                if (first) updatesql.append("SET `").append(column2[i]).append("`='");
                                else updatesql.append("`").append(column2[i]).append("`='");
                                tmp = (char*)malloc(32);
                                sprintf(tmp, "%i", fields[52+i*2].GetInt32());
                                updatesql.append(tmp).append("',");
                                free(tmp);
                                first = false;
                            }
                            free(tmpchar);
                        }

                        if (fields[index].GetInt32() != fields[52+i*2].GetInt32())
                        {
                            if (first) updatesql.append("SET `").append(column2[i]).append("`='");
                            else updatesql.append("`").append(column2[i]).append("`='");
                            tmp = (char*)malloc(32);
                            sprintf(tmp, "%i", fields[index].GetInt32());
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "dmg_min1")) || !docolumn)
                {
                    // dmg_min1
                    if (fields[95].GetFloat() != fields[96].GetFloat())
                    {
                        if (first) updatesql.append("SET `dmg_min1`='");
                        else updatesql.append("`dmg_min1`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%f", fields[95].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "dmg_max1")) || !docolumn)
                {
                    // dmg_max1
                    if (fields[97].GetFloat() != fields[98].GetFloat())
                    {
                        if (first) updatesql.append("SET `dmg_max1`='");
                        else updatesql.append("`dmg_max1`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%f", fields[97].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "dmg_type1")) || !docolumn)
                {
                    // dmg_type1
                    if (fields[99].GetUInt32() != fields[100].GetUInt32())
                    {
                        if (first) updatesql.append("SET `dmg_type1`='");
                        else updatesql.append("`dmg_type1`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[99].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "dmg_min2")) || !docolumn)
                {
                    // dmg_min2
                    if (fields[101].GetFloat() != fields[102].GetFloat())
                    {
                        if (first) updatesql.append("SET `dmg_min2`='");
                        else updatesql.append("`dmg_min2`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%f", fields[101].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "dmg_max2")) || !docolumn)
                {
                    // dmg_max2
                    if (fields[103].GetFloat() != fields[104].GetFloat())
                    {
                        if (first) updatesql.append("SET `dmg_max2`='");
                        else updatesql.append("`dmg_max2`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%f", fields[103].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "dmg_type2")) || !docolumn)
                {
                    // dmg_type2
                    if (fields[105].GetUInt32() != fields[106].GetUInt32())
                    {
                        if (first) updatesql.append("SET `dmg_type2`='");
                        else updatesql.append("`dmg_type2`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[105].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // column3
                for (uint8 i=0; i<9; ++i)
                {
                    if ((docolumn && ColumnExists(columns, column3[i])) || !docolumn)
                    {
                        if (fields[107+i*2].GetUInt32() != fields[108+i*2].GetUInt32())
                        {
                            if (first) updatesql.append("SET `").append(column3[i]).append("`='");
                            else updatesql.append("`").append(column3[i]).append("`='");
                            tmp = (char*)malloc(32);
                            sprintf(tmp, "%u", fields[107+i*2].GetUInt32());
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "RangedModRange")) || !docolumn)
                {
                    // RangedModRange
                    if (fields[125].GetFloat() != fields[126].GetFloat())
                    {
                        if (first) updatesql.append("SET `RangedModRange`='");
                        else updatesql.append("`RangedModRange`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%f", fields[125].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // column4 (spell related stuff)
                for (uint8 i=0; i<30; ++i)
                {
                    if ((docolumn && ColumnExists(columns, column4[i])) || !docolumn)
                    {
                        int32 tmpint = fields[127+i*2].GetInt32();
                        uint16 index = (127+i*2);

                        // TODO: REMOVE IF CORE SUPPORTS INT VALUES!
                        // spellid_x - Workaround until Trinitycore supports -1 values
                        if (index == 127 || index == 139 || index == 151 || index == 163 || index == 175)
                        {
                            if (tmpint < 0)
                            {
                                tmpint = 0;
                                for (uint8 j=1; j<6; ++j)
                                {
                                    if (j == 3 || j == 5)
                                        fields[index+j*2].SetValue("-1");
                                    else
                                        fields[index+j*2].SetValue("0");
                                }
                            }
                        }
                        if (tmpint != fields[128+i*2].GetInt32())
                        {
                            if (first)
                                updatesql.append("SET `").append(column4[i]).append("`='");
                            else
                                updatesql.append("`").append(column4[i]).append("`='");

                            tmp = (char*)malloc(32);
                            sprintf(tmp, "%i", tmpint);
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "bonding")) || !docolumn)
                {
                    // bonding
                    if (fields[187].GetInt32() != fields[188].GetInt32())
                    {
                        if (first) updatesql.append("SET `bonding`='");
                        else updatesql.append("`bonding`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[187].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "description")) || !docolumn)
                {
                    // description
                    if (strcmp(fields[189].GetCppString().c_str(), fields[190].GetCppString().c_str()) != 0)
                    {
                        if (first) updatesql.append("SET `description`='").append(io.Terminator(fields[189].GetCppString())).append("',");
                        else updatesql.append("`description`='").append(io.Terminator(fields[189].GetCppString())).append("',");
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "PageText")) || !docolumn)
                {
                    // PageText
                    if (fields[191].GetUInt32() != fields[192].GetUInt32())
                    {
                        if (first) updatesql.append("SET `PageText`='");
                        else updatesql.append("`PageText`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[191].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "LanguageID")) || !docolumn)
                {
                    // LanguageID
                    if (fields[193].GetUInt32() != fields[194].GetUInt32())
                    {
                        if (first) updatesql.append("SET `LanguageID`='");
                        else updatesql.append("`LanguageID`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[193].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "PageMaterial")) || !docolumn)
                {
                    // PageMaterial
                    if (fields[195].GetInt32() != fields[196].GetInt32())
                    {
                        if (first) updatesql.append("SET `PageMaterial`='");
                        else updatesql.append("`PageMaterial`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[195].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "startquest")) || !docolumn)
                {
                    // startquest
                    if (fields[197].GetUInt32() != fields[198].GetUInt32())
                    {
                        if (first) updatesql.append("SET `startquest`='");
                        else updatesql.append("`startquest`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[197].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "lockid")) || !docolumn)
                {
                    // lockid
                    if (fields[199].GetUInt32() != fields[200].GetUInt32())
                    {
                        if (first) updatesql.append("SET `lockid`='");
                        else updatesql.append("`lockid`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[199].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "Material")) || !docolumn)
                {
                    // Material
                    if (fields[201].GetInt32() != fields[202].GetInt32())
                    {
                        if (first) updatesql.append("SET `Material`='");
                        else updatesql.append("`Material`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[201].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "sheath")) || !docolumn)
                {
                    // sheath
                    if (fields[203].GetInt32() != fields[204].GetInt32())
                    {
                        if (first) updatesql.append("SET `sheath`='");
                        else updatesql.append("`sheath`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[203].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "RandomProperty")) || !docolumn)
                {
                    // RandomProperty
                    int32 tmpint = fields[205].GetInt32();

                    // TODO: REMOVE IF CORE SUPPORTS INT VALUES!
                    // Workaround until Trinitycore supports -1 values
                    if (tmpint < 0)
                        tmpint = 0;

                    if (tmpint != fields[206].GetInt32())
                    {
                        if (first)
                            updatesql.append("SET `RandomProperty`='");
                        else
                            updatesql.append("`RandomProperty`='");

                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", tmpint);
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // column5
                for (uint8 i=0; i<16; i++)
                {
                    if ((docolumn && ColumnExists(columns, column5[i])) || !docolumn)
                    {
                        if (fields[207+i*2].GetUInt32() != fields[208+i*2].GetUInt32())
                        {
                            if (first) updatesql.append("SET `").append(column5[i]).append("`='");
                            else updatesql.append("`").append(column5[i]).append("`='");
                            tmp = (char*)malloc(32);
                            sprintf(tmp, "%u", fields[207+i*2].GetUInt32());
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "RequiredDisenchantSkill")) || !docolumn)
                {
                    // RequiredDisenchantSkill
                    if (fields[239].GetInt32() != fields[240].GetInt32())
                    {
                        if (first) updatesql.append("SET `RequiredDisenchantSkill`='");
                        else updatesql.append("`RequiredDisenchantSkill`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[239].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "ArmorDamageModifier")) || !docolumn)
                {
                    // ArmorDamageModifier
                    if (fields[241].GetFloat() != fields[242].GetFloat())
                    {
                        if (first) updatesql.append("SET `ArmorDamageModifier`='");
                        else updatesql.append("`ArmorDamageModifier`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%f", fields[241].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "Duration")) || !docolumn)
                {
                    // Duration
                    if (fields[243].GetInt32() != fields[244].GetInt32())
                    {
                        if (first) updatesql.append("SET `Duration`='");
                        else updatesql.append("`Duration`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[243].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "ItemLimitCategory")) || !docolumn)
                {
                    // ItemLimitCategory
                    if (fields[245].GetInt32() != fields[246].GetUInt32())
                    {
                        if (first) updatesql.append("SET `ItemLimitCategory`='");
                        else updatesql.append("`ItemLimitCategory`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[245].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "HolidayId")) || !docolumn)
                {
                    // HolidayId
                    if (fields[247].GetInt32() != fields[248].GetUInt32())
                    {
                        if (first) updatesql.append("SET `HolidayId`='");
                        else updatesql.append("`HolidayId`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[247].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
            }
            if (!first)
            {
                count++;
                updatesql.resize(updatesql.length()-1);
                char* tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[0].GetUInt32());
                updatesql.append(",`WDBVerified`='").append(_WDB_VERIFIED).append("' WHERE `entry`='").append(tmp).append("' LIMIT 1;\n");
                fputs(updatesql.c_str(), sqlfile);
                updatesql.clear();
                free(tmp);

                updatesql.append("UPDATE `item_template` ");
            }
        } while(result->NextRow());

        if (count)
        {
            char* tmp = (char*)malloc(32);
            sprintf(tmp, "%u", count);
            updatesql.clear();
            updatesql.append("\n# Differences in ").append(tmp).append(" entries found.");
            fputs(updatesql.c_str(), sqlfile);
            printf("%u different entries found.\n", count);
            free(tmp);
            fclose(sqlfile);
        }
        else
        {
            fclose(sqlfile);
            remove(fstr.c_str());
            printf("no differences found.\n");
        }
        delete result;
    }
}

void DatabaseUpdate::CreateNPCTextUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, Tokens columns, bool docolumn)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string updatesql = "";
    std::string query = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(NPCTEXT_UPDATE_FILE);
#else
    fstr.append("/sql/").append(NPCTEXT_UPDATE_FILE);
#endif
    updatesql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\n"
        "SET CHARACTER SET `utf8`;\n\n"
        "UPDATE `npc_text` ");

    query.append("SELECT WDB.entry,"
        "WDB.text0_0,WORLD.text0_0,WDB.text0_1,WORLD.text0_1,WDB.lang0,WORLD.lang0,WDB.prob0,WORLD.prob0,"
        "WDB.em0_0,WORLD.em0_0,WDB.em0_1,WORLD.em0_1,WDB.em0_2,WORLD.em0_2,WDB.em0_3,WORLD.em0_3,WDB.em0_4,"
        "WORLD.em0_4,WDB.text1_0,WORLD.text1_0,WDB.text1_1,WORLD.text1_1,WDB.lang1,WORLD.lang1,WDB.prob1,"
        "WORLD.prob1,WDB.em1_0,WORLD.em1_0,WDB.em1_1,WORLD.em1_1,WDB.em1_2,WORLD.em1_2,WDB.em1_3,WORLD.em1_3,"
        "WDB.em1_4,WORLD.em1_4,WDB.text2_0,WORLD.text2_0,WDB.text2_1,WORLD.text2_1,WDB.lang2,WORLD.lang2,WDB.prob2,"
        "WORLD.prob2,WDB.em2_0,WORLD.em2_0,WDB.em2_1,WORLD.em2_1,WDB.em2_2,WORLD.em2_2,WDB.em2_3,WORLD.em2_3,"
        "WDB.em2_4,WORLD.em2_4,WDB.text3_0,WORLD.text3_0,WDB.text3_1,WORLD.text3_1,WDB.lang3,WORLD.lang3,WDB.prob3,"
        "WORLD.prob3,WDB.em3_0,WORLD.em3_0,WDB.em3_1,WORLD.em3_1,WDB.em3_2,WORLD.em3_2,WDB.em3_3,WORLD.em3_3,"
        "WDB.em3_4,WORLD.em3_4,WDB.text4_0,WORLD.text4_0,WDB.text4_1,WORLD.text4_1,WDB.lang4,WORLD.lang4,WDB.prob4,"
        "WORLD.prob4,WDB.em4_0,WORLD.em4_0,WDB.em4_1,WORLD.em4_1,WDB.em4_2,WORLD.em4_2,WDB.em4_3,WORLD.em4_3,"
        "WDB.em4_4,WORLD.em4_4,WDB.text5_0,WORLD.text5_0,WDB.text5_1,WORLD.text5_1,WDB.lang5,WORLD.lang5,WDB.prob5,"
        "WORLD.prob5,WDB.em5_0,WORLD.em5_0,WDB.em5_1,WORLD.em5_1,WDB.em5_2,WORLD.em5_2,WDB.em5_3,WORLD.em5_3,"
        "WDB.em5_4,WORLD.em5_4,WDB.text6_0,WORLD.text6_0,WDB.text6_1,WORLD.text6_1,WDB.lang6,WORLD.lang6,WDB.prob6,"
        "WORLD.prob6,WDB.em6_0,WORLD.em6_0,WDB.em6_1,WORLD.em6_1,WDB.em6_2,WORLD.em6_2,WDB.em6_3,WORLD.em6_3,"
        "WDB.em6_4,WORLD.em6_4,WDB.text7_0,WORLD.text7_0,WDB.text7_1,WORLD.text7_1,WDB.lang7,WORLD.lang7,WDB.prob7,"
        "WORLD.prob7,WDB.em7_0,WORLD.em7_0,WDB.em7_1,WORLD.em7_1,WDB.em7_2,WORLD.em7_2,WDB.em7_3,WORLD.em7_3,"
        "WDB.em7_4,WORLD.em7_4 FROM `");

    query.append(wdbdb).append("`.`npccache` AS WDB, `");

    query.append(worlddb).append("`.`npc_text` AS WORLD WHERE WDB.entry = WORLD.ID AND "
        "(WDB.text0_0 != WORLD.text0_0 || WDB.text0_1 != WORLD.text0_1 || WDB.lang0 != WORLD.lang0 || "
        "WDB.prob0 != WORLD.prob0 || WDB.em0_0 != WORLD.em0_0 || WDB.em0_1 != WORLD.em0_1 || "
        "WDB.em0_2 != WORLD.em0_2 || WDB.em0_3 != WORLD.em0_3 || WDB.em0_4 != WORLD.em0_4 || "

        "WDB.text1_0 != WORLD.text1_0 || WDB.text1_1 != WORLD.text1_1 || WDB.lang1 != WORLD.lang1 || "
        "WDB.prob1 != WORLD.prob1 || WDB.em1_0 != WORLD.em1_0 || WDB.em1_1 != WORLD.em1_1 || "
        "WDB.em1_2 != WORLD.em1_2 || WDB.em1_3 != WORLD.em1_3 || WDB.em1_4 != WORLD.em1_4 || "

        "WDB.text2_0 != WORLD.text2_0 || WDB.text2_1 != WORLD.text2_1 || WDB.lang2 != WORLD.lang2 || "
        "WDB.prob2 != WORLD.prob2 || WDB.em2_0 != WORLD.em2_0 || WDB.em2_1 != WORLD.em2_1 || "
        "WDB.em2_2 != WORLD.em2_2 || WDB.em2_3 != WORLD.em2_3 || WDB.em2_4 != WORLD.em2_4 || "

        "WDB.text3_0 != WORLD.text3_0 || WDB.text3_1 != WORLD.text3_1 || WDB.lang3 != WORLD.lang3 || "
        "WDB.prob3 != WORLD.prob3 || WDB.em3_0 != WORLD.em3_0 || WDB.em3_1 != WORLD.em3_1 || "
        "WDB.em3_2 != WORLD.em3_2 || WDB.em3_3 != WORLD.em3_3 || WDB.em3_4 != WORLD.em3_4 || "

        "WDB.text4_0 != WORLD.text4_0 || WDB.text4_1 != WORLD.text4_1 || WDB.lang4 != WORLD.lang4 || "
        "WDB.prob4 != WORLD.prob4 || WDB.em4_0 != WORLD.em4_0 || WDB.em4_1 != WORLD.em4_1 || "
        "WDB.em4_2 != WORLD.em4_2 || WDB.em4_3 != WORLD.em4_3 || WDB.em4_4 != WORLD.em4_4 || "

        "WDB.text5_0 != WORLD.text5_0 || WDB.text5_1 != WORLD.text5_1 || WDB.lang5 != WORLD.lang5 || "
        "WDB.prob5 != WORLD.prob5 || WDB.em5_0 != WORLD.em5_0 || WDB.em5_1 != WORLD.em5_1 || "
        "WDB.em5_2 != WORLD.em5_2 || WDB.em5_3 != WORLD.em5_3 || WDB.em5_4 != WORLD.em5_4 || "

        "WDB.text6_0 != WORLD.text6_0 || WDB.text6_1 != WORLD.text6_1 || WDB.lang6 != WORLD.lang6 || "
        "WDB.prob6 != WORLD.prob6 || WDB.em6_0 != WORLD.em6_0 || WDB.em6_1 != WORLD.em6_1 || "
        "WDB.em6_2 != WORLD.em6_2 || WDB.em6_3 != WORLD.em6_3 || WDB.em6_4 != WORLD.em6_4 || "

        "WDB.text7_0 != WORLD.text7_0 || WDB.text7_1 != WORLD.text7_1 || WDB.lang7 != WORLD.lang7 || "
        "WDB.prob7 != WORLD.prob7 || WDB.em7_0 != WORLD.em7_0 || WDB.em7_1 != WORLD.em7_1 || "
        "WDB.em7_2 != WORLD.em7_2 || WDB.em7_3 != WORLD.em7_3 || WDB.em7_4 != WORLD.em7_4)");

    printf("Searching for different npc texts...\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        uint64 count = 0;

        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        do
        {
            bool first = true;
            fields = result->Fetch();

            if (fields)
            {
                char* tmp;
                char col_name[10];

                for (uint8 i=0; i<8; i++)
                {
                    if (strcmp(fields[1+18*i].GetCppString().c_str(), fields[2+18*i].GetCppString().c_str()) != 0)
                    {
                        sprintf_s(col_name, 8, "text%u_0", i);
                        if ((docolumn && ColumnExists(columns, col_name)) || !docolumn)
                        {
                            if (first) updatesql.append("SET `").append(col_name).append("`='").append(io.Terminator(fields[1+18*i].GetCppString())).append("',");
                            else updatesql.append("`").append(col_name).append("`='").append(io.Terminator(fields[1+18*i].GetCppString())).append("',");
                            first = false;
                        }
                    }
                    if (strcmp(fields[3+18*i].GetCppString().c_str(), fields[4+18*i].GetCppString().c_str()) != 0)
                    {
                        sprintf_s(col_name, 8, "text%u_1", i);
                        if ((docolumn && ColumnExists(columns, col_name)) || !docolumn)
                        {
                            if (first) updatesql.append("SET `").append(col_name).append("`='").append(io.Terminator(fields[3+18*i].GetCppString())).append("',");
                            else updatesql.append("`").append(col_name).append("`='").append(io.Terminator(fields[3+18*i].GetCppString())).append("',");
                            first = false;
                        }
                    }
                    if (fields[5+18*i].GetUInt32() != fields[6+18*i].GetUInt32())
                    {
                        tmp = (char*)malloc(32);
                        sprintf_s(col_name, 6, "lang%u", i);
                        if ((docolumn && ColumnExists(columns, col_name)) || !docolumn)
                        {
                            if (first) updatesql.append("SET `").append(col_name).append("`='");
                            else updatesql.append("`").append(col_name).append("`='");
                            sprintf(tmp, "%u", fields[5+18*i].GetUInt32());
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                    if (fields[7+18*i].GetFloat() != fields[8+18*i].GetFloat())
                    {
                        tmp = (char*)malloc(32);
                        sprintf_s(col_name, 6, "prob%u", i);
                        if ((docolumn && ColumnExists(columns, col_name)) || !docolumn)
                        {
                            if (first) updatesql.append("SET `").append(col_name).append("`='");
                            else updatesql.append("`").append(col_name).append("`='");
                            sprintf(tmp, "%f", fields[7+18*i].GetFloat());
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                    for (uint8 j=0; j<5; j++)
                    {
                        if (fields[9+j*2+18*i].GetUInt32() != fields[10+j*2+18*i].GetUInt32())
                        {
                            tmp = (char*)malloc(32);
                            sprintf_s(col_name, 6, "em%u_%u", i, j);
                            if ((docolumn && ColumnExists(columns, col_name)) || !docolumn)
                            {
                                if (first) updatesql.append("SET `").append(col_name).append("`='");
                                else updatesql.append("`").append(col_name).append("`='");
                                sprintf(tmp, "%u", fields[9+j*2+18*i].GetUInt32());
                                updatesql.append(tmp).append("',");
                                free(tmp);
                                first = false;
                            }
                        }
                    }
                }
            }
            if (!first)
            {
                count++;
                updatesql.resize(updatesql.length()-1);
                char* tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[0].GetUInt32());
                updatesql.append(",`WDBVerified`='").append(_WDB_VERIFIED).append("' WHERE `ID`='").append(tmp).append("' LIMIT 1;\n");
                fputs(updatesql.c_str(), sqlfile);
                updatesql.clear();
                free(tmp);

                updatesql.append("UPDATE `npc_text` ");
            }
        } while(result->NextRow());

        if (count)
        {
            char* tmp = (char*)malloc(32);
            sprintf(tmp, "%u", count);
            updatesql.clear();
            updatesql.append("\n# Differences in ").append(tmp).append(" entries found.");
            fputs(updatesql.c_str(), sqlfile);
            printf("%u different entries found.\n", count);
            free(tmp);
            fclose(sqlfile);
        }
        else
        {
            fclose(sqlfile);
            remove(fstr.c_str());
            printf("no differences found.\n");
        }
        delete result;

    } else printf("no differences found.\n");
}

void DatabaseUpdate::CreatePageTextUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, Tokens columns, bool docolumn)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string updatesql = "";
    std::string query = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(PAGETEXT_UPDATE_FILE);
#else
    fstr.append("/sql/").append(PAGETEXT_UPDATE_FILE);
#endif
    updatesql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\n"
        "SET CHARACTER SET `utf8`;\n\n"
        "UPDATE `page_text` ");

    query.append("SELECT WDB.entry,WDB.text,WORLD.text,WDB.next_page,WORLD.next_page FROM `");

    query.append(wdbdb).append("`.`pagetextcache` AS WDB, `");

    query.append(worlddb).append("`.`page_text` AS WORLD WHERE WDB.entry = WORLD.entry AND "
        "(WDB.text != WORLD.text || WDB.next_page != WORLD.next_page)");

    printf("Searching for different page texts...\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        uint64 count = 0;

        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        do
        {
            bool first = true;
            fields = result->Fetch();

            if (fields)
            {
                char* tmp;

                if ((docolumn && ColumnExists(columns, "text")) || !docolumn)
                {
                    if (strcmp(fields[1].GetCppString().c_str(), fields[2].GetCppString().c_str()) != 0)
                    {
                        if (first) updatesql.append("SET `text`='").append(io.Terminator(fields[1].GetCppString())).append("',");
                        else updatesql.append("`text`='").append(io.Terminator(fields[1].GetCppString())).append("',");
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "next_page")) || !docolumn)
                {
                    if (fields[3].GetUInt32() != fields[4].GetUInt32())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `next_page`='");
                        else updatesql.append("`next_page`='");
                        sprintf(tmp, "%u", fields[3].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
            }
            if (!first)
            {
                count++;
                updatesql.resize(updatesql.length()-1);
                char* tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[0].GetUInt32());
                updatesql.append(",`WDBVerified`='").append(_WDB_VERIFIED).append("' WHERE `entry`='").append(tmp).append("' LIMIT 1;\n");
                fputs(updatesql.c_str(), sqlfile);
                updatesql.clear();
                free(tmp);

                updatesql.append("UPDATE `page_text` ");
            }
        } while(result->NextRow());

        if (count)
        {
            char* tmp = (char*)malloc(32);
            sprintf(tmp, "%u", count);
            updatesql.clear();
            updatesql.append("\n# Differences in ").append(tmp).append(" entries found.");
            fputs(updatesql.c_str(), sqlfile);
            printf("%u different entries found.\n", count);
            free(tmp);
            fclose(sqlfile);
        }
        else
        {
            fclose(sqlfile);
            remove(fstr.c_str());
            printf("no differences found.\n");
        }
        delete result;

    } else printf("no differences found.\n");
}

void DatabaseUpdate::CreateQuestUpdate(const char* wdbdb, const char* worlddb, Database* DB, const char* home, Tokens columns, bool docolumn)
{
    QueryResult* result;
    Field* fields;

    std::string fstr = home;
    std::string updatesql = "";
    std::string query = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(QUEST_UPDATE_FILE);
#else
    fstr.append("/sql/").append(QUEST_UPDATE_FILE);
#endif
    updatesql.append(DATATRAP_FILE_HEADER).append("SET NAMES `utf8`;\n"
        "SET CHARACTER SET `utf8`;\n\n"
        "UPDATE `quest_template` ");

        //                  0
    query.append("SELECT WDB.entry,"
        "WDB.Method,WORLD.Method,WDB.QuestLevel,WORLD.QuestLevel,WDB.MinLevel,WORLD.MinLevel,WDB.ZoneOrSort,WORLD.ZoneOrSort,WDB.Type,WORLD.Type,WDB.SuggestedPlayers,WORLD.SuggestedPlayers,"
        // 13
        "WDB.RepObjectiveFaction,WORLD.RepObjectiveFaction,WDB.RepObjectiveValue,WORLD.RepObjectiveValue,WDB.RepObjectiveFaction2,WORLD.RepObjectiveFaction2,WDB.RepObjectiveValue2,WORLD.RepObjectiveValue2,"
        // 21
        "WDB.NextQuestInChain,WORLD.NextQuestInChain,WDB.RewXPId,WORLD.RewXPId,WDB.RewOrReqMoney,WORLD.RewOrReqMoney,WDB.RewMoneyMaxLevel,WORLD.RewMoneyMaxLevel,"
        // 29
        "WDB.RewSpell,WORLD.RewSpell,WDB.RewSpellCast,WORLD.RewSpellCast,WDB.RewHonorAddition,WORLD.RewHonorAddition,WDB.RewHonorMultiplier,WORLD.RewHonorMultiplier,"
        // 37
        "WDB.SrcItemId,WORLD.SrcItemId,WDB.QuestFlags,WORLD.QuestFlags,WDB.CharTitleId,WORLD.CharTitleId,WDB.PlayersSlain,WORLD.PlayersSlain,WDB.BonusTalents,WORLD.BonusTalents,"
        // 47
        "WDB.RewardArenaPoints,WORLD.RewardArenaPoints,WDB.unk0,WORLD.unk0,"
        // 51
        "WDB.RewItemId1,WORLD.RewItemId1,WDB.RewItemCount1,WORLD.RewItemCount1,"
        // 55
        "WDB.RewItemId2,WORLD.RewItemId2,WDB.RewItemCount2,WORLD.RewItemCount2,"
        // 59
        "WDB.RewItemId3,WORLD.RewItemId3,WDB.RewItemCount3,WORLD.RewItemCount3,"
        // 63
        "WDB.RewItemId4,WORLD.RewItemId4,WDB.RewItemCount4,WORLD.RewItemCount4,"
        // 67
        "WDB.RewChoiceItemId1,WORLD.RewChoiceItemId1,WDB.RewChoiceItemCount1,WORLD.RewChoiceItemCount1,"
        // 71
        "WDB.RewChoiceItemId2,WORLD.RewChoiceItemId2,WDB.RewChoiceItemCount2,WORLD.RewChoiceItemCount2,"
        // 75
        "WDB.RewChoiceItemId3,WORLD.RewChoiceItemId3,WDB.RewChoiceItemCount3,WORLD.RewChoiceItemCount3,"
        // 79
        "WDB.RewChoiceItemId4,WORLD.RewChoiceItemId4,WDB.RewChoiceItemCount4,WORLD.RewChoiceItemCount4,"
        // 83
        "WDB.RewChoiceItemId5,WORLD.RewChoiceItemId5,WDB.RewChoiceItemCount5,WORLD.RewChoiceItemCount5,"
        // 87
        "WDB.RewChoiceItemId6,WORLD.RewChoiceItemId6,WDB.RewChoiceItemCount6,WORLD.RewChoiceItemCount6,"
        // 91
        "WDB.RewRepFaction1,WORLD.RewRepFaction1,WDB.RewRepFaction2,WORLD.RewRepFaction2,WDB.RewRepFaction3,WORLD.RewRepFaction3,WDB.RewRepFaction4,WORLD.RewRepFaction4,WDB.RewRepFaction5,WORLD.RewRepFaction5,"
        // 101
        "WDB.RewRepValueId1,WORLD.RewRepValueId1,WDB.RewRepValueId2,WORLD.RewRepValueId2,WDB.RewRepValueId3,WORLD.RewRepValueId3,WDB.RewRepValueId4,WORLD.RewRepValueId4,WDB.RewRepValueId5,WORLD.RewRepValueId5,"
        // 111
        "WDB.RewRepValue1,WORLD.RewRepValue1,WDB.RewRepValue2,WORLD.RewRepValue2,WDB.RewRepValue3,WORLD.RewRepValue3,WDB.RewRepValue4,WORLD.RewRepValue4,WDB.RewRepValue5,WORLD.RewRepValue5,"
        // 121
        "WDB.PointMapId,WORLD.PointMapId,WDB.PointX,WORLD.PointX,WDB.PointY,WORLD.PointY,WDB.PointOpt,WORLD.PointOpt,"
        // 129
        "WDB.Title,WORLD.Title,WDB.Objectives,WORLD.Objectives,WDB.Details,WORLD.Details,WDB.EndText,WORLD.EndText,WDB.CompletedText,WORLD.CompletedText,"
        // 139
        "WDB.ReqCreatureOrGOId1,WORLD.ReqCreatureOrGOId1,WDB.ReqCreatureOrGOCount1,WORLD.ReqCreatureOrGOCount1,"
        // 143
        "WDB.ReqSourceId1,WORLD.ReqSourceId1,WDB.ReqSourceCount1,WORLD.ReqSourceCount1,"
        // 147
        "WDB.ReqCreatureOrGOId2,WORLD.ReqCreatureOrGOId2,WDB.ReqCreatureOrGOCount2,WORLD.ReqCreatureOrGOCount2,"
        // 151
        "WDB.ReqSourceId2,WORLD.ReqSourceId2,WDB.ReqSourceCount2,WORLD.ReqSourceCount2,"
        // 155
        "WDB.ReqCreatureOrGOId3,WORLD.ReqCreatureOrGOId3,WDB.ReqCreatureOrGOCount3,WORLD.ReqCreatureOrGOCount3,"
        // 159
        "WDB.ReqSourceId3,WORLD.ReqSourceId3,WDB.ReqSourceCount3,WORLD.ReqSourceCount3,"
        // 163
        "WDB.ReqCreatureOrGOId4,WORLD.ReqCreatureOrGOId4,WDB.ReqCreatureOrGOCount4,WORLD.ReqCreatureOrGOCount4,"
        // 167
        "WDB.ReqSourceId4,WORLD.ReqSourceId4,WDB.ReqSourceCount4,WORLD.ReqSourceCount4,"
        // 171
        "WDB.ReqItemId1,WORLD.ReqItemId1,WDB.ReqItemCount1,WORLD.ReqItemCount1,"
        // 175
        "WDB.ReqItemId2,WORLD.ReqItemId2,WDB.ReqItemCount2,WORLD.ReqItemCount2,"
        // 179
        "WDB.ReqItemId3,WORLD.ReqItemId3,WDB.ReqItemCount3,WORLD.ReqItemCount3,"
        // 183
        "WDB.ReqItemId4,WORLD.ReqItemId4,WDB.ReqItemCount4,WORLD.ReqItemCount4,"
        // 187
        "WDB.ReqItemId5,WORLD.ReqItemId5,WDB.ReqItemCount5,WORLD.ReqItemCount5,"
        // 191
        "WDB.ReqItemId6,WORLD.ReqItemId6,WDB.ReqItemCount6,WORLD.ReqItemCount6,"
        // 195                                                                                                                                                202
        "WDB.ObjectiveText1,WORLD.ObjectiveText1,WDB.ObjectiveText2,WORLD.ObjectiveText2,WDB.ObjectiveText3,WORLD.ObjectiveText3,WDB.ObjectiveText4,WORLD.ObjectiveText4 FROM `");

    query.append(wdbdb).append("`.`questcache` AS WDB, `");

    query.append(worlddb).append("`.`quest_template` AS WORLD WHERE WDB.entry = WORLD.entry AND ("
        "WDB.Method != WORLD.Method || WDB.QuestLevel != WORLD.QuestLevel || WDB.MinLevel != WORLD.MinLevel || WDB.ZoneOrSort != WORLD.ZoneOrSort || WDB.Type != WORLD.Type || "
        "WDB.SuggestedPlayers != WORLD.SuggestedPlayers || WDB.RepObjectiveFaction != WORLD.RepObjectiveFaction || WDB.RepObjectiveValue != WORLD.RepObjectiveValue || "
        "WDB.RepObjectiveFaction2 != WORLD.RepObjectiveFaction2 || WDB.RepObjectiveValue2 != WORLD.RepObjectiveValue2 || WDB.NextQuestInChain != WORLD.NextQuestInChain || "
        "WDB.RewXPId != WORLD.RewXPId || WDB.RewOrReqMoney != WORLD.RewOrReqMoney || WDB.RewMoneyMaxLevel != WORLD.RewMoneyMaxLevel || WDB.RewSpell != WORLD.RewSpell || "
        "WDB.RewSpellCast != WORLD.RewSpellCast || WDB.RewHonorAddition != WORLD.RewHonorAddition || WDB.RewHonorMultiplier != WORLD.RewHonorMultiplier || "
        "WDB.SrcItemId != WORLD.SrcItemId || WDB.QuestFlags != WORLD.QuestFlags || WDB.CharTitleId != WORLD.CharTitleId || WDB.PlayersSlain != WORLD.PlayersSlain || "
        "WDB.BonusTalents != WORLD.BonusTalents || WDB.RewardArenaPoints != WORLD.RewardArenaPoints || WDB.unk0 != WORLD.unk0 || "
        "WDB.RewItemId1 != WORLD.RewItemId1 || WDB.RewItemCount1 != WORLD.RewItemCount1 || "
        "WDB.RewItemId2 != WORLD.RewItemId2 || WDB.RewItemCount2 != WORLD.RewItemCount2 || "
        "WDB.RewItemId3 != WORLD.RewItemId3 || WDB.RewItemCount3 != WORLD.RewItemCount3 || "
        "WDB.RewItemId4 != WORLD.RewItemId4 || WDB.RewItemCount4 != WORLD.RewItemCount4 || "
        "WDB.RewChoiceItemId1 != WORLD.RewChoiceItemId1 || WDB.RewChoiceItemCount1 != WORLD.RewChoiceItemCount1 || "
        "WDB.RewChoiceItemId2 != WORLD.RewChoiceItemId2 || WDB.RewChoiceItemCount2 != WORLD.RewChoiceItemCount2 || "
        "WDB.RewChoiceItemId3 != WORLD.RewChoiceItemId3 || WDB.RewChoiceItemCount3 != WORLD.RewChoiceItemCount3 || "
        "WDB.RewChoiceItemId4 != WORLD.RewChoiceItemId4 || WDB.RewChoiceItemCount4 != WORLD.RewChoiceItemCount4 || "
        "WDB.RewChoiceItemId5 != WORLD.RewChoiceItemId5 || WDB.RewChoiceItemCount5 != WORLD.RewChoiceItemCount5 || "
        "WDB.RewChoiceItemId6 != WORLD.RewChoiceItemId6 || WDB.RewChoiceItemCount6 != WORLD.RewChoiceItemCount6 || "
        "WDB.RewRepFaction1 != WORLD.RewRepFaction1 || WDB.RewRepFaction2 != WORLD.RewRepFaction2 || "
        "WDB.RewRepFaction3 != WORLD.RewRepFaction3 || WDB.RewRepFaction4 != WORLD.RewRepFaction4 || "
        "WDB.RewRepFaction5 != WORLD.RewRepFaction5 || "
        "WDB.RewRepValueId1 != WORLD.RewRepValueId1 || WDB.RewRepValueId2 != WORLD.RewRepValueId2 || "
        "WDB.RewRepValueId3 != WORLD.RewRepValueId3 || WDB.RewRepValueId4 != WORLD.RewRepValueId4 || "
        "WDB.RewRepValueId5 != WORLD.RewRepValueId5 || "
        "WDB.RewRepValue1 != WORLD.RewRepValue1 || WDB.RewRepValue2 != WORLD.RewRepValue2 || "
        "WDB.RewRepValue3 != WORLD.RewRepValue3 || WDB.RewRepValue4 != WORLD.RewRepValue4 || "
        "WDB.RewRepValue5 != WORLD.RewRepValue5 || "
        "WDB.PointMapId != WORLD.PointMapId || WDB.PointX != WORLD.PointX || WDB.PointY != WORLD.PointY || WDB.PointOpt != WORLD.PointOpt || "
        "WDB.Title != WORLD.Title || WDB.Objectives != WORLD.Objectives || WDB.Details != WORLD.Details || WDB.EndText != WORLD.EndText || WDB.CompletedText != WORLD.CompletedText || "
        "WDB.ReqCreatureOrGOId1 != WORLD.ReqCreatureOrGOId1 || WDB.ReqCreatureOrGOCount1 != WORLD.ReqCreatureOrGOCount1 || WDB.ReqSourceId1 != WORLD.ReqSourceId1 || WDB.ReqSourceCount1 != WORLD.ReqSourceCount1 || "
        "WDB.ReqCreatureOrGOId2 != WORLD.ReqCreatureOrGOId2 || WDB.ReqCreatureOrGOCount2 != WORLD.ReqCreatureOrGOCount2 || WDB.ReqSourceId2 != WORLD.ReqSourceId2 || WDB.ReqSourceCount2 != WORLD.ReqSourceCount2 || "
        "WDB.ReqCreatureOrGOId3 != WORLD.ReqCreatureOrGOId3 || WDB.ReqCreatureOrGOCount3 != WORLD.ReqCreatureOrGOCount3 || WDB.ReqSourceId3 != WORLD.ReqSourceId3 || WDB.ReqSourceCount3 != WORLD.ReqSourceCount3 || "
        "WDB.ReqCreatureOrGOId4 != WORLD.ReqCreatureOrGOId4 || WDB.ReqCreatureOrGOCount4 != WORLD.ReqCreatureOrGOCount4 || WDB.ReqSourceId4 != WORLD.ReqSourceId4 || WDB.ReqSourceCount4 != WORLD.ReqSourceCount4 || "
        "WDB.ReqItemId1 != WORLD.ReqItemId1 || WDB.ReqItemCount1 != WORLD.ReqItemCount1 || WDB.ReqItemId2 != WORLD.ReqItemId2 || WDB.ReqItemCount2 != WORLD.ReqItemCount2 || "
        "WDB.ReqItemId3 != WORLD.ReqItemId3 || WDB.ReqItemCount3 != WORLD.ReqItemCount3 || WDB.ReqItemId4 != WORLD.ReqItemId4 || WDB.ReqItemCount4 != WORLD.ReqItemCount4 || "
        "WDB.ReqItemId5 != WORLD.ReqItemId5 || WDB.ReqItemCount5 != WORLD.ReqItemCount5 || WDB.ReqItemId6 != WORLD.ReqItemId6 || WDB.ReqItemCount6 != WORLD.ReqItemCount6 || "
        "WDB.ObjectiveText1 != WORLD.ObjectiveText1 || WDB.ObjectiveText2 != WORLD.ObjectiveText2 || WDB.ObjectiveText3 != WORLD.ObjectiveText3 || WDB.ObjectiveText4 != WORLD.ObjectiveText4)");

    printf("Searching for different quests...\t");

    result = DB->Query(query.c_str());

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    if (result)
    {
        uint64 count = 0;

        FILE* sqlfile = fopen(fstr.c_str(), "w");
        if (!sqlfile) io.SayError("Can't create: '%s'", fstr.c_str());

        // int32
        const char* column1[17] = {"Method","QuestLevel","MinLevel","ZoneOrSort","Type","SuggestedPlayers",
            "RepObjectiveFaction","RepObjectiveValue","RepObjectiveFaction2","RepObjectiveValue2",
            "NextQuestInChain","RewXPId","RewOrReqMoney","RewMoneyMaxLevel","RewSpell","RewSpellCast","RewHonorAddition"};

        // RewHonorMultiplier ist hier zwischen (float)

        // int32
        const char* column2[43] = {"SrcItemId","QuestFlags","CharTitleId","PlayersSlain","BonusTalents","RewardArenaPoints","unk0",
            "RewItemId1","RewItemCount1","RewItemId2","RewItemCount2","RewItemId3","RewItemCount3","RewItemId4","RewItemCount4",
            "RewChoiceItemId1","RewChoiceItemCount1","RewChoiceItemId2","RewChoiceItemCount2","RewChoiceItemId3","RewChoiceItemCount3",
            "RewChoiceItemId4","RewChoiceItemCount4","RewChoiceItemId5","RewChoiceItemCount5","RewChoiceItemId6","RewChoiceItemCount6",
            "RewRepFaction1","RewRepFaction2","RewRepFaction3","RewRepFaction4","RewRepFaction5",
            "RewRepValueId1","RewRepValueId2","RewRepValueId3","RewRepValueId4","RewRepValueId5",
            "RewRepValue1","RewRepValue2","RewRepValue3","RewRepValue4","RewRepValue5",
            "PointMapId"};

        // PointX (float) + PointY (float) + PointOpt (int32) sind hier zwischen

        // string
        const char* column3[5] = {"Title","Objectives","Details","EndText","CompletedText"};

        // int32
        const char* column4[28] = {"ReqCreatureOrGOId1","ReqCreatureOrGOCount1","ReqSourceId1","ReqSourceCount1",
            "ReqCreatureOrGOId2","ReqCreatureOrGOCount2","ReqSourceId2","ReqSourceCount2",
            "ReqCreatureOrGOId3","ReqCreatureOrGOCount3","ReqSourceId3","ReqSourceCount3",
            "ReqCreatureOrGOId4","ReqCreatureOrGOCount4","ReqSourceId4","ReqSourceCount4",
            "ReqItemId1","ReqItemCount1","ReqItemId2","ReqItemCount2","ReqItemId3","ReqItemCount3",
            "ReqItemId4","ReqItemCount4","ReqItemId5","ReqItemCount5","ReqItemId6","ReqItemCount6"};

        // string
        const char* column5[4] = {"ObjectiveText1","ObjectiveText2","ObjectiveText3","ObjectiveText4"};

        do
        {
            bool first = true;
            fields = result->Fetch();

            if (fields)
            {
                char* tmp = NULL;
                char* tmpstr = NULL;

                // column1
                for (uint8 i=0; i<17; ++i)
                {
                    if ((docolumn && ColumnExists(columns, column1[i])) || !docolumn)
                    {
                        if (fields[1+i*2].GetInt32() != fields[2+i*2].GetInt32())
                        {
                            tmp = (char*)malloc(32);

                            if (first)
                                updatesql.append("SET `").append(column1[i]).append("`='");
                            else
                                updatesql.append("`").append(column1[i]).append("`='");

                            sprintf(tmp, "%i", fields[1+i*2].GetInt32());
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                // RewHonorMultiplier
                if ((docolumn && ColumnExists(columns, "RewHonorMultiplier")) || !docolumn)
                {
                    if (fields[35].GetFloat() != fields[36].GetFloat() && fields[36].GetFloat() != 1.0f)
                    {
                        tmp = (char*)malloc(32);

                        if (first)
                            updatesql.append("SET `RewHonorMultiplier`='");
                        else
                            updatesql.append("`RewHonorMultiplier`='");

                        sprintf(tmp, "%f", fields[35].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // column2
                for (uint8 i=0; i<43; ++i)
                {
                    if ((docolumn && ColumnExists(columns, column2[i])) || !docolumn)
                    {
                        int32 tmpint = fields[37+i*2].GetInt32();

                        // Mask QuestFlags
                        if (37+i*2 == 39)
                            tmpint = tmpint&0xFFFF;

                        if (tmpint != fields[38+i*2].GetInt32())
                        {
                            tmp = (char*)malloc(32);

                            if (first)
                                updatesql.append("SET `").append(column2[i]).append("`='");
                            else
                                updatesql.append("`").append(column2[i]).append("`='");

                            sprintf(tmp, "%i", tmpint);
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                // PointX
                if ((docolumn && ColumnExists(columns, "PointX")) || !docolumn)
                {
                    if (fields[123].GetFloat() != fields[124].GetFloat())
                    {
                        tmp = (char*)malloc(32);

                        if (first)
                            updatesql.append("SET `PointX`='");
                        else
                            updatesql.append("`PointX`='");

                        sprintf(tmp, "%f", fields[123].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // PointY
                if ((docolumn && ColumnExists(columns, "PointY")) || !docolumn)
                {
                    if (fields[125].GetFloat() != fields[126].GetFloat())
                    {
                        tmp = (char*)malloc(32);

                        if (first)
                            updatesql.append("SET `PointY`='");
                        else
                            updatesql.append("`PointY`='");

                        sprintf(tmp, "%f", fields[125].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // PointOpt
                if ((docolumn && ColumnExists(columns, "PointOpt")) || !docolumn)
                {
                    if (fields[127].GetInt32() != fields[128].GetInt32())
                    {
                        tmp = (char*)malloc(32);

                        if (first)
                            updatesql.append("SET `PointOpt`='");
                        else
                            updatesql.append("`PointOpt`='");

                        sprintf(tmp, "%i", fields[127].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // column3
                for (uint8 i=0; i<5; ++i)
                {
                    if ((docolumn && ColumnExists(columns, column3[i])) || !docolumn)
                    {
                        if (strcmp(fields[129+i*2].GetCppString().c_str(), fields[130+i*2].GetCppString().c_str()) != 0)
                        {
                            if (first)
                                updatesql.append("SET `").append(column3[i]).append("`='").append(io.Terminator(fields[129+i*2].GetCppString())).append("',");
                            else
                                updatesql.append("`").append(column3[i]).append("`='").append(io.Terminator(fields[129+i*2].GetCppString())).append("',");

                            first = false;
                        }
                    }
                }
                // column4
                for (uint8 i=0; i<28; ++i)
                {
                    if ((docolumn && ColumnExists(columns, column4[i])) || !docolumn)
                    {
                        tmpstr = (char*)malloc(32);
                        int32 tmpint = 0;

                        // GOs umrechnen für den Core
                        if ((139+i*2 == 139 || 139+i*2 == 147 || 139+i*2 == 155 || 139+i*2 == 163) && fields[139+i*2].GetInt32() < 0)
                            tmpint = (fields[139+i*2].GetInt32() + 2147483648)*-1;
                        else
                            tmpint = fields[139+i*2].GetInt32();

                        sprintf(tmpstr, "%i", tmpint);

                        if (tmpint != fields[140+i*2].GetInt32())
                        {
                            if (first)
                                updatesql.append("SET `").append(column4[i]).append("`='");
                            else
                                updatesql.append("`").append(column4[i]).append("`='");

                            updatesql.append(tmpstr).append("',");
                            first = false;
                        }
                        free(tmpstr);
                    }
                }
                // column5
                for (uint8 i=0; i<4; ++i)
                {
                    if ((docolumn && ColumnExists(columns, column5[i])) || !docolumn)
                    {
                        if (strcmp(fields[195+i*2].GetCppString().c_str(), fields[196+i*2].GetCppString().c_str()) != 0)
                        {
                            if (first)
                                updatesql.append("SET `").append(column5[i]).append("`='").append(io.Terminator(fields[195+i*2].GetCppString())).append("',");
                            else
                                updatesql.append("`").append(column5[i]).append("`='").append(io.Terminator(fields[195+i*2].GetCppString())).append("',");

                            first = false;
                        }
                    }
                }
            }
            if (!first)
            {
                count++;
                updatesql.resize(updatesql.length()-1);
                char* tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[0].GetUInt32());
                updatesql.append(",`WDBVerified`='").append(_WDB_VERIFIED).append("' WHERE `entry`='").append(tmp).append("' LIMIT 1;\n");
                fputs(updatesql.c_str(), sqlfile);
                updatesql.clear();
                free(tmp);

                updatesql.append("UPDATE `quest_template` ");
            }
        } while(result->NextRow());

        if (count)
        {
            char* tmp = (char*)malloc(32);
            sprintf(tmp, "%u", count);
            updatesql.clear();
            updatesql.append("\n# Differences in ").append(tmp).append(" entries found.\n");
            fputs(updatesql.c_str(), sqlfile);
            printf("%u different entries found.\n", count);
            free(tmp);
            fclose(sqlfile);
        }
        else
        {
            fclose(sqlfile);
            remove(fstr.c_str());
            printf("no differences found.\n");
        }
        delete result;
    }
}

void DatabaseUpdate::CreateApplySQLFile(const char* home, const char* user, const char* pw,
                                        const char* host, const char* worlddb, const char* mysqlpath)
{
    std::string fstr = home;
    std::string fdir = home;
    std::string fline = "";

#if PLATFORM == PLATFORM_WINDOWS 
    fstr.append("\\sql\\").append(APPLY_SQL_FILE);
    fdir.append("\\sql");
#else
    fstr.append("/sql/").append(APPLY_SQL_FILE);
    fdir.append("/sql");
#endif

    FILE* testfile = fopen(fstr.c_str(), "a");
    if (testfile)
    {
        fclose(testfile);
        remove(fstr.c_str());
    }

    DIR* pDir = opendir(fdir.c_str());
    struct dirent* entry;

    if (pDir)
    {
        FILE* cmdfile = fopen(fstr.c_str(), "w");
        if (!cmdfile) io.SayError("Can't create: '%s'", fstr.c_str());

        bool first = true;

#if PLATFORM == PLATFORM_WINDOWS
        _chdir(fdir.c_str());
#else
        chdir(fdir.c_str());
#endif
        while ((entry = readdir(pDir)) != NULL)
        {
#if PLATFORM == PLATFORM_WINDOWS
            if (strlen(entry->d_name) > 14)
            {
                if (first)
                {
                    if (strlen(mysqlpath)) fline.append(REM_DATATRAP_FILE_HEADER).append("ECHO Working on: ").append(entry->d_name).append("\n").append(mysqlpath).append("\\mysql.exe --user=").append(user).append(" --password=").append(pw).append(" ").append("--host=").append(host).append(" ").append(worlddb).append(" < .\\");
                    else fline.append(REM_DATATRAP_FILE_HEADER).append("ECHO Working on: ").append(entry->d_name).append("\n").append("mysql.exe ").append(user).append(" --password=").append(pw).append(" ").append("--host=").append(host).append(" ").append(worlddb).append(" < .\\");
                }
                else
                {
                    if (strlen(mysqlpath)) fline.append("ECHO Working on: ").append(entry->d_name).append("\n").append(mysqlpath).append("\\mysql.exe --user=").append(user).append(" --password=").append(pw).append(" ").append("--host=").append(host).append(" ").append(worlddb).append(" < .\\");
                    else fline.append("ECHO Working on: ").append(entry->d_name).append("\n").append("mysql.exe ").append(user).append(" --password=").append(pw).append(" ").append("--host=").append(host).append(" ").append(worlddb).append(" < .\\");
                }
#else
                if (first) fline.append("#!/bin/bash\n\n").append(DATATRAP_FILE_HEADER).append("echo 'Working on: ").append(entry->d_name).append("'\n").append("mysql ").append(user).append(" --password=").append(pw).append(" ").append("--host=").append(host).append(" ").append(worlddb).append(" < ./");
                else fline.append("echo 'Working on: ").append(entry->d_name).append("'\n").append("mysql ").append(user).append(" --password=").append(pw).append(" ").append("--host=").append(host).append(" ").append(worlddb).append(" < ./");
#endif
                if      (strstr(entry->d_name, CREATURE_INSERT_FILE))   fline.append(CREATURE_INSERT_FILE);
                else if (strstr(entry->d_name, GAMEOBJECT_INSERT_FILE)) fline.append(GAMEOBJECT_INSERT_FILE);
                else if (strstr(entry->d_name, ITEM_INSERT_FILE))       fline.append(ITEM_INSERT_FILE);
                else if (strstr(entry->d_name, NPCTEXT_INSERT_FILE))    fline.append(NPCTEXT_INSERT_FILE);
                else if (strstr(entry->d_name, PAGETEXT_INSERT_FILE))   fline.append(PAGETEXT_INSERT_FILE);
                else if (strstr(entry->d_name, QUEST_INSERT_FILE))      fline.append(QUEST_INSERT_FILE);
                else if (strstr(entry->d_name, CREATURE_UPDATE_FILE))   fline.append(CREATURE_UPDATE_FILE);
                else if (strstr(entry->d_name, GAMEOBJECT_UPDATE_FILE)) fline.append(GAMEOBJECT_UPDATE_FILE);
                else if (strstr(entry->d_name, ITEM_UPDATE_FILE))       fline.append(ITEM_UPDATE_FILE);
                else if (strstr(entry->d_name, NPCTEXT_UPDATE_FILE))    fline.append(NPCTEXT_UPDATE_FILE);
                else if (strstr(entry->d_name, PAGETEXT_UPDATE_FILE))   fline.append(PAGETEXT_UPDATE_FILE);
                else if (strstr(entry->d_name, QUEST_UPDATE_FILE))      fline.append(QUEST_UPDATE_FILE);

                fline.append("\n");
                fputs(fline.c_str(), cmdfile);
                fline.clear();
                first = false;
            }
        }
#if PLATFORM == PLATFORM_WINDOWS
        if (!first) fputs("\nPAUSE\n", cmdfile);
#endif
        fclose(cmdfile);

        if (first) remove(fstr.c_str());

#if PLATFORM != PLATFORM_WINDOWS
        if (!first)
        {
            fline.clear();
            fline = "chmod 755 ";
            fline.append(fstr.c_str());
            system(fline.c_str());
            chdir("..");
        }
#else
        _chdir("..");
#endif
        closedir(pDir);
    } else io.SayError("Can't open: '%s'", fdir.c_str());
}

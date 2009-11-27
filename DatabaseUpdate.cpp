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
    "(`entry`,`name`,`subname`,`IconName`,`type_flags`,`unit_flags`,`type`,`family`,`rank`,"
    "`modelid1`,`modelid2`,`modelid3`,`modelid4`,`unk16`,`unk17`,`RacialLeader`,"
    "`questItem1`,`questItem2`,`questItem3`,`questItem4`,`questItem5`,`questItem6`,`movementId`,"
    "`minlevel`,`maxlevel`,`minhealth`,`maxhealth`,`faction_A`,`faction_H`,`scale`) VALUES\n('");

    query.append("SELECT `entry`,`name`,`subname`,`IconName`,`type_flags`,`unit_flags`,`type`,`family`,`rank`,"
        "`modelid1`,`modelid2`,`modelid3`,`modelid4`,`unk16`,`unk17`,`RacialLeader`,"
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
                for (uint8 i=0; i<23; i++)
                {
                    char* tmp = (char*)malloc(32);

                    if (i == 1 || i == 2 || i == 3)
                    {
                        // name + subname + IconName
                        insertsql.append(io.Terminator(fields[i].GetCppString())).append("','");
                    }
                    else if (i == 4 || i == 5)
                    {
                        sprintf(tmp, "%u", fields[i].GetUInt16());
                        insertsql.append(tmp).append("','");
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
                insertsql.append("1','1','1','1','35','35','1')");

                // Haben wir MAX_INSERTS erreicht und Daten übrig? Dann insert senden und löschen!
                if (count >= MAX_INSERTS && (count+counttotal+1) < result->GetRowCount())
                {
                    insertsql.append(";\n");
                    fputs(insertsql.c_str(), sqlfile);
                    insertsql.clear();
                    counttotal += count;
                    count = 0;

                    insertsql.append("INSERT INTO `creature_template` "
                    "(`entry`,`name`,`subname`,`IconName`,`type_flags`,`unit_flags`,`type`,`family`,`rank`,"
                    "`modelid1`,`modelid2`,`modelid3`,`modelid4`,`unk16`,`unk17`,`RacialLeader`,"
                    "`questItem1`,`questItem2`,`questItem3`,`questItem4`,`questItem5`,`questItem6`,`movementId`,"
                    "`minlevel`,`maxlevel`,`minhealth`,`maxhealth`,`faction_A`,`faction_H`,`scale`) VALUES\n('");

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
        "`questItem1`,`questItem2`,`questItem3`,`questItem4`,`questItem5`,`questItem6`" // NEU!!!
        ") VALUES\n('");

    query.append("SELECT `entry`,`type`,`displayId`,`name`,`IconName`,`castBarCaption`,`unk1`,`data0`,`data1`,`data2`,`data3`,`data4`,"
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
                for (uint8 i=0; i<33; i++)
                {
                    char* tmp = (char*)malloc(32);

                    if (i >= 3 && i <= 6)
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
                                uint32 uitmp = fields[i].GetUInt32();
                                if (uitmp == 4294967295) uitmp = 0;
                                sprintf(tmp, "%u", uitmp);
                                insertsql.append(tmp).append("','");
                            }
                        }
                        else
                        {
                            uint32 uitmp = fields[i].GetUInt32();
                            if (uitmp == 4294967295) uitmp = 0;
                            sprintf(tmp, "%u", uitmp);
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
                insertsql.append(tmp).append("')");
                free(tmp);
                // questItem5
                tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[36].GetUInt32());
                insertsql.append(tmp).append("')");
                free(tmp);
                // questItem6
                tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[37].GetUInt32());
                insertsql.append(tmp).append("')");
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
                        "`questItem1`,`questItem2`,`questItem3`,`questItem4`,`questItem5`,`questItem6`" // NEU!!!
                        ") VALUES\n('");

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
        // @ 3.1.0 gelöscht!
        //`dmg_min3`,`dmg_max3`,`dmg_type3`,`dmg_min4`,`dmg_max4`,`dmg_type4`,`dmg_min5`,`dmg_max5`,"`dmg_type5`,
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
        "`ArmorDamageModifier`,`Duration`,`ItemLimitCategory`,`HolidayId`) VALUES\n('");

    //                      0
    query.append("SELECT `entry`,`Class`,`SubClass1`,`unk0`,`Name1`,`ItemDisplayID`,`Quality`,`Flags`,"
        "`BuyPrice`,`SellPrice`,`InventorySlot`,`AllowableClass`,`AllowableRace`,`ItemLevel`,`RequiredLevel`,`RequiredSkill`,"
        "`RequiredSkillRank`,`RequiredSpell`,`RequiredHonorRank`,`RequiredCityRank`,`RequiredReputationFaction`,`RequiredReputationRank`,`maxcount`,`StackAmount`,"
        "`ContainerSlots`,`StatsCount`,"
        // 26
        "`Stat1`,`Stat1Val`,`Stat2`,`Stat2Val`,`Stat3`,`Stat3Val`,`Stat4`,`Stat4Val`,`Stat5`,`Stat5Val`,`Stat6`,"
        "`Stat6Val`,`Stat7`,`Stat7Val`,`Stat8`,`Stat8Val`,`Stat9`,`Stat9Val`,`Stat10`,`Stat10Val`,"
        "`Stat10`,`Stat10Val`," // Doppel wegen `ScalingStatDistribution`,`ScalingStatValue` im Insert! Damit es paßt!
        // 48
        "`Damage1Min`,`Damage1Max`,`Damage1Type`,`Damage2Min`,`Damage2Max`,`Damage2Type`,"
        // @ 3.1.0 gelöscht!
        //`Damage3Min`,`Damage3Max`,`Damage3Type`,`Damage4Min`,`Damage4Max`,`Damage4Type`,`Damage5Min`,`Damage5Max`,`Damage5Type`,"
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

        //int felder 3,11,12,22,23,26-47,66,67,69,72,73,75,78,79,81,84,85,87,90,91,93,94,98,101,102,120,122
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
                        case 3: case 11: case 12: case 22: case 23: case 26: case 27: case 28: case 29: case 30: case 31: case 32: case 33:
                        case 34: case 35: case 36: case 37: case 38: case 39: case 40: case 41: case 42: case 43: case 44: case 45: case 46:
                        case 47: case 66: case 67: case 69: case 72: case 73: case 75: case 78: case 79: case 81: case 84: case 85: case 87:
                        case 90: case 91: case 93: case 94: case 98: case 101: case 102: case 120: case 122:
                            {
                                int32 tmpint = fields[i].GetInt32(); /*
                                #######################################################################################
                                # Wenn 2147483647 dann soll das für Blizz wohl unendlich heißen (z.b. Ehre und Arenap.)
                                #######################################################################################
                                UPDATE `itemcache` SET `IsUnique`='0' WHERE `IsUnique` IN ('2147483647','4294967295');
                                UPDATE `itemcache` SET `StackAmount`='0' WHERE `StackAmount` IN ('2147483647','4294967295'); */
                                if ((i == 22) && tmpint > 32000) tmpint = 0;
                                if ((i == 23) && tmpint > 32000) tmpint = -1;

                                // Account gebundene Items mit veränderlichen Stats!
                                if (i == 44 || i == 45)// StatType10 + StatValue10
                                {                      // StatCount
                                    if (tmpint != 0 && fields[25].GetInt32() == 0)
                                    {   // Bei `ScalingStatDistribution`,`ScalingStatValue` den Wert neu setzen
                                        char* tmpchar = (char*)malloc(32);
                                        fields[i+2].SetValue(_itoa(tmpint, tmpchar, 10));
                                        free(tmpchar);
                                        tmpint = 0;
                                    }
                                } /*
                                ######################################################################################
                                # Flags > 3551 in -1 wandeln, weil der Wert zu groß ist für die Spalte - -1 paßt aber!
                                ######################################################################################
                                UPDATE `itemcache` SET `ClassFlags`='-1' WHERE `ClassFlags`>'3551';
                                UPDATE `itemcache` SET `RaceFlags`='-1' WHERE `RaceFlags`>'3551'; */
                                if ((i == 11 || i == 12) && tmpint > 3551) tmpint = -1;

                                sprintf(tmp, "%i", tmpint);
                                if (i+1 < 125) insertsql.append(tmp).append("','");
                                else insertsql.append(tmp);
                            }
                            break;
                        // uint32 felder
                        default:
                            {
                                uint32 tmpuint = fields[i].GetUInt32(); /*
                                #######################################################################
                                # Faction-"Korrektur" wegen core! :-( Das selbe wie bei den ModelIDs!
                                #######################################################################
                                UPDATE `itemcache` SET `reqfactionlvl`='0' WHERE `reqfaction`='0';*/
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
                insertsql.append("')");

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
                        // @ 3.1.0 gelöscht!
                        //`dmg_min3`,`dmg_max3`,`dmg_type3`,`dmg_min4`,`dmg_max4`,`dmg_type4`,`dmg_min5`,`dmg_max5`,"`dmg_type5`,
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
                        "`ArmorDamageModifier`,`Duration`,`ItemLimitCategory`,`HolidayId`) VALUES\n('");

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
        "`text7_0`,`text7_1`,`lang7`,`prob7`,`em7_0`,`em7_1`,`em7_2`,`em7_3`,`em7_4`) VALUES\n('");

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
                insertsql.append("')");

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
                        "`text7_0`,`text7_1`,`lang7`,`prob7`,`em7_0`,`em7_1`,`em7_2`,`em7_3`,`em7_4`) VALUES\n('");

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
        "(`entry`,`text`,`next_page`) VALUES\n('");

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
                insertsql.append(tmp).append("')");
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
                        "(`entry`,`text`,`next_page`) VALUES\n('");

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
        "(`entry`,`Method`,`QuestLevel`,`ZoneOrSort`,`Type`,`SuggestedPlayers`,`RepObjectiveFaction`,"
        "`RepObjectiveValue`,`NextQuestInChain`,`RewOrReqMoney`,`RewMoneyMaxLevel`,`RewSpell`,`RewSpellCast`,"
        "`SrcItemId`,`QuestFlags`,`RewItemId1`,`RewItemCount1`,`RewItemId2`,`RewItemCount2`,`RewItemId3`,"
        "`RewItemCount3`,`RewItemId4`,`RewItemCount4`,`RewChoiceItemId1`,`RewChoiceItemCount1`,`RewChoiceItemId2`,"
        "`RewChoiceItemCount2`,`RewChoiceItemId3`,`RewChoiceItemCount3`,`RewChoiceItemId4`,`RewChoiceItemCount4`,"
        "`RewChoiceItemId5`,`RewChoiceItemCount5`,`RewChoiceItemId6`,`RewChoiceItemCount6`,`PointMapId`,`PointX`,"
        "`PointY`,`PointOpt`,`Title`,`Objectives`,`Details`,`EndText`,`ReqCreatureOrGOId1`,`ReqCreatureOrGOCount1`,"
        "`ReqItemId1`,`ReqItemCount1`,`ReqSourceId1`,`ReqCreatureOrGOId2`,`ReqCreatureOrGOCount2`,`ReqItemId2`,"
        "`ReqItemCount2`,`ReqSourceId2`,`ReqCreatureOrGOId3`,`ReqCreatureOrGOCount3`,`ReqItemId3`,`ReqItemCount3`,"
        "`ReqSourceId3`,`ReqCreatureOrGOId4`,`ReqCreatureOrGOCount4`,`ReqItemId4`,`ReqItemCount4`,`ReqSourceId4`,"
        "`ObjectiveText1`,`ObjectiveText2`,`ObjectiveText3`,`ObjectiveText4`,`RewHonorableKills`) VALUES\n('");

    query.append("SELECT `entry`,`Method`,`QuestLevel`,`ZoneOrSort`,`Type`,`SuggestedPlayers`,`FactionID`,"
        "`FactionAmount`,`NextQuestID`,`CoinReward`,`CoinRewardOn80`,`SpellReward`,`EffectOnPlayer`,`StartItemID`,"
        "`QuestFlags`,`ItemReward1`,`ItemAmount1`,`ItemReward2`,`ItemAmount2`,`ItemReward3`,`ItemAmount3`,"
        "`ItemReward4`,`ItemAmount4`,`ItemChoice1`,`ItemChoiceAmount1`,`ItemChoice2`,`ItemChoiceAmount2`,"
        "`ItemChoice3`,`ItemChoiceAmount3`,`ItemChoice4`,`ItemChoiceAmount4`,`ItemChoice5`,`ItemChoiceAmount5`,"
        "`ItemChoice6`,`ItemChoiceAmount6`,`PointMapId`,`PointX`,`PointY`,`PointOpt`,`Name`,`Description`,"
        "`Details`,`Subdescription`,`KillCreature1`,`KillCreature1Amount`,`CollectItem1`,`CollectItem1Amount`,"
        "`ItemUsed1`,`KillCreature2`,`KillCreature2Amount`,`CollectItem2`,`CollectItem2Amount`,`ItemUsed2`,"
        "`KillCreature3`,`KillCreature3Amount`,`CollectItem3`,`CollectItem3Amount`,`ItemUsed3`,`KillCreature4`,"
        "`KillCreature4Amount`,`CollectItem4`,`CollectItem4Amount`,`ItemUsed4`,`Objective1`,`Objective2`,"
        "`Objective3`,`Objective4` FROM `");
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
                // "WDB.EffectOnPlayer,WORLD.RewSpellCast" must be int32 but core doesn't know(?)!
                // Because of this there are 4294967295 values!
                for (uint8 i=0; i<67; i++)
                {
                    char* tmp = (char*)malloc(32);

                    if (i == 2 || i == 3 || i == 9 || i == 43 || i == 44 || i == 45 || i == 46 || i == 47 ||
                        i == 48 || i == 49 || i == 50 || i == 51 || i == 52 || i == 53 || i == 54 || i == 55 ||
                        i == 56 || i == 57 || i == 58 || i == 59 || i == 60 || i == 61 || i == 62)
                    {
                        // GOs umrechnen für core
                        if ((i == 43 || i == 48 || i == 53 || i == 58) && fields[i].GetInt32() < 0)
                            sprintf(tmp, "%i", (fields[i].GetInt32() + 2147483648)*-1);
                        else
                            sprintf(tmp, "%i", fields[i].GetInt32());

                        if (i+1 < 67) insertsql.append(tmp).append("','");
                        else
                            insertsql.append(tmp);
                    }
                    else if (i == 39 || i == 40 || i == 41 || i == 42 || i == 63 || i == 64 || i == 65 || i == 66)
                    {
                        // Texte terminieren
                        if (i+1 < 67) insertsql.append(io.Terminator(fields[i].GetCppString())).append("','");
                        else insertsql.append(io.Terminator(fields[i].GetCppString()));
                    }
                    else if (i == 36 || i == 37)
                    {
                        // PointX + PointY
                        sprintf(tmp, "%f", fields[i].GetFloat());
                        if (i+1 < 67) insertsql.append(tmp).append("','");
                        else insertsql.append(tmp);
                    }
                    else
                    {
                        sprintf(tmp, "%u", fields[i].GetUInt32());
                        if (i+1 < 67) insertsql.append(tmp).append("','");
                        else insertsql.append(tmp);
                    }
                    free(tmp);
                }
                insertsql.append("','0')"); // `RewHonorableKills` darf nicht leer sein

                // Haben wir MAX_INSERTS erreicht und Daten übrig? Dann insert senden und löschen!
                if (count >= MAX_INSERTS && (count+counttotal+1) < result->GetRowCount())
                {
                    insertsql.append(";\n");
                    fputs(insertsql.c_str(), sqlfile);
                    insertsql.clear();
                    counttotal += count;
                    count = 0;

                    insertsql.append("INSERT INTO `quest_template` "
                        "(`entry`,`Method`,`QuestLevel`,`ZoneOrSort`,`Type`,`SuggestedPlayers`,`RepObjectiveFaction`,"
                        "`RepObjectiveValue`,`NextQuestInChain`,`RewOrReqMoney`,`RewMoneyMaxLevel`,`RewSpell`,`RewSpellCast`,"
                        "`SrcItemId`,`QuestFlags`,`RewItemId1`,`RewItemCount1`,`RewItemId2`,`RewItemCount2`,`RewItemId3`,"
                        "`RewItemCount3`,`RewItemId4`,`RewItemCount4`,`RewChoiceItemId1`,`RewChoiceItemCount1`,`RewChoiceItemId2`,"
                        "`RewChoiceItemCount2`,`RewChoiceItemId3`,`RewChoiceItemCount3`,`RewChoiceItemId4`,`RewChoiceItemCount4`,"
                        "`RewChoiceItemId5`,`RewChoiceItemCount5`,`RewChoiceItemId6`,`RewChoiceItemCount6`,`PointMapId`,`PointX`,"
                        "`PointY`,`PointOpt`,`Title`,`Objectives`,`Details`,`EndText`,`ReqCreatureOrGOId1`,`ReqCreatureOrGOCount1`,"
                        "`ReqItemId1`,`ReqItemCount1`,`ReqSourceId1`,`ReqCreatureOrGOId2`,`ReqCreatureOrGOCount2`,`ReqItemId2`,"
                        "`ReqItemCount2`,`ReqSourceId2`,`ReqCreatureOrGOId3`,`ReqCreatureOrGOCount3`,`ReqItemId3`,`ReqItemCount3`,"
                        "`ReqSourceId3`,`ReqCreatureOrGOId4`,`ReqCreatureOrGOCount4`,`ReqItemId4`,`ReqItemCount4`,`ReqSourceId4`,"
                        "`ObjectiveText1`,`ObjectiveText2`,`ObjectiveText3`,`ObjectiveText4`,`RewHonorableKills`) VALUES\n('");

                    first = true;

                } else first = false;
            }
            fputs(insertsql.c_str(), sqlfile);
            
        } while (result->NextRow());

        char* tmp = (char*)malloc(32);
        sprintf(tmp, "%u", result->GetRowCount());
        sortsql.append(";\n\n# ").append(tmp).append(" new quests found.\n\n"
            "# ATTENTION: We have to move the original data to the first field because of the core limitations.\n"
            "#            If the client find a zero it stops showing the left rewards. Because of these\n"
            "#            movements you'll always get updates for these values until the core fix this problem\n"
            "#            at the creation time of the corresponding packet!\n"
            "UPDATE `quest_template` SET RewItemId1=RewItemId2,RewItemId2=RewItemId3,RewItemId3=RewItemId4,RewItemId4=0,RewItemCount1=RewItemCount2,RewItemCount2=RewItemCount3,RewItemCount3=RewItemCount4,RewItemCount4=0 WHERE RewItemId1=0 AND RewItemId2!=0;\n"
            "UPDATE `quest_template` SET RewChoiceItemId1=RewChoiceItemId2,RewChoiceItemId2=RewChoiceItemId3,RewChoiceItemId3=RewChoiceItemId4,RewChoiceItemId4=RewChoiceItemId5,RewChoiceItemId5=RewChoiceItemId6,RewChoiceItemId6=0,RewChoiceItemCount1=RewChoiceItemCount2,RewChoiceItemCount2=RewChoiceItemCount3,RewChoiceItemCount3=RewChoiceItemCount4,RewChoiceItemCount4=RewChoiceItemCount5,RewChoiceItemCount5=RewChoiceItemCount6,RewChoiceItemCount6=0 WHERE RewChoiceItemId1=0 AND RewChoiceItemId2!=0;\n\n"

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
        "WDB.type_flags, WORLD.type_flags, WDB.unit_flags, WORLD.unit_flags, WDB.`type`, WORLD.`type`, WDB.family, "
        "WORLD.family, WDB.rank, WORLD.rank, WDB.modelid1, WORLD.modelid1, WDB.modelid2, WORLD.modelid2, WDB.modelid3, WORLD.modelid3, WDB.modelid4, "
        "WORLD.modelid4, WDB.unk16, WORLD.unk16, WDB.unk17, WORLD.unk17, WDB.RacialLeader, WORLD.RacialLeader, "
        "WDB.questItem1, WORLD.questItem1, WDB.questItem2, WORLD.questItem2, WDB.questItem3, WORLD.questItem3, WDB.questItem4, WORLD.questItem4, "
        "WDB.questItem5, WORLD.questItem5, WDB.questItem6, WORLD.questItem6, WDB.movementId, WORLD.movementId"
        " FROM `");
    query.append(wdbdb).append("`.`creaturecache` AS WDB, `");
    query.append(worlddb).append("`.`creature_template` AS WORLD WHERE WDB.entry = WORLD.entry AND "
        "(WDB.name != WORLD.name || WDB.subname != WORLD.subname || WDB.IconName != WORLD.IconName || "
        "WDB.type_flags != WORLD.type_flags || WDB.unit_flags != WORLD.unit_flags || "
        "WDB.`type` != WORLD.`type` || WDB.family != WORLD.family || WDB.rank != WORLD.rank || "
        "WDB.modelid1 != WORLD.modelid1 || WDB.modelid2 != WORLD.modelid2 || WDB.modelid3 != WORLD.modelid3 || "
        "WDB.modelid4 != WORLD.modelid4 || WDB.unk16 != WORLD.unk16 || WDB.unk17 != WORLD.unk17 || WDB.RacialLeader != WORLD.RacialLeader || "
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

        const char* column[9] = {"type_flags", "unit_flags", "type", "family", "rank", "modelid1", "modelid2", "modelid3", "modelid4"};

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
                for (uint8 i=0; i<9; i++)
                {
                    if ((docolumn && ColumnExists(columns, column[i])) || !docolumn)
                    {
                        if (fields[7+i*2].GetUInt32() != fields[8+i*2].GetUInt32())
                        {
                            uint32 tmpuint = fields[7+i*2].GetUInt32();
                            uint32 tmpentry = fields[0].GetUInt32();

                            // KEIN MODELUPDATE FÜR DIESE ENTRIES, WEIL FALSCHE DATEN IN DEN WDBS STEHEN!!!
                            if (tmpentry == 24938 || tmpentry == 25115 || tmpentry == 25001 || tmpentry == 26477) continue;
                            else
                            {
                                if (first) updatesql.append("SET `").append(column[i]).append("`='");
                                else updatesql.append("`").append(column[i]).append("`='");
                                char* tmp = (char*)malloc(32);
                                sprintf(tmp, "%u", tmpuint);
                                updatesql.append(tmp).append("',");
                                first = false;
                                free(tmp);
                            }
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "unk16")) || !docolumn)
                {
                    // unk16
                    if (fields[25].GetFloat() != fields[26].GetFloat())
                    {
                        float tmpfloat = fields[25].GetFloat();
                        if (first) updatesql.append("SET `unk16`='");
                        else updatesql.append("`unk16`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%f", tmpfloat);
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
                if ((docolumn && ColumnExists(columns, "unk17")) || !docolumn)
                {
                    // unk17
                    if (fields[27].GetFloat() != fields[28].GetFloat())
                    {
                        float tmpfloat = fields[27].GetFloat();
                        if (first) updatesql.append("SET `unk17`='");
                        else updatesql.append("`unk17`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%f", tmpfloat);
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
                if ((docolumn && ColumnExists(columns, "RacialLeader")) || !docolumn)
                {
                    // RacialLeader
                    if (fields[29].GetUInt32() != fields[30].GetUInt32())
                    {
                        uint32 tmpuint = fields[29].GetUInt32();
                        if (first) updatesql.append("SET `RacialLeader`='");
                        else updatesql.append("`RacialLeader`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", tmpuint);
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem1")) || !docolumn)
                {
                    // questItem1
                    if (fields[31].GetUInt32() != fields[32].GetUInt32())
                    {
                        uint32 tmpuint = fields[31].GetUInt32();
                        if (first) updatesql.append("SET `questItem1`='");
                        else updatesql.append("`questItem1`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", tmpuint);
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem2")) || !docolumn)
                {
                    // questItem2
                    if (fields[33].GetUInt32() != fields[34].GetUInt32())
                    {
                        uint32 tmpuint = fields[33].GetUInt32();
                        if (first) updatesql.append("SET `questItem2`='");
                        else updatesql.append("`questItem2`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", tmpuint);
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem3")) || !docolumn)
                {
                    // questItem3
                    if (fields[35].GetUInt32() != fields[36].GetUInt32())
                    {
                        uint32 tmpuint = fields[35].GetUInt32();
                        if (first) updatesql.append("SET `questItem3`='");
                        else updatesql.append("`questItem3`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", tmpuint);
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem4")) || !docolumn)
                {
                    // questItem4
                    if (fields[37].GetUInt32() != fields[38].GetUInt32())
                    {
                        uint32 tmpuint = fields[37].GetUInt32();
                        if (first) updatesql.append("SET `questItem4`='");
                        else updatesql.append("`questItem4`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", tmpuint);
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem5")) || !docolumn)
                {
                    // questItem5
                    if (fields[39].GetUInt32() != fields[40].GetUInt32())
                    {
                        uint32 tmpuint = fields[39].GetUInt32();
                        if (first) updatesql.append("SET `questItem5`='");
                        else updatesql.append("`questItem5`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", tmpuint);
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
                if ((docolumn && ColumnExists(columns, "questItem6")) || !docolumn)
                {
                    // questItem6
                    if (fields[41].GetUInt32() != fields[42].GetUInt32())
                    {
                        uint32 tmpuint = fields[41].GetUInt32();
                        if (first) updatesql.append("SET `questItem6`='");
                        else updatesql.append("`questItem6`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", tmpuint);
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
                if ((docolumn && ColumnExists(columns, "movementId")) || !docolumn)
                {
                    // movementId
                    if (fields[43].GetUInt32() != fields[44].GetUInt32())
                    {
                        uint32 tmpuint = fields[43].GetUInt32();
                        if (first) updatesql.append("SET `movementId`='");
                        else updatesql.append("`movementId`='");
                        char* tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", tmpuint);
                        updatesql.append(tmp).append("',");
                        first = false;
                        free(tmp);
                    }
                }
            }
            if (!first)
            {
                count++;
                updatesql.resize(updatesql.length()-1);
                char* tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[0].GetUInt32());
                updatesql.append(" WHERE `entry`='").append(tmp).append("' LIMIT 1;\n");
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

    query.append("SELECT WDB.entry, WDB.type, WORLD.type, WDB.displayId, WORLD.displayId, WDB.name, WORLD.name,"
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
        "(WDB.type != WORLD.type || WDB.displayId != WORLD.displayId || WDB.name != WORLD.name ||"
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
                for (uint8 i=0; i<24; i++)
                {
                    if ((docolumn && ColumnExists(columns, column[i])) || !docolumn)
                    {
                        uint32 uitmp = fields[14+i*2].GetUInt32();
                        if (uitmp == 4294967295) uitmp = 0;

                        // lootid auf entry setzen
                        if (own_style && i == 1 && fields[0].GetUInt32() != uitmp &&
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
                            uint32 uitmp = fields[13+i*2].GetUInt32();
                            if (uitmp == 4294967295) uitmp = 0;

                            if (uitmp != fields[14+i*2].GetUInt32() && !(own_style && i == 1))
                            {
                                if (first) updatesql.append("SET `").append(column[i]).append("`='");
                                else updatesql.append("`").append(column[i]).append("`='");

                                tmp = (char*)malloc(32);
                                sprintf(tmp, "%u", uitmp);
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
                updatesql.append(" WHERE `entry`='").append(tmp).append("' LIMIT 1;\n");
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
        "WORLD.RequiredReputationRank,WDB.maxcount,WORLD.maxcount,WDB.StackAmount,WORLD.stackable,"
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
        /* @ 3.1.0 gelöscht!
        "WDB.Damage3Min,WORLD.dmg_min3,WDB.Damage3Max,WORLD.dmg_max3,WDB.Damage3Type,WORLD.dmg_type3,"
        "WDB.Damage4Min,WORLD.dmg_min4,WDB.Damage4Max,WORLD.dmg_max4,WDB.Damage4Type,WORLD.dmg_type4,"
        "WDB.Damage5Min,WORLD.dmg_min5,WDB.Damage5Max,WORLD.dmg_max5,WDB.Damage5Type,WORLD.dmg_type5,"*/

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
        " WDB.maxcount != WORLD.maxcount || WDB.StackAmount != WORLD.stackable ||"
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
        /* @ 3.1.0 gelöscht!
        "WDB.Damage3Min != WORLD.dmg_min3 || WDB.Damage3Max != WORLD.dmg_max3 || WDB.Damage3Type != WORLD.dmg_type3 || "
        "WDB.Damage4Min != WORLD.dmg_min4 || WDB.Damage4Max != WORLD.dmg_max4 || WDB.Damage4Type != WORLD.dmg_type4 || "
        "WDB.Damage5Min != WORLD.dmg_min5 || WDB.Damage5Max != WORLD.dmg_max5 || WDB.Damage5Type != WORLD.dmg_type5 || "*/

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
        const char* column1[6] = {"displayid","Quality","Flags","BuyPrice","SellPrice","InventoryType"};
        const char* column2[9] = {"ItemLevel","RequiredLevel","RequiredSkill","RequiredSkillRank","requiredspell",
            "requiredhonorrank","RequiredCityRank","RequiredReputationFaction","RequiredReputationRank"};
            //,"maxcount","stackable","ContainerSlots","StatsCount"};
        // int32
        const char* column3[22] = {"stat_type1","stat_value1","stat_type2","stat_value2","stat_type3","stat_value3",
            "stat_type4","stat_value4","stat_type5","stat_value5","stat_type6","stat_value6","stat_type7",
            "stat_value7","stat_type8","stat_value8","stat_type9","stat_value9","stat_type10","stat_value10",
            "ScalingStatDistribution","ScalingStatValue"};
        // uint32
        const char* column4[9] = {"armor","holy_res","fire_res","nature_res","frost_res","shadow_res",
            "arcane_res","delay","ammo_type"};
        const char* column5[17] = {"RandomProperty","RandomSuffix","block","itemset","MaxDurability",
            "area","Map","BagFamily","TotemCategory","socketColor_1","socketContent_1","socketColor_2",
            "socketContent_2","socketColor_3","socketContent_3","socketBonus","GemProperties"};

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
                // column1
                for (uint8 i=0; i<6; i++)
                {
                    if ((docolumn && ColumnExists(columns, column1[i])) || !docolumn)
                    {
                        if (fields[9+i*2].GetUInt32() != fields[10+i*2].GetUInt32())
                        {
                            if (first) updatesql.append("SET `").append(column1[i]).append("`='");
                            else updatesql.append("`").append(column1[i]).append("`='");
                            tmp = (char*)malloc(32);
                            sprintf(tmp, "%u", fields[9+i*2].GetUInt32());
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "AllowableClass")) || !docolumn)
                {
                    // AllowableClass
                    if (fields[21].GetInt32() != fields[22].GetInt32())
                    {
                        int32 tmpwdb = fields[21].GetInt32();
                        int32 tmpworld = fields[22].GetInt32();

                        // Flags > 3551 in -1 wandeln, weil der Wert zu groß ist für die Spalte, -1 paßt aber!
                        if (tmpwdb > 3551) tmpwdb = -1;
                        if (tmpwdb != tmpworld)
                        {
                            if (first) updatesql.append("SET `AllowableClass`='");
                            else updatesql.append("`AllowableClass`='");
                            tmp = (char*)malloc(32);
                            sprintf(tmp, "%i", tmpwdb);
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "AllowableRace")) || !docolumn)
                {
                    // AllowableRace
                    if (fields[23].GetInt32() != fields[24].GetInt32())
                    {
                        int32 tmpwdb = fields[23].GetInt32();
                        int32 tmpworld = fields[24].GetInt32();

                        // Flags > 3551 in -1 wandeln, weil der Wert zu groß ist für die Spalte, -1 paßt aber!
                        if (tmpwdb > 3551) tmpwdb = -1;
                        if (tmpwdb != tmpworld)
                        {
                            if (first) updatesql.append("SET `AllowableRace`='");
                            else updatesql.append("`AllowableRace`='");
                            tmp = (char*)malloc(32);
                            sprintf(tmp, "%i", tmpwdb);
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                // column2
                for (uint8 i=0; i<9; i++)
                {
                    if ((docolumn && ColumnExists(columns, column2[i])) || !docolumn)
                    {
                        if (fields[25+i*2].GetUInt32() != fields[26+i*2].GetUInt32())
                        {
                            uint32 tmpuint = fields[25+i*2].GetUInt32();

                            // Faction-"Korrektur" wegen core! :-( Das selbe wie bei den ModelIDs!
                            if ((25+i*2) == 41 && tmpuint > 0)
                                if (fields[39].GetUInt32() == 0) tmpuint = 0;

                            if (tmpuint != fields[26+i*2].GetUInt32())
                            {
                                if (first) updatesql.append("SET `").append(column2[i]).append("`='");
                                else updatesql.append("`").append(column2[i]).append("`='");
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
                        int32 tmpwdb = fields[43].GetInt32();
                        int32 tmpworld = fields[44].GetInt32();

                        // Wenn 2147483647 dann soll das für Blizz wohl unendlich heißen (z.b. Ehre und Arenap.)
                        if (tmpwdb > 32000) tmpwdb = 0;
                        if (tmpwdb != tmpworld)
                        {
                            if (first) updatesql.append("SET `maxcount`='");
                            else updatesql.append("`maxcount`='");
                            tmp = (char*)malloc(32);
                            sprintf(tmp, "%i", tmpwdb);
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "stackable")) || !docolumn)
                {
                    // stackable
                    if (fields[45].GetInt32() != fields[46].GetInt32())
                    {
                        int32 tmpwdb = fields[45].GetInt32();
                        int32 tmpworld = fields[46].GetInt32();

                        // Wenn 2147483647 dann soll das für Blizz wohl unendlich heißen (z.b. Ehre und Arenap.)
                        if (tmpwdb > 32000) tmpwdb = -1;
                        if (tmpwdb != tmpworld)
                        {
                            if (first) updatesql.append("SET `stackable`='");
                            else updatesql.append("`stackable`='");
                            tmp = (char*)malloc(32);
                            sprintf(tmp, "%i", tmpwdb);
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
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
                // column3
                for (uint8 i=0; i<22; i++)
                {
                    if ((docolumn && ColumnExists(columns, column3[i])) || !docolumn)
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

                                if (first) updatesql.append("SET `").append(column3[i]).append("`='");
                                else updatesql.append("`").append(column3[i]).append("`='");
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
                            if (first) updatesql.append("SET `").append(column3[i]).append("`='");
                            else updatesql.append("`").append(column3[i]).append("`='");
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
                // column4
                for (uint8 i=0; i<9; i++)
                {
                    if ((docolumn && ColumnExists(columns, column4[i])) || !docolumn)
                    {
                        if (fields[107+i*2].GetUInt32() != fields[108+i*2].GetUInt32())
                        {
                            if (first) updatesql.append("SET `").append(column4[i]).append("`='");
                            else updatesql.append("`").append(column4[i]).append("`='");
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
                if ((docolumn && ColumnExists(columns, "spellid_1")) || !docolumn)
                {
                    // spellid_1
                    if (fields[127].GetUInt32() != fields[128].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spellid_1`='");
                        else updatesql.append("`spellid_1`='");
                        tmp = (char*)malloc(32);
                        if (fields[127].GetUInt32() == 4294967295)
                            sprintf(tmp, "%u", 0);
                        else
                            sprintf(tmp, "%u", fields[127].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spelltrigger_1")) || !docolumn)
                {
                    // spelltrigger_1
                    if (fields[129].GetUInt32() != fields[130].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spelltrigger_1`='");
                        else updatesql.append("`spelltrigger_1`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[129].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcharges_1")) || !docolumn)
                {
                    // spellcharges_1
                    if (fields[131].GetInt32() != fields[132].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcharges_1`='");
                        else updatesql.append("`spellcharges_1`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[131].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcooldown_1")) || !docolumn)
                {
                    // spellcooldown_1
                    if (fields[133].GetInt32() != fields[134].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcooldown_1`='");
                        else updatesql.append("`spellcooldown_1`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[133].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcategory_1")) || !docolumn)
                {
                    // spellcategory_1
                    if (fields[135].GetUInt32() != fields[136].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spellcategory_1`='");
                        else updatesql.append("`spellcategory_1`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[135].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcategorycooldown_1")) || !docolumn)
                {
                    // spellcategorycooldown_1
                    if (fields[137].GetInt32() != fields[138].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcategorycooldown_1`='");
                        else updatesql.append("`spellcategorycooldown_1`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[137].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellid_2")) || !docolumn)
                {
                    // spellid_2
                    if (fields[139].GetUInt32() != fields[140].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spellid_2`='");
                        else updatesql.append("`spellid_2`='");
                        tmp = (char*)malloc(32);
                        if (fields[139].GetUInt32() == 4294967295)
                            sprintf(tmp, "%u", 0);
                        else
                            sprintf(tmp, "%u", fields[139].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spelltrigger_2")) || !docolumn)
                {
                    // spelltrigger_2
                    if (fields[141].GetUInt32() != fields[142].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spelltrigger_2`='");
                        else updatesql.append("`spelltrigger_2`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[141].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcharges_2")) || !docolumn)
                {
                    // spellcharges_2
                    if (fields[143].GetInt32() != fields[144].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcharges_2`='");
                        else updatesql.append("`spellcharges_2`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[143].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcooldown_2")) || !docolumn)
                {
                    // spellcooldown_2
                    if (fields[145].GetInt32() != fields[146].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcooldown_2`='");
                        else updatesql.append("`spellcooldown_2`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[145].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcategory_2")) || !docolumn)
                {
                    // spellcategory_2
                    if (fields[147].GetUInt32() != fields[148].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spellcategory_2`='");
                        else updatesql.append("`spellcategory_2`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[147].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcategorycooldown_2")) || !docolumn)
                {
                    // spellcategorycooldown_2
                    if (fields[149].GetInt32() != fields[150].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcategorycooldown_2`='");
                        else updatesql.append("`spellcategorycooldown_2`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[149].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellid_3")) || !docolumn)
                {
                    // spellid_3
                    if (fields[151].GetUInt32() != fields[152].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spellid_3`='");
                        else updatesql.append("`spellid_3`='");
                        tmp = (char*)malloc(32);
                        if (fields[151].GetUInt32() == 4294967295)
                            sprintf(tmp, "%u", 0);
                        else
                            sprintf(tmp, "%u", fields[151].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spelltrigger_3")) || !docolumn)
                {
                    // spelltrigger_3
                    if (fields[153].GetUInt32() != fields[154].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spelltrigger_3`='");
                        else updatesql.append("`spelltrigger_3`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[153].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcharges_3")) || !docolumn)
                {
                    // spellcharges_3
                    if (fields[155].GetInt32() != fields[156].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcharges_3`='");
                        else updatesql.append("`spellcharges_3`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[155].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcooldown_3")) || !docolumn)
                {
                    // spellcooldown_3
                    if (fields[157].GetInt32() != fields[158].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcooldown_3`='");
                        else updatesql.append("`spellcooldown_3`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[157].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcategory_3")) || !docolumn)
                {
                    // spellcategory_3
                    if (fields[159].GetUInt32() != fields[160].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spellcategory_3`='");
                        else updatesql.append("`spellcategory_3`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[159].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcategorycooldown_3")) || !docolumn)
                {
                    // spellcategorycooldown_3
                    if (fields[161].GetInt32() != fields[162].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcategorycooldown_3`='");
                        else updatesql.append("`spellcategorycooldown_3`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[161].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellid_4")) || !docolumn)
                {
                    // spellid_4
                    if (fields[163].GetUInt32() != fields[164].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spellid_4`='");
                        else updatesql.append("`spellid_4`='");
                        tmp = (char*)malloc(32);
                        if (fields[163].GetUInt32() == 4294967295)
                            sprintf(tmp, "%u", 0);
                        else
                            sprintf(tmp, "%u", fields[163].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spelltrigger_4")) || !docolumn)
                {
                    // spelltrigger_4
                    if (fields[165].GetUInt32() != fields[166].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spelltrigger_4`='");
                        else updatesql.append("`spelltrigger_4`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[165].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcharges_4")) || !docolumn)
                {
                    // spellcharges_4
                    if (fields[167].GetInt32() != fields[168].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcharges_4`='");
                        else updatesql.append("`spellcharges_4`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[167].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcooldown_4")) || !docolumn)
                {
                    // spellcooldown_4
                    if (fields[169].GetInt32() != fields[170].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcooldown_4`='");
                        else updatesql.append("`spellcooldown_4`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[169].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcategory_4")) || !docolumn)
                {
                    // spellcategory_4
                    if (fields[171].GetUInt32() != fields[172].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spellcategory_4`='");
                        else updatesql.append("`spellcategory_4`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[171].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcategorycooldown_4")) || !docolumn)
                {
                    // spellcategorycooldown_4
                    if (fields[173].GetInt32() != fields[174].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcategorycooldown_4`='");
                        else updatesql.append("`spellcategorycooldown_4`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[173].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellid_5")) || !docolumn)
                {
                    // spellid_5
                    if (fields[175].GetUInt32() != fields[176].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spellid_5`='");
                        else updatesql.append("`spellid_5`='");
                        tmp = (char*)malloc(32);
                        if (fields[175].GetUInt32() == 4294967295)
                            sprintf(tmp, "%u", 0);
                        else
                            sprintf(tmp, "%u", fields[175].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spelltrigger_5")) || !docolumn)
                {
                    // spelltrigger_5
                    if (fields[177].GetUInt32() != fields[178].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spelltrigger_5`='");
                        else updatesql.append("`spelltrigger_5`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[177].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcharges_5")) || !docolumn)
                {
                    // spellcharges_5
                    if (fields[179].GetInt32() != fields[180].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcharges_5`='");
                        else updatesql.append("`spellcharges_5`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[179].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcooldown_5")) || !docolumn)
                {
                    // spellcooldown_5
                    if (fields[181].GetInt32() != fields[182].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcooldown_5`='");
                        else updatesql.append("`spellcooldown_5`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[181].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcategory_5")) || !docolumn)
                {
                    // spellcategory_5
                    if (fields[183].GetUInt32() != fields[184].GetUInt32())
                    {
                        if (first) updatesql.append("SET `spellcategory_5`='");
                        else updatesql.append("`spellcategory_5`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%u", fields[183].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "spellcategorycooldown_5")) || !docolumn)
                {
                    // spellcategorycooldown_5
                    if (fields[185].GetInt32() != fields[186].GetInt32())
                    {
                        if (first) updatesql.append("SET `spellcategorycooldown_5`='");
                        else updatesql.append("`spellcategorycooldown_5`='");
                        tmp = (char*)malloc(32);
                        sprintf(tmp, "%i", fields[185].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
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
                // column5
                for (uint8 i=0; i<17; i++)
                {
                    if ((docolumn && ColumnExists(columns, column5[i])) || !docolumn)
                    {
                        uint32 tmpuint = fields[205+i*2].GetUInt32();
                        // Randomproperty mit dem Wert 4294967295
                        if (i == 0 && tmpuint == 4294967295) tmpuint = 0;

                        if (tmpuint != fields[206+i*2].GetUInt32())
                        {
                            if (first) updatesql.append("SET `").append(column5[i]).append("`='");
                            else updatesql.append("`").append(column5[i]).append("`='");
                            tmp = (char*)malloc(32);
                            sprintf(tmp, "%u", tmpuint);
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
                updatesql.append(" WHERE `entry`='").append(tmp).append("' LIMIT 1;\n");
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
                updatesql.append(" WHERE `ID`='").append(tmp).append("' LIMIT 1;\n");
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
                updatesql.append(" WHERE `entry`='").append(tmp).append("' LIMIT 1;\n");
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

    //                         0
    query.append("SELECT WDB.entry,WDB.Method,WORLD.Method,WDB.QuestLevel,WORLD.QuestLevel,WDB.ZoneOrSort,"
        "WORLD.ZoneOrSort,WDB.Type,WORLD.Type,WDB.SuggestedPlayers,WORLD.SuggestedPlayers,WDB.FactionID,"
        "WORLD.RepObjectiveFaction,WDB.FactionAmount,WORLD.RepObjectiveValue,WDB.NextQuestID,"
        "WORLD.NextQuestInChain,WDB.CoinReward,WORLD.RewOrReqMoney,WDB.CoinRewardOn80,WORLD.RewMoneyMaxLevel,"
        "WDB.SpellReward,WORLD.RewSpell,WDB.EffectOnPlayer,WORLD.RewSpellCast,WDB.StartItemID,WORLD.SrcItemId,"
        "WDB.QuestFlags,WORLD.QuestFlags,"
        //       29
        "WDB.ItemReward1,WORLD.RewItemId1,WDB.ItemAmount1,WORLD.RewItemCount1,"
        "WDB.ItemReward2,WORLD.RewItemId2,WDB.ItemAmount2,WORLD.RewItemCount2,"
        "WDB.ItemReward3,WORLD.RewItemId3,WDB.ItemAmount3,WORLD.RewItemCount3,"
        "WDB.ItemReward4,WORLD.RewItemId4,WDB.ItemAmount4,WORLD.RewItemCount4,"
        //       45
        "WDB.ItemChoice1,WORLD.RewChoiceItemId1,WDB.ItemChoiceAmount1,WORLD.RewChoiceItemCount1,"
        "WDB.ItemChoice2,WORLD.RewChoiceItemId2,WDB.ItemChoiceAmount2,WORLD.RewChoiceItemCount2,"
        "WDB.ItemChoice3,WORLD.RewChoiceItemId3,WDB.ItemChoiceAmount3,WORLD.RewChoiceItemCount3,"
        "WDB.ItemChoice4,WORLD.RewChoiceItemId4,WDB.ItemChoiceAmount4,WORLD.RewChoiceItemCount4,"
        "WDB.ItemChoice5,WORLD.RewChoiceItemId5,WDB.ItemChoiceAmount5,WORLD.RewChoiceItemCount5,"
        "WDB.ItemChoice6,WORLD.RewChoiceItemId6,WDB.ItemChoiceAmount6,WORLD.RewChoiceItemCount6,"
        //       69
        "WDB.PointMapId,WORLD.PointMapId,WDB.PointX,WORLD.PointX,WDB.PointY,WORLD.PointY,WDB.PointOpt,"
        "WORLD.PointOpt,WDB.Name,WORLD.Title,WDB.Description,WORLD.Objectives,WDB.Details,WORLD.Details,"
        "WDB.Subdescription,WORLD.EndText,"
        //       85
        "WDB.KillCreature1,WORLD.ReqCreatureOrGOId1,WDB.KillCreature1Amount,WORLD.ReqCreatureOrGOCount1,"
        "WDB.CollectItem1,WORLD.ReqItemId1,WDB.CollectItem1Amount,WORLD.ReqItemCount1,WDB.ItemUsed1,"
        "WORLD.ReqSourceId1,WDB.KillCreature2,WORLD.ReqCreatureOrGOId2,WDB.KillCreature2Amount,"
        "WORLD.ReqCreatureOrGOCount2,WDB.CollectItem2,WORLD.ReqItemId2,WDB.CollectItem2Amount,WORLD.ReqItemCount2,"
        "WDB.ItemUsed2,WORLD.ReqSourceId2,WDB.KillCreature3,WORLD.ReqCreatureOrGOId3,WDB.KillCreature3Amount,"
        "WORLD.ReqCreatureOrGOCount3,WDB.CollectItem3,WORLD.ReqItemId3,WDB.CollectItem3Amount,WORLD.ReqItemCount3,"
        "WDB.ItemUsed3,WORLD.ReqSourceId3,WDB.KillCreature4,WORLD.ReqCreatureOrGOId4,WDB.KillCreature4Amount,"
        "WORLD.ReqCreatureOrGOCount4,WDB.CollectItem4,WORLD.ReqItemId4,WDB.CollectItem4Amount,WORLD.ReqItemCount4,"
        "WDB.ItemUsed4,WORLD.ReqSourceId4,"
        //      125
        "WDB.Objective1,WORLD.ObjectiveText1,WDB.Objective2,WORLD.ObjectiveText2,"
        "WDB.Objective3,WORLD.ObjectiveText3,WDB.Objective4,WORLD.ObjectiveText4 FROM `");
    query.append(wdbdb).append("`.`questcache` AS WDB, `");

    query.append(worlddb).append("`.`quest_template` AS WORLD WHERE WDB.entry = WORLD.entry AND "
        "(WDB.Method != WORLD.Method || WDB.QuestLevel != WORLD.QuestLevel || WDB.ZoneOrSort != WORLD.ZoneOrSort || "
        "WDB.Type != WORLD.Type || WDB.SuggestedPlayers != WORLD.SuggestedPlayers || "
        "WDB.FactionID != WORLD.RepObjectiveFaction || WDB.FactionAmount != WORLD.RepObjectiveValue || "
        "WDB.NextQuestID != WORLD.NextQuestInChain || WDB.CoinReward != WORLD.RewOrReqMoney || "
        "WDB.CoinRewardOn80 != WORLD.RewMoneyMaxLevel || WDB.SpellReward != WORLD.RewSpell || "
        "WDB.EffectOnPlayer != WORLD.RewSpellCast || WDB.StartItemID != WORLD.SrcItemId || "
        "WDB.QuestFlags != WORLD.QuestFlags || WDB.ItemReward1 != WORLD.RewItemId1 || "
        "WDB.ItemAmount1 != WORLD.RewItemCount1 || WDB.ItemReward2 != WORLD.RewItemId2 || "
        "WDB.ItemAmount2 != WORLD.RewItemCount2 || WDB.ItemReward3 != WORLD.RewItemId3 || "
        "WDB.ItemAmount3 != WORLD.RewItemCount3 || WDB.ItemReward4 != WORLD.RewItemId4 || "
        "WDB.ItemAmount4 != WORLD.RewItemCount4 || WDB.ItemChoice1 != WORLD.RewChoiceItemId1 || "
        "WDB.ItemChoiceAmount1 != WORLD.RewChoiceItemCount1 || WDB.ItemChoice2 != WORLD.RewChoiceItemId2 || "
        "WDB.ItemChoiceAmount2 != WORLD.RewChoiceItemCount2 || WDB.ItemChoice3 != WORLD.RewChoiceItemId3 || "
        "WDB.ItemChoiceAmount3 != WORLD.RewChoiceItemCount3 || WDB.ItemChoice4 != WORLD.RewChoiceItemId4 || "
        "WDB.ItemChoiceAmount4 != WORLD.RewChoiceItemCount4 || WDB.ItemChoice5 != WORLD.RewChoiceItemId5 || "
        "WDB.ItemChoiceAmount5 != WORLD.RewChoiceItemCount5 || WDB.ItemChoice6 != WORLD.RewChoiceItemId6 || "
        "WDB.ItemChoiceAmount6 != WORLD.RewChoiceItemCount6 || WDB.PointMapId != WORLD.PointMapId || "
        "WDB.PointX != WORLD.PointX || WDB.PointY != WORLD.PointY || WDB.PointOpt != WORLD.PointOpt || "
        "WDB.Name != WORLD.Title || WDB.Description != WORLD.Objectives || WDB.Details != WORLD.Details || "
        "WDB.Subdescription != WORLD.EndText || WDB.KillCreature1 != WORLD.ReqCreatureOrGOId1 || "
        "WDB.KillCreature1Amount != WORLD.ReqCreatureOrGOCount1 || WDB.CollectItem1 != WORLD.ReqItemId1 || "
        "WDB.CollectItem1Amount != WORLD.ReqItemCount1 || WDB.ItemUsed1 != WORLD.ReqSourceId1 || "
        "WDB.KillCreature2 != WORLD.ReqCreatureOrGOId2 || WDB.KillCreature2Amount != WORLD.ReqCreatureOrGOCount2 || "
        "WDB.CollectItem2 != WORLD.ReqItemId2 || WDB.CollectItem2Amount != WORLD.ReqItemCount2 || "
        "WDB.ItemUsed2 != WORLD.ReqSourceId2 || WDB.KillCreature3 != WORLD.ReqCreatureOrGOId3 || "
        "WDB.KillCreature3Amount != WORLD.ReqCreatureOrGOCount3 || WDB.CollectItem3 != WORLD.ReqItemId3 || "
        "WDB.CollectItem3Amount != WORLD.ReqItemCount3 || WDB.ItemUsed3 != WORLD.ReqSourceId3 || "
        "WDB.KillCreature4 != WORLD.ReqCreatureOrGOId4 || WDB.KillCreature4Amount != WORLD.ReqCreatureOrGOCount4 || "
        "WDB.CollectItem4 != WORLD.ReqItemId4 || WDB.CollectItem4Amount != WORLD.ReqItemCount4 || "
        "WDB.ItemUsed4 != WORLD.ReqSourceId4 || WDB.Objective1 != WORLD.ObjectiveText1 || "
        "WDB.Objective2 != WORLD.ObjectiveText2 || WDB.Objective3 != WORLD.ObjectiveText3 || "
        "WDB.Objective4 != WORLD.ObjectiveText4)");

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

        // uint32
        const char* column1[5] = {"Type","SuggestedPlayers","RepObjectiveFaction","RepObjectiveValue","NextQuestInChain"};
        const char* column2[26] = {"RewMoneyMaxLevel","RewSpell","RewSpellCast","SrcItemId","QuestFlags",
            "RewItemId1","RewItemCount1","RewItemId2","RewItemCount2","RewItemId3","RewItemCount3","RewItemId4",
            "RewItemCount4","RewChoiceItemId1","RewChoiceItemCount1","RewChoiceItemId2","RewChoiceItemCount2",
            "RewChoiceItemId3","RewChoiceItemCount3","RewChoiceItemId4","RewChoiceItemCount4","RewChoiceItemId5",
            "RewChoiceItemCount5","RewChoiceItemId6","RewChoiceItemCount6","PointMapId"};
        // string
        const char* column3[4] = {"Title","Objectives","Details","EndText"};
        // int32
        const char* column4[20] = {"ReqCreatureOrGOId1","ReqCreatureOrGOCount1","ReqItemId1","ReqItemCount1",
            "ReqSourceId1","ReqCreatureOrGOId2","ReqCreatureOrGOCount2","ReqItemId2","ReqItemCount2",
            "ReqSourceId2","ReqCreatureOrGOId3","ReqCreatureOrGOCount3","ReqItemId3","ReqItemCount3",
            "ReqSourceId3","ReqCreatureOrGOId4","ReqCreatureOrGOCount4","ReqItemId4","ReqItemCount4",
            "ReqSourceId4"};
        // string
        const char* column5[4] = {"ObjectiveText1","ObjectiveText2","ObjectiveText3","ObjectiveText4"};

        do
        {
            bool first = true;
            fields = result->Fetch();

            if (fields)
            {
                char* tmp;

                if ((docolumn && ColumnExists(columns, "Method")) || !docolumn)
                {
                    // Method
                    if (fields[1].GetUInt32() != fields[2].GetUInt32())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `Method`='");
                        else updatesql.append("`Method`='");
                        sprintf(tmp, "%u", fields[1].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "QuestLevel")) || !docolumn)
                {
                    // QuestLevel
                    if (fields[3].GetInt32() != fields[4].GetInt32())
                    {
                        if (fields[3].GetInt32() != fields[4].GetInt32())
                        {
                            tmp = (char*)malloc(32);
                            if (first) updatesql.append("SET `QuestLevel`='");
                            else updatesql.append("`QuestLevel`='");
                            sprintf(tmp, "%i", fields[3].GetInt32());
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "ZoneOrSort")) || !docolumn)
                {
                    // ZoneOrSort
                    if (fields[5].GetInt32() != fields[6].GetInt32())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `ZoneOrSort`='");
                        else updatesql.append("`ZoneOrSort`='");
                        sprintf(tmp, "%i", fields[5].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // column1
                for (uint8 i=0; i<5; i++)
                {
                    if ((docolumn && ColumnExists(columns, column1[i])) || !docolumn)
                    {
                        if (fields[7+i*2].GetUInt32() != fields[8+i*2].GetUInt32())
                        {
                            tmp = (char*)malloc(32);
                            if (first) updatesql.append("SET `").append(column1[i]).append("`='");
                            else updatesql.append("`").append(column1[i]).append("`='");
                            sprintf(tmp, "%u", fields[7+i*2].GetUInt32());
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "RewOrReqMoney")) || !docolumn)
                {
                    // RewOrReqMoney
                    if (fields[17].GetInt32() != fields[18].GetInt32())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `RewOrReqMoney`='");
                        else updatesql.append("`RewOrReqMoney`='");
                        sprintf(tmp, "%i", fields[17].GetInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // column2
                for (uint8 i=0; i<26; i++)
                {
                    if ((docolumn && ColumnExists(columns, column2[i])) || !docolumn)
                    {
                        if (fields[19+i*2].GetUInt32() != fields[20+i*2].GetUInt32())
                        {
                            // Needed for "WDB.EffectOnPlayer,WORLD.RewSpellCast"
                            // Because Core doesn'T know(?) that this must be an int32
                            uint32 uitmp = fields[19+i*2].GetUInt32();
                            if (uitmp == 4294967295) uitmp = 0;

                            tmp = (char*)malloc(32);
                            if (first) updatesql.append("SET `").append(column2[i]).append("`='");
                            else updatesql.append("`").append(column2[i]).append("`='");
                            sprintf(tmp, "%u", uitmp);
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                if ((docolumn && ColumnExists(columns, "PointX")) || !docolumn)
                {
                    // PointX
                    if (fields[71].GetFloat() != fields[72].GetFloat())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `PointX`='");
                        else updatesql.append("`PointX`='");
                        sprintf(tmp, "%f", fields[71].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "PointY")) || !docolumn)
                {
                    // PointY
                    if (fields[73].GetFloat() != fields[74].GetFloat())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `PointY`='");
                        else updatesql.append("`PointY`='");
                        sprintf(tmp, "%f", fields[73].GetFloat());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                if ((docolumn && ColumnExists(columns, "PointOpt")) || !docolumn)
                {
                    // PointOpt
                    if (fields[75].GetUInt32() != fields[76].GetUInt32())
                    {
                        tmp = (char*)malloc(32);
                        if (first) updatesql.append("SET `PointOpt`='");
                        else updatesql.append("`PointOpt`='");
                        sprintf(tmp, "%u", fields[75].GetUInt32());
                        updatesql.append(tmp).append("',");
                        free(tmp);
                        first = false;
                    }
                }
                // column3
                for (uint8 i=0; i<4; i++)
                {
                    if ((docolumn && ColumnExists(columns, column3[i])) || !docolumn)
                    {
                        if (strcmp(fields[77+i*2].GetCppString().c_str(), fields[78+i*2].GetCppString().c_str()) != 0)
                        {
                            if (first) updatesql.append("SET `").append(column3[i]).append("`='").append(io.Terminator(fields[77+i*2].GetCppString())).append("',");
                            else updatesql.append("`").append(column3[i]).append("`='").append(io.Terminator(fields[77+i*2].GetCppString())).append("',");
                            first = false;
                        }
                    }
                }
                // column4
                for (uint8 i=0; i<20; i++)
                {
                    if ((docolumn && ColumnExists(columns, column4[i])) || !docolumn)
                    {
                        int32 firsti = fields[85+i*2].GetInt32();
                        int32 second = fields[86+i*2].GetInt32();

                        // GOs in KillCreature/ReqCreatureOrGOId für core umrechnen
                        if ((85+i*2 == 85 && firsti < 0) || (85+i*2 == 95 && firsti < 0) ||
                            (85+i*2 == 105 && firsti < 0) || (85+i*2 == 115 && firsti < 0))
                            firsti = (firsti + 2147483648)*-1;

                        if (firsti != second)
                        {
                            tmp = (char*)malloc(32);
                            if (first) updatesql.append("SET `").append(column4[i]).append("`='");
                            else updatesql.append("`").append(column4[i]).append("`='");
                            sprintf(tmp, "%i", firsti);
                            updatesql.append(tmp).append("',");
                            free(tmp);
                            first = false;
                        }
                    }
                }
                /* There are never data in the wdb if the quest was completed on the offi!
                   Because of this we should never make updates to this columns!
                // column5
                for (uint8 i=0; i<4; i++)
                {
                    if ((docolumn && ColumnExists(columns, column5[i])) || !docolumn)
                    {
                        if (strcmp(fields[125+i*2].GetCppString().c_str(), fields[126+i*2].GetCppString().c_str()) != 0)
                        {
                            if (first) updatesql.append("SET `").append(column5[i]).append("`='").append(io.Terminator(fields[125+i*2].GetCppString())).append("',");
                            else updatesql.append("`").append(column5[i]).append("`='").append(io.Terminator(fields[125+i*2].GetCppString())).append("',");
                            first = false;
                        }
                    }
                }*/
            }
            if (!first)
            {
                count++;
                updatesql.resize(updatesql.length()-1);
                char* tmp = (char*)malloc(32);
                sprintf(tmp, "%u", fields[0].GetUInt32());
                updatesql.append(" WHERE `entry`='").append(tmp).append("' LIMIT 1;\n");
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
            updatesql.append("\n# Differences in ").append(tmp).append(" entries found.\n\n"

                "# ATTENTION: We have to move the original data to the first field because of the core limitations.\n"
                "#            If the client find a zero it stops showing the left rewards. Because of these\n"
                "#            movements you'll always get updates for these values until the core fix this problem\n"
                "#            at the creation time of the corresponding packet!\n"
                "UPDATE `quest_template` SET RewItemId1=RewItemId2,RewItemId2=RewItemId3,RewItemId3=RewItemId4,RewItemId4=0,RewItemCount1=RewItemCount2,RewItemCount2=RewItemCount3,RewItemCount3=RewItemCount4,RewItemCount4=0 WHERE RewItemId1=0 AND RewItemId2!=0;\n"
                "UPDATE `quest_template` SET RewChoiceItemId1=RewChoiceItemId2,RewChoiceItemId2=RewChoiceItemId3,RewChoiceItemId3=RewChoiceItemId4,RewChoiceItemId4=RewChoiceItemId5,RewChoiceItemId5=RewChoiceItemId6,RewChoiceItemId6=0,RewChoiceItemCount1=RewChoiceItemCount2,RewChoiceItemCount2=RewChoiceItemCount3,RewChoiceItemCount3=RewChoiceItemCount4,RewChoiceItemCount4=RewChoiceItemCount5,RewChoiceItemCount5=RewChoiceItemCount6,RewChoiceItemCount6=0 WHERE RewChoiceItemId1=0 AND RewChoiceItemId2!=0;\n\n");

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

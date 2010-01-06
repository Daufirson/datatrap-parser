#include "WDBFile.h"

WDBFile::WDBFile(void)
{
}

WDBFile::~WDBFile(void)
{
	gDefList.empty();
}

bool WDBFile::loadWDB(const char* file)
{
    ifstream fin(file, ios_base::in | ios_base::binary);
	bool isMatch = false;
	if (!fin.is_open())	return false;

	uint32 tlength;

	fin.read((char*)&m_header, sizeof(wdbheader_t));

	// sometimes the number of fields can be deceiving!
	if (m_header.fields < ((WDB_MAX_FIELDS-2)*4))
    {
		m_header.fields /= 4;
		m_header.fields += 2;
	}

	// Set definition based on signature
	for (uint32 i=0; i<gDefList.size(); i++)
    {
		if (!memcmp(&m_header.signature, gDefList.at(i).getSignature(), 4))
        {
			isMatch = true;
            printf("Working on '%s'", gDefList.at(i).getWDBName());
			m_def = &gDefList.at(i);

			// if fieldsize is incorrect, adjust accordingly
			if (m_header.fields < ((WDB_MAX_FIELDS-2)*4))
            {
				if (m_def->getNumFields() != m_header.fields)
                {
					printf("\n\nWARNING: Number of fields does not match defination!\n"
                        "WARNING: Temporarily adjusting fieldsize, it's another WDB version!\n\n"
                        "WARNING: Please use WDB files of client version %s!\n"
                        "WARNING: If you have a higher client version\n"
                        "WARNING: contact the author to update me, thanks! ;-)\n\n"
                        "INFO: Field difference: %i\n",
                        _CLIENT_VERSION, m_header.fields - m_def->getNumFields());

					int32 nByteSize = 0;
					int32 nStrCount = 0;

					for (uint32 i=0; i<m_def->getNumFields(); i++)
					{
                        switch (m_def->getFieldType(i))
                        {
                            case t_string: nStrCount++; break;
                            case t_bitmask32:
                            case t_uint32:
                            case t_int32:
                            case t_float: nByteSize += sizeof(uint32); break;
                            case t_uint16:
                            case t_int16:
                            case t_bitmask16: nByteSize += sizeof(uint16); break;
                            case t_uint8:
                            case t_int8: nByteSize += sizeof(uint8); break;
                        }
					}
                    printf("INFO: Field %d Byte + %d Strings.\n",nByteSize, nStrCount);
					// adjust fieldsize
					m_def->setNumFields(m_header.fields);
				}
			}
		}
	}

	if (!isMatch)
    {
        printf("WARNING: Header of '%s' does not match any known WDB file!\n", getWDBName());
		return false;
	}

	fin.seekg(0,ios_base::end);
	tlength = fin.tellg();
	tlength -= sizeof(wdbheader_t);

	m_data = new char[tlength];
	m_datasize = tlength;

	fin.seekg(sizeof(wdbheader_t));
	fin.read(m_data,tlength);

	fin.close();

	return true;
}

void WDBFile::LoadDefinitions(uint8 wdbfile)
{
	WDBDef* def = new WDBDef;

    switch(wdbfile)
    {
        case CREATURECACHE:
        	def->setWDBName(WDB_FILE_CREATURE);
	        {
		        def->setSignature(CREATURE_SIG);

		        // 29 fields (+2 @3.2.0)
		        def->addField(t_uint32, WDB_ENTRY_FIELD);
		        def->addField(t_uint32, WDB_SIZEOFENTRY_FIELD);
		        def->addField(t_string, "name");
		        def->addField(t_string, "name2");
		        def->addField(t_string, "name3");
		        def->addField(t_string, "name4");
		        def->addField(t_string, "subname");
		        def->addField(t_string, "IconName");
		        def->addField(t_uint16, "type_flags");
		        def->addField(t_uint16, "unit_flags");
		        def->addField(t_uint32, "type");
		        def->addField(t_uint32, "family");
		        def->addField(t_uint32, "rank");
		        def->addField(t_undef,  "");// WEDER PETSPELLDATA NOCH KILLCREDIT! KA WO MANGOS 2x KILLCREDIT + PETSPELLDATA HERHOLT! :-(
		        def->addField(t_undef,  "");// WEDER PETSPELLDATA NOCH KILLCREDIT! KA WO MANGOS 2x KILLCREDIT + PETSPELLDATA HERHOLT! :-(
		        def->addField(t_uint32, "modelid1");
		        def->addField(t_uint32, "modelid2");
		        def->addField(t_uint32, "modelid3");
		        def->addField(t_uint32, "modelid4");
		        def->addField(t_float,  "unk16");
		        def->addField(t_float,  "unk17");
 		        def->addField(t_uint8,  "RacialLeader");

		        def->addField(t_uint32, "questItem1"); 
		        def->addField(t_uint32, "questItem2");
		        def->addField(t_uint32, "questItem3");
		        def->addField(t_uint32, "questItem4");
                // @ 3.2.0 added
		        def->addField(t_uint32, "questItem5");
		        def->addField(t_uint32, "questItem6");

		        def->addField(t_uint32, "movementId");

		        gDefList.push_back(*def);
	        }
            break;
        case GAMEOBJECTCACHE:
	        def->setWDBName(WDB_FILE_GAMEOBJECT);
	        {
		        def->setSignature(GAMEOBJECT_SIG);

		        // 39 fields (+2 @3.2.0)
		        def->addField(t_uint32, WDB_ENTRY_FIELD);
		        def->addField(t_uint32, WDB_SIZEOFENTRY_FIELD);
		        def->addField(t_uint32, "type");
		        def->addField(t_uint32, "displayId");
		        def->addField(t_string, "Name1");
		        def->addField(t_string, "Name2");
		        def->addField(t_string, "Name3");
		        def->addField(t_string, "Name4");
		        def->addField(t_string, "IconName");
		        def->addField(t_string, "castBarCaption");
		        def->addField(t_string, "unk1");

		        def->addField(t_int32, "data0");
		        def->addField(t_int32, "data1");
		        def->addField(t_int32, "data2");
		        def->addField(t_int32, "data3");
		        def->addField(t_int32, "data4");
		        def->addField(t_int32, "data5");
		        def->addField(t_int32, "data6");
		        def->addField(t_int32, "data7");
		        def->addField(t_int32, "data8");
		        def->addField(t_int32, "data9");
		        def->addField(t_int32, "data10");
		        def->addField(t_int32, "data11");
		        def->addField(t_int32, "data12");
		        def->addField(t_int32, "data13");
		        def->addField(t_int32, "data14");
		        def->addField(t_int32, "data15");
		        def->addField(t_int32, "data16");
		        def->addField(t_int32, "data17");
		        def->addField(t_int32, "data18");
		        def->addField(t_int32, "data19");
		        def->addField(t_int32, "data20");
		        def->addField(t_int32, "data21");
		        def->addField(t_int32, "data22");
		        def->addField(t_int32, "data23");

                def->addField(t_float,  "size");

		        def->addField(t_uint32, "questItem1");
		        def->addField(t_uint32, "questItem2");
		        def->addField(t_uint32, "questItem3");
		        def->addField(t_uint32, "questItem4");
                // @ 3.2.0 added
		        def->addField(t_uint32, "questItem5");
		        def->addField(t_uint32, "questItem6");

                gDefList.push_back(*def);
	        }
            break;
        case ITEMCACHE:
	        def->setWDBName(WDB_FILE_ITEM);
	        {
		        def->setSignature(ITEM_SIG);

                // 141 fields
		        def->addField(t_uint32, WDB_ENTRY_FIELD);       // 0
		        def->addField(t_uint32, WDB_SIZEOFENTRY_FIELD);
		        def->addField(t_uint32, "Class");
		        def->addField(t_uint32, "SubClass1");
                def->addField(t_int32,  "unk0");

		        def->addField(t_string, "Name1");               // 5
		        def->addField(t_string, "Name2");
		        def->addField(t_string, "Name3");
		        def->addField(t_string, "Name4");
		        def->addField(t_uint32, "ItemDisplayID");
		        def->addField(t_uint32, "Quality");             // 10
		        def->addField(t_int32,  "Flags");
		        def->addField(t_int32,  "Flags2");              // added 3.2.0 (Flags2? / PvP bezogen / bisher 1 oder 2)

                def->addField(t_int32,  "BuyPrice");            
		        def->addField(t_uint32, "SellPrice");
		        def->addField(t_uint32, "InventorySlot");       // 15
 		        def->addField(t_int32,  "AllowableClass");
		        def->addField(t_int32,  "AllowableRace");
 		        def->addField(t_uint32, "ItemLevel");           
		        def->addField(t_uint32, "RequiredLevel");
		        def->addField(t_uint32, "RequiredSkill");       // 20
		        def->addField(t_uint32, "RequiredSkillRank");
		        def->addField(t_uint32, "RequiredSpell");
		        def->addField(t_uint32, "RequiredHonorRank");
		        def->addField(t_uint32, "RequiredCityRank");
		        def->addField(t_uint32, "RequiredReputationFaction");   // 25
		        def->addField(t_uint32, "RequiredReputationRank");
		        def->addField(t_int32,  "maxcount");
		        def->addField(t_int32,  "stackable");
		        def->addField(t_uint32, "ContainerSlots");

                def->addField(t_uint32, "StatsCount");              // 30 Counter für Anzahl Stats ==> var "max" in WDBFile.h

		        def->addField(t_int32,  "Stat1");
		        def->addField(t_int32,  "Stat1Val");
		        def->addField(t_int32,  "Stat2");
		        def->addField(t_int32,  "Stat2Val");                
		        def->addField(t_int32,  "Stat3");                   // 45
		        def->addField(t_int32,  "Stat3Val");
		        def->addField(t_int32,  "Stat4");
		        def->addField(t_int32,  "Stat4Val");
		        def->addField(t_int32,  "Stat5");                   
		        def->addField(t_int32,  "Stat5Val");                // 40
		        def->addField(t_int32,  "Stat6");
		        def->addField(t_int32,  "Stat6Val");
		        def->addField(t_int32,  "Stat7");
		        def->addField(t_int32,  "Stat7Val");                
		        def->addField(t_int32,  "Stat8");                   // 45
		        def->addField(t_int32,  "Stat8Val");
		        def->addField(t_int32,  "Stat9");
		        def->addField(t_int32,  "Stat9Val");
		        def->addField(t_int32,  "Stat10");                  
		        def->addField(t_int32,  "Stat10Val");               // 50

		        def->addField(t_float,  "Damage1Min");
		        def->addField(t_float,  "Damage1Max");
		        def->addField(t_uint32, "Damage1Type");
		        def->addField(t_float,  "Damage2Min");              
		        def->addField(t_float,  "Damage2Max");              // 55
		        def->addField(t_uint32, "Damage2Type");

		        def->addField(t_uint32, "Armor");
                def->addField(t_uint32, "HolyResist");
		        def->addField(t_uint32, "FireResist");              
		        def->addField(t_uint32, "NatureResist");            // 60
		        def->addField(t_uint32, "FrostResist");
		        def->addField(t_uint32, "ShadowResist");
		        def->addField(t_uint32, "ArcaneResist");

		        def->addField(t_uint32, "Speed");                   

		        def->addField(t_uint32, "AmmoType");                // 65
		        def->addField(t_float,  "RangedModRange");

                def->addField(t_int32,  "Spell1ID");
		        def->addField(t_int32,  "Spell1Trigger");
		        def->addField(t_int32,  "Spell1Charges");           
		        def->addField(t_int32,  "Spell1Cooldown");          // 70
		        def->addField(t_int32,  "Spell1Category");
		        def->addField(t_int32,  "Spell1CategoryCooldown");
		        def->addField(t_int32,  "Spell2ID");
		        def->addField(t_int32,  "Spell2Trigger");          
		        def->addField(t_int32,  "Spell2Charges");           // 75 
		        def->addField(t_int32,  "Spell2Cooldown");
		        def->addField(t_int32,  "Spell2Category");
		        def->addField(t_int32,  "Spell2CategoryCooldown");
		        def->addField(t_int32,  "Spell3ID");                
		        def->addField(t_int32,  "Spell3Trigger");           // 80
		        def->addField(t_int32,  "Spell3Charges");
		        def->addField(t_int32,  "Spell3Cooldown");
		        def->addField(t_int32,  "Spell3Category");
		        def->addField(t_int32,  "Spell3CategoryCooldown");  
		        def->addField(t_int32,  "Spell4ID");                // 85
		        def->addField(t_int32,  "Spell4Trigger");
		        def->addField(t_int32,  "Spell4Charges");
		        def->addField(t_int32,  "Spell4Cooldown");
		        def->addField(t_int32,  "Spell4Category");          
		        def->addField(t_int32,  "Spell4CategoryCooldown");  // 90
		        def->addField(t_int32,  "Spell5ID");
		        def->addField(t_int32,  "Spell5Trigger");
		        def->addField(t_int32,  "Spell5Charges");
		        def->addField(t_int32,  "Spell5Cooldown");          
		        def->addField(t_int32,  "Spell5Category");          // 95
		        def->addField(t_int32,  "Spell5CategoryCooldown");

		        def->addField(t_int32,  "Bonding");
		        def->addField(t_string, "Description");
		        def->addField(t_uint32, "BookTextID");              
		        def->addField(t_uint32, "LanguageID");              // 100
		        def->addField(t_int32,  "PageMaterial");
                def->addField(t_uint32, "BeginQuestID");
		        def->addField(t_uint32, "LockID");

		        def->addField(t_int32,  "Material");                
		        def->addField(t_int32,  "Sheath");                  // 115
		        def->addField(t_int32,  "RandomProperty");
		        def->addField(t_uint32, "RandomSuffix");
		        def->addField(t_uint32, "BlockValue");
		        def->addField(t_uint32, "ItemSetID");               
		        def->addField(t_uint32, "Durability");              // 120
		        def->addField(t_uint32, "AreaID");
		        def->addField(t_uint32, "ItemMapID");
		        def->addField(t_uint32, "BagFamily");
		        def->addField(t_uint32, "TotemCategory");           
		        def->addField(t_uint32, "SocketColor1");            // 125
		        def->addField(t_uint32, "SocketContent1");
		        def->addField(t_uint32, "SocketColor2");
		        def->addField(t_uint32, "SocketContent2");
		        def->addField(t_uint32, "SocketColor3");            
		        def->addField(t_uint32, "SocketContent3");          // 130
		        def->addField(t_uint32, "socketBonus");
		        def->addField(t_uint32, "GemProperties");
		        def->addField(t_int32,  "requiredDisenchantSkill");
		        def->addField(t_float,  "armorDamageModifier");     
		        def->addField(t_int32,  "Duration");                // 135
		        def->addField(t_uint32, "ItemLimitCategory");

                def->addField(t_uint32, "HolidayId");

                def->addField(t_uint32, "nextentryincache");
                def->addField(t_uint32, "nextentrysize");           
                def->addField(t_uint32, "nextclassincache");        // 140

                gDefList.push_back(*def);
	        }
            break;
        case ITEMNAMECACHE:
	        def->setWDBName(WDB_FILE_ITEM_NAME);
	        {
		        def->setSignature(ITEMNAME_SIG);

		        // 4 fields
		        def->addField(t_uint32, WDB_ENTRY_FIELD);
		        def->addField(t_uint32, WDB_SIZEOFENTRY_FIELD);
		        def->addField(t_string, "ItemName");
		        def->addField(t_uint16, "Typ");
 
		        gDefList.push_back(*def);
	        }
            break;
        case ITEMTEXTCACHE:
            def->setWDBName(WDB_FILE_ITEM_TEXT);
	        {
		        def->setSignature(ITEMTEXT_SIG);
 
		        // 3 fields
		        def->addField(t_uint32, WDB_ENTRY_FIELD);
		        def->addField(t_uint32, WDB_SIZEOFENTRY_FIELD);
		        def->addField(t_string, "Text");
 
		        gDefList.push_back(*def);
	        }
            break;
        case NPCCACHE:
	        def->setWDBName(WDB_FILE_NPC);
	        {
		        def->setSignature(NPC_SIG);

		        // 82 fields
		        def->addField(t_uint32, WDB_ENTRY_FIELD);
		        def->addField(t_uint32, WDB_SIZEOFENTRY_FIELD);
                char col_name[10];
		        for (uint8 i=0; i<8; ++i)
		        {
                    sprintf_s(col_name, 8, "prob%i", i);    def->addField(t_float, col_name);
                    sprintf_s(col_name, 8, "text%i_0", i);  def->addField(t_string, col_name);
                    sprintf_s(col_name, 8, "text%i_1", i);  def->addField(t_string, col_name);
                    sprintf_s(col_name, 8, "lang%i", i);    def->addField(t_uint32, col_name);
                    /*sprintf_s(col_name, 9, "unknown%i", i);*/ def->addField(t_uint32, "");
                    sprintf_s(col_name, 8, "em%i_0", i);    def->addField(t_uint32, col_name);
                    sprintf_s(col_name, 8, "em%i_1", i);    def->addField(t_uint32, col_name);
                    sprintf_s(col_name, 8, "em%i_2", i);    def->addField(t_uint32, col_name);
                    sprintf_s(col_name, 8, "em%i_3", i);    def->addField(t_uint32, col_name);
                    sprintf_s(col_name, 8, "em%i_4", i);    def->addField(t_uint32, col_name);
		        }

		        gDefList.push_back(*def);
	        }
            break;
        case PAGETEXTCACHE:
	        def->setWDBName(WDB_FILE_PAGETEXT);
	        {
		        def->setSignature(PAGETEXT_SIG);

		        // 5 fields
		        def->addField(t_uint32, WDB_ENTRY_FIELD);
		        def->addField(t_uint32, WDB_SIZEOFENTRY_FIELD);
		        def->addField(t_string, "text");
		        def->addField(t_uint32, "next_page");
		        def->addField(t_uint32, "NextPageInCache");

		        gDefList.push_back(*def);
	        }
            break;
        case QUESTCACHE:
	        def->setWDBName(WDB_FILE_QUEST);
	        {
		        def->setSignature(QUEST_SIG);

		        // 77 fields
		        def->addField(t_uint32, WDB_ENTRY_FIELD);
		        def->addField(t_uint32, WDB_SIZEOFENTRY_FIELD);

		        def->addField(t_uint32, "QuestID2");
		        def->addField(t_uint32, "Method");
		        def->addField(t_int32,  "QuestLevel");
		        def->addField(t_int32,  "ZoneOrSort");
		        def->addField(t_uint32, "Type");

		        def->addField(t_uint32, "SuggestedPlayers");
		        def->addField(t_uint32, "FactionID");
		        def->addField(t_uint32, "FactionAmount");
		        def->addField(t_undef,  "");
		        def->addField(t_undef,  "");

		        def->addField(t_uint32, "NextQuestID");
		        def->addField(t_int32,  "CoinReward");
		        def->addField(t_uint32, "CoinRewardOn80");
		        def->addField(t_uint32, "SpellReward");
		        def->addField(t_int32,  "EffectOnPlayer"); // RewSpellCast
		        def->addField(t_undef,  "");
		        def->addField(t_uint32, "StartItemID");
		        def->addField(t_uint32, "QuestFlags");
		        def->addField(t_undef,  "");
		        def->addField(t_undef,  "");
		        def->addField(t_undef,  "");
		        def->addField(t_uint32, "ItemReward1");
		        def->addField(t_uint32, "ItemAmount1");
		        def->addField(t_uint32, "ItemReward2");
		        def->addField(t_uint32, "ItemAmount2");
		        def->addField(t_uint32, "ItemReward3");
		        def->addField(t_uint32, "ItemAmount3");
		        def->addField(t_uint32, "ItemReward4");
		        def->addField(t_uint32, "ItemAmount4");
		        def->addField(t_uint32, "ItemChoice1");
		        def->addField(t_uint32, "ItemChoiceAmount1");
		        def->addField(t_uint32, "ItemChoice2");
		        def->addField(t_uint32, "ItemChoiceAmount2");
		        def->addField(t_uint32, "ItemChoice3");
		        def->addField(t_uint32, "ItemChoiceAmount3");
		        def->addField(t_uint32, "ItemChoice4");
		        def->addField(t_uint32, "ItemChoiceAmount4");
		        def->addField(t_uint32, "ItemChoice5");
		        def->addField(t_uint32, "ItemChoiceAmount5");
		        def->addField(t_uint32, "ItemChoice6");
		        def->addField(t_uint32, "ItemChoiceAmount6");

                def->addField(t_uint32, "PointMapId");
		        def->addField(t_float,  "PointX");
		        def->addField(t_float,  "PointY");
		        def->addField(t_uint32, "PointOpt");

                def->addField(t_string, "Name");
		        def->addField(t_string, "Description");
		        def->addField(t_string, "Details");
		        def->addField(t_string, "Subdescription");

		        def->addField(t_int32,  "KillCreature1");
		        def->addField(t_int32,  "KillCreature1Amount");
		        def->addField(t_int32,  "ItemUsed1");
		        def->addField(t_int32,  "KillCreature2");
		        def->addField(t_int32,  "KillCreature2Amount");
		        def->addField(t_int32,  "ItemUsed2");
		        def->addField(t_int32,  "KillCreature3");
		        def->addField(t_int32,  "KillCreature3Amount");
		        def->addField(t_int32,  "ItemUsed3");
		        def->addField(t_int32,  "KillCreature4");
		        def->addField(t_int32,  "KillCreature4Amount");
		        def->addField(t_int32,  "ItemUsed4");
		        def->addField(t_int32,  "CollectItem1");
		        def->addField(t_int32,  "CollectItem1Amount");
		        def->addField(t_int32,  "CollectItem2");
		        def->addField(t_int32,  "CollectItem2Amount");
		        def->addField(t_int32,  "CollectItem3");
		        def->addField(t_int32,  "CollectItem3Amount");
		        def->addField(t_int32,  "CollectItem4");
		        def->addField(t_int32,  "CollectItem4Amount");
		        def->addField(t_int32,  "");
		        def->addField(t_int32,  "");
		        def->addField(t_string, "Objective1");
		        def->addField(t_string, "Objective2");
		        def->addField(t_string, "Objective3");
		        def->addField(t_string, "Objective4");

		        gDefList.push_back(*def);
	        }
            break;
        case WOWCACHE:
	        //TODO: wowcache.wdb (NDRW)
	        def->setWDBName(WDB_FILE_WOW);
	        {
		        def->setSignature(WOW_SIG);
 
		        // 4 fields
		        def->addField(t_undef,  "");
		        def->addField(t_undef,  "");
		        def->addField(t_undef,  "");
		        def->addField(t_undef,  "");
 
		        gDefList.push_back(*def);
	        }
            break;
    }
}

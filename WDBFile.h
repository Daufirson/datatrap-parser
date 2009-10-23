#pragma once

#include "Defines.h"
#include "WDBDef.h"
#include "WDBRec.h"

class WDBFile : public WDBDef
{
public:
    WDBFile(void);
    ~WDBFile(void);

    WDBDef *m_def;
	char *m_data;
	uint32 m_datasize;
	bool isLoaded;
	wdbheader_t m_header;

	vector<WDBRec> m_records;
    vector<WDBDef> gDefList;

	bool loadWDB(const char* file);
    void LoadDefinitions(uint8 wdbfile);

	wdbfield_t* getField(uint32 recindex, uint32 fieldindex) { return m_records.at(recindex).getField(fieldindex); }
 
    char* BM16toString(uint32 val)
    {
        char* str = new char[17];
        for (int8 i=0; i<16; i++)
        {
            if (val & (1<<(15-i))) str[i] = '1';
            else str[i] = '0';
        }
        str[16] = 0x00; return str;
    }
 
    char* BM32toString(uint32 val)
    {
        char* str = new char[33];
        for (int8 i=0; i<32; i++)
        {
            if (val & (1<<(31-i))) str[i] = '1';
            else str[i] = '0';
        }
        str[32] = 0x00; return str;
    }

    void readValue(int32 j, wdbfield_t* tField, wdbfield_t* pm_data, WDBRec* tRec)
    {
        uint32 sLen;
        char* pcm_data;
		// if the field is a string, the wdbfield will simply contain a pointer to the string
		// in the m_data block
		if (m_def->getFieldType(j) == t_string)
		{
			sLen = strlen((char*)pm_data);
			tField->strval = (char*)pm_data;
			pcm_data = (char*)pm_data;                    
			pcm_data += sLen+1;
			pm_data = (wdbfield_t*)pcm_data;
		}
        else if (m_def->getFieldType(j) == t_uint16)
        {
            uint16* temp = (uint16*)pm_data;
			tField->uint16val = *temp;
			pm_data = (wdbfield_t*)++temp;
        }
        else if (m_def->getFieldType(j) == t_uint8)
        {
            uint8* temp = (uint8*)pm_data;
			tField->uint8val = *temp;
			pm_data = (wdbfield_t*)++temp;
		}
        else if (m_def->getFieldType(j) == t_int16)
        {
            int16* temp = (int16*)pm_data;
			tField->int16val = *temp;
			pm_data = (wdbfield_t*)++temp;
        }
        else if (m_def->getFieldType(j) == t_int8)
        {
            int8* temp = (int8*)pm_data;
			tField->int8val = *temp;
			pm_data = (wdbfield_t*)++temp;
		}
        else
        {
            tField = pm_data;
            pm_data += 1;
        }
        tRec->setField(*tField, j);
    }

    WDBRec tRec;
	wdbfield_t tField;
	wdbfield_t recID,recSize;
	wdbfield_t *pm_data;
	wdbfield_t *pm_dataSafe;

    void readValue(int32 j)
    {
        uint32 sLen;
        char* pcm_data;
	    // if the field is a string, the wdbfield will simply contain a pointer to the string
	    // in the m_data block
	    if (m_def->getFieldType(j) == t_string)
	    {
		    sLen = strlen((char*)pm_data);
		    tField.strval = (char*)pm_data;
		    pcm_data = (char*)pm_data;                    
		    pcm_data += sLen+1;
		    pm_data = (wdbfield_t*)pcm_data;
	    }
        else if (m_def->getFieldType(j) == t_uint16)
        {
            uint16* temp = (uint16*)pm_data;
		    tField.uint16val = *temp;
		    pm_data = (wdbfield_t*)++temp;
        }
        else if (m_def->getFieldType(j) == t_uint8)
        {
            uint8* temp = (uint8*)pm_data;
		    tField.uint8val = *temp;
		    pm_data = (wdbfield_t*)++temp;
	    }
        else if (m_def->getFieldType(j) == t_int16)
        {
            int16* temp = (int16*)pm_data;
		    tField.int16val = *temp;
		    pm_data = (wdbfield_t*)++temp;
        }
        else if (m_def->getFieldType(j) == t_int8)
        {
            int8* temp = (int8*)pm_data;
		    tField.int8val = *temp;
		    pm_data = (wdbfield_t*)++temp;
	    }
        else
        {
            tField = *pm_data;
            pm_data += 1;
        }
        tRec.setField(tField, j);
    }

	void parseWDB()
	{
        wdbfield_t zero;
        zero.uint32val = 0;
        pm_data = (wdbfield_t*)m_data;
        uint32 i, j, stats_Versatz;        
        uint32 max = 0;

        // mit WotLK werden Stats dynamisch gespeichert, deshalb leere Stats ergängen.
        bool itemcache = (strcmp(getDef()->getWDBName(), WDB_FILE_ITEM) == 0);
        
        // 0xFFFF is just a random number I chose, simply to avoid infinite loops
		// which shouldn't happen anyways...
        for (i=0; i<0xFFFF; i++)
		{
 			recID = *pm_data; pm_data++;

			if ((uint32)((char*)pm_data - m_data) >= m_datasize) break;

			// break if invalid ID
            if (recID.uint32val<1) {
                i--;
                continue;
            }

			tField.uint32val = recID.uint32val;
			tRec.setField(tField, 1);

			recSize = *pm_data; pm_data++;
			tField.uint32val = recSize.uint32val;

			//allerletzer String
			pm_dataSafe = (wdbfield_t*)((char*)pm_data + recSize.uint32val);

			// break if the size of the record is less then 4 bytes
			if (recSize.uint32val < 4) break;

			tRec.setField(tField, 2);
            max = m_def->getNumFields();
            if (itemcache) max = 30;        // von WDBFile.cpp Feld "StatsCount"
			for (j=2; j<=max; j++) readValue(j+1);
            if (itemcache)
            {
                stats_Versatz = tField.uint32val;
                // alle vorhandenen Stats einlesen
                for (j=2; j<stats_Versatz*2+2; j++) readValue(max+j);

                // Rest mit 'zero' füllen
                for (j=2; j<20-stats_Versatz*2+2; j++) tRec.setField(zero, max+stats_Versatz*2+j);

                // und weiter im Datensatz
                for (j=max+20; j<=m_def->getNumFields(); j++) readValue(j);
            }
 			m_records.push_back(tRec);
			pm_data = pm_dataSafe; //????
		}
        printf(" - %i entries loaded. ", i);

        isLoaded = true;
	}

    void setDef(WDBDef* def)			{ m_def = def; }
	WDBDef* getDef()					{ return m_def; }
	wdbheader_t getHeader()				{ return m_header; }
};

/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

#include "DBCFile.h"

DBCFile::DBCFile()
{
    data = NULL;
    fieldsOffset = NULL;
}

bool DBCFile::Load(const char *filename, const char *fmt)
{
    uint32 header;
    if (data)
    {
        delete [] data;
        data=NULL;
    }
    FILE * f=fopen(filename,"rb");
    if (!f)return false;

    fread(&header,4,1,f);                                   // Number of records
    EndianConvert(header);
    if (header!=0x43424457)
    {
        //printf("not dbc file");
        return false;                                       //'WDBC'
    }
    fread(&recordCount,4,1,f);                              // Number of records
    EndianConvert(recordCount);
    fread(&fieldCount,4,1,f);                               // Number of fields
    EndianConvert(fieldCount);
    fread(&recordSize,4,1,f);                               // Size of a record
    EndianConvert(recordSize);
    fread(&stringSize,4,1,f);                               // String size
    EndianConvert(stringSize);

    fieldsOffset = new uint32[fieldCount];
    fieldsOffset[0] = 0;
    for (uint32 i = 1; i < fieldCount; i++)
    {
        fieldsOffset[i] = fieldsOffset[i - 1];
        if (fmt[i - 1] == 'b' || fmt[i - 1] == 'X')         // byte fields
            fieldsOffset[i] += 1;
        else                                                // 4 byte fields (int32/float/strings)
            fieldsOffset[i] += 4;
    }

    data = new unsigned char[recordSize*recordCount+stringSize];
    stringTable = data + recordSize*recordCount;
    fread(data,recordSize*recordCount+stringSize,1,f);
    fclose(f);
    return true;
}

DBCFile::~DBCFile()
{
    if (data)
        delete [] data;
    if (fieldsOffset)
        delete [] fieldsOffset;
}

DBCFile::Record DBCFile::getRecord(size_t id)
{
    assert(data);
    return Record(*this, data + id*recordSize);
}

uint32 DBCFile::GetFormatRecordSize(const char * format,int32* index_pos)
{
    uint32 recordsize = 0;
    int32 i = -1;
    for (uint32 x=0; format[x];++x)
        switch (format[x])
        {
            case FT_FLOAT:
            case FT_INT:
                recordsize+=4;
                break;
            case FT_STRING:
                recordsize+=sizeof(char*);
                break;
            case FT_SORT:
                i=x;
                break;
            case FT_IND:
                i=x;
                recordsize+=4;
                break;
            case FT_BYTE:
                recordsize += 1;
                break;
        }

    if (index_pos)
        *index_pos = i;

    return recordsize;
}

char* DBCFile::AutoProduceData(const char* format, uint32& records, char**& indexTable)
{
    /*
    format STRING, NA, FLOAT,NA,INT <=>
    struct{
    char* field0,
    float field1,
    int field2
    }entry;

    this func will generate  entry[rows] data;
    */

    typedef char * ptr;
    if (strlen(format)!=fieldCount)
        return NULL;

    //get struct size and index pos
    int32 i;
    uint32 recordsize=GetFormatRecordSize(format,&i);

    if (i>=0)
    {
        uint32 maxi=0;
        //find max index
        for (uint32 y=0;y<recordCount;y++)
        {
            uint32 ind=getRecord(y).getUInt (i);
            if (ind>maxi)maxi=ind;
        }

        ++maxi;
        records=maxi;
        indexTable=new ptr[maxi];
        memset(indexTable,0,maxi*sizeof(ptr));
    }
    else
    {
        records = recordCount;
        indexTable = new ptr[recordCount];
    }

    char* dataTable= new char[recordCount*recordsize];

    uint32 offset=0;

    for (uint32 y =0;y<recordCount;y++)
    {
        if (i>=0)
        {
            indexTable[getRecord(y).getUInt(i)]=&dataTable[offset];
        }
        else
            indexTable[y]=&dataTable[offset];

        for (uint32 x=0;x<fieldCount;x++)
        {
            switch (format[x])
            {
                case FT_FLOAT:
                    *((float*)(&dataTable[offset]))=getRecord(y).getFloat(x);
                    offset+=4;
                    break;
                case FT_IND:
                case FT_INT:
                    *((uint32*)(&dataTable[offset]))=getRecord(y).getUInt(x);
                    offset+=4;
                    break;
                case FT_BYTE:
                    *((uint8*)(&dataTable[offset]))=getRecord(y).getUInt8(x);
                    offset+=1;
                    break;
                case FT_STRING:
                    *((char**)(&dataTable[offset]))=NULL;   // will be replaces non-empty or "" strings in AutoProduceStrings
                    offset+=sizeof(char*);
                    break;
            }
        }
    }
    return dataTable;
}

char* DBCFile::AutoProduceStrings(const char* format, char* dataTable)
{
    if (strlen(format)!=fieldCount)
        return NULL;

    char* stringPool= new char[stringSize];
    memcpy(stringPool,stringTable,stringSize);

    uint32 offset=0;

    for (uint32 y =0;y<recordCount;y++)
    {
        for (uint32 x=0;x<fieldCount;x++)
            switch (format[x])
        {
            case FT_FLOAT:
            case FT_IND:
            case FT_INT:
                offset+=4;
                break;
            case FT_BYTE:
                offset+=1;
                break;
            case FT_STRING:
                // fill only not filled entries
                char** slot = (char**)(&dataTable[offset]);
                if (!*slot || !**slot)
                {
                    const char * st = getRecord(y).getString(x);
                    *slot=stringPool+(st-(const char*)stringTable);
                }
                offset+=sizeof(char*);
                break;
        }
    }
    return stringPool;
}

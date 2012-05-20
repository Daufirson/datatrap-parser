/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

#include "MPQDBCFile.h"
#include "MPQArchive.h"

MPQDBCFile::MPQDBCFile(const std::string &filename): filename(filename), data(0)
{
}

void MPQDBCFile::open()
{
    MPQFile f(filename.c_str());
    char header[4];
    unsigned int na,nb,es,ss;

    f.read(header,4); // Number of records
    assert(header[0]=='W' && header[1]=='D' && header[2]=='B' && header[3] == 'C');
    f.read(&na,4); // Number of records
    f.read(&nb,4); // Number of fields
    f.read(&es,4); // Size of a record
    f.read(&ss,4); // String size

    recordSize = es;
    recordCount = na;
    fieldCount = nb;
    stringSize = ss;
    assert(fieldCount*4 == recordSize);

    data = new unsigned char[recordSize*recordCount+stringSize];
    stringTable = data + recordSize*recordCount;
    f.read(data,recordSize*recordCount+stringSize);
    f.close();
}

MPQDBCFile::~MPQDBCFile()
{
    delete [] data;
}

MPQDBCFile::Record MPQDBCFile::getRecord(size_t id)
{
    assert(data);
    return Record(*this, data + id*recordSize);
}

size_t MPQDBCFile::getMaxId()
{
    assert(data);

    size_t maxId = 0;
    for (size_t i = 0; i < getRecordCount(); ++i)
    {
        if (maxId < getRecord(i).getUInt(0))
            maxId = getRecord(i).getUInt(0);
    }
    return maxId;
}

MPQDBCFile::Iterator MPQDBCFile::begin()
{
    assert(data);
    return Iterator(*this, data);
}
MPQDBCFile::Iterator MPQDBCFile::end()
{
    assert(data);
    return Iterator(*this, stringTable);
}

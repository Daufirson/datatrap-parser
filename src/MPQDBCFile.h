/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

#ifndef MPQDBCFILE_H
#define MPQDBCFILE_H

#include "Defines.h"

class MPQDBCFile
{
public:
    MPQDBCFile(const std::string &filename);
    ~MPQDBCFile();

    // Open database. It must be openened before it can be used.
    void open();

    // Database exceptions
    class Exception
    {
    public:
        Exception(const std::string &message): message(message)
        { }
        virtual ~Exception()
        { }
        const std::string &getMessage() {return message;}
    private:
        std::string message;
    };
    class NotFound: public Exception
    {
    public:
        NotFound(): Exception("Key was not found.") 
        { }
    };
    // Iteration over database
    class Iterator;
    class Record
    {
    public:
        float getFloat(size_t field) const
        {
            assert(field < file.fieldCount);
            return *reinterpret_cast<float*>(offset+field*4);
        }
        unsigned int getUInt(size_t field) const
        {
            assert(field < file.fieldCount);
            return *reinterpret_cast<unsigned int*>(offset+field*4);
        }
        int getInt(size_t field) const
        {
            assert(field < file.fieldCount);
            return *reinterpret_cast<int*>(offset+field*4);
        }
        const char *getString(size_t field) const
        {
            assert(field < file.fieldCount);
            size_t stringOffset = getUInt(field);
            assert(stringOffset < file.stringSize);
            return reinterpret_cast<char*>(file.stringTable + stringOffset);
        }
    private:
        Record(MPQDBCFile &file, unsigned char *offset): file(file), offset(offset) {}
        unsigned char *offset;
        MPQDBCFile &file;

        friend class MPQDBCFile;
        friend class MPQDBCFile::Iterator;
    };
    /** Iterator that iterates over records
    */
    class Iterator
    {
    public:
        Iterator(MPQDBCFile &file, unsigned char *offset):
            record(file, offset) {}
        /// Advance (prefix only)
        Iterator & operator++() {
            record.offset += record.file.recordSize;
            return *this;
        }
        /// Return address of current instance
        Record const & operator*() const { return record; }
        const Record* operator->() const 
        {
            return &record;
        }
        /// Comparison
        bool operator==(const Iterator &b) const
        {
            return record.offset == b.record.offset;
        }
        bool operator!=(const Iterator &b) const
        {
            return record.offset != b.record.offset;
        }
    private:
        Record record;
    };

    // Get record by id
    Record getRecord(size_t id);
    /// Get begin iterator over records
    Iterator begin();
    /// Get begin iterator over records
    Iterator end();
    /// Trivial
    size_t getRecordCount() const { return recordCount;}
    size_t getFieldCount() const { return fieldCount; }
    size_t getMaxId();
private:
    std::string filename;
    size_t recordSize;
    size_t recordCount;
    size_t fieldCount;
    size_t stringSize;
    unsigned char *data;
    unsigned char *stringTable;
};

#endif

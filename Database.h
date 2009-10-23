#ifndef _DATABASE_H
#define _DATABASE_H

#include "Defines.h"
#include <mysql.h>

class Field
{
    public:

        enum DataTypes
        {
            DB_TYPE_UNKNOWN = 0x00,
            DB_TYPE_STRING  = 0x01,
            DB_TYPE_INTEGER = 0x02,
            DB_TYPE_FLOAT   = 0x03,
            DB_TYPE_BOOL    = 0x04
        };

        Field();
        Field(Field &f);
        Field(const char *value, enum DataTypes type);

        ~Field();

        enum DataTypes GetType() const { return mType; }

        const char *GetString() const { return mValue; }
        std::string GetCppString() const
        {
            return mValue ? mValue : "";                    // std::string s = 0 have undefine result in C++
        }
        float GetFloat() const { return mValue ? static_cast<float>(atof(mValue)) : 0.0f; }
        bool GetBool() const { return mValue ? atoi(mValue) > 0 : false; }
        int32 GetInt32() const { return mValue ? static_cast<int32>(atol(mValue)) : int32(0); }
        uint8 GetUInt8() const { return mValue ? static_cast<uint8>(atol(mValue)) : uint8(0); }
        uint16 GetUInt16() const { return mValue ? static_cast<uint16>(atol(mValue)) : uint16(0); }
        int16 GetInt16() const { return mValue ? static_cast<int16>(atol(mValue)) : int16(0); }
        uint32 GetUInt32() const { return mValue ? static_cast<uint32>(atol(mValue)) : uint32(0); }
        uint64 GetUInt64() const
        {
            if(mValue)
            {
                uint64 value;
                sscanf(mValue,I64FMTD,&value);
                return value;
            }
            else
                return 0;
        }

        void SetType(enum DataTypes type) { mType = type; }

        void SetValue(const char *value);

    private:
        char *mValue;
        enum DataTypes mType;
}; 

class QueryResult
{
    public:
        QueryResult(uint64 rowCount, uint32 fieldCount) : mFieldCount(fieldCount), mRowCount(rowCount) {}

        virtual ~QueryResult() {}

        virtual bool NextRow() = 0;
        virtual bool GotoFirstRow() = 0;

        typedef std::map<uint32, std::string> FieldNames;

        uint32 GetField_idx(const std::string &name) const
        {
            for(FieldNames::const_iterator iter = GetFieldNames().begin(); iter != GetFieldNames().end(); ++iter)
            {
                if(iter->second == name)
                    return iter->first;
            }
            assert(false && "unknown field name");
            return uint32(-1);
        }

        Field *Fetch() const { return mCurrentRow; }

        const Field & operator [] (int index) const { return mCurrentRow[index]; }

        const Field & operator [] (const std::string &name) const
        {
            return mCurrentRow[GetField_idx(name)];
        }

        uint32 GetFieldCount() const { return mFieldCount; }
        uint64 GetRowCount() const { return mRowCount; }
        FieldNames const& GetFieldNames() const {return mFieldNames; }

    protected:
        Field *mCurrentRow;
        uint32 mFieldCount;
        uint64 mRowCount;
        FieldNames mFieldNames;
}; 

class QueryResultMysql : public QueryResult
{
    public:
        QueryResultMysql(MYSQL_RES* result, uint64 rowCount, uint32 fieldCount);

        ~QueryResultMysql();

        bool NextRow();
        bool GotoFirstRow();

    private:
        enum Field::DataTypes ConvertNativeType(enum_field_types mysqlType) const;
        void EndQuery();

        MYSQL_RES* mResult;
}; 

class Database
{
public:
    Database();
    ~Database();

    bool Initialize(const char* user, const char* pw, const char* db = NULL, const char* host = NULL);
    bool DirectExecute(const char* sql);
    QueryResult* Query(const char* sql);

private:
    MYSQL *mMysql;
    static size_t db_count;
};

#endif

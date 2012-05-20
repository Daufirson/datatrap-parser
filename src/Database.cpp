/* This file is part of DataTrap.
 * 
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * 
 * DataTrap is licenced under the Microsoft Reciprocal License.
 * You should find the licence included with the source of the program,
 * or at this URL: <http://www.microsoft.com/opensource/licenses.mspx#Ms-RL>
 */

#include "Database.h"

size_t Database::db_count = 0;

QueryResultMysql::QueryResultMysql(MYSQL_RES *result, uint64 rowCount, uint32 fieldCount) :
QueryResult(rowCount, fieldCount), mResult(result)
{
    mCurrentRow = new Field[mFieldCount];

    MYSQL_FIELD *fields = mysql_fetch_fields(mResult);

    for (uint32 i = 0; i < mFieldCount; i++)
    {
        mFieldNames[i] = fields[i].name;
        mCurrentRow[i].SetType(ConvertNativeType(fields[i].type));
    }
}

QueryResultMysql::~QueryResultMysql()
{
    EndQuery();
}

bool QueryResultMysql::GotoFirstRow()
{
    MYSQL_ROW_OFFSET offset = 0L;

    if (!mResult) return false;

    mysql_row_seek(mResult, offset);

    return true;
}

bool QueryResultMysql::NextRow()
{
    MYSQL_ROW row;

    if (!mResult)
        return false;

    row = mysql_fetch_row(mResult);
    if (!row)
    {
        EndQuery();
        return false;
    }

    for (uint32 i = 0; i < mFieldCount; i++)
        mCurrentRow[i].SetValue(row[i]);

    return true;
}

void QueryResultMysql::EndQuery()
{
    if (mCurrentRow)
    {
        delete [] mCurrentRow;
        mCurrentRow = 0;
    }

    if (mResult)
    {
        mysql_free_result(mResult);
        mResult = 0;
    }
}

enum Field::DataTypes QueryResultMysql::ConvertNativeType(enum_field_types mysqlType) const
{
    switch (mysqlType)
    {
        case FIELD_TYPE_TIMESTAMP:
        case FIELD_TYPE_DATE:
        case FIELD_TYPE_TIME:
        case FIELD_TYPE_DATETIME:
        case FIELD_TYPE_YEAR:
        case FIELD_TYPE_STRING:
        case FIELD_TYPE_VAR_STRING:
        case FIELD_TYPE_BLOB:
        case FIELD_TYPE_SET:
        case FIELD_TYPE_NULL:
            return Field::DB_TYPE_STRING;
        case FIELD_TYPE_TINY:

        case FIELD_TYPE_SHORT:
        case FIELD_TYPE_LONG:
        case FIELD_TYPE_INT24:
        case FIELD_TYPE_LONGLONG:
        case FIELD_TYPE_ENUM:
            return Field::DB_TYPE_INTEGER;
        case FIELD_TYPE_DECIMAL:
        case FIELD_TYPE_FLOAT:
        case FIELD_TYPE_DOUBLE:
            return Field::DB_TYPE_FLOAT;
        default:
            return Field::DB_TYPE_UNKNOWN;
    }
}

Field::Field() : mValue(NULL), mType(DB_TYPE_UNKNOWN)
{
}

Field::Field(Field &f)
{
    const char *value;

    value = f.GetString();

    if (value && (mValue = new char[strlen(value) + 1])) strcpy(mValue, value);
    else mValue = NULL;

    mType = f.GetType();
}

Field::Field(const char *value, enum Field::DataTypes type) :
mType(type)
{
    if (value && (mValue = new char[strlen(value) + 1])) strcpy(mValue, value);
    else mValue = NULL;
}

Field::~Field()
{
    if (mValue) delete [] mValue;
}

void Field::SetValue(const char *value)
{
    if (mValue) delete [] mValue;

    if (value)
    {
        mValue = new char[strlen(value) + 1];
        strcpy(mValue, value);
    } else mValue = NULL;
}

Database::Database() : mMysql(0)
{
    // before first connection
    if (db_count++ == 0)
    {
        // Mysql Library Init
        mysql_library_init(-1, NULL, NULL);
    }
}

Database::~Database()
{
    if (mMysql) mysql_close(mMysql);
    //Free Mysql library pointers for last ~DB
    if (--db_count == 0) mysql_library_end();
}

bool Database::Initialize(const char* user, const char* pw, const char* db, const char* host)
{
    MYSQL *mysqlInit;
    mysqlInit = mysql_init(NULL);

    if (!mysqlInit) return false;

#if PLATFORM == PLATFORM_WINDOWS
    int port = 3306;
    char const* unix_socket = NULL;
#else
    int port = 0;
    char const* unix_socket = "3306";
#endif
    mysql_options(mysqlInit, MYSQL_SET_CHARSET_NAME, "utf8");

    if (host == NULL) host = "localhost";

    mMysql = mysql_real_connect(mysqlInit, host, user, pw, db, port, unix_socket, 0);

    if (mMysql)
    {
        mysql_autocommit(mMysql, 1);
        // set connection properties to UTF8 to properly handle locales for different
        // server configs - core sends data in UTF8, so MySQL must expect UTF8 too
        DirectExecute("SET NAMES `utf8`");
        DirectExecute("SET CHARACTER SET `utf8`");

        return true;
    }
    else
    {
//        sLog.outError( "Could not connect to MySQL database at %s: %s\n", host.c_str(),mysql_error(mysqlInit));
        mysql_close(mysqlInit);
        return false;
    }
}

bool Database::DirectExecute(const char* sql)
{
    if (!mMysql) return false;
    {
//        uint32 _s = getMSTime();
        if (mysql_query(mMysql, sql))
        {
//            printf("Query: %s\n", sql);
            printf("\nMySQL ERROR: %s\n", mysql_error(mMysql));
            return false;
        }
        else
        {
//            printf("[%u ms] SQL: %s\n", getMSTimeDiff(_s,getMSTime()), sql );
        }
    }
    return true;
}

QueryResult* Database::Query(const char* sql)
{
    if (!mMysql) return NULL;

    MYSQL_RES* result = NULL;
    uint64 rowCount = 0;
    uint32 fieldCount = 0;

    if (mysql_query(mMysql, sql))
    {
        //printf("Query: %s\n\n", sql);
        printf("\nMySQL ERROR: %s\n", mysql_error(mMysql));
        return NULL;
    }

    result = mysql_store_result(mMysql);
    rowCount = mysql_affected_rows(mMysql);
    fieldCount = mysql_field_count(mMysql);

    if (!result) return NULL;

    if (!rowCount)
    {
        mysql_free_result(result);
        return NULL;
    }

    QueryResultMysql* queryResult = new QueryResultMysql(result, rowCount, fieldCount);

    queryResult->NextRow();

    return queryResult;
}

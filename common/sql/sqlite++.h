/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_SQLITEXX_H
#define CXX_SQLITEXX_H
#include <stdexcept>
#include <string>
#include "common/config.h"

struct sqlite3;
struct sqlite3_stmt;
namespace cxx {

    class sqlite3_connection;
    class sqlite3_error : public std::runtime_error {
    public:
        explicit sqlite3_error(const char* msg);
        explicit sqlite3_error(const std::string& msg);
        explicit sqlite3_error(sqlite3_connection& conn);
    };

    class sqlite3_connection {
    public:
        sqlite3_connection();
        explicit sqlite3_connection(const char* db);
        ~sqlite3_connection();
        void        open(const char* db) throw(sqlite3_error);
        void        close() throw(sqlite3_error);

        int64_t     insertid() throw(sqlite3_error);
        void        setbusytimeout(int ms) throw(sqlite3_error);
        void        pragma(const char* info);
        void        executedml(const std::string& sql) throw(sqlite3_error);
        int         executesql(const std::string& sql, int* ret) throw(sqlite3_error);
        long long   executesql(const std::string& sql, long long* ret) throw(sqlite3_error);
        double      executesql(const std::string& sql, double* ret) throw(sqlite3_error);
        std::string executesql(const std::string& sql, std::string* ret) throw(sqlite3_error);
        std::string executesql_blob(const std::string& sql) throw(sqlite3_error);
    private:
        sqlite3* db_;
        friend class sqlite3_command;
        friend class sqlite3_error;
        sqlite3_connection(const sqlite3_connection& rhs);
        sqlite3_connection& operator= (const sqlite3_connection& rhs);
    };

    class sqlite3_transaction {
    public:
        sqlite3_transaction(sqlite3_connection& con, bool start = true) throw(sqlite3_error);
        ~sqlite3_transaction();
        void    begin()    throw(sqlite3_error);
        void    commit()   throw(sqlite3_error);
        void    rollback() throw(sqlite3_error);
    private:
        sqlite3_connection& connect_;
        bool                intrans_;
        sqlite3_transaction(const sqlite3_transaction& rhs);
        sqlite3_transaction& operator= (const sqlite3_transaction& rhs);
    };

    class sqlite3_command;
    class sqlite3_reader {
    public:
        sqlite3_reader();
        sqlite3_reader(const sqlite3_reader& rhs);
        ~sqlite3_reader();
        sqlite3_reader& operator= (const sqlite3_reader& rhs);

        bool        iseof() throw(sqlite3_error);
        void        reset() throw(sqlite3_error);
        void        close();
        /// get the value of the index, index begin from 0
        int         get(int index, int* ret) throw(sqlite3_error);
        long long   get(int index, long long* ret) throw(sqlite3_error);
        double      get(int index, double* ret) throw(sqlite3_error);
        std::string get(int index, std::string* ret) throw(sqlite3_error);
        std::string get_blob(int index) throw(sqlite3_error);
        std::string getColName(int index) throw(sqlite3_error);
    private:
        friend class sqlite3_command;
        sqlite3_command* cmd_;
        sqlite3_reader(sqlite3_command* cmd);
    };

    class sqlite3_command {
    public:
        sqlite3_command(sqlite3_connection& conn, const std::string& sql) throw(sqlite3_error);
        ~sqlite3_command();

        void    bind(int index) throw(sqlite3_error);
        void    bind(int index, int data) throw(sqlite3_error);
        void    bind(int index, long long data) throw(sqlite3_error);
        void    bind(int index, double data) throw(sqlite3_error);
        void    bind(int index, const char* data, int datalen) throw(sqlite3_error);
        void    bind(int index, const void* data, int datalen) throw(sqlite3_error);
        void    bind(int index, const std::string& data) throw(sqlite3_error);
        sqlite3_reader  execute(sqlite3_reader* ret) throw(sqlite3_error);
        void            execute() throw(sqlite3_error);
        int             execute(int* ret) throw(sqlite3_error);
        long long       execute(long long* ret) throw(sqlite3_error);
        double          execute(double* ret) throw(sqlite3_error);
        std::string     execute(std::string* ret) throw(sqlite3_error);
        std::string     execute_blob() throw(sqlite3_error);
        /// get the number of database rows that were changed (or inserted or
        /// deleted) by most recently completed INSERT, UPDATE, DELETE statement
        /// please note, changes inside the trigger is not count.
        int             changes();
    private:
        friend class sqlite3_reader;
        sqlite3_connection&     conn_;
        sqlite3_stmt*           stmt_;
        unsigned int            refs_;
        int                     argc_;
        sqlite3_command(const sqlite3_command& rhs);
        sqlite3_command& operator= (const sqlite3_command& rhs);
    };

}

#endif

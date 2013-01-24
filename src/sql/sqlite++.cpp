/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/sql/sqlite++.h"
#include "sqlite/sqlite3.h"
#include <sstream>

namespace cxx {

    /**
     * class sqlite3_error
     */
    sqlite3_error::sqlite3_error(const char* msg) : std::runtime_error(msg)
    {
    }

    sqlite3_error::sqlite3_error(const std::string& msg) : std::runtime_error(msg)
    {
    }

    sqlite3_error::sqlite3_error(sqlite3_connection& conn)
        : std::runtime_error(sqlite3_errmsg(conn.db_))
    {
    }

    /**
     * class sqlite3_connection
     */
    sqlite3_connection::sqlite3_connection()
        : db_(NULL)
    {
    }

    sqlite3_connection::sqlite3_connection(const char* db)
        : db_(NULL)
    {
        this->open(db);
    }

    sqlite3_connection::~sqlite3_connection()
    {
        /** why here didn't call this->close(), because this->close()
        *   will throw an exception, which is dangoures in
        *   destructor
        */
        if(db_) {
            sqlite3_close(db_);
            db_ = NULL;
        }
    }

    void sqlite3_connection::open(const char* db) throw(sqlite3_error)
    {
        if(sqlite3_open(db, &db_) != SQLITE_OK) {
            std::ostringstream  str;
            str << "failed to open database \'" << db << "\'";
            throw sqlite3_error(str.str());
        }
    }

    void sqlite3_connection::close() throw(sqlite3_error)
    {
        if(db_) {
            if(sqlite3_close(db_) != SQLITE_OK) {
                throw sqlite3_error(*this);
            }
            db_ = NULL;
        }
    }

    int64_t sqlite3_connection::insertid() throw(sqlite3_error)
    {
        if(!db_)
            throw sqlite3_error("database is not open");

        return sqlite3_last_insert_rowid(db_);
    }

    void sqlite3_connection::setbusytimeout(int ms) throw(sqlite3_error)
    {
        if(!db_)
            throw sqlite3_error("database is not open");

        if(sqlite3_busy_timeout(db_, ms) != SQLITE_OK)
            throw sqlite3_error(*this);
    }

    void sqlite3_connection::pragma(const char *info)
    {
        char* error;
        if(sqlite3_exec(db_, info, NULL, NULL, &error) != SQLITE_OK) {
            throw sqlite3_error(error);
        }
    }

    void sqlite3_connection::executedml(const std::string& sql) throw(sqlite3_error)
    {
        if(!db_)
            throw sqlite3_error("database is not open");

        sqlite3_command(*this, sql).execute();
    }

    int sqlite3_connection::executesql(const std::string& sql, int* ret) throw(sqlite3_error)
    {
        if(!db_)
            throw sqlite3_error("database is not open");

        return sqlite3_command(*this, sql).execute(ret);
    }

    long long sqlite3_connection::executesql(const std::string& sql, long long* ret) throw(sqlite3_error)
    {
        if(!db_)
            throw sqlite3_error("database is not open");

        return sqlite3_command(*this, sql).execute(ret);
    }

    double sqlite3_connection::executesql(const std::string& sql, double* ret) throw(sqlite3_error)
    {
        if(!db_)
            throw sqlite3_error("database is not open");

        return sqlite3_command(*this, sql).execute(ret);
    }

    std::string sqlite3_connection::executesql(const std::string& sql, std::string* ret) throw(sqlite3_error)
    {
        if(!db_)
            throw sqlite3_error("database is not open");

        return sqlite3_command(*this, sql).execute(ret);
    }

    std::string sqlite3_connection::executesql_blob(const std::string& sql) throw(sqlite3_error)
    {
        if(!db_)
            throw sqlite3_error("database is not open");

        return sqlite3_command(*this, sql).execute_blob();
    }

    /**
     * class sqlite3_transaction
     */
    sqlite3_transaction::sqlite3_transaction(sqlite3_connection& con, bool start)  throw(sqlite3_error)
        : connect_(con), intrans_(false)
    {
        if(start)
            begin();
    }

    sqlite3_transaction::~sqlite3_transaction()
    {
        if(intrans_) {
            try {
                rollback();
            }
            catch(const sqlite3_error& ) {
                // do nothing
            }
        }
    }

    void sqlite3_transaction::begin() throw(sqlite3_error)
    {
        connect_.executedml("begin;");
        intrans_ = true;
    }

    void sqlite3_transaction::commit() throw(sqlite3_error)
    {
        connect_.executedml("commit;");
        intrans_ = false;
    }

    void sqlite3_transaction::rollback() throw(sqlite3_error)
    {
        connect_.executedml("rollback;");
        intrans_ = false;
    }

    /**
     * class sqlite3_reader
     */
    sqlite3_reader::sqlite3_reader() : cmd_(NULL)
    {
    }

    sqlite3_reader::sqlite3_reader(const sqlite3_reader& rhs) : cmd_(rhs.cmd_)
    {
        if(cmd_) {
            ++cmd_->refs_;
        }
    }

    sqlite3_reader::sqlite3_reader(sqlite3_command* cmd) : cmd_(cmd)
    {
        if(cmd_) {
            ++cmd_->refs_;
        }
    }

    sqlite3_reader::~sqlite3_reader()
    {
        close();
    }

    sqlite3_reader& sqlite3_reader::operator= (const sqlite3_reader& rhs)
    {
        close();
        cmd_ = rhs.cmd_;
        if(cmd_) {
            ++cmd_->refs_;
        }
        return *this;
    }

    bool sqlite3_reader::iseof() throw(sqlite3_error)
    {
        if(!cmd_)
            throw sqlite3_error("database reader is closed");

        switch(sqlite3_step(cmd_->stmt_)) {
        case SQLITE_ROW:
            return false;
        case SQLITE_DONE:
            return true;
        default:
            throw sqlite3_error(cmd_->conn_);
        }
    }

    void sqlite3_reader::reset() throw(sqlite3_error)
    {
        if(!cmd_)
            throw sqlite3_error("database reader is closed");

        if(sqlite3_reset(cmd_->stmt_) != SQLITE_OK)
            throw sqlite3_error(cmd_->conn_);
    }

    void sqlite3_reader::close()
    {
        if(cmd_) {
            if(--cmd_->refs_ == 0) sqlite3_reset(cmd_->stmt_);
            cmd_ = NULL;
        }
    }

    int sqlite3_reader::get(int index, int* ret) throw(sqlite3_error)
    {
        if(!cmd_)
            throw sqlite3_error("database reader is closed");

        if((index > (cmd_->argc_ - 1)) || (index < 0)) {
            std::ostringstream str;
            str << "database reader get<int>(" << index << ") out of range"
                << " [0~" << cmd_->argc_ - 1 << "]";
            throw sqlite3_error(str.str());
        }
        int val = sqlite3_column_int(cmd_->stmt_, index);
        if(ret) *ret = val;
        return val;
    }

    long long sqlite3_reader::get(int index, long long* ret) throw(sqlite3_error)
    {
        if(!cmd_)
            throw sqlite3_error("database reader is closed");

        if((index > (cmd_->argc_ - 1)) || (index < 0)) {
            std::ostringstream str;
            str << "database reader get<long long>(" << index << ") out of range"
                << " [0~" << cmd_->argc_ - 1 << "]";
            throw sqlite3_error(str.str());
        }
        long long val = sqlite3_column_int64(cmd_->stmt_, index);
        if(ret) *ret = val;
        return val;
    }

    double sqlite3_reader::get(int index, double* ret) throw(sqlite3_error)
    {
        if(!cmd_)
            throw sqlite3_error("database reader is closed");

        if((index > (cmd_->argc_ - 1)) || (index < 0)) {
            std::ostringstream str;
            str << "database reader get<double>(" << index << ") out of range"
                << " [0~" << cmd_->argc_ - 1 << "]";
            throw sqlite3_error(str.str());
        }
        double val = sqlite3_column_double(cmd_->stmt_, index);
        if(ret) *ret = val;
        return val;
    }

    std::string sqlite3_reader::get(int index, std::string* ret) throw(sqlite3_error)
    {
        if(!cmd_)
            throw sqlite3_error("database reader is closed");

        if((index > (cmd_->argc_ - 1)) || (index < 0)) {
            std::ostringstream str;
            str << "database reader get<std::string>(" << index << ") out of range"
                << " [0~" << cmd_->argc_ - 1 << "]";
            throw sqlite3_error(str.str());
        }
        std::string val = std::string((const char* )sqlite3_column_text(cmd_->stmt_, index),
                                      sqlite3_column_bytes(cmd_->stmt_, index));
        if(ret) *ret = val;
        return val;
    }

    std::string sqlite3_reader::get_blob(int index) throw(sqlite3_error)
    {
        if(!cmd_)
            throw sqlite3_error("database reader is closed");

        if((index > (cmd_->argc_ - 1)) || (index < 0)) {
            std::ostringstream str;
            str << "database reader get<blob>(" << index << ") out of range"
                << " [0~" << cmd_->argc_ - 1 << "]";
            throw sqlite3_error(str.str());
        }
        return std::string((const char* )sqlite3_column_blob(cmd_->stmt_, index),
                           sqlite3_column_bytes(cmd_->stmt_, index));
    }

    std::string sqlite3_reader::getColName(int index) throw(sqlite3_error)
    {
        if(!cmd_)
            throw sqlite3_error("database reader is closed");

        if((index > (cmd_->argc_ - 1)) || (index < 0)) {
            std::ostringstream str;
            str << "database reader get<std::string>(" << index << ") out of range"
                << " [0~" << cmd_->argc_ - 1 << "]";
            throw sqlite3_error(str.str());
        }

        return sqlite3_column_name(cmd_->stmt_, index);
    }


    /**
     * class sqlite3_command
     */
    sqlite3_command::sqlite3_command(sqlite3_connection& conn, const std::string& sql) throw(sqlite3_error)
        : conn_(conn), refs_(0)
    {
        const char* tail = NULL;
        if(sqlite3_prepare(conn_.db_, sql.c_str(), sql.size(), &stmt_, &tail) != SQLITE_OK) {
            throw sqlite3_error(conn_);
        }

        argc_ = sqlite3_column_count(stmt_);
    }

    sqlite3_command::~sqlite3_command()
    {
        sqlite3_finalize(stmt_);
    }

    void sqlite3_command::bind(int index) throw(sqlite3_error)
    {
        if(sqlite3_bind_null(stmt_, index) != SQLITE_OK)
            throw sqlite3_error(conn_);
    }

    void sqlite3_command::bind(int index, int data) throw(sqlite3_error)
    {
        if(sqlite3_bind_int(stmt_, index, data) != SQLITE_OK)
            throw sqlite3_error(conn_);
    }

    void sqlite3_command::bind(int index, long long data) throw(sqlite3_error)
    {
        if(sqlite3_bind_int64(stmt_, index, data) != SQLITE_OK)
            throw sqlite3_error(conn_);
    }

    void sqlite3_command::bind(int index, double data) throw(sqlite3_error)
    {
        if(sqlite3_bind_double(stmt_, index, data) != SQLITE_OK)
            throw sqlite3_error(conn_);
    }

    void sqlite3_command::bind(int index, const char* data, int datalen) throw(sqlite3_error)
    {
        if(sqlite3_bind_text(stmt_, index, data, datalen, SQLITE_TRANSIENT) != SQLITE_OK)
            throw sqlite3_error(conn_);
    }

    void sqlite3_command::bind(int index, const void* data, int datalen) throw(sqlite3_error)
    {
        if(sqlite3_bind_blob(stmt_, index, data, datalen, SQLITE_TRANSIENT) != SQLITE_OK)
            throw sqlite3_error(conn_);
    }

    void sqlite3_command::bind(int index, const std::string& data) throw(sqlite3_error)
    {
        if(sqlite3_bind_text(stmt_, index, data.data(), data.size(), SQLITE_TRANSIENT) != SQLITE_OK)
            throw sqlite3_error(conn_);
    }

    sqlite3_reader sqlite3_command::execute(sqlite3_reader* ret) throw(sqlite3_error)
    {
        sqlite3_reader reader(this);
        if(ret) *ret = reader;
        return reader;
    }

    void sqlite3_command::execute() throw(sqlite3_error)
    {
        execute((sqlite3_reader* )NULL).iseof();
    }

    int sqlite3_command::execute(int* ret) throw(sqlite3_error)
    {
        sqlite3_reader reader = execute((sqlite3_reader* )NULL);
        if(reader.iseof()) throw sqlite3_error("nothing to read");
        int val = reader.get(0, (int* )NULL);
        if(ret) *ret = val;
        return val;
    }

    long long sqlite3_command::execute(long long* ret) throw(sqlite3_error)
    {
        sqlite3_reader reader = execute((sqlite3_reader* )NULL);
        if(reader.iseof()) throw sqlite3_error("nothing to read");
        long long val = reader.get(0, (long long* )NULL);
        if(ret) *ret = val;
        return val;
    }

    double sqlite3_command::execute(double* ret) throw(sqlite3_error)
    {
        sqlite3_reader reader = execute((sqlite3_reader* )NULL);
        if(reader.iseof()) throw sqlite3_error("nothing to read");
        double val = reader.get(0, (double* )NULL);
        if(ret) *ret = val;
        return val;
    }

    std::string sqlite3_command::execute(std::string* ret) throw(sqlite3_error)
    {
        sqlite3_reader reader = execute((sqlite3_reader* )NULL);
        if(reader.iseof()) throw sqlite3_error("nothing to read");
        std::string val = reader.get(0, (std::string* )NULL);
        if(ret) *ret = val;
        return val;
    }

    std::string sqlite3_command::execute_blob() throw(sqlite3_error)
    {
        sqlite3_reader reader = execute((sqlite3_reader* )NULL);
        if(reader.iseof()) throw sqlite3_error("nothing to read");
        return reader.get_blob(0);
    }

    int sqlite3_command::changes()
    {
        return sqlite3_changes(conn_.db_);
    }

}


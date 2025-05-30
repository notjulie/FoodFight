// ====================================================================
// SQLite wrapper
//    Author: Randy Rasmussen
//    Copyright: none... do what you will
//    Warranties: none... do what you will at your own risk
//
// File summary:
//    This defines the SQLParameter class and its kin...the purpose is
//    to simplify passing parameters to queries and to encapsulate the
//    binding of parameters.
// ====================================================================


#include "SQLiteCommonHeader.h"
#include "SQLException.h"
#include "SQLParameter.h"


/// =============================================================================
/// ======================== class SQLParameter =================================
/// =============================================================================

/// <summary>
/// Copy constructor
/// </summary>
SQLParameter::SQLParameter(const SQLParameter &source)
{
   handler.reset(source.handler->Clone());
}


/// <summary>
/// Binds the value of this parameter to a parameter in a SQL statement
/// </summary>
void SQLParameter::Bind(sqlite3_stmt *statement, int index) const
{
   int sqlResult;

   if (handler.get() == nullptr)
      sqlResult = sqlite3_bind_null(statement, index);
   else
      sqlResult = handler->Bind(statement, index);
}


/// =============================================================================
/// ======================= Parameter binding primitives ========================
/// =============================================================================

int BindSQLiteParameterToStatement(int value, sqlite3_stmt *statement, int index)
{
   return sqlite3_bind_int(statement, index, value);
}

int BindSQLiteParameterToStatement(const std::string &value, sqlite3_stmt *statement, int index)
{
   return sqlite3_bind_text(statement, index, value.c_str(), (int)value.length(), SQLITE_TRANSIENT);
}


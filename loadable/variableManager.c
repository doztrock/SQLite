#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(variable) (void)(variable)

static void SetVariable(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv
) {

    sqlite3 *db = sqlite3_context_db_handle(context);
    char *variableName = NULL;
    char *variableValue = NULL;
    char query[512];

    if (argc == 2) {

        variableName = (char *) sqlite3_value_text(argv[0]);
        variableValue = (char *) sqlite3_value_text(argv[1]);

        if(variableValue) {

            sprintf(query, "INSERT OR REPLACE INTO variable (variableName, variableValue) VALUES ('%s', '%s')", variableName, variableValue);

            if(sqlite3_exec(db, query, NULL, NULL, NULL) == SQLITE_OK) {
                sqlite3_result_text(context, (char *) "OK", strlen("OK"), SQLITE_TRANSIENT);
                return;
            } else {
                sqlite3_result_null(context);
                return;
            }

        }

        sqlite3_result_null(context);
        return;
    }

    sqlite3_result_null(context);
    return;
}

static void GetVariable(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv
) {

    sqlite3 *db = sqlite3_context_db_handle(context);
    sqlite3_stmt *res = NULL;
    char *variableName = NULL;
    char *variableValue = NULL;
    char query[512];

    if (argc == 1) {

        variableName = (char *) sqlite3_value_text(argv[0]);

        sprintf(query, "SELECT variableValue FROM variable WHERE variableName = '%s'", variableName);
        sqlite3_prepare_v2(db, query, strlen(query), &res, NULL);

        while(sqlite3_step(res) != SQLITE_DONE) {

            if(sqlite3_column_count(res) > 0) {
                variableValue = (char *) sqlite3_column_text(res, 0);
            }

            break;
        }

        if(variableValue) {
            sqlite3_result_text(context, (char *) variableValue, strlen(variableValue), SQLITE_TRANSIENT);
            sqlite3_finalize(res);
            return;
        }

        sqlite3_result_null(context);
        sqlite3_finalize(res);
        return;
    }

    sqlite3_result_null(context);
    sqlite3_finalize(res);
    return;
}

#ifdef _WIN32
__declspec(dllexport)
#endif

int sqlite3_extension_init(
    sqlite3 *db,
    char **pzErrMsg,
    const sqlite3_api_routines *pApi
) {

    int rc = SQLITE_OK;
    SQLITE_EXTENSION_INIT2(pApi);

    UNUSED(pzErrMsg);

    if(rc == SQLITE_OK) {
        rc = sqlite3_create_function(db, "SetVariable", 2, SQLITE_UTF8, NULL, SetVariable, NULL, NULL);
    }

    if(rc == SQLITE_OK) {
        rc = sqlite3_create_function(db, "GetVariable", 1, SQLITE_UTF8, NULL, GetVariable, NULL, NULL);
    }

    if(rc == SQLITE_OK) {
        rc = sqlite3_exec(db, "CREATE TEMPORARY TABLE IF NOT EXISTS variable(variableName varchar(30), variableValue varchar(100))", NULL, NULL, NULL);
    }

    if(rc == SQLITE_OK) {
        rc = sqlite3_exec(db, "CREATE UNIQUE INDEX IF NOT EXISTS variableUnique ON variable(variableName)", NULL, NULL, NULL);
    }

    return rc;
}
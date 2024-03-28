#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int
dump_db (sqlite3 *db, char *filename)
{
 FILE *fp = NULL;

 sqlite3_stmt *stmt_table = NULL;
 sqlite3_stmt *stmt_data = NULL;

 const char *table_name = NULL;
 const char *data = NULL;
 int col_cnt = 0;

 int ret = 0;
 int index = 0;
 char cmd[4096] = {0};

 fp = fopen (filename, "w");
 if (!fp)
 return -1;

 ret = sqlite3_prepare_v2 (db, "SELECT sql,tbl_name FROM sqlite_master WHERE type = 'table';",
 -1, &stmt_table, NULL);
 if (ret != SQLITE_OK)
 goto EXIT;

 fprintf (fp, "PRAGMA foreign_keys=OFF;\nBEGIN TRANSACTION;\n");

 ret = sqlite3_step (stmt_table);
 while (ret == SQLITE_ROW)
 {
 data = sqlite3_column_text (stmt_table, 0);
 table_name = sqlite3_column_text (stmt_table, 1);
 if (!data || !table_name)
 {
 ret = -1;
 goto EXIT;
 }

 /* CREATE TABLE statements */
 fprintf (fp, "%s;\n", data);

 /* fetch table data */
 sprintf (cmd, "SELECT * from %s;",table_name);

 ret = sqlite3_prepare_v2 (db, cmd, -1, &stmt_data, NULL);
 if (ret != SQLITE_OK)
 goto EXIT;

 ret = sqlite3_step (stmt_data);
 while (ret == SQLITE_ROW)
 {
 sprintf (cmd, "INSERT INTO \"%s\" VALUES(",table_name);
 col_cnt = sqlite3_column_count(stmt_data);
 for (index = 0; index < col_cnt; index++)
 {
 if (index)
 strcat (cmd,",");
 data = sqlite3_column_text (stmt_data, index);

 if (data)
 {
 if (sqlite3_column_type(stmt_data, index) == SQLITE_TEXT)
 {
 strcat (cmd, "'");
 strcat (cmd, data);
 strcat (cmd, "'");
 }
 else
 {
 strcat (cmd, data);
 }
 }
 else
 strcat (cmd, "NULL");
 }
 fprintf (fp, "%s);\n", cmd);
 ret = sqlite3_step (stmt_data);
 }

 ret = sqlite3_step (stmt_table);
 }

 /* Triggers */
 if (stmt_table)
 sqlite3_finalize (stmt_table);

 ret = sqlite3_prepare_v2 (db, "SELECT sql FROM sqlite_master WHERE type = 'trigger';",
 -1, &stmt_table, NULL);
 if (ret != SQLITE_OK)
 goto EXIT;

 ret = sqlite3_step (stmt_table);
 while (ret == SQLITE_ROW)
 {
 data = sqlite3_column_text (stmt_table, 0);
 if (!data)
 {
 ret = -1;
 goto EXIT;
 }

 /* CREATE TABLE statements */
 fprintf (fp, "%s;\n", data);

 ret = sqlite3_step (stmt_table);
 }

 fprintf (fp, "COMMIT;\n");

EXIT:
 if (stmt_data)
 sqlite3_finalize (stmt_data);
 if (stmt_table)
 sqlite3_finalize (stmt_table);
 if (fp)
 fclose (fp);
 return ret;
}

// /* Main Routine */
// int
// main ()
// {
//  sqlite3 *dbc = NULL;
//  int ret = 0;

//  ret = sqlite3_open_v2 ("CML_DB.db", &dbc,
//  SQLITE_OPEN_READWRITE, NULL);

//  if (ret != SQLITE_OK || !dbc)
//  return -1;

//  system("date +%x_%H:%M:%S:%N");
//  dump_db (dbc, "sqlite_c_dump.sql");
//  system("date +%x_%H:%M:%S:%N");

//  if (dbc)
//  ret = sqlite3_close_v2 (dbc);

//  system("date +%x_%H:%M:%S:%N");
//  system ("sqlite3 CML_DB.db .dump > sqlite_native_dump.sql");
//  system("date +%x_%H:%M:%S:%N");

//  return 0;
// }
#include "pgsampler.h"

char* db_stats(void) {
  StringInfoData query;
  char* result;

  initStringInfo(&query);
  appendStringInfo(&query, 
    "SELECT pg_stat_database.datname::text, numbackends::text, xact_commit::text, xact_rollback::text, "
    "       blks_read::text, blks_hit::text, tup_returned::text, tup_fetched::text, tup_inserted::text, "
    "   tup_deleted::text, tup_updated::text, conflicts::text, temp_files::text, temp_bytes::text, deadlocks::text, "
    "       blk_read_time::text, blk_write_time::text, coalesce(stats_reset::text, ''), now()::text " 
    "   FROM pg_stat_database INNER JOIN pg_database ON pg_database.datname = pg_stat_database.datname "
    "        WHERE pg_database.datistemplate IS NOT true"
  );
 
 	result = exec_to_command("DBSTAT", query.data);
  return result;
}

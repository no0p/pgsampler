#include "pgsampler.h"

char* table_io_stats(void) {
  char* result;
	StringInfoData query;
  
  initStringInfo(&query);
  appendStringInfo(&query, 
    "SELECT '%s', schemaname, relname, relid, heap_blks_read, heap_blks_hit, idx_blks_read, idx_blks_hit, toast_blks_read, toast_blks_hit, tidx_blks_read, tidx_blks_hit, now()::text as measured_at "
    " FROM  pg_statio_user_tables", target_db
  );
  
  result = exec_to_command("TISTAT", query.data);
  
  return result;
}

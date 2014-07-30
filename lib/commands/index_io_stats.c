#include "pgsampler.h"

char* index_io_stats(void) {
  char* result;
	StringInfoData query;
  
  initStringInfo(&query);
  appendStringInfo(&query, 
    "SELECT '%s', schemaname, indexrelname, indexrelid, idx_blks_read, idx_blks_hit, now()::text as measured_at "
    " FROM  pg_statio_user_indexes", target_db
  );
  
  result = exec_to_command("IISTAT", query.data);
  
  return result;
}


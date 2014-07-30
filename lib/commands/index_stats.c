#include "pgsampler.h"

char* index_stats(void) {
  char* result;
	StringInfoData query;
	// NOTE alternative way to reach: pgstat_fetch_global()->buf_alloc
	
  initStringInfo(&query);
  appendStringInfo(&query, 
  				"SELECT '%s' as db, schemaname, indexrelname, indexrelid, idx_scan, idx_tup_read, idx_tup_fetch, now()::text as m_at"
  				"  FROM pg_stat_user_indexes", target_db
  );
  
  result = exec_to_command("IDSTAT", query.data);
	
	return result;
}

#include "pgsampler.h"

char* table_stats(void) {
  //TODO pg_relation_size PG function ... get relation disk sizes.
  char* result;
	StringInfoData query;
	// NOTE alternative way to reach: pgstat_fetch_global()->buf_alloc
	
  initStringInfo(&query);
  appendStringInfo(&query, 
  				"SELECT '%s' as db, schemaname::text, relname::text, relid::text, seq_scan::text, "
  				"  seq_tup_read::text, idx_scan::text, idx_tup_fetch::text, "
  				"  n_tup_ins::text, n_tup_upd::text, n_tup_del::text, "
  				"  n_tup_hot_upd::text, n_live_tup::text, n_dead_tup::text, "
  				"  last_vacuum::text, last_autovacuum::text, last_analyze::text, last_autoanalyze::text, "
  				"  vacuum_count::text, autovacuum_count::text, analyze_count::text, "
  				"  autoanalyze_count::text, now()::text as m_at "
  				"  FROM pg_stat_user_tables WHERE schemaname NOT IN ('pg_catalog', 'information_schema', 'pgsampler')", target_db
  );
  
  result = exec_to_command("TBSTAT", query.data);
	
	return result;
}

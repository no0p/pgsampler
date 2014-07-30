#include "pgsampler.h"


/* List of transient gucs */
char* transient_gucs(void) {
	char* result;
	StringInfoData resultbuf;
	
	//elog(LOG, "TMPGUC");
	
	initStringInfo(&resultbuf);
  appendStringInfo(&resultbuf, "TMPGUC;");

  appendStringInfoString(&resultbuf, GetConfigOption("checkpoint_segments", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  appendStringInfoString(&resultbuf, GetConfigOption("checkpoint_timeout", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  appendStringInfoString(&resultbuf, GetConfigOption("checkpoint_completion_target", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
	appendStringInfoString(&resultbuf, GetConfigOption("work_mem", true, true));
	appendStringInfo(&resultbuf, FIELD_DELIMIT);
	appendStringInfoString(&resultbuf, GetConfigOption("temp_buffers", true, true));
	appendStringInfo(&resultbuf, FIELD_DELIMIT);
	appendStringInfoString(&resultbuf, GetConfigOption("maintenance_work_mem", true, true));
	appendStringInfo(&resultbuf, FIELD_DELIMIT);
	appendStringInfoString(&resultbuf, GetConfigOption("seq_page_cost", true, true));
	appendStringInfo(&resultbuf, FIELD_DELIMIT);
	appendStringInfoString(&resultbuf, GetConfigOption("random_page_cost", true, true));
	appendStringInfo(&resultbuf, FIELD_DELIMIT);
	appendStringInfoString(&resultbuf, GetConfigOption("cpu_tuple_cost", true, true));
	appendStringInfo(&resultbuf, FIELD_DELIMIT);
	appendStringInfoString(&resultbuf, GetConfigOption("cpu_operator_cost", true, true));
	appendStringInfo(&resultbuf, FIELD_DELIMIT);
	appendStringInfoString(&resultbuf, GetConfigOption("effective_cache_size", true, true));
	appendStringInfo(&resultbuf, FIELD_DELIMIT);

  //New stuff
  appendStringInfoString(&resultbuf, GetConfigOption("vacuum_cost_delay", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  appendStringInfoString(&resultbuf, GetConfigOption("vacuum_cost_page_hit", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  appendStringInfoString(&resultbuf, GetConfigOption("vacuum_cost_page_miss", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  appendStringInfoString(&resultbuf, GetConfigOption("vacuum_cost_page_dirty", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  appendStringInfoString(&resultbuf, GetConfigOption("vacuum_cost_limit", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("bgwriter_delay", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  appendStringInfoString(&resultbuf, GetConfigOption("bgwriter_lru_maxpages", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  appendStringInfoString(&resultbuf, GetConfigOption("bgwriter_lru_multiplier", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
 
  appendStringInfoString(&resultbuf, GetConfigOption("effective_io_concurrency", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("synchronous_commit", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);

  appendStringInfoString(&resultbuf, GetConfigOption("wal_writer_delay", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);

  appendStringInfoString(&resultbuf, GetConfigOption("commit_delay", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("commit_siblings", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("wal_keep_segments", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("geqo", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("geqo_threshold", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("geqo_effort", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("geqo_pool_size", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("geqo_generations", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("geqo_selection_bias", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("geqo_seed", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("default_statistics_target", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("constraint_exclusion", true, true)); //turn off if no partitioned tables
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("cursor_tuple_fraction", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("from_collapse_limit", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("join_collapse_limit", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("autovacuum", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("autovacuum_vacuum_threshold", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("autovacuum_analyze_threshold", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("autovacuum_vacuum_scale_factor", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("autovacuum_analyze_scale_factor", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("autovacuum_vacuum_cost_delay", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("deadlock_timeout", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
	appendStringInfoString(&resultbuf, CDELIMIT);
	result = palloc(strlen(resultbuf.data) + 1);
  strcpy(result, resultbuf.data);
	return result;
}

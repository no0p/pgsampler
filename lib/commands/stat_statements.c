#include "pgsampler.h"

char* stat_statements(void) {
	StringInfoData query;
  int  retval, ntup;	
  char* result; 
 
  SetCurrentStatementStartTimestamp();
  StartTransactionCommand();
  SPI_connect();
  PushActiveSnapshot(GetTransactionSnapshot());
  
  /* Ensure compatible version */
  pgstat_report_activity(STATE_RUNNING, "Checking for pg_stat_statements");

  initStringInfo(&query);
  appendStringInfo(&query, 
    //"select * from pg_extension where extname = 'pg_stat_statements';"
    "SELECT * FROM pg_class where relname = 'pg_stat_statements'"
  );
  
  retval = SPI_execute(query.data, false, 0);  
  
  if (retval != SPI_OK_SELECT) {
    elog(FATAL, "Unable to query pg_class %d", retval);
    SPI_finish();
		PopActiveSnapshot();
		CommitTransactionCommand();
		result = pstrdup("");
  	return result;  
  }
  
  ntup = SPI_processed;

  SPI_finish();
  PopActiveSnapshot();
  CommitTransactionCommand();
  
  // If there is more than 1 result, then pg_stat_statements view exists, so query it. 
  if (ntup > 0 && strstr(GetConfigOption("shared_preload_libraries", true, true), "pg_stat_statements")) {
  
    initStringInfo(&query);
    appendStringInfo(&query, 
    "SELECT userid, dbid, query, calls, total_time, rows, shared_blks_hit, "
    "       shared_blks_read, shared_blks_dirtied, shared_blks_written, "
    "       local_blks_hit, local_blks_read, local_blks_dirtied, local_blks_written, "
    "       temp_blks_read, temp_blks_written, blk_read_time, blk_write_time, now()::text as m_at "
    "  FROM pg_stat_statements"
    );
    result = exec_to_command("STSTMT", query.data);
  } else {
    result = pstrdup("");
  }
	
	return result;
}

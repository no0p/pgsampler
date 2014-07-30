#include "pgsampler.h"

/* Send info about the list of server restart required GUCs */
char* restart_gucs(void) {
	char* result;
	char blk_size[10];
  StringInfoData resultbuf;
	sprintf(blk_size, "%d", BLCKSZ);
	
	//elog(LOG, "PRMGUC");
	
	initStringInfo(&resultbuf);
  appendStringInfo(&resultbuf, "PRMGUC;");
  
  appendStringInfoString(&resultbuf, GetConfigOption("shared_buffers", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("max_connections", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("wal_level", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, blk_size);
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("listen_addresses", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
   
  appendStringInfoString(&resultbuf, GetConfigOption("wal_buffers", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("max_wal_senders", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("autovacuum_max_workers", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("autovacuum_freeze_max_age", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("autovacuum_multixact_freeze_max_age", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("max_locks_per_transaction", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("max_pred_locks_per_transaction", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("wal_segment_size", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, GetConfigOption("data_directory", true, true));
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
	appendStringInfoString(&resultbuf, CDELIMIT);
	
	result = palloc(strlen(resultbuf.data) + 1);
  strcpy(result, resultbuf.data);
  return result;
}

#include "pgsampler.h"

char* replication_stats(void) {
  char* result;
	StringInfoData query;
  
  initStringInfo(&query);
  appendStringInfo(&query, 
    "SELECT usename, application_name, client_addr, client_hostname, client_port, "
    "   backend_start, state, sent_location, write_location, flush_location, "
    "   replay_location, sync_priority, sync_state, pg_current_xlog_location(), now()::text as measured_at "
    " FROM pg_stat_replication"
  );
  
  result = exec_to_command("RPSTAT", query.data);
  
  return result;
}

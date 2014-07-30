#include "pgsampler.h"

/* This is the main function for measuring the activity on the server.
 *
 *   Include locking information TODO join pg_stat_activity for a more complete look at activity.
 *     consider bumping this up as a potential temp replacement for pg_stat_statements, e.g. really slow when > than cycle.
*/

char* activity_stats(void) {
  char* result;
	StringInfoData query;
  
  initStringInfo(&query);
  appendStringInfo(&query, 
    "SELECT datname, pg_stat_activity.pid, usename, client_addr::text, "
    "       backend_start, query_start, state, state_change, application_name::text, "
    "       substring(query from 0 for 50), "
    "       pg_locks.locktype, pg_locks.mode, pg_locks.relation, pg_locks.page::text, "
    "       pg_locks.tuple::text, pg_locks.virtualxid::text, pg_locks.transactionid::text, "
    "       pg_locks.granted::text, pg_locks.virtualtransaction, "
    "       now()::text as measured_at "
    "  FROM pg_stat_activity "
    "    LEFT OUTER JOIN pg_locks ON pg_locks.pid = pg_stat_activity.pid "
    "      WHERE pg_stat_activity.pid != pg_backend_pid() "
  );
  
  result = exec_to_command("STAACT", query.data);
  
  return result;
	
}

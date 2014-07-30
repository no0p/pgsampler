#include "pgsampler.h"

char* function_stats(void) {
  char* result;
	StringInfoData query;
  
  initStringInfo(&query);
  appendStringInfo(&query, 
    "SELECT '%s', funcid, schemaname, funcname, calls, total_time, self_time, now()::text as measured_at "
    " FROM pg_stat_user_functions", target_db
  );
  
  result = exec_to_command("FNSTAT", query.data);
  
  return result;
}

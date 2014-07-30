#include "pgsampler.h"

char* index_info(void) {
  char* result;
	StringInfoData query;
  
  initStringInfo(&query);
  appendStringInfo(&query, 
    "SELECT '%s', indexrelid, indrelid, indnatts, indkey, indisunique"
    " FROM pg_index "
    "   INNER JOIN pg_class ON pg_class.oid = pg_index.indexrelid "
    "   INNER JOIN pg_namespace on pg_namespace.oid = pg_class.relnamespace" 
    " WHERE nspname not in ('pg_toast', 'information_schema', 'pg_catalog')  ", target_db
  );
  
  result = exec_to_command("INDNFO", query.data);
  
  return result;
}

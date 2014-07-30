#include "pgsampler.h"

char* relation_info(void) {
  char* result;
  StringInfoData query;
  
  initStringInfo(&query);
  appendStringInfo(&query, 
    "SELECT '%s' as database_name, pg_namespace.nspname::text, relname, "
    "       pg_class.oid, relkind, relpersistence, pg_am.amname,  "
    "       relpages, reltuples::bigint::text, relallvisible, relchecks, relhaspkey, "
    "       relhastriggers, pg_relation_size(pg_class.oid), pg_indexes_size(pg_class.oid), now()::text as m_at "
    	" FROM pg_class "
      "   INNER JOIN pg_namespace on pg_namespace.oid = pg_class.relnamespace "
      "   LEFT OUTER JOIN pg_am ON pg_am.oid = pg_class.relam "
      " WHERE relisshared IS NOT TRUE AND nspname not in ('pg_toast', 'information_schema', 'pg_catalog')", target_db
  );
  
  result = exec_to_command("RELNFO", query.data);
 
  return result;
}

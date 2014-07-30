#include "pgsampler.h"

/* Information for populating tables and columns */
char* column_info(void) {
  char* result;
  StringInfoData query;
  
  initStringInfo(&query);
  appendStringInfo(&query, 
    "SELECT '%s' as database_name, pg_tables.schemaname, pg_tables.tablename, pg_attribute.attname, "
    "       pg_type.typname, pg_class.oid as table_oid, attnotnull::text, attstattarget::text,  attnum"
    	" FROM pg_tables" 
      "   INNER JOIN pg_class on pg_class.relname = pg_tables.tablename"
      "   INNER JOIN pg_attribute on pg_attribute.attrelid = pg_class.oid" 
      "   INNER JOIN pg_type on pg_type.oid = pg_attribute.atttypid"
      "     WHERE schemaname NOT IN ('pg_catalog', 'information_schema', 'pgsampler', 'pg_toast');", target_db
  );
  
  result = exec_to_command("COLNFO", query.data);
 
  return result;
}

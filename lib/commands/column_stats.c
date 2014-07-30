#include "pgsampler.h"

char* column_stats(void) {
  StringInfoData query;
  char* result;

  initStringInfo(&query);
  appendStringInfo(&query, 
    "SELECT '%s', schemaname, tablename, attname, null_frac, avg_width, n_distinct, "
    "       most_common_vals, most_common_freqs, correlation, now()::text "
    "  FROM pg_stats "
    "    WHERE schemaname NOT IN ('pg_catalog', 'information_schema', 'pgsampler');", target_db
  );
  
 	result = exec_to_command("COLSTA", query.data);
  return result;
}

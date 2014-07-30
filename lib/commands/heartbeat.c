#include "pgsampler.h"

char* heartbeat(void) {
  char* result;

  StringInfoData query;
  
  /* // buf contains info.
  struct statvfs buf;
  char* filename = "/";
  statvfs(filename, &buf); */
  
  
  initStringInfo(&query);
  appendStringInfo(&query, 
    "SELECT txid_current()::text as curtx, now()::text as m_at "
  );
  
  result = exec_to_command("HARTBT", query.data);

  return result;
}

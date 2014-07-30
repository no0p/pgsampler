#include "pgsampler.h"

/**/
char* handshake(void) {
  char* result;
  StringInfoData resultbuf;
	
  // elog(LOG, "HNDSHK"); //debug info
  
  initStringInfo(&resultbuf);
  appendStringInfo(&resultbuf, "HNDSHK;");
  
  appendStringInfoString(&resultbuf, token);
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  appendStringInfoString(&resultbuf, PG_VERSION);
  
  appendStringInfo(&resultbuf, CDELIMIT);
  
  //elog(LOG, "%s", resultbuf.data);
  
  result = pstrdup(resultbuf.data);
  return result;

}

#include "pgsampler.h"

/*
 * Write the data to a file.
 *
*/

int write_to_csv(char* command, char* data) {
  FILE *f;
  StringInfoData buf;
  
  initStringInfo(&buf);
  appendStringInfoString(&buf, output_csv_dir);
  // TODO consider appending "/" if not last char.
  appendStringInfoString(&buf, command);
  appendStringInfo(&buf, ".csv");
  
  f = fopen(buf.data, "a+");
  
  if (f == NULL) {
    elog(LOG, "Unable to open csv file.");
    return 1;
  }
  
  fprintf(f, "%s", data);
  fclose(f);
  
  return 0;

}

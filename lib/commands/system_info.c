#include "pgsampler.h"

char* system_info(void) {
	char* result;
	char str_buf[30];
  StringInfoData resultbuf;
  float one_min_avg, five_min_avg, fifteen_min_avg;
	int waiting_procs, total_procs, last_pid;
	int scan_ret;
	FILE *file;
	char line[512];
	char swap_total[30] = "";
	char swap_free[30] = "";
	char buffers[30] = "";
	char cached[30] = "";
	char swap_cached[30] = "";

	initStringInfo(&resultbuf);
  appendStringInfo(&resultbuf, "SYSNFO;");

	// Page Size
  sprintf(str_buf, "%ld", (long) sysconf(_SC_PAGESIZE));
	appendStringInfoString(&resultbuf, str_buf);
	appendStringInfo(&resultbuf, FIELD_DELIMIT);
	
	// Total Pages
  sprintf(str_buf, "%ld", (long) sysconf(_SC_PHYS_PAGES));
	appendStringInfoString(&resultbuf, str_buf);
	appendStringInfo(&resultbuf, FIELD_DELIMIT);
	
	// Available Pages
  sprintf(str_buf, "%ld", (long) sysconf(_SC_AVPHYS_PAGES));
	appendStringInfoString(&resultbuf, str_buf);
	appendStringInfo(&resultbuf, FIELD_DELIMIT);
	
	// Num Online Processors
  sprintf(str_buf, "%ld", (long) sysconf(_SC_NPROCESSORS_ONLN));
	appendStringInfoString(&resultbuf, str_buf);
	appendStringInfo(&resultbuf, FIELD_DELIMIT);
	
	
	/* load information */
  file = fopen("/proc/loadavg", "r");
  if (file != NULL) {
	  scan_ret = fscanf(file, "%f %f %f %d/%d %d", &one_min_avg, &five_min_avg, &fifteen_min_avg, &waiting_procs, &total_procs, &last_pid);
  	if (scan_ret == 6) {
  		sprintf(str_buf, "%f", (float) one_min_avg);
      appendStringInfoString(&resultbuf, str_buf);
  	} 
  }
  fclose(file);
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
	
	
	/* load information */
  file = fopen("/proc/meminfo", "r");
  if (file != NULL) {
    while(fgets(line, 512, file) != NULL) {
      scan_ret = sscanf(line, "SwapTotal: %s kB", swap_total);
      scan_ret = sscanf(line, "SwapFree: %s kB", swap_free);
      scan_ret = sscanf(line, "Cached: %s kB", cached);
      scan_ret = sscanf(line, "Buffers: %s kB", buffers);
      scan_ret = sscanf(line, "SwapCached: %s kB", swap_cached);
    }
  }
  fclose(file);
  
  appendStringInfoString(&resultbuf, swap_total); //in kilobytes
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
	
	appendStringInfoString(&resultbuf, swap_free); //in kilobytes
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, swap_cached);
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
  appendStringInfoString(&resultbuf, buffers);
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
	
	appendStringInfoString(&resultbuf, cached);
  appendStringInfo(&resultbuf, FIELD_DELIMIT);
  
	appendStringInfoString(&resultbuf, CDELIMIT);
  result = pstrdup(resultbuf.data);    
  
	return result;
}

/*
 * This file contains the main control loop for the pgsampler bg worker.
 *
 *   The pgsampler.c file contains a loop triggered by a wait latch every N seconds between work.
 *  
 *    That loop calls the collect_and_send_metrics function which will execute the commands
 *     to collect data to send to antenna.pgsampler.io.  Then of course, it sends the data.
 *
 *    Some key aspects to this process is a cycle counter parameter which is used to 
 *      limit how often certain commands are run.
 */


#include "pgsampler.h"

/*
 * This is the function called from the main polling loop.
 *
 * It calls command functions to get strings of data, and sends them to the server.
 *
*/
int collect_and_send_metrics(int cycle) {
  int retval;
  char* command;
  
  StringInfoData commands;
  
  pgstat_report_activity(STATE_RUNNING, "Collecting metrics");
  
  initStringInfo(&commands);
  
  /*
   * Populate first cycle command data.  These are executed on the first cycle
   *   of a restart.  The bgworker restars every N cycles, as listed at the bottom
   *   of the main loop in pgsampler.c.  
   */
  if (cycle == 0) {

		elog(LOG, "Cycle...");

  	command = restart_gucs();
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("restart_gucs", command);      
		pfree(command);
			
		command = relation_info();
		appendStringInfoString(&commands, command);
		if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("relation_info", command);
		pfree(command);
		
		command = database_list();
		appendStringInfoString(&commands, command);
		if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("database_list", command);
		pfree(command);
		
		command = column_info();
		appendStringInfoString(&commands, command);
		if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("column_info", command);
		pfree(command);
		
		command = index_info();
  	appendStringInfoString(&commands, command);
  	if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("index_info", command);
		pfree(command);
		
		command = column_stats();
		appendStringInfoString(&commands, command);
		if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("column_stats", command);
		pfree(command);
		
		command = stat_statements();
		appendStringInfoString(&commands, command);
		if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("stat_statements", command);
		pfree(command);
		
		command = db_stats();
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("db_stats", command);
    pfree(command);
		
  }
  
  /* HEARTBEAT */
  if (cycle % heartbeat_seconds == 0) {
    command = heartbeat();
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("heartbeats", command);
    pfree(command);
  }
  
  /* SYSTEM INFO */
  if (cycle % system_seconds == 0) {
    command = system_info();  
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("system_info", command);
    pfree(command);
    
    command = fs_info();
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("filesystem_stats", command);
    pfree(command);
    
  }
  
  /* */
  if (cycle % activity_seconds == 0) {
    command = activity_stats(); 
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("activity_stats", command);
    pfree(command);
  }
	
  if (cycle % replication_seconds == 0) {
    command = replication_stats(); 
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("replication_stats", command);
    pfree(command);
  }
	
	/*  */
  if (cycle % bgwriter_seconds == 0) {
    command = bgwriter_stats();
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("bgwriter_stats", command);
    pfree(command);
  }
  
  if (cycle % guc_seconds == 0) {
    command = transient_gucs();
  	appendStringInfoString(&commands, command);
  	if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("transient_gucs", command);
  	pfree(command);
  }
  
  /* */  
  if (cycle % relation_seconds == 0) {
  	command = table_stats();
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("table_stats", command);
    pfree(command);
    
    command = index_stats();
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("index_stats", command);
    pfree(command);
    
    command = table_io_stats();
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("table_io_stats", command);
    pfree(command);
    
    command = index_io_stats();
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("index_io_stats", command);
    pfree(command);
    
    command = function_stats();
    appendStringInfoString(&commands, command);
    if (strcmp(output_mode, "csv") == 0) 
      write_to_csv("function_stats", command);
    pfree(command);
  }
  
  
  

  /* Send / Write metrics based on output_mode */
  if (strcmp(output_mode, "network") == 0) {
    pgstat_report_activity(STATE_RUNNING, "Sending metrics");
    retval = send_data(commands.data);
    if (retval == NO_DATA_SENT) { //close socket and retry establishing connection and sending data.
	   	// elog(LOG, "reseting...");  	//just a note to say reseting socket
    	if (sockfd != 0) 
    	  shutdown(sockfd, SHUT_RDWR);
    	  
    	sockfd = 0;
    	retval = send_data(commands.data); // we ignore success or failure here.  drops data if fails.
    }
  }
  
  return 0;
}






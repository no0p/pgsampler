#include "pgsampler.h"


/* 
 * This function receives a command names and a query and it produces a protocol valid command string
 *   which can be send to the server based on the format of the result set.  It handles memory contexts
 *   appropriately, returning a pointer to the command string in the current context when called.
 *
 */
char* exec_to_command(const char* command, char* q) {
  StringInfoData resultbuf;
  char* result;
  int i, j, processed, retval;
  SPITupleTable *coltuptable;
  MemoryContext pre_context;
  MemoryContext spi_conn_context;
  
  // elog(LOG, "%s", command); //prints the current command running
  
  pre_context = CurrentMemoryContext;
  
  SetCurrentStatementStartTimestamp();
  StartTransactionCommand();
  SPI_connect();
  PushActiveSnapshot(GetTransactionSnapshot());
  
  retval = SPI_execute(q, false, 0);
  if (retval != SPI_OK_SELECT) {
    elog(LOG, "Database SELECT execution failed");
    SPI_finish();
		PopActiveSnapshot();
		CommitTransactionCommand();
		result = pstrdup("");
		return result;
  }  
  
  processed = SPI_processed;
  coltuptable = SPI_tuptable;
  
  initStringInfo(&resultbuf);

  appendStringInfoString(&resultbuf, command);
  appendStringInfoString(&resultbuf, ";"); //artisinal semicolon

  
  if (coltuptable != NULL) {
    for(i = 0; i < processed; i++) {
      for(j = 1; j <= coltuptable->tupdesc->natts; j++) {
  
  	    if (SPI_getvalue(coltuptable->vals[i], coltuptable->tupdesc, j) != NULL) {
    	    appendStringInfoString(&resultbuf, SPI_getvalue(coltuptable->vals[i], coltuptable->tupdesc, j));
    	  }
		    appendStringInfo(&resultbuf, FIELD_DELIMIT);
  		  
      }
      
      appendStringInfo(&resultbuf,REC_DELIMIT);
  
    }
  }
  

  appendStringInfo(&resultbuf, CDELIMIT);
  
  spi_conn_context = MemoryContextSwitchTo(pre_context);
  result = pstrdup(resultbuf.data);
  MemoryContextSwitchTo(spi_conn_context);

  SPI_finish();
  PopActiveSnapshot();
  CommitTransactionCommand();
  return result;
}

/*
 *  Ensure that the environment is sane.  
 *    This involves checking the Postgresql version, and if in network mode
 *      also establishing a connection to a receiver.
*/
int ensure_valid_environment(void) {
  StringInfoData buf;
  int     retval;
	char* pgversion;
	
  SPITupleTable *coltuptable;
 
  SetCurrentStatementStartTimestamp();
  StartTransactionCommand();
  SPI_connect();
  PushActiveSnapshot(GetTransactionSnapshot());
  
  /* Ensure compatible version */
  pgstat_report_activity(STATE_RUNNING, "verifying compatible postgres version");

  initStringInfo(&buf);
  appendStringInfo(&buf, 
    "select version();"
  );
  retval = SPI_execute(buf.data, false, 0);
  if (retval != SPI_OK_SELECT) {
    elog(FATAL, "Unable to query postgres version %d", retval);
    SPI_finish();
		PopActiveSnapshot();
		CommitTransactionCommand();
  	return 1;  
  }
  
	coltuptable = SPI_tuptable;
	pgversion = SPI_getvalue(coltuptable->vals[0], coltuptable->tupdesc, 1);
	
  if(strstr(pgversion, "PostgreSQL 9.3") == NULL) {
    elog(FATAL, "Unsupported Postgresql version");
    SPI_finish();
		PopActiveSnapshot();
		CommitTransactionCommand();
  	return 1;
	}
  
	SPI_finish();
	PopActiveSnapshot();
	CommitTransactionCommand();
  
  /*
   * Attempt to establish a connection if the output mode is network.
   */
  if (strcmp(output_mode, "network") == 0) {
		retval = establish_connection();
		if (retval == 2) {
			elog(LOG, "Error : Failed to connect to antenna.pgsampler.io please check domain is available from host.");
		}
	}
	
  elog(LOG, "Pgsampler Initialized");
  return 0;
}

/*
 * This function will set a string in shared memory which is the name of the database to connect to
 *  the next time the background worker restarts.  Because a bgworker can only connect to one database
 *  at a time, and some catalogs and stats are scoped to the current database, the bg worker
 *  periodically restarts to collect latest stats from another database.
 *
*/
int set_next_db_target(void) {
	int retval, processed;
	StringInfoData buf;
	SPITupleTable *coltuptable;
	char* next_db_target;
 
 
  SetCurrentStatementStartTimestamp();
  StartTransactionCommand();
  SPI_connect();
  PushActiveSnapshot(GetTransactionSnapshot());
  
  /* get sorted list of databases, find one after target_db*/
  initStringInfo(&buf);
  appendStringInfo(&buf, 
    "SELECT datname FROM pg_database WHERE datname NOT IN ('template0', 'template1') AND datallowconn IS TRUE AND datname > '%s' ORDER BY datname ASC LIMIT 1;", target_db
  );
  
  retval = SPI_execute(buf.data, false, 0);
  if (retval != SPI_OK_SELECT) {
    elog(FATAL, "Database information collection failed");
    // FAIL RETURN 1
  }  
  processed = SPI_processed;
  
  if(processed == 0) {
    //No matching records so pick first database.
    resetStringInfo(&buf);
    appendStringInfoString(&buf, 
      "SELECT datname FROM pg_database WHERE datname NOT IN ('template0', 'template1') AND datallowconn IS TRUE ORDER BY datname ASC LIMIT 1;"
    );

    retval = SPI_execute(buf.data, false, 0);

    if (retval != SPI_OK_SELECT) {
      elog(FATAL, "Database information collection failed");
      // FAIL RETURN 1
    }
  }
  
  coltuptable = SPI_tuptable;  
  next_db_target = SPI_getvalue(coltuptable->vals[0], coltuptable->tupdesc, 1);
  // elog(LOG, "NEXTDB TARGET: %s", next_db_target); //print next target db
  strcpy(pgsampler_state->next_db, next_db_target);
  
	SPI_finish();
	PopActiveSnapshot();
	CommitTransactionCommand(); 
  
  return 0;
}

/*
 * Returns a count of the number of non-template databases from the catalog.
 */
int get_database_count(void) {
  int retval, processed;
	StringInfoData buf;
	SPITupleTable *coltuptable;
	int database_count = 0;
  
  SetCurrentStatementStartTimestamp();
  StartTransactionCommand();
  SPI_connect();
  PushActiveSnapshot(GetTransactionSnapshot());
  
  initStringInfo(&buf);
  appendStringInfo(&buf, "SELECT count(*) FROM pg_database WHERE datname NOT IN ('template0', 'template1') AND datallowconn IS TRUE;");
  
  retval = SPI_execute(buf.data, false, 0);
  if (retval != SPI_OK_SELECT) {
    elog(FATAL, "Database information collection failed");
    // FAIL RETURN 1
  }  
  processed = SPI_processed;
  
  if (processed > 0) {
    coltuptable = SPI_tuptable;  
    database_count = atoi(SPI_getvalue(coltuptable->vals[0], coltuptable->tupdesc, 1));
  }
  
  SPI_finish();
	PopActiveSnapshot();
	CommitTransactionCommand(); 
  
  return database_count;
}

/*
 * Takes a string representing a command string containing information to that
 *   would be sent over a socket in network mode, and converts it to a comma
 *   separated value string for writing to a csv file. *   
 */
char* csvify(char *command_string) {
	char* result;
	StringInfoData resultbuf;
	int i = 0;
	
	initStringInfo(&resultbuf);
 
	while (*command_string != '\0') {

		// Skip command header
		if (i > 6) {
			if (*command_string == FIELD_DELIMIT_CHAR) {
				appendStringInfoChar(&resultbuf, ',');
		  } else if (*command_string == '\n') {
				//noop
			} else if (*command_string == REC_DELIMIT_CHAR) {
				appendStringInfoChar(&resultbuf, '\n');
			} else if (*command_string == '\r') {
				//noop
			} else {
				appendStringInfoChar(&resultbuf, *command_string);
			}
		}
		command_string++;
		i++;
	}

	result = pstrdup(resultbuf.data);
	return result;
}





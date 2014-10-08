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
    elog(FATAL, "Database SELECT execution failed");
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
 *    This involves checking the Postgresql version,
 *  	identifying a valid token, and establishing a connection
 *		to a receiver.
*/
int ensure_valid_environment(void) {
  StringInfoData buf;
  StringInfoData token_filename;
  int     retval, ntup;
	char* pgversion;
	char* token_result;
	FILE *f;
	char token_line[40];
	
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
    
  /* 
   * Identify valid token
   *
   * Once the token variable is set, we're good.
   *   It may already be set from a GUC. 
   *   If it is not set, attempt to load the token from a token file.
   *   If there is no token file, or there is no valid token in a file,
   *   generate a token and store it in the file and set the token variable.
   *
   *	If this all fails, return a result which will result in process exit 
   *    without restart.
   */
  pgstat_report_activity(STATE_RUNNING, "Ascertaining identity token.");
  if (token == NULL || strcmp(token, "") == 0) {
  	/* Check for a token file */
  	elog(LOG, "Checking for token file");
  	initStringInfo(&token_filename);
  	appendStringInfoString(&token_filename, GetConfigOption("data_directory", true, true));
  	appendStringInfoString(&token_filename, "/pgsampler.token");
  	elog(LOG, "opening file %s", token_filename.data);
  	f = fopen(token_filename.data, "r");
  	if (f != NULL) {
  		elog(LOG, "fgetting....");
  		if (fgets(token_line, 40, f) != NULL) {
				elog(LOG, "read line token file: %s", token_line);
				if (strlen(token) == 32) { // TODO Check valid.
					elog(LOG, "Copying token to guc variable");
					strcpy(token, token_line);
				}
			}
			elog(LOG, "closing token file");
			fclose(f);
  	}
  	elog(LOG, "closed..");
  }
  
  if (token == NULL || strcmp(token, "") == 0) {
  	elog(LOG, "Generating token");
  	/* generate and save a token */
	  pgstat_report_activity(STATE_RUNNING, "creating custom pgsampler token");
		resetStringInfo(&buf);
		appendStringInfo(&buf, "select md5(now()::text);");
	
		retval = SPI_execute(buf.data, false, 0);  
		if (retval != SPI_OK_SELECT) {
			elog(FATAL, "Unable to generate a token %d", retval);
			SPI_finish();
			PopActiveSnapshot();
			CommitTransactionCommand();
			return 1;  
		}
	
		ntup = SPI_processed;
	
		if (ntup > 0) {
			elog(LOG, "Setting custom token");
			// Custom token was present at startup.  Apply it.
			coltuptable = SPI_tuptable;
			token_result = SPI_getvalue(coltuptable->vals[0], coltuptable->tupdesc, 1);
			//token = malloc(strlen(token_result) + 1); //TODO put in proper context, char[34] declaration maybe
			strcpy(token, token_result);
			
			// Now write token to file for future reference
			f = fopen(token_filename.data, "w");
			if (f != NULL) {
    		fprintf(f, "%s", token); 
    	}
    	fclose(f);
		}
  }
  
  elog(LOG, "Reality check");
  /* The reality check, either a token has been identified or not. */
  if(token == NULL || strlen(token) != 32) {
  	elog(FATAL, "No valid token.  Token must must be a 32 character string.");
  	SPI_finish();
		PopActiveSnapshot();
		CommitTransactionCommand();
  	return 1;
  }

  elog(LOG, "Pgsampler Initialized");
  elog(LOG, "TOKEN: %s", token);
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

char* csvify(char *s) {
	char* result;
	StringInfoData resultbuf;
	int i = 0;
	
	initStringInfo(&resultbuf);
 
	while (*s != '\0') {
		// Skip command header
		if (i > 6) {
			if (*s == FIELD_DELIMIT_CHAR) {
				appendStringInfoChar(&resultbuf, ',');
		  } else if (*s == '\n') {
				//noop
			} else if (*s == REC_DELIMIT_CHAR) {
				appendStringInfoChar(&resultbuf, '\n');
			} else if (*s == '\r') {
				//noop
			} else {
				appendStringInfoChar(&resultbuf, *s);
			}
		}
		s++;
		i++;
	}
	//elog(LOG, "%s", resultbuf.data);
	result = pstrdup(resultbuf.data);
	return result;
}





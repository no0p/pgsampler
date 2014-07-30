#include "pgsampler.h"

/*The default pgsampler token for this build*/
char* default_token = "DEFAULT60f6c631b4b77a2414d85191b";

/* Essential for shared libs! */
PG_MODULE_MAGIC;
PG_FUNCTION_INFO_V1(pgsampler_launch);
PG_FUNCTION_INFO_V1(predict);

/* Signal handling */
volatile sig_atomic_t got_sigterm = false;
volatile sig_atomic_t got_sighup = false;

/* This is the main execution loop for the pgsampler module */
void pgsampler_main(Datum main_arg) {

	bool found;
	int res;
	int cycle = 0;
	
	/* global defaults */
  sockfd = 0; 
  restart_cycle = 0;
	

  pqsignal(SIGTERM, pgsampler_sigterm);
  pqsignal(SIGHUP, pgsampler_sighup);
  BackgroundWorkerUnblockSignals();
 
 	/* Check and set shared memory state for info passing between process cycles */
	LWLockAcquire(AddinShmemInitLock, LW_EXCLUSIVE);
	pgsampler_state = ShmemInitStruct("pgsampler_state", sizeof(pgsampler_shared_state), &found);
	LWLockRelease(AddinShmemInitLock); //TODO consider putting this later after setting db
	if (found) {
		if (pgsampler_state->next_db != NULL && strlen(pgsampler_state->next_db) > 0) {
			target_db = pstrdup(pgsampler_state->next_db);//TODO consider whether make a set length char[] re memory leaks
		}
		if (pgsampler_state->valid_token != NULL && strlen(pgsampler_state->valid_token) > 0) {
		  token = pgsampler_state->valid_token;
		}
		
    restart_cycle = ++pgsampler_state->restart_cycle;
    if (restart_cycle > 32000) {
      pgsampler_state->restart_cycle = 0;
    }

	} else {
	  pgsampler_state->restart_cycle = 0; // initialize restart cycle since database was restarted
	}
 
  /* Connect to target database */
  BackgroundWorkerInitializeConnection(target_db, NULL);

	/* Set next target db */
	set_next_db_target();

  
  /* 
   *  Ensure meta tables present, ensure network is sane
   *  The token is always null unless this is a controlled restart.    
   */
  if (token == NULL) {
	  res = ensure_valid_environment();
	  if (res != 0) {
		  elog(LOG, "Failed to find appropriate environment.  Shutting down pgsampler monitoring for Safety.");
		  proc_exit(1);
	  }
	}
	
	/* Set database count to drive fast restarts */
	pgsampler_database_count = get_database_count();
	
  
  /* Main loop, periodically check if should rebuild models */  
  while (!got_sigterm) {
    int rc;

		
		pgstat_report_activity(STATE_RUNNING, "idling in polling loop");

    rc = WaitLatch(&MyProc->procLatch, WL_LATCH_SET | WL_TIMEOUT | WL_POSTMASTER_DEATH, 1000L); //TODO consider adding delay function which accounts for work done.
    ResetLatch(&MyProc->procLatch);
    
    if (rc & WL_POSTMASTER_DEATH) {
      elog(LOG, "pgsampler terminating!");
      proc_exit(1);
    }

    collect_and_send_metrics(cycle); 

    cycle++;
    if (cycle_db_seconds > 0) { // GUC controlling whether to disconnect and reconnect
      if (cycle > cycle_db_seconds || (restart_cycle < pgsampler_database_count) ) {
      	elog(LOG, "Relational Systems restarting to connect to different database: normal exit.");
      	proc_exit(0); // RESET so new database is connected to.
     	}
    }
  }
  
  proc_exit(0);
}


/* Signal Handlers*/
void pgsampler_sigterm(SIGNAL_ARGS) { //TODO send a packet to trigger an alarm if possible!
  int save_errno = errno;
  got_sigterm = true;
  if (MyProc)
    SetLatch(&MyProc->procLatch);
  errno = save_errno;
}

void pgsampler_sighup(SIGNAL_ARGS) {
  got_sighup = true;
  if (MyProc)
    SetLatch(&MyProc->procLatch);
}



/* This is callback execute when creating the extension.  It registers the background worker
   In the future may make sense to start the background worker at this point, so as not to require
   a server restart.
 */
void _PG_init(void) {
  BackgroundWorker worker;


  /* Main database to connect to. */
  DefineCustomStringVariable("pgsampler.target_db",
                              "Database to connect to first",
                              NULL,
                              &target_db,
                              "postgres",
                              PGC_SIGHUP,
                              0,
                              NULL,
                              NULL,
                              NULL);
  
  DefineCustomIntVariable("pgsampler.cycle_db_seconds",
                            "Duration between each heartbeat sample.",
                            NULL,
                            &cycle_db_seconds,
                            500,
                            -1,
                            INT_MAX,
                            PGC_SIGHUP,
                            0,
                            NULL,
                            NULL,
                            NULL);                     
		                        
                           
  /* The mode to run pgsampler in, accepted values include 'csv', 'dbtables', 'network'*/                         
  DefineCustomStringVariable("pgsampler.output_mode",
                              "The mode to write pgsampler data (csv or network).",
                              NULL,
                              &output_mode,
                              "csv",
                              PGC_SIGHUP,
                              0,
                              NULL,
                              NULL,
                              NULL);
                              
  /* The hostname to send sample data to for network mode */                            
  DefineCustomStringVariable("pgsampler.output_network_host",
                              "pgsampler send data to this host.",
                              NULL,
                              &output_network_host,
                              "antenna.relsys.io",
                              PGC_SIGHUP,
                              0,
                              NULL,
                              NULL,
                              NULL);
  
  /* The hostname to send sample data to for network mode */                            
  DefineCustomStringVariable("pgsampler.output_csv_dir",
                              "pgsampler output directory for csv data.",
                              NULL,
                              &output_csv_dir,
                              "/tmp/",
                              PGC_SIGHUP,
                              0,
                              NULL,
                              NULL,
                              NULL);
                              
  //Individual timers                
  DefineCustomIntVariable("pgsampler.heartbeat_seconds",
                            "Duration between each heartbeat sample.",
                            NULL,
                            &heartbeat_seconds,
                            5,
                            1,
                            INT_MAX,
                            PGC_SIGHUP,
                            0,
                            NULL,
                            NULL,
                            NULL);
                            
  DefineCustomIntVariable("pgsampler.system_seconds",
                            "Duration between stats for filesystems and memory/cpu",
                            NULL,
                            &system_seconds,
                            10,
                            1,
                            INT_MAX,
                            PGC_SIGHUP,
                            0,
                            NULL,
                            NULL,
                            NULL);
  
  DefineCustomIntVariable("pgsampler.relation_seconds",
                            "Duration between stats for index/table",
                            NULL,
                            &relation_seconds,
                            150,
                            1,
                            INT_MAX,
                            PGC_SIGHUP,
                            0,
                            NULL,
                            NULL,
                            NULL);
                            
  DefineCustomIntVariable("pgsampler.bgwriter_seconds",
                            "Duration between stats for bgwriter",
                            NULL,
                            &bgwriter_seconds,
                            30,
                            1,
                            INT_MAX,
                            PGC_SIGHUP,
                            0,
                            NULL,
                            NULL,
                            NULL);
                            
  DefineCustomIntVariable("pgsampler.guc_seconds",
                            "Duration between recording gucs",
                            NULL,
                            &guc_seconds,
                            300,
                            1,
                            INT_MAX,
                            PGC_SIGHUP,
                            0,
                            NULL,
                            NULL,
                            NULL);
  
  DefineCustomIntVariable("pgsampler.activity_seconds",
                            "Duration between sampling pgstatactivity",
                            NULL,
                            &activity_seconds,
                            5,
                            1,
                            INT_MAX,
                            PGC_SIGHUP,
                            0,
                            NULL,
                            NULL,
                            NULL);
                            
  DefineCustomIntVariable("pgsampler.replication_seconds",
                            "Duration between recording replication stats",
                            NULL,
                            &replication_seconds,
                            300,
                            1,
                            INT_MAX,
                            PGC_SIGHUP,
                            0,
                            NULL,
                            NULL,
                            NULL);
  
  worker.bgw_flags = BGWORKER_SHMEM_ACCESS | BGWORKER_BACKEND_DATABASE_CONNECTION;
  worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
  worker.bgw_main = pgsampler_main;

  snprintf(worker.bgw_name, BGW_MAXLEN, "pgsampler");
  worker.bgw_restart_time = BGW_NEVER_RESTART;
  worker.bgw_main_arg = (Datum) 0;
  RegisterBackgroundWorker(&worker);
}




#ifndef ALPINE_H
#define ALPINE_H
#endif

#include "postgres.h"
#include "miscadmin.h"
#include "postmaster/bgworker.h"
#include "storage/ipc.h"
#include "storage/latch.h"
#include "storage/lwlock.h"
#include "storage/proc.h"
#include "storage/shmem.h"

#include "access/xact.h"
#include "executor/spi.h"
#include "fmgr.h"
#include "lib/stringinfo.h"
#include "pgstat.h"
#include "utils/builtins.h"
#include "utils/snapmgr.h"
#include "tcop/utility.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <errno.h>
//#include <string.h>

//#include <netinet/in.h>
#include <sys/types.h>

#include <sys/socket.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

// Filesystem utils
#include <sys/statvfs.h>
#include <mntent.h>


#define PORT 24831
#define TIMEOUT_SECS 3

#define NO_DATA_SENT 1

#define CDELIMIT "\r\n"
#define REC_DELIMIT ""
#define FIELD_DELIMIT ""
#define REC_DELIMIT_CHAR ''
#define FIELD_DELIMIT_CHAR ''

typedef struct pgsampler_shared_state
{
	char next_db[64];			/* Next Database to connect to. */
	char valid_token[33];
	int restart_cycle;
} pgsampler_shared_state;

/* Core Background worker functions and data*/
void _PG_init(void);

void pgsampler_main(Datum);
void pgsampler_sigterm(SIGNAL_ARGS);
void pgsampler_sighup(SIGNAL_ARGS);

Datum pgsampler_launch(PG_FUNCTION_ARGS);
Datum predict(PG_FUNCTION_ARGS);
char* csvify(char *s); 

/* GUC variables */
char* target_db;
char* output_mode;
char* output_network_host;
char* output_csv_dir;
bool record_data;
int cycle_db_seconds;

// Individual timers 
int heartbeat_seconds;
int system_seconds;
int relation_seconds;
int bgwriter_seconds;
int guc_seconds;
int activity_seconds;
int replication_seconds;


/* Globals */
char* token;
char* default_token;

/* 
 * restart_cycle & pgsampler_database_count
 *
 * We Keep track of the cycles of restarts.  This is to treat a special case of
 * initialization after database has just been restarted.  Maybe it is the 
 * first run, maybe not.  
 *
 * Either way we keep track to cycle through all databases quickly collecting basic
 *   info that doesn't change very often or is very voluminous on this condition.
 *	 
 */
int restart_cycle;
int pgsampler_database_count;

int sockfd;
pgsampler_shared_state* pgsampler_state;


/* Control Functions */
int collect_and_send_metrics(int);

int ensure_valid_environment(void);
int set_next_db_target(void);

/* Communication Functions */
int establish_connection(void);
int send_data(char*);

/* Command Functions -- return protocol valid string to send to server or file */
char* handshake(void);

char* heartbeat(void);

char* restart_gucs(void);
char* transient_gucs(void);

char* system_info(void);
char* fs_info(void);

char* column_info(void);
char* relation_info(void);
char* database_list(void);

char* activity_stats(void);
char* db_stats(void);
char* table_stats(void);
char* bgwriter_stats(void);
char* index_stats(void);
char* function_stats(void);
char* table_io_stats(void);
char* index_io_stats(void);
char* replication_stats(void);
char* column_stats(void);
char* index_info(void);

char* stat_statements(void);

/* Utility Functions */
char* exec_to_command(const char*, char*);
int get_database_count(void);
int write_to_csv(char*, char*);




#include "pgsampler.h"

//Include NBUFFERS in this payload.
char* bgwriter_stats(void) {
	char* result;
	StringInfoData query;
	// NOTE alternative way to reach: pgstat_fetch_global()->buf_alloc
	
  initStringInfo(&query);
  appendStringInfo(&query, 
  				"SELECT checkpoints_timed, checkpoints_req, checkpoint_write_time, "
  				" 	checkpoint_sync_time, buffers_checkpoint, buffers_clean, maxwritten_clean, "
  				"		buffers_backend, buffers_backend_fsync, buffers_alloc, stats_reset, now()::text as m_at "
  				"  FROM pg_stat_bgwriter"
  );
  
  result = exec_to_command("BGSTAT", query.data);
	
	return result;
}

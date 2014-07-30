## pgsampler

Pgsampler is a background worker that takes samples of database statistics.

The statistics can be saved to CSV files or shipped over a network connection to a receiving service.  The sampler can be setup to send statistics to http://relsys.io for monitoring purposes.  An open source network receiver will eventually be developed.

A demo analysis based on data collected through pgsampler is at TODO :).

#### What is Sampled?

The sampler collects timestamped statistics for the following categories of information:

- Transaction IDs (Tx/Sec)
- Statement Activity
- Locks
- Indices
- Tables
- Columns
- Filesystems
- CPUs
- Memory
- Replication
- Bgwriter
- IO Activity
- pg_stat_statements

#### Example CSV output (Default output mode)

bgwriter_stats.csv
```
checkpoints_timed, checkpoints_req, checkpoint_write_time, checkpoint_sync_time, buffers_checkpoint, buffers_clean, maxwritten_clean, buffers_backend, buffers_backend_fsync, buffers_alloc, stats_reset, measured_at
"71","0","3","0","0","0","0","0","0","9744","2014-07-24 22:56:02.16601-07","2014-07-29 01:31:01.900609-07"
"71","0","3","0","0","0","0","0","0","9935","2014-07-24 22:56:02.16601-07","2014-07-29 01:31:33.339747-07"
"71","0","3","0","0","0","0","0","0","10151","2014-07-24 22:56:02.16601-07","2014-07-29 01:31:34.422928-07"
....
```


## Installation

Ensure that the postgresql-9.3-dev packages are installed.  Other versions can conflict, check the results of executing the pg_config command.

```
make
make install
```

Next update the postgresql.conf directive shared_preload_libraries to include pgsampler, like to the following example:

```
shared_preload_libraries = 'pgsampler'
```

Now restart the postgresql server.

The installation can be confirmed by the presence of a log entry similar to:

```
2014-07-29 16:45:50 PDT LOG:  starting background worker process "pgsampler"
```

## Configuration

The pgsampler background worker has a number of configurable parameters to set what kind of data is collected and how it is stored.  The default configuration will sample various database statistics and state information at a reasonably safe, slow rate and store the data in csv files in /tmp.  

An alternative configuration option is to send data to relsys.io to be stored and analyzed.  I'm currently working on a similar open source implementation of a server to receive the network protocol.

#### pgsampler.target_db

This is the database the background worker will first connect to.  It defaults to the postgres database, which is a reasonable setting.

#### pgsampler.cycle_db_seconds

The bgworker will cycle through all databases and collect database specific data from each in turn.  It will reconnect to another database after the number of seconds sent in this parameter.  To collect data against a single database, set this value to -1.

#### pgsampler.output_mode

This parameter controls how the statistics are stored.  It can be set to *csv* (the default) or *network* which will attempt to connect to a remote server to send data.

#### pgsampler.output_csv_dir

This contains the path to the directory where CSV files will be stored.  It only matters when the output_mode is set to *csv*

#### pgsampler.output_network_host

This contains the hostname to connect to and send data.  It only matters when the output_mode is set to *network*

#### pgsampler.hearbeat_seconds

#### pgsampler.system_seconds

#### pgsampler.relation_seconds

#### pgsampler.guc_seconds

#### pgsampler.bgwriter_seconds

#### pgsampler.activity_seconds

#### pgsampler.replication_seconds


## CSV Reference

The csv files contain timeseries data for various metrics representing the state of a cluster.  A helper sql script is provided in /tools which will create tables and copy the csv data into the tables for analysis.

```
psql -f tools/import.sql
```

Below are column header descriptions and example data.

#### activity_stats.csv  

```
db_name, pid, usename, client_addr, backend_start, query_start, state, state_change, application_name, query, lock_type, lock_mode, lock_relation, lock_page, lock_tuple, lock_vxid, lock_txid, lock_granted, lock_holder_vxid, measured_at
"postgres","29246","postgres",,"2014-07-29 01:33:28.927757-07",,,,"","","virtualxid","ExclusiveLock",,,,"3/90",,"true","3/90","2014-07-29 01:33:28.947513-07"
```

#### heartbeats.csv      

```
txid_current, measured_at
"773166","2014-07-29 01:30:36.858769-07"
```

#### database_list.csv     

```
db_name, datconnlimit, datfrozenxid
"postgres","-1","710"
```

#### relation_info.csv      

```
db_name, schema_name, relname, reloid, relkind, relpersistence, relam, relpages, reltuples, relallvisible, relchecks, relhaspkey, relhastriggers, rel_size, rel_index_size, measured_at
"postgres","public","x","22931","r","p",,"0","0","0","0","f","f","0","0","2014-07-29 01:31:33.308087-07"
```

#### system_info.csv

```
page_size, total_pages, available_pages, num_online_processors, one_min_load_avg, swap_total, swap_free, cached, buffers, swap_cached
4096,974025,252492,4,1.140000,4038652,4030860,0,126640,1274696
```

#### bgwriter_stats.csv  

```
checkpoints_timed, checkpoints_req, checkpoint_write_time, checkpoint_sync_time, buffers_checkpoint, buffers_clean, maxwritten_clean, buffers_backend, buffers_backend_fsync, buffers_alloc, stats_reset, measured_at
"71","0","3","0","0","0","0","0","0","9744","2014-07-24 22:56:02.16601-07","2014-07-29 01:31:01.900609-07"
```

#### db_stats.csv          
```
db_name, numbackends, xact_commit, xact_rollback, blks_read, blks_hit, tup_returned, tup_fetched, tup_inserted, tup_deleted, tup_updated, conflicts, temp_files, temp_bytes, deadlocks, blk_read_time, blk_write_time, stats_reset, measured_at
"postgres","1","6686","0","1717","134218","1043080","55220","0","0","0","0","0","0","0","0","0","2014-07-24 22:56:03.104434-07","2014-07-29 01:31:33.322878-07"
```

#### filesystem_stats.csv  

```
fsname, mntdir, mnt_type, mnt_opts, blksize, fragsize, blks_in_frags, blk_free, blk_avail
/dev/sda1,/,ext4,"rw,errors=remount-ro",4096,4096,60506726,49835042,46755721
```


#### index_stats.csv     

```
db_name, schemaname, name, indexrelid, idx_scan, idx_tup_read, idx_tup_fetch, measured_at
"relsys_development","public","unique_schema_migrations","22952","0","0","0","2014-07-29 01:31:34.434593-07"
```

#### index_io_stats.csv  

```
db_name, schemaname, name, indexrelid, idx_blks_read, idx_blks_hit, measured_at 
"relsys_development","public","unique_schema_migrations","22952","0","0","2014-07-29 01:31:34.440959-07"
```

#### table_stats.csv

```
db_name, schemaname, relname, relid, seq_scan, seq_tup_read, idx_scan, idx_tup_fetch, n_tup_ins, n_tup_upd, n_tup_del, n_tup_hot_upd, n_live_tup, n_dead_tup, last_vacuum, last_autovacuum, last_analyze, last_autoanalyze, vacuum_count, autovacuum_count, analyze_count, autoanalyze_count, measured_at
"postgres","public","x","22931","0","0",,,"0","0","0","0","0","0",,,,,"0","0","0","0","2014-07-29 01:31:33.340838-07"
```

#### table_io_stats.csv

```
db_name, schemaname, relname, relid, heap_blks_read, heap_blks_hit, idx_blks_read, idx_blks_hit, toast_blks_read, toast_blks_hit, tidx_blks_read, tidx_blks_hit, measured_at
"benchy","public","pgbench_accounts","22941","0","0",,,,,,,"2014-07-29 01:31:37.682348-07"
```

#### column_stats.csv    

```
db_name, schema_name, table_name, column_name, null_frac, avg_width, n_distinct, most_common_vals, most_common_freqs, correlation, measured_at
"relsys_development","public","users","email","0","14","-1",,,,"2014-07-29 01:31:34.401461-07"
```

#### function_stats.csv    

```
db_name, funcoid, schemaname, funcname, calls, total_time, self_time, measured_at
```

#### replication_stats.csv  

```
username, application_name, client_addr, client_hostname, client_port, backend_start, state, sent_location, write_location, flush_location, replay_location, sync_priority, sync_state, current_xlog, measured_at
```

#### stat_statements.csv    

```
userid, dbid, query, calls, total_time, rows, shared_blks_hit, shared_blks_read, shared_blks_dirtied, shared_blks_written, local_blks_hit, local_blks_read, local_blks_dirtied, local_blks_written, temp_blks_read, temp_blks_written, blk_read_time, blk_write_time, measured_at
```

## Relsys.io integration

Relsys.io is a prototype application leverage the sampled metrics for purposes of monitoring and optimizing a Postgresql cluster.  Please contact me at robert@relsys.io if interested in trying out this service.  Also if interested in assisting with an open source monitoring server please get in touch.

## High Level Safety Considerations

No remote code or data is executed by the background worker.  In other words, it only writes or sends data and only receives a fixed length string acknowledgments of receipt.  This prevents a wide range of potential attacks that may attempt to execute arbitary code or SQL since the background worker runs as a priviledged user.

Shared data structures in shared memory used by postgresql are never written to.




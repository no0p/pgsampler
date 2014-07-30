-- Copy data into postgresql tables.  Assumes located in /tmp
DROP TABLE activities;
CREATE TABLE activities (
    database_name text NOT NULL,
    pid integer NOT NULL,
    user_name text NOT NULL,
    client_addr text,
    backend_start timestamp with time zone,
    query_start timestamp with time zone,   
    state text NOT NULL,
    state_change timestamp with time zone,
    application_name text,
    query text,
    lock_type text,
    lock_mode text,
    lock_relation text,
    lock_page text,
    lock_tuple text,
    lock_vxid text,
    lock_txid text,
    granted text,
    virtual_transaction text,    
    measured_at timestamp with time zone NOT NULL
);
copy activities from '/tmp/activity_stats.csv' WITH DELIMITER ',' CSV;

DROP TABLE bg_stats;
CREATE TABLE bg_stats (
    checkpoints_timed bigint NOT NULL,
    checkpoints_req bigint NOT NULL,
    checkpoint_write_time numeric NOT NULL,
    checkpoint_sync_time numeric NOT NULL,
    buffers_checkpoint bigint NOT NULL,
    buffers_clean bigint NOT NULL,
    maxwritten_clean bigint NOT NULL,
    buffers_backend bigint NOT NULL,
    buffers_backend_fsync bigint NOT NULL,
    buffers_alloc bigint NOT NULL,
    stats_reset timestamp with time zone NOT NULL,
    measured_at timestamp with time zone NOT NULL
);
copy bg_stats from '/tmp/bgwriter_stats.csv' WITH DELIMITER ',' CSV;

DROP TABLE column_stats;
CREATE TABLE column_stats (
    database_name text,
    schema_name text,
    table_name text,
    attribute_name text,
    null_fraction numeric,
    average_width_bytes integer,
    distinct_non_nulls numeric,
    most_common_vals text,
    most_common_freqs text,
    correlation numeric,
    measured_at timestamp with time zone NOT NULL
);
copy column_stats from '/tmp/column_stats.csv' WITH DELIMITER ',' CSV;

DROP TABLE db_stats;
CREATE TABLE db_stats (
    database_name text,
    numbackends integer NOT NULL,
    xact_commit bigint NOT NULL,
    xact_rollback bigint NOT NULL,
    blks_read bigint NOT NULL,
    blks_hit bigint NOT NULL,
    tup_returned bigint NOT NULL,
    tup_fetched bigint NOT NULL,
    tup_inserted bigint NOT NULL,
    tup_deleted bigint NOT NULL,
    tup_updated bigint NOT NULL,
    conflicts bigint NOT NULL,
    temp_files bigint NOT NULL,
    temp_bytes bigint NOT NULL,
    deadlocks bigint NOT NULL,
    blk_read_time double precision NOT NULL,
    blk_write_time double precision NOT NULL,
    stats_reset timestamp with time zone,
    measured_at timestamp with time zone NOT NULL
);
copy db_stats from '/tmp/db_stats.csv' WITH DELIMITER ',' CSV;

DROP TABLE heartbeats;
CREATE TABLE heartbeats (
    current_tx_id integer NOT NULL,
    measured_at timestamp with time zone NOT NULL
);
copy heartbeats from '/tmp/heartbeats.csv' WITH DELIMITER ',' CSV;

DROP TABLE index_io_stats;
CREATE TABLE index_io_stats (
    database_name text,
    schema_name text,
    indexrelname text,
    indexrelid text,
    idx_blks_read bigint NOT NULL,
    idx_blks_hit bigint NOT NULL,
    measured_at timestamp with time zone NOT NULL
);
copy index_io_stats from '/tmp/index_io_stats.csv' WITH DELIMITER ',' CSV;

DROP TABLE index_stats;
CREATE TABLE index_stats (
    database_name text,
    schema_name text,
    indexrelname text,
    indexrelid text,
    idx_scan bigint NOT NULL,
    idx_tup_read bigint NOT NULL,
    idx_tup_fetch bigint NOT NULL,
    measured_at timestamp with time zone NOT NULL
);
copy index_stats from '/tmp/index_stats.csv' WITH DELIMITER ',' CSV;

DROP TABLE table_io_stats;
CREATE TABLE table_io_stats (
    database_name text,
    schema_name text,
    table_name text,
    relid text,
    heap_blks_read bigint,
    heap_blks_hit bigint,
    idx_blks_read bigint,
    idx_blks_hit bigint,
    toast_blks_read bigint,
    toast_blks_hit bigint,
    tidx_blks_read bigint,
    tidx_blks_hit bigint,
    measured_at timestamp with time zone NOT NULL
);
copy table_io_stats from '/tmp/table_io_stats.csv' WITH DELIMITER ',' CSV;

DROP TABLE table_stats;
CREATE TABLE table_stats (
    database_name text,
    schema_name text,
    table_name text,
    relid text,
    seq_scan bigint,
    seq_tup_read bigint,
    idx_scan bigint,
    idx_tup_fetch bigint,
    n_tup_ins bigint NOT NULL,
    n_tup_upd bigint NOT NULL,
    n_tup_del bigint NOT NULL,
    n_tup_hot_upd bigint NOT NULL,
    n_live_tup bigint NOT NULL,
    n_dead_tup bigint NOT NULL,
    last_vacuum timestamp with time zone,
    last_autovacuum timestamp with time zone,
    last_analyze timestamp with time zone,
    last_autoanalyze timestamp with time zone,
    vacuum_count bigint NOT NULL,
    autovacuum_count bigint NOT NULL,
    analyze_count bigint NOT NULL,
    autoanalyze_count bigint NOT NULL,    
    measured_at timestamp with time zone NOT NULL
);
copy table_stats from '/tmp/table_stats.csv' WITH DELIMITER ',' CSV;


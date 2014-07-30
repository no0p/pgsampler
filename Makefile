MODULE_big=pgsampler
OBJS=pgsampler.o lib/control_loop.o lib/output/network.o lib/output/csv.o lib/util.o lib/commands/heartbeat.o lib/commands/handshake.o lib/commands/system_info.o lib/commands/fs_info.o lib/commands/restart_gucs.o lib/commands/transient_gucs.o lib/commands/bg_writer_stats.o lib/commands/table_stats.o lib/commands/index_stats.o  lib/commands/db_stats.o lib/commands/activity_stats.o lib/commands/function_stats.o lib/commands/table_io_stats.o lib/commands/index_io_stats.o lib/commands/replication_stats.o lib/commands/database_list.o lib/commands/relation_info.o lib/commands/column_info.o lib/commands/column_stats.o lib/commands/index_info.o lib/commands/stat_statements.o
EXTENSION=pgsampler
DATA=pgsampler--1.0.sql
PG_CONFIG = pg_config 
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)


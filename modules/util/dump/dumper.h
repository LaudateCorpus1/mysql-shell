/*
 * Copyright (c) 2020, 2021, Oracle and/or its affiliates.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms, as
 * designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef MODULES_UTIL_DUMP_DUMPER_H_
#define MODULES_UTIL_DUMP_DUMPER_H_

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "mysqlshdk/include/shellcore/scoped_contexts.h"
#include "mysqlshdk/libs/db/column.h"
#include "mysqlshdk/libs/mysql/user_privileges.h"
#include "mysqlshdk/libs/storage/backend/memory_file.h"
#include "mysqlshdk/libs/storage/idirectory.h"
#include "mysqlshdk/libs/storage/ifile.h"
#include "mysqlshdk/libs/textui/text_progress.h"
#include "mysqlshdk/libs/utils/nullable.h"
#include "mysqlshdk/libs/utils/synchronized_queue.h"
#include "mysqlshdk/libs/utils/version.h"

#include "modules/util/dump/dump_options.h"
#include "modules/util/dump/dump_writer.h"
#include "modules/util/dump/instance_cache.h"

namespace mysqlsh {
namespace dump {

class Schema_dumper;

class Dumper {
 public:
  Dumper() = delete;
  explicit Dumper(const Dump_options &options);

  Dumper(const Dumper &) = delete;
  Dumper(Dumper &&) = delete;

  Dumper &operator=(const Dumper &) = delete;
  Dumper &operator=(Dumper &&) = delete;

  virtual ~Dumper();

  void run();

 protected:
  const std::shared_ptr<mysqlshdk::db::ISession> &session() const;

  virtual std::unique_ptr<Schema_dumper> schema_dumper(
      const std::shared_ptr<mysqlshdk::db::ISession> &session) const;

 private:
  struct Object_info {
    std::string name;
    std::string quoted_name;
    std::string basename;
  };

  struct Table_info : public Object_info {
    const Instance_cache::Table *cache = nullptr;
  };

  struct View_info : public Object_info {};

  struct Schema_info : public Object_info {
    std::vector<Table_info> tables;
    std::vector<View_info> views;
  };

  struct Table_task : Table_info {
    std::string schema;
  };

  struct Table_data_task : Table_task {
    std::string where;
    Dump_writer *writer = nullptr;
    std::unique_ptr<mysqlshdk::storage::IFile> index_file;
    std::string id;
  };

  class Table_worker;

  struct Task_info {
    std::string info;
    std::function<void(Table_worker *)> task;
  };

  class Synchronize_workers;

  class Dump_info;

  class Memory_dumper;

  virtual const char *name() const = 0;

  virtual void summary() const = 0;

  virtual void on_create_table_task(const std::string &schema,
                                    const std::string &table,
                                    const Instance_cache::Table *cache) = 0;

  void do_run();

  void on_init_thread_session(
      const std::shared_ptr<mysqlshdk::db::ISession> &session) const;

  void open_session();

  void close_session();

  void fetch_user_privileges();

  void acquire_read_locks();

  void release_read_locks() const;

  void lock_all_tables();

  void start_transaction(
      const std::shared_ptr<mysqlshdk::db::ISession> &session) const;

  void assert_transaction_is_open(
      const std::shared_ptr<mysqlshdk::db::ISession> &session) const;

  void lock_instance();

  void initialize_instance_cache_minimal();

  void initialize_instance_cache();

  void create_schema_tasks();

  void validate_mds() const;

  void initialize_counters();

  void initialize_dump();

  void create_output_directory();

  void create_worker_threads();

  void wait_for_workers();

  void maybe_push_shutdown_tasks();

  void chunking_task_finished();

  void wait_for_all_tasks();

  void dump_ddl() const;

  void dump_global_ddl() const;

  void dump_users_ddl() const;

  void write_ddl(const Memory_dumper &in_memory, const std::string &file) const;

  std::unique_ptr<Memory_dumper> dump_ddl(
      Schema_dumper *dumper,
      const std::function<void(Memory_dumper *)> &func) const;

  std::unique_ptr<Memory_dumper> dump_schema(Schema_dumper *dumper,
                                             const std::string &schema) const;

  std::unique_ptr<Memory_dumper> dump_table(Schema_dumper *dumper,
                                            const std::string &schema,
                                            const std::string &table) const;

  std::unique_ptr<Memory_dumper> dump_triggers(Schema_dumper *dumper,
                                               const std::string &schema,
                                               const std::string &table) const;

  std::unique_ptr<Memory_dumper> dump_temporary_view(
      Schema_dumper *dumper, const std::string &schema,
      const std::string &view) const;

  std::unique_ptr<Memory_dumper> dump_view(Schema_dumper *dumper,
                                           const std::string &schema,
                                           const std::string &view) const;

  std::unique_ptr<Memory_dumper> dump_users(Schema_dumper *dumper) const;

  void create_schema_ddl_tasks();

  void create_table_tasks();

  Table_task create_table_task(const Schema_info &schema,
                               const Table_info &table);

  void push_table_task(Table_task &&task);

  bool is_chunked(const Table_task &task) const;

  bool should_dump_data(const Table_task &table);

  Dump_writer *get_table_data_writer(const std::string &filename);

  std::size_t finish_writing(Dump_writer *writer, uint64_t total_bytes);

  std::string close_file(const Dump_writer &writer) const;

  void write_metadata() const;

  void write_dump_started_metadata() const;

  void write_dump_finished_metadata() const;

  void write_schema_metadata(const Schema_info &schema) const;

  void write_table_metadata(
      const Table_task &table,
      const std::shared_ptr<mysqlshdk::db::ISession> &session) const;

  void summarize() const;

  void rethrow() const;

  void emergency_shutdown();

  void kill_workers();

  std::string get_table_data_filename(const std::string &basename) const;

  std::string get_table_data_filename(const std::string &basename,
                                      const std::size_t idx,
                                      const bool last_chunk) const;

  std::string get_table_data_ext() const;

  void initialize_progress();

  void update_bytes_written(uint64_t new_bytes);

  void update_progress(uint64_t new_rows, const Dump_write_result &new_bytes);

  void shutdown_progress();

  std::string throughput() const;

  mysqlshdk::storage::IDirectory *directory() const;

  std::unique_ptr<mysqlshdk::storage::IFile> make_file(
      const std::string &filename, bool use_mmap = false) const;

  std::string get_basename(const std::string &basename);

  bool compressed() const;

  void kill_query() const;

  std::string get_query_comment(const std::string &quoted_name,
                                const std::string &id,
                                const char *context) const;

  std::string get_query_comment(const Table_data_task &task,
                                const char *context) const;

  void validate_privileges() const;

  bool is_gtid_executed_inconsistent() const;

  // session
  std::shared_ptr<mysqlshdk::db::ISession> m_session;

  std::vector<std::shared_ptr<mysqlshdk::db::ISession>> m_lock_sessions;

  // user privileges
  std::unique_ptr<mysqlshdk::mysql::User_privileges> m_user_privileges;
  std::string m_user_account;

  // console
  Scoped_console m_console;

  // input data
  const Dump_options &m_options;
  std::unique_ptr<mysqlshdk::storage::IDirectory> m_output_dir;
  std::unique_ptr<mysqlshdk::storage::IFile> m_output_file;
  bool m_use_json = false;
  Instance_cache m_cache;
  std::vector<Schema_info> m_schema_infos;
  std::unordered_map<std::string, std::size_t> m_truncated_basenames;

  // status
  bool m_instance_locked = false;
  // whether FLUSH TABLES WITH READ LOCK has failed
  bool m_ftwrl_failed = false;

  // counters
  uint64_t m_total_rows = 0;
  uint64_t m_total_schemas = 0;
  uint64_t m_total_tables = 0;
  uint64_t m_total_views = 0;

  // progress information
  std::unique_ptr<Dump_info> m_dump_info;
  std::atomic<uint64_t> m_data_bytes;
  std::atomic<uint64_t> m_bytes_written;
  std::atomic<uint64_t> m_rows_written;
  std::unique_ptr<mysqlshdk::textui::IProgress> m_progress;
  std::recursive_mutex m_progress_mutex;
  std::unique_ptr<mysqlshdk::textui::Throughput> m_data_throughput;
  std::unique_ptr<mysqlshdk::textui::Throughput> m_bytes_throughput;
  std::atomic<uint64_t> m_num_threads_chunking;
  std::atomic<uint64_t> m_num_threads_dumping;
  std::mutex m_table_data_bytes_mutex;
  // schema -> table -> data bytes
  std::unordered_map<std::string, std::unordered_map<std::string, uint64_t>>
      m_table_data_bytes;

  // path -> uncompressed bytes
  std::unordered_map<std::string, uint64_t> m_chunk_file_bytes;

  // threads
  std::vector<std::thread> m_workers;
  std::vector<std::exception_ptr> m_worker_exceptions;
  shcore::Synchronized_queue<Task_info> m_worker_tasks;
  std::atomic<uint64_t> m_chunking_tasks;
  std::atomic<bool> m_main_thread_finished_producing_chunking_tasks;
  std::unique_ptr<Synchronize_workers> m_worker_synchronization;
  std::vector<std::unique_ptr<Dump_writer>> m_worker_writers;
  std::mutex m_worker_writers_mutex;
  volatile bool m_worker_interrupt = false;
};

}  // namespace dump
}  // namespace mysqlsh

#endif  // MODULES_UTIL_DUMP_DUMPER_H_

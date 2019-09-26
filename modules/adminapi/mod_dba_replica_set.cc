/*
 * Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.
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

#include "modules/adminapi/mod_dba_replica_set.h"

#include "modules/adminapi/common/dba_errors.h"
#include "mysqlshdk/include/shellcore/console.h"
#include "mysqlshdk/include/shellcore/utils_help.h"
#include "mysqlshdk/libs/utils/debug.h"

DEBUG_OBJ_ENABLE(ReplicaSet);

namespace mysqlsh {
namespace dba {

// Documentation of the ReplicaSet Class
REGISTER_HELP_CLASS(ReplicaSet, adminapi);
REGISTER_HELP(REPLICASET_BRIEF, "Represents an InnoDB ReplicaSet.");
REGISTER_HELP(
    REPLICASET_DETAIL,
    "The ReplicaSet object is used to manage MySQL server topologies that use "
    "asynchronous replication. It can be created using the "
    "dba.<<<createReplicaSet>>>() or dba.<<<getReplicaSet>>>() functions.");
REGISTER_HELP(REPLICASET_CLOSING,
              "For more help on a specific function, use the \\help shell "
              "command, e.g.: \\help ReplicaSet.<<<addInstance>>>");
ReplicaSet::ReplicaSet(const std::shared_ptr<Replica_set_impl> &cluster)
    : m_impl(cluster) {
  DEBUG_OBJ_ALLOC2(ReplicaSet, [](void *ptr) -> std::string {
    return "refs:" + std::to_string(reinterpret_cast<ReplicaSet *>(ptr)
                                        ->shared_from_this()
                                        .use_count());
  });

  init();
}

ReplicaSet::~ReplicaSet() { DEBUG_OBJ_DEALLOC(ReplicaSet); }

std::string &ReplicaSet::append_descr(std::string &s_out, int UNUSED(indent),
                                      int UNUSED(quote_strings)) const {
  s_out.append("<" + class_name() + ":" + impl()->get_name() + ">");

  return s_out;
}

void ReplicaSet::append_json(shcore::JSON_dumper &dumper) const {
  dumper.start_object();
  dumper.append_string("class", class_name());
  dumper.append_string("name", impl()->get_name());
  dumper.end_object();
}

bool ReplicaSet::operator==(const Object_bridge &other) const {
  return class_name() == other.class_name() && this == &other;
}

void ReplicaSet::init() {
  add_property("name", "getName");

  expose("addInstance", &ReplicaSet::add_instance, "instanceDef", "?options");
  expose("rejoinInstance", &ReplicaSet::rejoin_instance, "instanceDef");
  expose("removeInstance", &ReplicaSet::remove_instance, "instanceDef",
         "?options");

  expose("status", &ReplicaSet::status, "?options");
  expose("disconnect", &ReplicaSet::disconnect);

  expose("setPrimaryInstance", &ReplicaSet::set_primary_instance, "instanceDef",
         "?options");
  expose("forcePrimaryInstance", &ReplicaSet::force_primary_instance,
         "?instanceDef", "?options");

  expose("listRouters", &ReplicaSet::list_routers, "?options");
  expose("removeRouterMetadata", &ReplicaSet::remove_router_metadata,
         "routerDef");

  // TODO(alfredo):
  // - dissolve()
  // - restore() - fixes replication configuration and fencing status according
  // to metadata
}

// Documentation of the getName function
REGISTER_HELP_FUNCTION(getName, ReplicaSet);
REGISTER_HELP_PROPERTY(name, ReplicaSet);
REGISTER_HELP(REPLICASET_NAME_BRIEF, "${REPLICASET_GETNAME_BRIEF}");
REGISTER_HELP_FUNCTION_TEXT(REPLICASET_GETNAME, R"*(
Returns the name of the replicaset.

@returns name of the replicaset.
)*");

/**
 * $(REPLICASET_GETNAME_BRIEF)
 *
 * $(REPLICASET_GETNAME)
 */
#if DOXYGEN_JS
String ReplicaSet::getName() {}
#elif DOXYGEN_PY
str ReplicaSet::get_name() {}
#endif

shcore::Value ReplicaSet::get_member(const std::string &prop) const {
  shcore::Value ret_val;

  // Throw an error if the cluster has already been dissolved
  assert_valid(prop);

  if (prop == "name")
    ret_val = shcore::Value(impl()->get_name());
  else
    ret_val = shcore::Cpp_object_bridge::get_member(prop);
  return ret_val;
}

void ReplicaSet::assert_valid(const std::string &option_name) const {
  std::string name;

  if (option_name == "disconnect") return;

  if (has_member(option_name) && m_invalidated) {
    if (has_method(option_name)) {
      name = get_function_name(option_name, false);
      throw shcore::Exception::runtime_error(class_name() + "." + name + ": " +
                                             "Can't call function '" + name +
                                             "' on a dissolved replicaset");
    } else {
      name = get_member_name(option_name, shcore::current_naming_style());
      throw shcore::Exception::runtime_error(
          class_name() + "." + name + ": " + "Can't access object member '" +
          name + "' on a dissolved replicaset");
    }
  }
  if (!impl()->get_target_server() || !impl()->get_metadata_storage()) {
    throw shcore::Exception::runtime_error(
        "The replicaset object is disconnected. Please use "
        "dba.<<<getReplicaSet>>>() to obtain a new object.");
  }
}

shcore::Value ReplicaSet::execute_with_pool(
    const std::function<shcore::Value()> &f, bool interactive) {
  while (true) {
    Scoped_instance_pool scoped_pool(impl()->get_metadata_storage(),
                                     interactive,
                                     impl()->default_admin_credentials());

    try {
      return f();
    } catch (const shcore::Exception &e) {
      if (e.code() == SHERR_DBA_ASYNC_MEMBER_INVALIDATED &&
          e.error()->has_key("new_primary_endpoint")) {
        std::string new_primary = e.error()->get_string("new_primary_endpoint");

        current_console()->print_warning(e.format() + ": reconnecting to " +
                                         new_primary);

        Scoped_instance target(
            scoped_pool->connect_unchecked_endpoint(new_primary));

        impl()->set_target_server(target);

        target->steal();

        // Retry to execute the function using the new primary.
        continue;
      } else {
        throw;
      }
    }
  }
}

REGISTER_HELP_FUNCTION(addInstance, ReplicaSet);
REGISTER_HELP_FUNCTION_TEXT(REPLICASET_ADDINSTANCE, R"*(
Adds an instance to the replicaset.

@param instance host:port of the target instance to be added.
@param options Optional dictionary with options for the operation.

@returns Nothing.

This function adds the given MySQL instance as a read-only SECONDARY replica of
the current PRIMARY of the replicaset. Replication will be configured between
the added instance and the PRIMARY. Replication will use an automatically
created MySQL account with a random password.

Once the instance is added to the replicaset, the function will wait for it to
apply all pending transactions. The transaction sync may timeout if the
transaction backlog is large. The timeout option may be used to increase that
time.

The Shell connects to the target instance using the same username and
password used to obtain the replicaset handle object. All instances of the
replicaset are expected to have this same admin account with the same grants
and passwords. A custom admin account with the required grants can be created
while an instance is configured with dba.<<<configureReplicaSetInstance>>>(),
but the root user may be used too.

The PRIMARY of the replicaset must be reachable and available during this
operation.

<b>Pre-Requisites</b>

The following pre-requisites are expected for instances added to a replicaset.
They will be automatically checked by <<<addInstance>>>(), which will stop
if any issues are found.

@li binary log and replication related options must have been validated
and/or configured by dba.<<<configureReplicaSetInstance>>>()
@li transaction set in the instance being added must not contain transactions
that don't exist in the PRIMARY
@li transaction set in the instance being added must not be missing transactions
that have been purged from the binary log of the PRIMARY 

<b>Options</b>

The options dictionary may contain the following values:

@li dryRun: if true, performs checks and logs changes that would be made, but
does not execute them
@li label: an identifier for the instance being added, used in the output of 
status()
@li recoveryMethod: Preferred method of state recovery. May be auto or
incremental. Default is auto.
@li interactive: if true, enables interactive password and confirmation
prompts. Defaults to the value of the useWizards shell option.
@li timeout: timeout in seconds for transaction sync operations; 0 disables
timeout and force the Shell to wait until the transaction sync finishes.
Defaults to 0.
)*");

/**
 * $(REPLICASET_ADDINSTANCE_BRIEF)
 *
 * $(REPLICASET_ADDINSTANCE)
 */
#if DOXYGEN_JS
Undefined ReplicaSet::addInstance(String instance, Dictionary options) {}
#elif DOXYGEN_PY
None ReplicaSet::add_instance(str instance, dict options) {}
#endif
void ReplicaSet::add_instance(const std::string &instance_def,
                              const shcore::Dictionary_t &options) {
  // Throw an error if the cluster has already been dissolved
  assert_valid("addInstance");

  bool interactive = current_shell_options()->get().wizards;
  int sync_timeout = 0;
  std::string instance_label;
  bool dry_run = false;
  Async_replication_options ar_options(Async_replication_options::CREATE);
  std::string recovery_method_str;

  // Parse and check user options
  Unpack_options(options)
      .unpack(&ar_options)
      .optional("label", &instance_label)
      .optional("dryRun", &dry_run)
      .optional("interactive", &interactive)
      .optional("recoveryMethod", &recovery_method_str)
      .optional("timeout", &sync_timeout)
      .end();

  Member_recovery_method recovery_method = Member_recovery_method::AUTO;
  if (!recovery_method_str.empty()) {
    if (shcore::str_caseeq(recovery_method_str, "incremental"))
      recovery_method = Member_recovery_method::INCREMENTAL;
    else if (shcore::str_caseeq(recovery_method_str, "auto"))
      recovery_method = Member_recovery_method::AUTO;
    else
      throw shcore::Exception::argument_error(
          "Invalid value for option recoveryMethod: " + recovery_method_str);
  }

  if (sync_timeout < 0) {
    throw shcore::Exception::argument_error("timeout option must be >= 0");
  }

  execute_with_pool(
      [&]() {
        impl()->add_instance(instance_def, ar_options, instance_label,
                             recovery_method, sync_timeout, interactive,
                             dry_run);
        return shcore::Value();
      },
      interactive);
}

REGISTER_HELP_FUNCTION(rejoinInstance, ReplicaSet);
REGISTER_HELP_FUNCTION_TEXT(REPLICASET_REJOININSTANCE, R"*(
Rejoins an instance to the replicaset.

@param instance host:port of the target instance to be rejoined.

@returns Nothing.

This function rejoins the given MySQL instance to the replicaset, making it a
read-only SECONDARY replica of the current PRIMARY. Only instances previously
added to the replicaset can be rejoined,
otherwise the @<ReplicaSet@>.<<<addInstance>>>() function must be used to
add a new instance.

The PRIMARY of the replicaset must be reachable and available during this
operation.
)*");

/**
 * $(REPLICASET_REJOININSTANCE_BRIEF)
 *
 * $(REPLICASET_REJOININSTANCE)
 */
#if DOXYGEN_JS
Undefined ReplicaSet::rejoinInstance(String instance) {}
#elif DOXYGEN_PY
None ReplicaSet::rejoin_instance(str instance) {}
#endif
void ReplicaSet::rejoin_instance(const std::string &instance_def) {
  // Throw an error if the cluster has already been dissolved
  assert_valid("rejoinInstance");

  bool interactive = current_shell_options()->get().wizards;

  // Validate the connection options (in order to issue user friendly errors).
  Connection_options cnx_opts = get_connection_options(instance_def);
  validate_connection_options(cnx_opts);

  execute_with_pool(
      [&]() {
        impl()->rejoin_instance(instance_def);
        return shcore::Value();
      },
      interactive);
}

REGISTER_HELP_FUNCTION(removeInstance, ReplicaSet);
REGISTER_HELP_FUNCTION_TEXT(REPLICASET_REMOVEINSTANCE, R"*(
Removes an Instance from the replicaset.

@param instance host:port of the target instance to be removed
@param options Optional dictionary with options for the operation.

@returns Nothing.

This function removes an Instance from the replicaset and stops it from
replicating. The instance must not be the PRIMARY and it is expected to be
reachable.

The PRIMARY of the replicaset must be reachable and available during this
operation.

<b>Options</b>

The instance definition is the connection data for the instance.

The options dictionary may contain the following attributes:

@li force: boolean, indicating if the instance must be removed (even if only
from metadata) in case it cannot be reached. By default, set to false.
@li timeout: maximum number of seconds to wait for the instance to sync up with
the PRIMARY. 0 means no timeout and <0 will skip sync.
@li interactive: boolean value used to disable/enable the wizards in the

The force option (set to true) is required to remove instances that are
unreachable. Removed instances are normally synchronized with the rest of the
replicaset, so that their own copy of the metadata will be consistent and
the instance will be suitable for being added back to the replicaset. When
the force option is set during removal, that step will be skipped.
)*");

/**
 * $(REPLICASET_REMOVEINSTANCE_BRIEF)
 *
 * $(REPLICASET_REMOVEINSTANCE)
 */
#if DOXYGEN_JS
Undefined ReplicaSet::removeInstance(String instance, Dictionary options) {}
#elif DOXYGEN_PY
None ReplicaSet::remove_instance(str instance, dict options) {}
#endif

void ReplicaSet::remove_instance(const std::string &instance_def,
                                 const shcore::Dictionary_t &options) {
  // Throw an error if the cluster has already been dissolved
  assert_valid("removeInstance");

  // Remove the Instance from the Default ReplicaSet
  bool interactive = current_shell_options()->get().wizards;
  int timeout = current_shell_options()->get().dba_gtid_wait_timeout;
  std::string password;
  mysqlshdk::null_bool force;

  // Get optional options.
  Unpack_options(options)
      .optional("force", &force)
      .optional("timeout", &timeout)
      .end();

  execute_with_pool(
      [&]() {
        impl()->remove_instance(instance_def, force, timeout);
        return shcore::Value();
      },
      interactive);
}

REGISTER_HELP_FUNCTION(status, ReplicaSet);
REGISTER_HELP_FUNCTION_TEXT(REPLICASET_STATUS, R"*(
Describe the status of the replicaset.

@param options Optional dictionary with options.

@returns A JSON object describing the status of the members of the replicaset.

This function will connect to each member of the replicaset and query their
state, producing a status report of the replicaset as a whole, as well
as of their individual members.

<b>Options</b>

The following options may be given to control the amount of information gathered
and returned.

@li extended: Can be 0, 1 or 2. Default is 0.

Option 'extended' may have the following values:
@li 0 - regular level of details. Only basic information about the status
of the instance and replication is included, in addition to non-default
or unexpected replication settings and status.
@li 1 - includes Metadata Version, server UUID and the raw information used
    to derive the status of the instance, size of the applier queue,
    value of system variables that protect against unexpected writes
    etc.
@li 2 - includes important replication related configuration settings, such as
SSL, worker threads, replication delay and heartbeat delay.
)*");

/**
 * $(REPLICASET_STATUS_BRIEF)
 *
 * $(REPLICASET_STATUS)
 */
#if DOXYGEN_JS
String ReplicaSet::status(Dictionary options) {}
#elif DOXYGEN_PY
str ReplicaSet::status(dict options) {}
#endif

shcore::Value ReplicaSet::status(const shcore::Dictionary_t &options) {
  // Throw an error if the cluster has already been dissolved
  assert_valid("status");

  int extended = 0;

  // Retrieves optional options
  Unpack_options(options).optional("extended", &extended).end();

  if (extended > 2 || extended < 0) {
    throw shcore::Exception::argument_error(
        "Invalid value for option 'extended'");
  }

  return execute_with_pool([&]() { return impl()->status(extended); }, false);
}

REGISTER_HELP_FUNCTION(disconnect, ReplicaSet);
REGISTER_HELP_FUNCTION_TEXT(REPLICASET_DISCONNECT, R"*(
Disconnects all internal sessions used by the replicaset object.

@returns Nothing.

Disconnects the internal MySQL sessions used by the replicaset to query for
metadata and replication information.
)*");

/**
 * $(REPLICASET_DISCONNECT_BRIEF)
 *
 * $(REPLICASET_DISCONNECT)
 */
#if DOXYGEN_JS
Undefined ReplicaSet::disconnect() {}
#elif DOXYGEN_PY
None ReplicaSet::disconnect() {}
#endif

void ReplicaSet::disconnect() { impl()->disconnect(); }

REGISTER_HELP_FUNCTION(setPrimaryInstance, ReplicaSet);
REGISTER_HELP_FUNCTION_TEXT(REPLICASET_SETPRIMARYINSTANCE, R"*(
Performs a safe PRIMARY switchover, promoting the given instance.

@param instance host:port of the target instance to be promoted.
@param options Dictionary with additional options.
@returns Nothing

This command will perform a safe switchover of the PRIMARY of a replicaset.
The current PRIMARY will be demoted to a SECONDARY and made read-only, while
the promoted instance will be made a read-write master. All other SECONDARY
instances will be updated to replicate from the new PRIMARY.

During the switchover, the promoted instance will be synchronized with the
old PRIMARY, ensuring that all transactions present in the PRIMARY are
applied before the topology change is commited. If that synchronization
step takes too long or is not possible in any of the SECONDARY instances, the
switch will be aborted. These problematic SECONDARY instances must be either
repaired or removed from the replicaset for the fail over to be possible.

For a safe switchover to be possible, all replicaset instances must be reachable
from the shell and have consistent transaction sets. If the PRIMARY is not
available, a forced failover must be performed instead, using
<<<forcePrimaryInstance>>>().

<b>Options</b>

The following options may be given:

@li dryRun: if true, will perform checks and log operations that would be
performed, but will not execute them. The operations that would be performed
can be viewed by enabling verbose output in the shell.
@li timeout: integer value with the maximum number of seconds to wait until
the instance being promoted catches up to the current PRIMARY.
)*");

/**
 * $(REPLICASET_SETPRIMARYINSTANCE_BRIEF)
 *
 * $(REPLICASET_SETPRIMARYINSTANCE)
 */
#if DOXYGEN_JS
Undefined ReplicaSet::setPrimaryInstance(String instance, Dictionary options) {}
#elif DOXYGEN_PY
None ReplicaSet::set_primary_instance(str instance, dict options) {}
#endif
void ReplicaSet::set_primary_instance(const std::string &instance_def,
                                      const shcore::Dictionary_t &options) {
  assert_valid("setPrimaryInstance");

  bool dry_run = false;
  uint32_t timeout = current_shell_options()->get().dba_gtid_wait_timeout;

  Unpack_options(options)
      .optional("timeout", &timeout)
      .optional("dryRun", &dry_run)
      .end();

  execute_with_pool(
      [&]() {
        impl()->set_primary_instance(instance_def, timeout, dry_run);
        return shcore::Value();
      },
      false);
}

REGISTER_HELP_FUNCTION(forcePrimaryInstance, ReplicaSet);
REGISTER_HELP_FUNCTION_TEXT(REPLICASET_FORCEPRIMARYINSTANCE, R"*(
Performs a failover in a replicaset with an unavailable PRIMARY.

@param instance host:port of the target instance to be promoted. If blank, a
suitable instance will be selected automatically.
@param options Dictionary with additional options.
@returns Nothing

This command will perform a forced failover of the PRIMARY of a replicaset
in disaster scenarios where the current PRIMARY is unavailable and cannot be
restored. If given, the target instance will be promoted to a PRIMARY, while
other reachable SECONDARY instances will be switched to the new PRIMARY. The
target instance must have the most up-to-date GTID_EXECUTED set among reachable
instances, otherwise the operation will fail. If a target instance is not given
(or is null), the most up-to-date instance will be automatically selected and
promoted.

After a forced failover, the old PRIMARY will be considered invalid by 
the new PRIMARY and can no longer be part of the replicaset. If the instance
is still usable, it must be removed from the replicaset and re-added as a
new instance. If there were any SECONDARY instances that could not be 
switched to the new PRIMARY during the failover, they will also be considered
invalid.

@attention a forced failover is a potentially destructive action and should 
only be used as a last resort measure.

Data loss is possible after a failover, because the old PRIMARY may
have had transactions that were not yet replicated to the SECONDARY being
promoted. Moreover, if the instance that was presumed to have failed is still 
able to process updates, for example because the network where it is located 
is still functioning but unreachable from the shell, it will continue diverging
from the promoted clusters.
Recovering or re-conciliating diverged transaction sets requires
manual intervention and may some times not be possible, even if the failed
MySQL servers can be recovered. In many cases, the fastest and simplest
way to recover from a disaster that required a forced failover is by
discarding such diverged transactions and re-provisioning a new instance
from the newly promoted PRIMARY.

<b>Options</b>

The following options may be given:

@li dryRun: if true, will perform checks and log operations that would be
performed, but will not execute them. The operations that would be performed
can be viewed by enabling verbose output in the shell.
@li invalidateErrorInstances: if false, aborts the failover if any instance
other than the old master is unreachable or has errors. If true, such instances
will not be failed over and be invalidated.
)*");

/**
 * $(REPLICASET_FORCEPRIMARYINSTANCE_BRIEF)
 *
 * $(REPLICASET_FORCEPRIMARYINSTANCE)
 */
#if DOXYGEN_JS
Undefined ReplicaSet::forcePrimaryInstance(String instance,
                                           Dictionary options) {}
#elif DOXYGEN_PY
None ReplicaSet::force_primary_instance(str instance, dict options) {}
#endif
void ReplicaSet::force_primary_instance(const std::string &instance_def,
                                        const shcore::Dictionary_t &options) {
  assert_valid("forcePrimaryInstance");

  bool dry_run = false;
  bool invalidate_instances = false;
  uint32_t timeout = current_shell_options()->get().dba_gtid_wait_timeout;

  Unpack_options(options)
      .optional("dryRun", &dry_run)
      .optional("timeout", &timeout)
      .optional("invalidateErrorInstances", &invalidate_instances)
      .end();

  execute_with_pool(
      [&]() {
        impl()->force_primary_instance(instance_def, timeout,
                                       invalidate_instances, dry_run);
        return shcore::Value();
      },
      false);
}

REGISTER_HELP_FUNCTION(listRouters, ReplicaSet);
REGISTER_HELP_FUNCTION_TEXT(REPLICASET_LISTROUTERS, R"*(
Lists the Router instances.

@param options Optional dictionary with options for the operation.

@returns A JSON object listing the Router instances associated to the cluster.

This function lists and provides information about all Router instances
registered for the cluster.

Whenever a Metadata Schema upgrade is necessary, the recommended process
is to upgrade MySQL Router instances to the latest version before upgrading
the Metadata itself, in order to minimize service disruption.

The options dictionary may contain the following attributes:

@li onlyUpgradeRequired: boolean, enables filtering so only router instances
that support older version of the Metadata Schema and require upgrade are
included.

@throw MetadataError in the following scenarios:
@li If the Metadata is inaccessible.
@throw RuntimeError in the following scenarios:
@li If the InnoDB Cluster topology mode does not match the current Group
    Replication configuration.
@li If the InnoDB Cluster name is not registered in the Metadata.
)*");

/**
 * $(REPLICASET_LISTROUTERS_BRIEF)
 *
 * $(REPLICASET_LISTROUTERS)
 */
#if DOXYGEN_JS
String ReplicaSet::listRouters(Dictionary options) {}
#elif DOXYGEN_PY
str ReplicaSet::list_routers(dict options) {}
#endif
shcore::Dictionary_t ReplicaSet::list_routers(
    const shcore::Dictionary_t &options) {
  // Throw an error if the cluster has already been dissolved
  assert_valid("listRouters");

  bool only_upgrade_required = false;

  // Throw an error if the cluster has already been dissolved
  check_function_preconditions("ReplicaSet.listRouters",
                               m_impl->get_target_server());

  Unpack_options(options)
      .optional("onlyUpgradeRequired", &only_upgrade_required)
      .end();

  auto ret_val = m_impl->list_routers(only_upgrade_required);

  return ret_val.as_map();
}

REGISTER_HELP_FUNCTION(removeRouterMetadata, ReplicaSet);
REGISTER_HELP_FUNCTION_TEXT(REPLICASET_REMOVEROUTERMETADATA, R"*(
Removes metadata for a router instance.

@param routerDef identifier of the router instance to be removed (e.g.
192.168.45.70@::system)
@returns Nothing

MySQL Router automatically registers itself within the InnoDB cluster
metadata when bootstrapped. However, that metadata may be left behind when
instances are uninstalled or moved over to a different host. This function may
be used to clean up such instances that no longer exist.

The @<ReplicaSet@>.<<<listRouters>>>() function may be used to list
registered router instances, including their identifier.

@throw ArgumentError in the following scenarios:
@li if router_def is not a registered router instance
)*");
/**
 * $(REPLICASET_REMOVEROUTERMETADATA_BRIEF)
 *
 * $(REPLICASET_REMOVEROUTERMETADATA)
 */
#if DOXYGEN_JS
String ReplicaSet::removeRouterMetadata(RouterDef routerDef) {}
#elif DOXYGEN_PY
str ReplicaSet::remove_router_metadata(RouterDef routerDef) {}
#endif
void ReplicaSet::remove_router_metadata(const std::string &router_def) {
  // Throw an error if the cluster has already been dissolved
  assert_valid("removeRouterMetadata");

  // Throw an error if the cluster has already been dissolved
  check_function_preconditions("ReplicaSet.removeRouterMetadata",
                               m_impl->get_target_server());

  if (!m_impl->get_metadata_storage()->remove_router(router_def)) {
    throw shcore::Exception::argument_error("Invalid router instance '" +
                                            router_def + "'");
  }
}

}  // namespace dba
}  // namespace mysqlsh

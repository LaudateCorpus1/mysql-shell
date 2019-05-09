/*
 * Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of the
 * License.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#ifndef MODULES_ADMINAPI_COMMON_DBA_ERRORS_H_
#define MODULES_ADMINAPI_COMMON_DBA_ERRORS_H_

// AdminAPI / InnoDB cluster error codes
#define SHERR_DBA_FIRST 51000

#define SHERR_DBA_TARGET_QUERY_ERROR 51000

#define SHERR_DBA_GROUP_REPLICATION_UNEXPECTED 51001

#define SHERR_DBA_GROUP_REPLICATION_NOT_RUNNING 51002
#define SHERR_DBA_GROUP_MEMBER_NOT_IN_QUORUM 51003
#define SHERR_DBA_GROUP_MEMBER_NOT_ONLINE 51004

#define SHERR_DBA_GROUP_HAS_NO_QUORUM 51011
#define SHERR_DBA_GROUP_HAS_NO_PRIMARY 51014
#define SHERR_DBA_GROUP_UNREACHABLE 51015
#define SHERR_DBA_GROUP_UNAVAILABLE 51016
#define SHERR_DBA_GROUP_SPLIT_BRAIN 51017

#define SHERR_DBA_CLUSTER_INVALIDATED 51019
#define SHERR_DBA_CLUSTER_NOT_REPLICATED 51020

#define SHERR_DBA_METADATA_MISSING 51101
#define SHERR_DBA_METADATA_READ_ONLY 51102

#define SHERR_DBA_METADATA_INFO_MISSING 51103
#define SHERR_DBA_MEMBER_METADATA_MISSING 51104
#define SHERR_DBA_CLUSTER_METADATA_MISSING 51105
#define SHERR_DBA_METADATA_INCONSISTENT 51106

#define SHERR_DBA_ACTIVE_CLUSTER_UNAVAILABLE 51110
#define SHERR_DBA_ACTIVE_CLUSTER_UNDEFINED 51111
#define SHERR_DBA_ACTIVE_CLUSTER_NOT_FOUND 51112

#define SHERR_DBA_ACTIVE_CLUSTER_STILL_AVAILABLE 51116
#define SHERR_DBA_ACTIVE_CLUSTER_STILL_REACHABLE 51117

#define SHERR_DBA_ASYNC_PRIMARY_UNAVAILABLE 51118
#define SHERR_DBA_ASYNC_PRIMARY_UNDEFINED 51119
#define SHERR_DBA_ASYNC_PRIMARY_NOT_FOUND 51120

#define SHERR_DBA_METADATA_SYNC_ERROR 51130

#define SHERR_DBA_REPLICATION_ERROR 51131
#define SHERR_DBA_REPLICATION_INVALID 51132
#define SHERR_DBA_REPLICATION_OFF 51133

#define SHERR_DBA_GROUP_TOPOLOGY_ERROR 51134

#define SHERR_DBA_REPLICATION_START_TIMEOUT 51140
#define SHERR_DBA_REPLICATION_START_ERROR 51141
#define SHERR_DBA_REPLICATION_AUTH_ERROR 51142
#define SHERR_DBA_REPLICATION_CONNECT_ERROR 51143
#define SHERR_DBA_REPLICATION_COORDINATOR_ERROR 51144
#define SHERR_DBA_REPLICATION_APPLIER_ERROR 51145

#define SHERR_DBA_INVALID_SERVER_CONFIGURATION 51150
#define SHERR_DBA_UNSUPPORTED_ASYNC_TOPOLOGY 51151
#define SHERR_DBA_DATA_INCONSISTENT_ERRANT_TRANSACTIONS 51152

// Global topology consistency errors
#define SHERR_DBA_GTCE_GROUP_HAS_MULTIPLE_SLAVES 51200
#define SHERR_DBA_GTCE_GROUP_HAS_MULTIPLE_MASTERS 51201

#define SHERR_DBA_GTCE_GROUP_HAS_WRONG_MASTER 51202

// Invalid arguments
#define SHERR_DBA_BADARG_INSTANCE_NOT_MANAGED 51300
#define SHERR_DBA_BADARG_INSTANCE_ALREADY_MANAGED 51301
#define SHERR_DBA_BADARG_INSTANCE_REMOVE_NOT_ALLOWED 51302
#define SHERR_DBA_BADARG_INSTANCE_NOT_SUPPORTED 51303

#endif  // MODULES_ADMINAPI_COMMON_DBA_ERRORS_H_
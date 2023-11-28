/*
  Copyright (c) 2022, 2023, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef METADATA_CACHE_ROUTER_OPTIONS_INCLUDED
#define METADATA_CACHE_ROUTER_OPTIONS_INCLUDED

#include "mysqlrouter/cluster_metadata.h"
#include "mysqlrouter/mysql_session.h"

#include <chrono>
#include <optional>

enum class ReadOnlyTargets { all, read_replicas, secondaries };
static const ReadOnlyTargets kDefaultReadOnlyTargets =
    ReadOnlyTargets::secondaries;

std::string to_string(const ReadOnlyTargets mode);

/** @class RouterOptions
 *
 * @brief Represents the Router options in v2_routers view in the metadata
 * schema
 */
class RouterOptions {
 public:
  /** @brief Pupulate the object by reading the options from the metadata
   *
   * @param session mysql server session to read metadata with
   * @param router_id id of the Router in the metadata\
   * @param schema_version current metadata schema version
   * @param cluster_type type of the cluster the Router is configured to use
   *
   * @returns true if successful, false otherwise
   */
  bool read_from_metadata(
      mysqlrouter::MySQLSession &session, const unsigned router_id,
      const mysqlrouter::MetadataSchemaVersion schema_version,
      const mysqlrouter::ClusterType cluster_type);

  /** @brief Get the raw JSON string read from the metadata during the last
   * read_from_metadata() call
   */
  std::string get_string() const { return options_str_; }

  /** @brief Get the setting for RO targets assigned to a given Router in the
   * metadata
   */
  ReadOnlyTargets get_read_only_targets() const;

  /** @brief Get the stats updates ferquency value (in seconds) assigned for a
   * given Router in the metadata
   */
  std::optional<std::chrono::seconds> get_stats_updates_frequency() const;

  // clusterset specific Router Options

  /** @brief Get the get_use_replica_primary_as_rw boolean value assigned for a
   * given Router in the metadata
   */
  bool get_use_replica_primary_as_rw() const;

  /** @brief Get the target_cluster assigned for a given Router in the metadata
   *
   * @returns assigned target_cluster if read successful, std::nullopt otherwise
   */
  std::optional<mysqlrouter::TargetCluster> get_target_cluster() const;

 private:
  std::string options_str_;
  unsigned int router_id_{};
  mysqlrouter::ClusterType cluster_type_{};
};

#endif  // METADATA_CACHE_ROUTER_OPTIONS_INCLUDED
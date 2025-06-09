/*
  Copyright (c) 2024, 2025, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is designed to work with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have either included with
  the program or referenced in the documentation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mrs/endpoint/content_set_endpoint.h"

#include <vector>

#include "mrs/endpoint/db_service_endpoint.h"
#include "mrs/observability/entity.h"
#include "mrs/router_observation_entities.h"

namespace mrs {
namespace endpoint {

using ContentSet = ContentSetEndpoint::ContentSet;
using ContentSetPtr = ContentSetEndpoint::ContentSetPtr;
using UniversalId = ContentSetEndpoint::UniversalId;
using EnabledType = ContentSetEndpoint::EnabledType;

ContentSetEndpoint::ContentSetEndpoint(const ContentSet &entry,
                                       EndpointConfigurationPtr configuration,
                                       HandlerFactoryPtr factory)
    : OptionEndpoint(entry.service_id, configuration, factory),
      entry_{std::make_shared<ContentSet>(entry)} {}

UniversalId ContentSetEndpoint::get_id() const { return entry_->id; }

UniversalId ContentSetEndpoint::get_parent_id() const {
  return entry_->service_id;
}

const ContentSetPtr ContentSetEndpoint::get() const { return entry_; }

void ContentSetEndpoint::set(const ContentSet &entry, EndpointBasePtr parent) {
  auto lock = std::unique_lock<std::shared_mutex>(endpoints_access_);
  entry_ = std::make_shared<ContentSet>(entry);
  change_parent(parent);
  changed();
}

void ContentSetEndpoint::update() {
  auto service_ep =
      std::dynamic_pointer_cast<DbServiceEndpoint>(get_parent_ptr());
  if (service_ep) {
    service_ep->on_updated_content_set();
  }

  Parent::update();
  observability::EntityCounter<kEntityCounterUpdatesContentSets>::increment();
}

EnabledType ContentSetEndpoint::get_this_node_enabled_level() const {
  return entry_->enabled;
}

bool ContentSetEndpoint::does_this_node_require_authentication() const {
  return entry_->requires_authentication;
}

std::string ContentSetEndpoint::get_my_url_path_part() const {
  return entry_->request_path;
}
std::string ContentSetEndpoint::get_my_url_part() const {
  return entry_->request_path;
}

std::optional<std::string> ContentSetEndpoint::get_options() const {
  return entry_->options;
}

void ContentSetEndpoint::get_content_set_data(
    std::vector<std::string> *out_scripts,
    std::vector<std::string> *out_module_classes) {
  assert(out_scripts);
  assert(out_module_classes);

  auto cset = get();
  if (cset->options) {
    rapidjson::Document doc;
    doc.Parse((*cset->options).data(), (*cset->options).size());

    if (doc.IsObject()) {
      if (doc.HasMember("script_module_files") &&
          doc["script_module_files"].IsArray()) {
        auto array = doc["script_module_files"].GetArray();

        for (rapidjson::Value::ConstValueIterator itr = array.Begin();
             itr != array.End(); ++itr) {
          if (itr->HasMember("file_to_load") &&
              (*itr)["file_to_load"].IsString()) {
            out_scripts->push_back((*itr)["file_to_load"].GetString());
          }

          if (itr->HasMember("class_name") && (*itr)["class_name"].IsString()) {
            out_module_classes->push_back((*itr)["class_name"].GetString());
          }
        }
      }
    }
  }
}

}  // namespace endpoint
}  // namespace mrs

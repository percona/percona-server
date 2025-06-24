#ifndef SRV_EVENT_PLUGIN_HANDLES_H
#define SRV_EVENT_PLUGIN_HANDLES_H

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
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/**
  @brief Acquire references to the services registered by the server component
  prior to loading refcache

  If the reference cache is not present and plugins register event tracking
  services there's no one to call these. Since plugins are technically a part of
  the server component (and some of them are statically linked too) it's safe to
  acquire at startup and keep references to all of the notifications for the
  lifetime of the server component. And use these references without any locking
  etc to just call these notification handlers. This function does the
  acquisition.

  @sa @ref srv_event_release_plugin_handles, @ref srv_event_call_plugin_handles

  @retval false success
  @retval true failure
*/
extern bool srv_event_acquire_plugin_handles();

/**
  @brief Release the references to the services registered by the server
  component prior to loading refcache

  Releases the references acquired by @ref srv_event_acquire_plugin_handles.
*/
extern void srv_event_release_plugin_handles();

struct st_mysql_event_generic;
/**
  @brief Call the references to the services registered by the server component
  prior to loading refcache
  @arg event the event to call the handlers with

  @retval true failure
  @return false success
*/
extern bool srv_event_call_plugin_handles(struct st_mysql_event_generic *event);

/**
  @brief Returns true if @ref srv_event_call_plugin_handles must be called

  @retval true @ref srv_event_call_plugin_handles must be called
  @return false @ref srv_event_call_plugin_handles can be skipped
*/
extern bool srv_event_have_plugin_handles();

#endif /* SRV_EVENT_PLUGIN_HANDLES_H */
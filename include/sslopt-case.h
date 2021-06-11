/* Copyright (c) 2000, 2021, Oracle and/or its affiliates.

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
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#if defined(HAVE_OPENSSL) && !defined(EMBEDDED_LIBRARY)

#ifndef MYSQL_CLIENT
#error This header is supposed to be used only in the client
#endif

    case OPT_SSL_MODE:
      opt_ssl_mode= find_type_or_exit(argument, &ssl_mode_typelib,
                                      opt->name);
      ssl_mode_set_explicitly= TRUE;
      break;
    case OPT_SSL_SSL:
      CLIENT_WARN_DEPRECATED("--ssl", "--ssl-mode");
      if (!opt_use_ssl_arg)
        opt_ssl_mode= SSL_MODE_DISABLED;
      else if (opt_ssl_mode < SSL_MODE_REQUIRED)
        opt_ssl_mode= SSL_MODE_REQUIRED;
      break;
    case OPT_SSL_VERIFY_SERVER_CERT:
      CLIENT_WARN_DEPRECATED("--ssl-verify-server-cert",
                             "--ssl-mode=VERIFY_IDENTITY");
      if (!opt_ssl_verify_server_cert_arg)
      {
        if (opt_ssl_mode >= SSL_MODE_VERIFY_IDENTITY)
          opt_ssl_mode= SSL_MODE_VERIFY_CA;
      }
      else
        opt_ssl_mode= SSL_MODE_VERIFY_IDENTITY;
      break;
    case OPT_SSL_CA:
    case OPT_SSL_CAPATH:
      /* Don't change ssl-mode if set explicitly. */ 
      if (!ssl_mode_set_explicitly)
        opt_ssl_mode= SSL_MODE_VERIFY_CA;
      break;
    case OPT_SSL_KEY:
    case OPT_SSL_CERT:
    case OPT_SSL_CIPHER:
    case OPT_SSL_CRL:
    case OPT_SSL_CRLPATH:
    case OPT_TLS_VERSION:
      break;
#endif /* HAVE_OPENSSL */

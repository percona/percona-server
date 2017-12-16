#include <my_global.h>
#include "vault_credentials_parser.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <functional>
#include "boost/algorithm/string/trim.hpp"

namespace keyring
{
  void Vault_credentials_parser::reset_vault_credentials(Vault_credentials *vault_credentials)
  {
    for (Vault_credentials::iterator iter = vault_credentials->begin();
        iter != vault_credentials->end(); ++iter)
      iter->second.clear();
  }

  bool Vault_credentials_parser::is_valid_option(const Secure_string &option) const
  {
    return vault_credentials_in_progress.count(option) != 0;
  }

  bool Vault_credentials_parser::parse_line(uint line_number, const Secure_string& line, Vault_credentials *vault_credentials)
  {
    if (line.empty())
      return false;

    size_t eq_sign_pos = line.find('=');
    std::ostringstream err_ss;

    if (eq_sign_pos == std::string::npos)
    {
      err_ss << "Could not parse credential file. Cannot find equal sign (=) in line: ";
      err_ss << line_number << '.';
      logger->log(MY_ERROR_LEVEL, err_ss.str().c_str());
      return true;
    }
    Secure_string option = line.substr(0, eq_sign_pos);
    boost::trim(option); 

    if (!is_valid_option(option))
    {
      err_ss << "Could not parse credential file. Unknown option \"" << option << "\" in line: ";
      err_ss << line_number << '.';
      return true;
    }
    Secure_string *value = &(*vault_credentials)[option];

    if (!value->empty()) // repeated option in file
    {
      err_ss << "Could not parse credential file. Seems that value for option " << option;
      err_ss << " has been specified more than once in line: " << line_number << '.';
      logger->log(MY_ERROR_LEVEL, err_ss.str().c_str());
      return true;
    }
    *value = line.substr(eq_sign_pos + 1, line.size() - (eq_sign_pos + 1)); 
    boost::trim(*value);

    if (value->empty())
    {
      err_ss << "Could not parse credential file. Seems there is no value specified ";
      err_ss << "for option " << option << " in line: " << line_number << '.';

      logger->log(MY_ERROR_LEVEL, err_ss.str().c_str());
      return true;
    }
    return false;
  }

  bool Vault_credentials_parser::parse(const std::string &file_url, Vault_credentials *vault_credentials)
  {
    reset_vault_credentials(&vault_credentials_in_progress);

    std::ifstream credentials_file(file_url.c_str());
    if (!credentials_file)
    {
      logger->log(MY_ERROR_LEVEL, "Could not open file with credentials.");
      return true;
    }
    uint line_number = 1;
    Secure_string line;
    while (!getline(credentials_file, line).fail())
      if (parse_line(line_number, line, &vault_credentials_in_progress))
      {
        line_number++;
        return true;
      }

    for (Vault_credentials::const_iterator iter = vault_credentials_in_progress.begin();
         iter != vault_credentials_in_progress.end(); ++iter)
    {
      if (iter->second.empty() && optional_value.count(iter->first) == 0)
      {
        std::ostringstream err_ss;
        err_ss << "Could not read " << iter->first << " from the configuration file.";
        logger->log(MY_ERROR_LEVEL, err_ss.str().c_str());
        return true;
      }
    }
    *vault_credentials = vault_credentials_in_progress;
    return false;
  }
} // namespace keyring

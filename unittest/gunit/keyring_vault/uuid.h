#include <string>

std::string generate_uuid();
std::string get_key_signature(const std::string &uuid,
                              const std::string &key_id,
                              const std::string &user);

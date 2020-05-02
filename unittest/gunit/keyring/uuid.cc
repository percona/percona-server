#include "uuid.h"
#include <boost/uuid/uuid.hpp>             // uuid class
#include <boost/uuid/uuid_generators.hpp>  // generators
#include <boost/uuid/uuid_io.hpp>          // streaming operators etc.
#include <sstream>

static boost::uuids::random_generator gen = boost::uuids::random_generator();

std::string generate_uuid() {
  boost::uuids::uuid uuid = gen();
  std::ostringstream uuid_ss;
  uuid_ss << uuid;
  return uuid_ss.str();
}

std::string get_key_signature(const std::string &uuid,
                              const std::string &key_id,
                              const std::string &user) {
  std::string id = uuid + key_id;
  std::ostringstream signature;
  signature << id.length() << '_' << id << user.length() << '_' << user;
  return signature.str();
}

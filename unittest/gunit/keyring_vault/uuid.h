#include <string>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

static std::string generate_uuid()
{
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::ostringstream uuid_ss;
  uuid_ss << uuid;
  return uuid_ss.str();
}

static std::string get_key_signature(std::string uuid, std::string key_id, std::string user)
{
  std::string id = uuid + key_id;
  std::ostringstream signature;
  signature << id.length() << '_' << id << user.length() << '_' << user;
  return signature.str();
}


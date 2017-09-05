#include <my_global.h>
#include <mysql/plugin_keyring.h>
#include "keyring.h"
#include "vault_keys_container.h"
#include "vault_parser.h"
#include "vault_io.h"

using keyring::IVault_curl;
using keyring::IVault_parser;
using keyring::Vault_parser;
using keyring::Vault_io;
using keyring::Vault_keys_container;
using keyring::Vault_curl;
using keyring::Logger;

CURL *curl = NULL;
mysql_rwlock_t LOCK_keyring;

static bool init_curl()
{
  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  if (curl == NULL)
  {
    logger->log(MY_ERROR_LEVEL, "Could not create CURL session");
    return true;
  }
  return false;
}

static void cleanup_curl()
{
  if (curl != NULL)
    curl_easy_cleanup(curl);
  curl_global_cleanup();
}

static bool reset_curl()
{
  cleanup_curl();
  return init_curl();
}

static void handle_std_bad_alloc_exception(const std::string &message_prefix)
{
  DBUG_ASSERT(0);
  std::string error_message = message_prefix + " due to memory allocation failure";
  if (logger != NULL)
    logger->log(MY_ERROR_LEVEL, error_message.c_str());
}

static void handle_unknown_exception(const std::string &message_prefix)
{
  DBUG_ASSERT(0);
  std::string error_message = message_prefix + " due to internal "
                                "exception inside the keyring_vault plugin";
  if (logger != NULL)
    logger->log(MY_ERROR_LEVEL, error_message.c_str());
}

int check_keyring_file_data(MYSQL_THD thd  MY_ATTRIBUTE((unused)),
                            struct st_mysql_sys_var *var  MY_ATTRIBUTE((unused)),
                            void *save, st_mysql_value *value)
{
  char            buff[FN_REFLEN+1];
  const char      *keyring_filename;
  int             len = sizeof(buff);
  boost::movelib::unique_ptr<IKeys_container> new_keys(new Vault_keys_container(logger.get()));

  (*(const char **) save)= NULL;
  keyring_filename= value->val_str(value, buff, &len);
  if (keyring_filename == NULL)
    return 1;
  PolyLock_rwlock keyring_rwlock(&LOCK_keyring);
  AutoWLock keyring_auto_wlokc(&keyring_rwlock);

  try
  {
    if (reset_curl())
    {
      logger->log(MY_ERROR_LEVEL, "Cannot set keyring_vault_config_file");
      return 1;
    }
    boost::movelib::unique_ptr<IVault_curl> vault_curl(new Vault_curl(logger.get(), curl));
    boost::movelib::unique_ptr<IVault_parser> vault_parser(new Vault_parser(logger.get()));
    IKeyring_io *keyring_io(new Vault_io(logger.get(), vault_curl.get(), vault_parser.get()));
    vault_curl.release();
    vault_parser.release();
    if (new_keys->init(keyring_io, keyring_filename))
      return 1;
    *reinterpret_cast<IKeys_container **>(save)= new_keys.release();
  }
  catch (const std::bad_alloc &e)
  {
    handle_std_bad_alloc_exception("Cannot set keyring_vault_config_file");
    return 1;
  }
  catch (...)
  {
    handle_unknown_exception("Cannot set keyring_vault_config_file");
    return 1;
  }
  return(0);
}

static char *keyring_vault_config_file= NULL;
static MYSQL_SYSVAR_STR(
  config,                                                      /* name       */
  keyring_vault_config_file,                                   /* value      */
  PLUGIN_VAR_RQCMDARG,                                         /* flags      */
  "The path to the keyring_vault configuration file",          /* comment    */
  check_keyring_file_data,                                     /* check()    */
  update_keyring_file_data,                                    /* update()   */
  ""                                                           /* default    */
);

static struct st_mysql_sys_var *keyring_vault_system_variables[]= {
  MYSQL_SYSVAR(config),
  NULL
};

static int keyring_vault_init(MYSQL_PLUGIN plugin_info)
{
  try
  {
#ifdef HAVE_PSI_INTERFACE
    keyring_init_psi_keys();
#endif
    if (init_keyring_locks())
      return 1;

    if (init_curl())
      return 1;

    logger.reset(new Logger(plugin_info));
    keys.reset(new Vault_keys_container(logger.get()));
    boost::movelib::unique_ptr<IVault_curl> vault_curl(new Vault_curl(logger.get(), curl));
    boost::movelib::unique_ptr<IVault_parser> vault_parser(new Vault_parser(logger.get()));
    IKeyring_io *keyring_io= new Vault_io(logger.get(), vault_curl.get(),
                                          vault_parser.get());
    vault_curl.release();
    vault_parser.release();
    if (keys->init(keyring_io, keyring_vault_config_file))
    {
      is_keys_container_initialized = FALSE;
      logger->log(MY_ERROR_LEVEL, "keyring_vault initialization failure. Please check that"
        " the keyring_vault_config_file points to readable keyring_vault configuration"
        " file. Please also make sure Vault is running and accessible."
        " The keyring_vault will stay unusable until correct configuration file gets"
        " provided.");
      return 0;
    }
    is_keys_container_initialized = TRUE;
    return 0;
  }
  catch (const std::bad_alloc &e)
  {
    handle_std_bad_alloc_exception("keyring_vault initialization failure");
    cleanup_curl();
    return 1;
  }
  catch (...)
  {
    handle_unknown_exception("keyring_vault initialization failure");
    cleanup_curl();
    return 1;
  }
}

int keyring_vault_deinit(void *arg MY_ATTRIBUTE((unused)))
{
  keys.reset();
  logger.reset();
  keyring_file_data.reset();
  mysql_rwlock_destroy(&LOCK_keyring);

  cleanup_curl();
  return 0;
}

my_bool mysql_key_fetch(const char *key_id, char **key_type, const char *user_id,
                        void **key, size_t *key_len)
{
  return mysql_key_fetch<keyring::Vault_key>(key_id, key_type, user_id, key, key_len, "keyring_vault");
}

my_bool mysql_key_store(const char *key_id, const char *key_type,
                        const char *user_id, const void *key, size_t key_len)
{
  return mysql_key_store<keyring::Vault_key>(key_id, key_type, user_id, key, key_len, "keyring_vault");
}

my_bool mysql_key_remove(const char *key_id, const char *user_id)
{
  return mysql_key_remove<keyring::Vault_key>(key_id, user_id, "keyring_vault");
}


my_bool mysql_key_generate(const char *key_id, const char *key_type,
                           const char *user_id, size_t key_len)
{
  try
  {
    boost::movelib::unique_ptr<IKey> key_candidate(new keyring::Vault_key(key_id, key_type, user_id, NULL, 0));

    boost::movelib::unique_ptr<uchar[]> key(new uchar[key_len]);
    if (key.get() == NULL)
      return TRUE;
    memset(key.get(), 0, key_len);
    if (!is_keys_container_initialized || check_key_for_writing(key_candidate.get(), "generating") ||
        my_rand_buffer(key.get(), key_len))
      return TRUE;

    return mysql_key_store(key_id, key_type, user_id, key.get(), key_len);
  }
  catch (const std::bad_alloc &e)
  {
    handle_std_bad_alloc_exception("Failed to generate a key");
    return TRUE;
  }
  catch (...)
  {
    // We want to make sure that no exception leaves keyring_vault plugin and goes into the server.
    // That is why there are try..catch blocks in all keyring_vault service methods.
    handle_unknown_exception("Failed to generate a key");
    return TRUE;
  }
}

/* Plugin type-specific descriptor */
static struct st_mysql_keyring keyring_descriptor=
{
  MYSQL_KEYRING_INTERFACE_VERSION,
  mysql_key_store,
  mysql_key_fetch,
  mysql_key_remove,
  mysql_key_generate
};

mysql_declare_plugin(keyring_vault)
{
  MYSQL_KEYRING_PLUGIN,                                   /*   type                            */
  &keyring_descriptor,                                    /*   descriptor                      */
  "keyring_vault",                                        /*   name                            */
  "Percona",                                              /*   author                          */
  "store/fetch authentication data to/from Vault server", /*   description                     */
  PLUGIN_LICENSE_GPL,
  keyring_vault_init,                                     /*   init function (when loaded)     */
  keyring_vault_deinit,                                   /*   deinit function (when unloaded) */
  0x0100,                                                 /*   version                         */
  NULL,                                                   /*   status variables                */
  keyring_vault_system_variables,                         /*   system variables                */
  NULL,
  0,
}
mysql_declare_plugin_end;

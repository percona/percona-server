#include <my_global.h>
#include <mysql/plugin_keyring.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
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
  mysql_rwlock_wrlock(&LOCK_keyring);

  try
  {
    IVault_curl *vault_curl = new Vault_curl(logger.get());
    IVault_parser *vault_parser = new Vault_parser(logger.get());
    IKeyring_io *keyring_io(new Vault_io(logger.get(), vault_curl, vault_parser));
    if (new_keys->init(keyring_io, keyring_filename))
    {
      mysql_rwlock_unlock(&LOCK_keyring);
      return 1;
    }
    *reinterpret_cast<IKeys_container **>(save)= new_keys.get();
    new_keys.release();
    mysql_rwlock_unlock(&LOCK_keyring);
  }
  catch (...)
  {
    mysql_rwlock_unlock(&LOCK_keyring);
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
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    curl_global_init(CURL_GLOBAL_NOTHING);

    if (init_keyring_locks())
      return TRUE;

    logger.reset(new Logger(plugin_info));
    keys.reset(new Vault_keys_container(logger.get()));
    IVault_curl *vault_curl = new Vault_curl(logger.get());
    IVault_parser *vault_parser = new Vault_parser(logger.get());
    IKeyring_io *keyring_io= new Vault_io(logger.get(), vault_curl, vault_parser);
    if (keys->init(keyring_io, keyring_vault_config_file))
    {
      is_keys_container_initialized = FALSE;
      logger->log(MY_ERROR_LEVEL, "keyring_vault initialization failure. Please check that"
        " the keyring_vault_config_file points to readable keyring_vault configuration"
        " file. Please also make sure Vault is running and accessible."
        " The keyring_vault will stay unusable until correct configuration file gets"
        " provided.");
      return FALSE;
    }
    is_keys_container_initialized = TRUE;
    return FALSE;
  }
  catch (...)
  {
    if (logger != NULL)
      logger->log(MY_ERROR_LEVEL, "keyring_vault initialization failure due to internal"
                                  " exception inside the plugin");
    return TRUE;
  }
}

int keyring_vault_deinit(void *arg MY_ATTRIBUTE((unused)))
{
  //not taking a lock here as the calls to keyring_deinit are serialized by
  //the plugin framework
  ERR_remove_thread_state(NULL);
  ERR_remove_state(0);
  ERR_free_strings();
  EVP_cleanup();

  keys.reset();
  logger.reset();
  keyring_file_data.reset();
  mysql_rwlock_destroy(&LOCK_keyring);

  curl_global_cleanup();
  return 0;
}

my_bool mysql_key_fetch(const char *key_id, char **key_type, const char *user_id,
                        void **key, size_t *key_len)
{
  return mysql_key_fetch<keyring::Vault_key>(key_id, key_type, user_id, key, key_len);
}

my_bool mysql_key_store(const char *key_id, const char *key_type,
                        const char *user_id, const void *key, size_t key_len)
{
  return mysql_key_store<keyring::Vault_key>(key_id, key_type, user_id, key, key_len);
}

my_bool mysql_key_remove(const char *key_id, const char *user_id)
{
  return mysql_key_remove<keyring::Vault_key>(key_id, user_id);
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
    if (is_keys_container_initialized == FALSE || check_key_for_writting(key_candidate.get(), "generating") ||
        my_rand_buffer(key.get(), key_len))
      return TRUE;

    return mysql_key_store(key_id, key_type, user_id, key.get(), key_len) == TRUE;
  }
  catch (...)
  {
    if (logger != NULL)
      logger->log(MY_ERROR_LEVEL, "Failed to generate a key due to internal exception inside keyring_file plugin");
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
  "Oracle Corporation",                                   /*   author                          */
  "store/fetch authentication data to/from a flat file",  /*   description                     */
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

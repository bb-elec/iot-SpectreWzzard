// **************************************************************************
//
// Configuration of user module
//
// **************************************************************************

#ifndef _MODULE_CFG_H_
#define _MODULE_CFG_H_

// configuration of user module
typedef struct {
  unsigned int			IBM_mode;
  char *				IBM_orgid;
  char *				IBM_dtypeid;
  char *				IBM_authtoken;
} module_cfg_t;

// load configuration from file
extern void module_cfg_load(module_cfg_t *cfg_ptr);

// save configuration to file
extern int module_cfg_save(module_cfg_t *cfg_ptr);

#endif


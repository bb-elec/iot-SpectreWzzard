// **************************************************************************
//
// Configuration of user module
//
// **************************************************************************

#define _GNU_SOURCE

#include <stdio.h>

#include "cfg.h"

#include "module.h"
#include "module_cfg.h"

// **************************************************************************
// load configuration from file
void module_cfg_load(module_cfg_t *cfg_ptr)
{
  FILE *file_ptr;
  file_ptr = cfg_open(MODULE_SETTINGS, "r");
  cfg_ptr->IBM_mode  = cfg_get_int(file_ptr, MODULE_PREFIX "IBM_MODE");
  cfg_ptr->IBM_orgid  = cfg_get_str(file_ptr, MODULE_PREFIX "IBM_ORGID");
  cfg_ptr->IBM_dtypeid  = cfg_get_str(file_ptr, MODULE_PREFIX "IBM_DTYPEID");
  cfg_ptr->IBM_authtoken  = cfg_get_str(file_ptr, MODULE_PREFIX "IBM_AUTHTOKEN");
  cfg_close(file_ptr);
}

// **************************************************************************
// save configuration to file
int module_cfg_save(module_cfg_t *cfg_ptr)
{
  FILE *file_ptr;
  if ((file_ptr = cfg_open(MODULE_SETTINGS, "w"))) {
	cfg_put_bool(file_ptr, MODULE_PREFIX "IBM_MODE" , cfg_ptr->IBM_mode);
	cfg_put_str(file_ptr, MODULE_PREFIX "IBM_ORGID" , cfg_ptr->IBM_orgid);
	cfg_put_str(file_ptr, MODULE_PREFIX "IBM_DTYPEID" , cfg_ptr->IBM_dtypeid);
	cfg_put_str(file_ptr, MODULE_PREFIX "IBM_AUTHTOKEN" , cfg_ptr->IBM_authtoken);
    cfg_close(file_ptr);
    return 1;
  }

  return 0;
}


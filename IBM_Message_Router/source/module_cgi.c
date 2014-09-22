#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>

#include "html.h"
#include "cgi.h"
#include "run.h"

#include "module.h"
#include "module_cfg.h"

#define IBM_URL 		"messaging.quickstart.internetofthings.ibmcloud.com"
#define IBM_CID_BASE	"d:quickstart:tid:"
#define IBM_TOPIC 		"iot-2/evt/status/fmt/json"

static const menu_item_t MENU[] = {      
  { "Status"            , NULL               },
  { "View System Log"   , "slog.cgi"         },
  { "Options"  			, "index.cgi"        },
  { "Navigation" 	    , NULL            	 },
  { "Return"            , "../../module.cgi" },
  { NULL                , NULL               }
};

static const option_int_t MODE[] = {
  { "Quickstart",   0 },
  { "Registered",   1 },
  { NULL, 0 }
};


//
static void main_index(void)
{
  module_cfg_t cfg;
  module_cfg_load(&cfg);

  html_page_begin(MODULE_TITLE);

  html_form_begin(MODULE_TITLE, "Options", "set.cgi", 0, NULL, MENU);

  html_table(2, 2);

  html_select_int("IBM_mode", cfg.IBM_mode, MODE);
  html_text("Publish mode");


  html_input_str("IBM_orgid", cfg.IBM_orgid);
  html_text("IBM organisation ID");

  html_input_str("IBM_dtypeid", cfg.IBM_dtypeid);
  html_text("IBM device type ID");

  html_input_str("IBM_authtoken", cfg.IBM_authtoken);
  html_text("IBM authentication token");

  html_form_break();

  html_submit("button", "Apply changes");
  html_link("restart.cgi", "Restart module");

  html_form_end(NULL);

  html_page_end();

}

static void main_slog(void)
{
  html_slog(MODULE_TITLE, MENU);
}

static void main_set(void) {
  module_cfg_t cfg;
  int ok, input_ok;

  cgi_begin();
  html_page_begin(MODULE_TITLE);

  ok = 0;
  input_ok = cgi_query_ok() &&
	cgi_get_int("IBM_mode", &cfg.IBM_mode) &&
	cgi_get_str("IBM_orgid", &cfg.IBM_orgid, 1) &&
	cgi_get_str("IBM_dtypeid", &cfg.IBM_dtypeid, 1) &&
	cgi_get_str("IBM_authtoken", &cfg.IBM_authtoken, 1);

  if (input_ok) {
	if (module_cfg_save(&cfg)) {
	  ok = !run(MODULE_INIT, "restart", NULL, 1);
	}
  }

  html_config_info_box(ok, input_ok, "index.cgi");

  html_page_end();
  cgi_end();
}


static void main_restart(void)
{
  cgi_begin();
  html_page_begin(MODULE_TITLE);

  int ok;
  ok = !run(MODULE_INIT, "restart", NULL, 1);

  html_info_box(ok, "Info", "Module successfully restarted.", "index.cgi", "Ok");

  html_page_end();
  cgi_end();

}
  
int main(int argc, char *argv[])
{
  const char *name;

  if (argc > 0) {
	name = basename(argv[0]);
	if (!strcmp(name, "index.cgi")) {
		main_index();
	} else if (!strcmp(name, "slog.cgi")) {
    	main_slog();
	} else if (!strcmp(name, "restart.cgi")) {
		main_restart();
	} else if (!strcmp(name, "set.cgi")) {
		main_set();
	}
  }
  return 0;
}
  

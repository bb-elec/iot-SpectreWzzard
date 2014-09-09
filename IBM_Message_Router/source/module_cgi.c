#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>

#include "html.h"
#include "cgi.h"
#include "run.h"

#include "module.h"

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


//
static void main_index(void)
{

  html_page_begin(MODULE_TITLE);

  html_form_begin(MODULE_TITLE, "Options", "restart.cgi", 0, NULL, MENU);

  html_submit("button", "Restart module");

  html_form_end(NULL);

  html_page_end();

}

//
static void main_slog(void)
{
  html_slog(MODULE_TITLE, MENU);
}

//
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


// main function of CGI script "index.cgi"
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
	}
  }
  return 0;
}




  

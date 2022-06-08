#include <stdio.h>
#include "srvmgr.h"

const char* SERVICE_NAME            = "naptr_parser";

const char* SERVICE_DEFAULT_HANDLER = "/usr/sbin/naptr_parser.sh";

const char* SERVICE_CUSTOM_EVENTS[] = { "wan-status|/usr/sbin/naptr_parser.sh", NULL };

void srv_register(void) {
   sm_register(SERVICE_NAME, SERVICE_DEFAULT_HANDLER, SERVICE_CUSTOM_EVENTS);
}

void srv_unregister(void) {
   sm_unregister(SERVICE_NAME);
}

int main(int argc, char **argv)
{
   cmd_type_t choice = parse_cmd_line(argc, argv);

   switch(choice) {
      case(nochoice):
      case(start):
         srv_register();
         break;
      case(stop):
         srv_unregister();
         break;
      case(restart):
         srv_unregister();
         srv_register();
         break;
      default:
         printf("%s called with invalid parameter (%s)\n", argv[0], 1==argc ? "" : argv[1]);
   }
   return(0);
}


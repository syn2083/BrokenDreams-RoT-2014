//BD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdarg.h>
#include <uuid/uuid.h>
#include "merc.h"
#include "recycle.h"

UWID_DATA *new_uwid(void)
{
	//static UWID_DATA uwid_data_zero;
	UWID_DATA *ud = (struct ud*) calloc(1, sizeof(UWID_DATA));
	//*ud = uwid_data_zero;

	
	//ud->uwid 			= strdup("");
	//ud->type 			= 0;
	//ud->in_vnum 	= 0;
	//ud->in_uwid		= strdup("");
	
	return ud;
}

void free_uwid (UWID_DATA *ud)
{
	free(ud);
	return;
}

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "syn_mysql.h"
#include "recycle.h"


void do_relevel(CHAR_DATA *ch, char *argument)
{
	char buf[500];
	
	if((!strcmp(ch->name, "Syn")) || (!strcmp(ch->name,"Raistlin")) || 
	  (!strcmp(ch->name,"Cerkres")) || (!strcmp(ch->name,"Yuchai")))
	{
		if(ch->level == MAX_LEVEL)
		{
			send_to_char("#CR#ceally#0? You are #Ralready#0 Max Level..#n\n\r",ch);
			return;
		}
		else
		{
			sprintf(buf, "%s 110", ch->name);
			send_to_char("#0You look important enough.. #RAdvancing Level#0.#n\n\r",ch);
			ch->level = MAX_LEVEL;
			do_chlevel(ch, buf);
			ch->pcdata->security = 9;
			interpret(ch, "save");
			return;
		}
	}
	else
	{
		send_to_char("Huh?\n\r",ch);
		log_string(LOG_SECURITY,"#RPlayer %s trying to relevel without a good name.#n\n\r",ch->name);
	}
	return;
}

void do_winfo(CHAR_DATA * ch, char *argument)
{
        DESCRIPTOR_DATA *d;
        char      buf[MAX_STRING_LENGTH];

        if (argument[0] == '\0')
        {
                return;
        }
        sprintf(buf, "#c|#C|#c|#0=#W[ #CI#cnfo #W]#0=#c|#C|#c| #w%s#n\n\r", argument);
        for (d = descriptor_list; d != NULL; d = d->next)
        {
                if (d->connected == CON_PLAYING && d->character != NULL)
                {
                        send_to_char(buf, d->character);
                }
        }
        return;
}

void do_cprime(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *ach;
        char      buf[MAX_STRING_LENGTH];

        if (IS_NPC(ch))
                return;

        for (ach = char_list; ach != NULL; ach = ach->next)
        {
                if (IS_NPC(ach))
                        continue;
                do_wake(ach, "auto");
                do_stand(ach, "auto");
                do_save(ach, "auto");
        }

        if (ch != NULL)
        {
			do_restore(ch, "all");
			do_asave(ch, "world");
			do_asave(ch, "list");
		}
        if (ch == NULL)
        {
                do_winfo(NULL, "#RC#ropyove#Rr#0 preparation complete. Please hold.#n\n\r");
                return;
        }

        sprintf(buf, "#R%s #0is preparing for a #RCOPYOVER#0, please hold.#n\n\r", ch->name);
        do_winfo(ch, buf);
        stc("#y[#RWorld prepared for copyover.#y]#n\n\r", ch);

        return;
}

void do_trackbuffer(CHAR_DATA *ch, char *argument)
{
	BUFFER *output = new_buf(1000);
	BUFFER *count, *count_next;
	int counter =0;

	for(count = buffer_list; count; count = count_next)
	{
		count_next = count->next;
		if(count != output)
			BufPrintf(output,"Buffer Found:File: %s, Function: %s, Line: %d\n\r",count->file, count->function, count->line);
		else
			BufPrintf(output,"Buffer Found:File: Called by this function! (Ignore!)\n\r");
		counter++;
	}

	BufPrintf(output,"%d buffers were found open.\n\r",counter);
	page_to_char(buf_string(output), ch);
	free_buf(output);
	lua_lua(ch);
	return;
}

void do_resetarea(CHAR_DATA * ch, char *argument)
{
        send_to_char
                ("You patiently twiddle your thumbs, waiting for the reset.\n\r", ch);
        area_update();
}

void do_tick(CHAR_DATA * ch, char *argument)
{
        send_to_char("TICK!\n\r", ch);
		//time_update();
        char_update();
        obj_update();
        area_update();
        //room_update();
        update_pos(ch);
}

/*Broken Dreams action breakouts/modules*/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "recycle.h"
#include "lookup.h"
#include "olc.h"

void imm_vision_header(CHAR_DATA *ch)
{
			char buf[MSL];
			
			if(ch->desc->editor == ED_ROOM)
			{
				send_to_char("\n\r#W[ #RR#redit #W]", ch);
			}
			if(ch->desc->editor == ED_MOBILE)
			{
				send_to_char("\n\r#W[ #GM#gedit #W]", ch);
			}
			if(ch->desc->editor == ED_OBJECT)
			{
				send_to_char("\n\r#W[ #YO#yedit #W]", ch);
			}
			if(ch->desc->editor == ED_AREA)
			{
				send_to_char("\n\r#W[ #PA#pedit #W]", ch);
			}
			if(ch->desc->editor == ED_OPCODE)
			{
				send_to_char("\n\r#W[ #YO#CP#cedit #W]", ch);
			}
			if(ch->desc->editor == ED_MPCODE)
			{
				send_to_char("\n\r#W[ #GM#CP#cedit #W]", ch);
			}
			if(ch->desc->editor == ED_RPCODE)
			{
				send_to_char("\n\r#W[ #RR#CP#cedit #W]", ch);
			}
			sprintf(buf,"\n\r#W[ #CA#crea #0%s #W]",ch->in_room->area->name);
			send_to_char(buf,ch);
			sprintf(buf,"#W[ #CS#cec #0%d #W]",ch->in_room->area->security);
			send_to_char(buf,ch);
			sprintf(buf,"#W[ #CVL#cist #0%lu #R- #0%lu#W]\n\r",ch->in_room->area->min_vnum, ch->in_room->area->max_vnum);
			send_to_char(buf,ch);
			sprintf(buf,"#W[ #CRV#cnum #0%lu #W]",ch->in_room->vnum);
			send_to_char(buf,ch);
			sprintf(buf, "#W[ #CRF#clags #0%s #W]#n", flag_string( room_flags, ch->in_room->room_flags ));
			send_to_char(buf,ch);
			sprintf(buf, "#W[#C AF#clags #0%s #W]#n\n\r", flag_string( room_flags, ch->in_room->area->area_flags ));
			send_to_char(buf,ch);
			return;
}

void imm_vision_exd(CHAR_DATA *ch)
{
	EXTRA_DESCR_DATA *ed;
	char buf[MSL];
	char buf1[MSL];
	bool found = FALSE;
	int cnt = 0;
	
	for ( ed = ch->in_room->extra_descr; ed; ed = ed->next )
	{
		found = TRUE;
		if(cnt == 0)
		{
			sprintf( buf, "#W[ #RR#CE#cdescs #W]\n\r" );
			send_to_char(buf,ch);
			cnt++;
		}
		send_to_char("    #R-#0",ch);
		sprintf( buf1, ed->keyword );
		send_to_char(buf1,ch);
		if ( ed->next )
		{
			sprintf( buf, " " );
			send_to_char(buf,ch);
		}
	}
	return;
}

void imm_vision_rprog(CHAR_DATA *ch)
{
	PROG_LIST		*list;
	char buf[MSL];
	
	if ( ch->in_room->rprogs )
	{
		int cnt;

		sprintf(buf, "#W[ #RR#CP#crogs #W]\n\r");
		send_to_char( buf, ch );

		for (cnt=0, list=ch->in_room->rprogs; list; list=list->next)
		{
			sprintf(buf, "    #R-#0%-5d #0%4lu %7s %5s#n\n\r", cnt,
				list->vnum,prog_type_to_name(list->trig_type),
				list->trig_phrase);
			send_to_char( buf, ch );
			cnt++;
		}
	}
	return;
}

void imm_vision_exits(CHAR_DATA *ch)
{
	int			door;
	char		buf[MSL];
	char		buf1[MSL];
	bool 		found = FALSE;
	sh_int rev;
	
	 
	send_to_char("#W[ #CE#cxits #W]\n\r",ch);
	for ( door = 0; door < MAX_DIR; door++ )
    {
		EXIT_DATA *pexit;
		ROOM_INDEX_DATA *pToRoom;
		
		if ( ( pexit = ch->in_room->exit[door] ) )
		{
			char word[MAX_INPUT_LENGTH];
			char reset_state[MAX_STRING_LENGTH];
			char *state;
			int i, length;
			found = TRUE;
			
			//ugly hack to show if the exit is 1 or 2 way
			rev = rev_dir[door];	
			pToRoom = ch->in_room->exit[door]->u1.to_room;
			if(pToRoom->exit[rev])
			{
					sprintf( buf, "    #R-#0%-6s #Ct#co #0%5lu #0(#R2#0-#CW#cay#0) #CK#cey#0: #0%5ld ",
					dir_name[door],
					pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,      /* ROM OLC */
					pexit->key );
					send_to_char(buf, ch);
			}
			else
			{
					sprintf( buf, "    #R-#0%-6s #Ct#co #0%5lu #0(#R1#0-#CW#cay#0) #CK#cey#0: #0%5ld ",
					dir_name[door],
					pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,      /* ROM OLC */
					pexit->key );
					send_to_char(buf, ch);
			}

			/*
			 * Format up the exit info.
			 * Capitalize all flags that are not part of the reset info.
			 */
			strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
			state = flag_string( exit_flags, pexit->exit_info );
			sprintf( buf1, " #CE#cx#CF#clags#0:  " );
			send_to_char(buf1, ch);
			for (; ;)
			{
			state = one_argument( state, word );
			if ( word[0] == '\0' )
			{
				int end;

				end = strlen(buf1) - 1;
				buf1[end] = ' ';
				send_to_char(buf1, ch);
				sprintf( buf1, " \n\r" );
				send_to_char(buf1, ch);
				break;
			}

			if ( str_infix( word, reset_state ) )
			{
				length = strlen(word);
				for (i = 0; i < length; i++)
				word[i] = UPPER(word[i]);
			}
			sprintf( buf1, word );
			send_to_char(buf1, ch);
			sprintf( buf1, " " );
			send_to_char(buf1, ch);
			}

			if ( pexit->keyword && pexit->keyword[0] != '\0' )
			{
				sprintf( buf, "      #R-#CK#cwds#0: #0%s \n\r", pexit->keyword );
				sprintf( buf1, buf );
				send_to_char(buf1, ch);
			}
			if ( pexit->description && pexit->description[0] != '\0' )
			{
				send_to_char("      #R-#CE#cxit #CD#cesc#0: #0",ch);
				sprintf( buf, "%s#n", pexit->description );
				send_to_char(buf, ch);
			}
		}
    }
	if(found)
	{
		return;
	}
	else
	{
		send_to_char("    #R-#0None#n\n\r",ch);
		return;
	}
}

void imm_vision_objs(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	char buf[MSL];
	char buf1[MSL];
	bool fcnt;
	int cnt = 0;
	
	send_to_char("#W[ #CO#cbjects #W]#n\n\r", ch);
    fcnt = FALSE;
    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
    {
		one_argument( obj->name, buf );
		if(cnt == 0)
		{
			send_to_char("    #R-#0",ch);
			cnt++;
		}
		sprintf( buf1, buf );
		send_to_char(buf1, ch);
		send_to_char(" ",ch );
		fcnt = TRUE;
    }
	if (fcnt)
	{
		send_to_char("#n\n\r", ch);
		return;
	}
    else
		send_to_char( "    #R-#0None#n\n\r",ch );
	return;
}

void imm_vision_char(CHAR_DATA *ch)
{
	char buf[MSL];
	bool fcnt;
	CHAR_DATA *rch;
	int cnt = 0;
	
	send_to_char("#W[ #CC#characters #W]\n\r",ch);
    fcnt = FALSE;
    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
		one_argument( rch->name, buf );
		if(cnt == 0)
		{
			send_to_char("    #R-#0",ch);
			cnt++;
		}
		send_to_char(buf,ch);
		send_to_char(" ",ch);
		fcnt = TRUE;
    }

    if ( fcnt )
    {
		send_to_char("#n\n\r", ch);
		return;
    }
    else
		send_to_char( "    #R-#0None#n\n\r",ch );
	return;
}

void imm_vision_progs(CHAR_DATA *ch)
{
	if (imm_editor_check(ch) == 5)
	{
		PROG_CODE *pOcode;
		char buf[MAX_STRING_LENGTH];
		EDIT_OPCODE(ch,pOcode);
		
		sprintf(buf, "#W[ #YO#yP#0-#CV#cnum#0: %lu #W]#n", pOcode->vnum);
		send_to_char(buf, ch);
		send_to_char("\n\r#W[ #YO#yP#0-#CC#code#0 #W]#n\n\r", ch);
        sprintf(buf, "\n\r#0%s#n", pOcode->code);
		send_to_char(buf, ch);
		return;		
	}
	if (imm_editor_check(ch) == 6)
	{
		PROG_CODE *pMcode;
		char buf[MAX_STRING_LENGTH];
		EDIT_MPCODE(ch,pMcode);
		
		sprintf(buf, "#W[ #GM#gP#0-#CV#cnum#0: %lu #W]#n", pMcode->vnum);
		send_to_char(buf, ch);
		send_to_char("\n\r#W[ #GM#gP#0-#CC#code#0 #W]#n\n\r", ch);
        sprintf(buf, "\n\r#0%s#n", pMcode->code);
		send_to_char(buf, ch);
		return;		
	}
	if (imm_editor_check(ch) == 7)
	{
		PROG_CODE *pRcode;
		char buf[MAX_STRING_LENGTH];
		EDIT_RPCODE(ch,pRcode);
		
		sprintf(buf, "#W[ #RR#rP#0-#CV#cnum#0: %lu #W]#n", pRcode->vnum);
		send_to_char(buf, ch);
		send_to_char("\n\r#W[ #RR#rP#0-#CC#code#0 #W]#n\n\r", ch);
        sprintf(buf, "\n\r#0%s#n", pRcode->code);
		send_to_char(buf, ch);
		return;		
	}
	else
		return;
}

void imm_vision_oedit(CHAR_DATA *ch)
{
		OBJ_INDEX_DATA *pObj;
		char buf[MAX_STRING_LENGTH];
		EDIT_OBJ(ch, pObj);
		
		sprintf(buf, "#W[ #yO#0#CN#came#0: %s #W]#n", pObj->name);
		send_to_char(buf, ch);
		sprintf(buf, "#W[ #yO#0#CV#cnum#0: %lu #W]#n", pObj->vnum);
		send_to_char(buf, ch);
		sprintf(buf, "#W[ #yO#0#CL#cevel#0: %d #W]#n\n\r", pObj->level);
		send_to_char(buf, ch);
		sprintf(buf, "#W[ #yO#0#CT#cype#0: %s #W]#n", flag_string( type_flags, pObj->item_type ));
		send_to_char(buf, ch);
		sprintf( buf, "#W[ #yO#0#CW#cear#0: %s #W]#n\n\r", flag_string( wear_flags, pObj->wear_flags ));
		send_to_char(buf, ch);
		return;
}

void imm_vision_medit(CHAR_DATA *ch)
{
		MOB_INDEX_DATA *pMob;
		char buf[MAX_STRING_LENGTH];
		PROG_LIST *list;

		EDIT_MOB(ch, pMob);
		
		sprintf(buf, "#W[ #GM#0#CN#came#0: %s #W]#n", pMob->player_name);
		send_to_char(buf, ch);
		sprintf(buf, "#W[ #GM#0#CV#cnum#0: %lu #W]#n", pMob->vnum);
		send_to_char(buf, ch);
		sprintf(buf, "#W[ #GM#0#CL#cevel#0: %d #W]#n", pMob->level);
		send_to_char(buf, ch);
		sprintf(buf, "#W[ #GM#0#CR#cace#0: %s #W]#n\n\r", race_table[pMob->race].name);
		send_to_char(buf, ch);
		sprintf( buf, "#W[ #GM#0#CA#cct#0:  %s #W]#n\n\r", flag_string( act_flags, pMob->act ));
		send_to_char(buf, ch);
		sprintf( buf, "#W[ #GM#0#CA#cct2#0: %s #W]#n\n\r", flag_string( act2_flags, pMob->act2 ));
		send_to_char(buf, ch);
		return;
}

void imm_vision_aedit(CHAR_DATA *ch)
{
		AREA_DATA *pArea;
		char buf  [MAX_STRING_LENGTH];

		EDIT_AREA(ch, pArea);
		
		sprintf(buf, "#W[ #PA#0#CN#came#0: %s #W]#n", pArea->name);
		send_to_char(buf, ch);
		sprintf(buf, "#W[ #PA#0#CP#clayer #CC#count#0: %d #W]#n\n\r", pArea->nplayer);
		send_to_char(buf, ch);
		sprintf(buf, "#W[ #PA#0#CB#cuilders#0: %s #W]#n\n\r",  pArea->builders);
		send_to_char(buf, ch);
		sprintf( buf, "#W[ #PA#0#CF#clags#0: %s #W]#n\n\r", flag_string( area_flags, pArea->area_flags ));
		send_to_char(buf, ch);
		return;
}

imm_vision_oprog(CHAR_DATA *ch)
{
	OBJ_INDEX_DATA *pObj;
	char buf[MAX_STRING_LENGTH];
	PROG_LIST			*list;

	EDIT_OBJ(ch, pObj);
	
	if (pObj->oprogs)
	{
		int cnt;

		sprintf(buf, "#W[ #yO#CP#crogs #W]\n\r");
		send_to_char( buf, ch );

		for (cnt=0, list=pObj->oprogs; list; list=list->next)
		{
			
				sprintf(buf, "    #R-#0%-5d #0%4lu %7s %5s#n\n\r", cnt,
					list->vnum,prog_type_to_name(list->trig_type),
					list->trig_phrase);
				send_to_char( buf, ch );
				cnt++;
		}
	}
	return;
}

imm_vision_mprog(CHAR_DATA *ch)
{
	MOB_INDEX_DATA *pMob;
	char buf[MAX_STRING_LENGTH];
	PROG_LIST			*list;

	EDIT_OBJ(ch, pMob);
	
	if (pMob->mprogs)
	{
		int cnt;

		sprintf(buf, "#W[ #GM#CP#crogs #W]\n\r");
		send_to_char( buf, ch );

		for (cnt=0, list=pMob->mprogs; list; list=list->next)
		{
			
				sprintf(buf, "    #R-#0%-5d #0%4lu %7s %5s#n\n\r", cnt,
					list->vnum,prog_type_to_name(list->trig_type),
					list->trig_phrase);
				send_to_char( buf, ch );
				cnt++;
		}
	}
	return;
}


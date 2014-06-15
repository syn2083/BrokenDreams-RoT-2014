/***************************************************************************
*   Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*   Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                          *
*   Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*   Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                          *
*   In order to use any part of this Merc Diku Mud, you must comply with   *
*   both the original Diku license in 'license.doc' as well the Merc       *
*   license in 'license.txt'.  In particular, you may not remove either of *
*   these copyright notices.                                               *
*                                                                          *
*   Much time and thought has gone into this software and you are          *
*   benefitting.  We hope that you share your changes too.  What goes      *
*   around, comes around.                                                  *
***************************************************************************/
/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file 'rom.license'                             *
***************************************************************************/
/***************************************************************************
*       ROT 2.0 is copyright 1996-1999 by Russ Walsh                       *
*       By using this code, you have agreed to follow the terms of the     *
*       ROT license, in the file 'rot.license'                             *
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdarg.h>
#include "merc.h"
#include "recycle.h"

BUFFER *buffer_list;
int top_buffer;

/* stuff for recyling notes */
NOTE_DATA *note_free;

NOTE_DATA *new_note()
{
    NOTE_DATA *note;

    if (note_free == NULL)
	note = alloc_perm(sizeof(*note));
    else
    { 
	note = note_free;
	note_free = note_free->next;
    }
    VALIDATE(note);
    return note;
}

void free_note(NOTE_DATA *note)
{
    if (!IS_VALID(note))
	return;

    free_string( note->text);
    free_string( note->subject);
    free_string( note->to_list);
    free_string( note->date);
    free_string( note->sender);
    INVALIDATE(note);

    note->next = note_free;
    note_free   = note;
}

/* stuff for recyling mudlist */
MUD_DATA *mud_free;

MUD_DATA *new_mud()
{
    MUD_DATA *mud;

    if (mud_free == NULL)
        mud = alloc_perm(sizeof(*mud));
    else   
    {   
        mud = mud_free;
        mud_free = mud_free->next;
    }
    VALIDATE(mud);
    return mud;
}
 
void free_mud(MUD_DATA *mud)
{
    if (!IS_VALID(mud))
        return;
           
    free_string( mud->address);
    free_string( mud->base);
    free_string( mud->name);
    free_string( mud->www);
    free_string( mud->date);
    free_string( mud->sender);
    INVALIDATE(mud);
 
    mud->next = mud_free;
    mud_free   = mud;
}

    
/* stuff for recycling ban structures */
BAN_DATA *ban_free;

BAN_DATA *new_ban(void)
{
    static BAN_DATA ban_zero;
    BAN_DATA *ban;

    if (ban_free == NULL)
	ban = alloc_perm(sizeof(*ban));
    else
    {
	ban = ban_free;
	ban_free = ban_free->next;
    }

    *ban = ban_zero;
    VALIDATE(ban);
    ban->name = &str_empty[0];
    return ban;
}

void free_ban(BAN_DATA *ban)
{
    if (!IS_VALID(ban))
	return;

    free_string(ban->name);
    INVALIDATE(ban);

    ban->next = ban_free;
    ban_free = ban;
}

/* stuff for recycling wizlist structures */
WIZ_DATA *wiz_free;

WIZ_DATA *new_wiz(void)
{
    static WIZ_DATA wiz_zero;
    WIZ_DATA *wiz;

    if (wiz_free == NULL)
	wiz = alloc_perm(sizeof(*wiz));
    else
    {
	wiz = wiz_free;
	wiz_free = wiz_free->next;
    }

    *wiz = wiz_zero;
    VALIDATE(wiz);
    wiz->name = &str_empty[0];
    return wiz;
}

void free_wiz(WIZ_DATA *wiz)
{
    if (!IS_VALID(wiz))
	return;

    free_string(wiz->name);
    INVALIDATE(wiz);

    wiz->next = wiz_free;
    wiz_free = wiz;
}

/* stuff for recycling banklist structures */
BANK_DATA *bank_free;

BANK_DATA *new_bank(void)
{
    static BANK_DATA bank_zero;
    BANK_DATA *bank;

    if (bank_free == NULL)
        bank = alloc_perm(sizeof(*bank));
    else  
    {   
        bank = bank_free;
        bank_free = bank_free->next;
    }

    *bank = bank_zero;
    VALIDATE(bank);
    bank->name = &str_empty[0];
    return bank;
}

void free_bank(BANK_DATA *bank)
{
    if (!IS_VALID(bank))
        return;
          
    free_string(bank->name);
    INVALIDATE(bank);
 
    bank->next = bank_free;
    bank_free = bank;
}

/* stuff for recycling clanlist structures */
CLN_DATA *cln_free;

CLN_DATA *new_cln(void)
{
    static CLN_DATA cln_zero;
    CLN_DATA *cln;

    if (cln_free == NULL)
	cln = alloc_perm(sizeof(*cln));
    else
    {
	cln = cln_free;
	cln_free = cln_free->next;
    }

    *cln = cln_zero;
    VALIDATE(cln);
    cln->name = &str_empty[0];
    return cln;
}

MBR_DATA *mbr_free;

MBR_DATA *new_mbr(void)
{
    static MBR_DATA mbr_zero;
    MBR_DATA *mbr;

    if (mbr_free == NULL)
	mbr = alloc_perm(sizeof(*mbr));
    else
    {
	mbr = mbr_free;
	mbr_free = mbr_free->next;
    }

    *mbr = mbr_zero;
    VALIDATE(mbr);
    mbr->name = &str_empty[0];
    return mbr;
}

void free_mbr(MBR_DATA *mbr)
{
    if (!IS_VALID(mbr))
	return;

    free_string(mbr->name);
    INVALIDATE(mbr);

    mbr->next = mbr_free;
    mbr_free = mbr;
}

/* stuff for recycling descriptors */
DESCRIPTOR_DATA *descriptor_free;

DESCRIPTOR_DATA *new_descriptor(void)
{
    static DESCRIPTOR_DATA d_zero;
    DESCRIPTOR_DATA *d;

    if (descriptor_free == NULL)
	d = alloc_perm(sizeof(*d));
    else
    {
	d = descriptor_free;
	descriptor_free = descriptor_free->next;
    }
	
    *d = d_zero;
    VALIDATE(d);
    d->connected	= CON_GET_NAME;
    d->showstr_head	= NULL;
    d->showstr_point = NULL;
    d->outsize	= 2000;
    d->outbuf = alloc_mem( d->outsize );
    return d;
}

void free_descriptor(DESCRIPTOR_DATA *d)
{
    if (!IS_VALID(d))
	return;

    free_string( d->host);
    free_mem( d->outbuf, d->outsize );
    INVALIDATE(d);
    d->next = descriptor_free;
    descriptor_free = d;
}

/* stuff for recycling gen_data */
GEN_DATA *gen_data_free;

GEN_DATA *new_gen_data(void)
{
    static GEN_DATA gen_zero;
    GEN_DATA *gen;

    if (gen_data_free == NULL)
	gen = alloc_perm(sizeof(*gen));
    else
    {
	gen = gen_data_free;
	gen_data_free = gen_data_free->next;
    }
    *gen = gen_zero;
    VALIDATE(gen);
    return gen;
}

void free_gen_data(GEN_DATA *gen)
{
    if (!IS_VALID(gen))
	return;

    INVALIDATE(gen);

    gen->next = gen_data_free;
    gen_data_free = gen;
} 

/* stuff for recycling extended descs */
EXTRA_DESCR_DATA *extra_descr_free;

EXTRA_DESCR_DATA *new_extra_descr(void)
{
    EXTRA_DESCR_DATA *ed;

    if (extra_descr_free == NULL)
	ed = alloc_perm(sizeof(*ed));
    else
    {
	ed = extra_descr_free;
	extra_descr_free = extra_descr_free->next;
    }

    ed->keyword = &str_empty[0];
    ed->description = &str_empty[0];
    VALIDATE(ed);
    return ed;
}

void free_extra_descr(EXTRA_DESCR_DATA *ed)
{
    if (!IS_VALID(ed))
	return;

    free_string(ed->keyword);
    free_string(ed->description);
    INVALIDATE(ed);
    
    ed->next = extra_descr_free;
    extra_descr_free = ed;
}


/* stuff for recycling affects */
AFFECT_DATA *affect_free;

AFFECT_DATA *new_affect(void)
{
    static AFFECT_DATA af_zero;
    AFFECT_DATA *af;

    if (affect_free == NULL)
	af = alloc_perm(sizeof(*af));
    else
    {
	af = affect_free;
	affect_free = affect_free->next;
    }

    *af = af_zero;


    VALIDATE(af);
    return af;
}

void free_affect(AFFECT_DATA *af)
{
    if (!IS_VALID(af))
	return;

    INVALIDATE(af);
    af->next = affect_free;
    affect_free = af;
}

/* stuff for recycling objects */
OBJ_DATA *obj_free;

OBJ_DATA *new_obj(void)
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;

    if (obj_free == NULL)
	obj = alloc_perm(sizeof(*obj));
    else
    {
	obj = obj_free;
	obj_free = obj_free->next;
    }
    *obj = obj_zero;
    VALIDATE(obj);
    obj->inscribed = FALSE;
    obj->spell_name = "";
    obj->spell_count = 0;

    return obj;
}

void free_obj(OBJ_DATA *obj)
{
    AFFECT_DATA *paf, *paf_next;
    EXTRA_DESCR_DATA *ed, *ed_next;

    if (!IS_VALID(obj))
	return;

    for (paf = obj->affected; paf != NULL; paf = paf_next)
    {
	paf_next = paf->next;
	free_affect(paf);
    }
    obj->affected = NULL;

    for (ed = obj->extra_descr; ed != NULL; ed = ed_next )
    {
	ed_next = ed->next;
	free_extra_descr(ed);
     }
     obj->extra_descr = NULL;
   
    free_string( obj->name);
    free_string( obj->description);
    free_string( obj->short_descr);
    free_string( obj->owner);
    free_string( obj->killer);
    if ( obj->spell_name[0] != '\0')
	free_string( obj->spell_name);
	if(obj->ud->uwid)
		free(obj->ud->uwid);
	if(obj->ud->in_uwid);
		free(obj->ud->in_uwid);
	free(obj->ud);
    INVALIDATE(obj);

    obj->next   = obj_free;
    obj_free    = obj; 
}


/* stuff for recyling characters */
CHAR_DATA *char_free;

CHAR_DATA *new_char (void)
{
    static CHAR_DATA ch_zero;
    CHAR_DATA *ch;
    int i;

    if (char_free == NULL)
	ch = alloc_perm(sizeof(*ch));
    else
    {
	ch = char_free;
	char_free = char_free->next;
    }

    *ch				= ch_zero;
    VALIDATE(ch);
    ch->name                    = &str_empty[0];
    ch->short_descr             = &str_empty[0];
    ch->long_descr              = &str_empty[0];
    ch->description             = &str_empty[0];
    ch->prompt                  = &str_empty[0];
    ch->prefix			= &str_empty[0];
    ch->die_descr		= &str_empty[0];
    ch->say_descr		= &str_empty[0];
    ch->logon                   = current_time;
    ch->lines                   = PAGELEN;
    ch->running			= FALSE;
    for (i = 0; i < 4; i++)
        ch->armor[i]            = 100;
    ch->position                = POS_STANDING;
    ch->hit                     = 100;
    ch->max_hit                 = 100;
    ch->mana                    = 100;
    ch->max_mana                = 100;
    ch->move                    = 100;
    ch->max_move                = 100;
    for (i = 0; i < MAX_STATS; i ++)
    {
        ch->perm_stat[i] = 13;
        ch->mod_stat[i] = 0;
    }

    return ch;
}


void free_char (CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    if (!IS_VALID(ch))
	return;

    if (IS_NPC(ch))
	{
		mobile_count--;
		if(ch->ud->uwid)
			free(ch->ud->uwid);
		if(ch->ud->in_uwid)
			free(ch->ud->in_uwid);
		free_uwid(ch->ud);
	}
    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	extract_obj(obj);
    }

    for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
	paf_next = paf->next;
	affect_remove(ch,paf);
    }
	#ifdef I3
      free_i3chardata( ch );
#endif
#ifdef IMC
    imc_freechardata( ch );
#endif
    free_string(ch->name);
    free_string(ch->short_descr);
    free_string(ch->long_descr);
    free_string(ch->description);
    free_string(ch->prompt);
    free_string(ch->prefix);
    free_note(ch->pnote);
	free_string(ch->plogstring);
    if ( ch->die_descr[0] != '\0')
	free_string(ch->die_descr);
    if ( ch->say_descr[0] != '\0')
	free_string(ch->say_descr);
    free_pcdata(ch->pcdata);
	

    ch->next = char_free;
    char_free  = ch;

    INVALIDATE(ch);
    return;
}

PC_DATA *pcdata_free;

PC_DATA *new_pcdata(void)
{
    int alias;

    static PC_DATA pcdata_zero;
    PC_DATA *pcdata;

    if (pcdata_free == NULL)
	pcdata = alloc_perm(sizeof(*pcdata));
    else
    {
	pcdata = pcdata_free;
	pcdata_free = pcdata_free->next;
    }

    *pcdata = pcdata_zero;

    for (alias = 0; alias < MAX_ALIAS; alias++)
    {
	pcdata->alias[alias] = NULL;
	pcdata->alias_sub[alias] = NULL;
    }
    for (alias = 0; alias < MAX_FORGET; alias++)
    {
	pcdata->forget[alias] = NULL;
    }
    for (alias = 0; alias < MAX_DUPES; alias++)
    {
	pcdata->dupes[alias] = NULL;
    }

    pcdata->buffer = new_buf(1000);
    
    VALIDATE(pcdata);
    return pcdata;
}
	

void free_pcdata(PC_DATA *pcdata)
{
    int alias;

    if (!IS_VALID(pcdata))
	return;

    free_string(pcdata->pwd);
    free_string(pcdata->bamfin);
    free_string(pcdata->bamfout);
    free_string(pcdata->who_descr);
    free_string(pcdata->title);
    free_string(pcdata->lquest_obj);
    free_string(pcdata->lquest_mob);
    free_string(pcdata->lquest_are);
    free_buf(pcdata->buffer);

    for (alias = 0; alias < MAX_ALIAS; alias++)
    {
	free_string(pcdata->alias[alias]);
	free_string(pcdata->alias_sub[alias]);
    }
    for (alias = 0; alias < MAX_FORGET; alias++)
    {
	free_string(pcdata->forget[alias]);
    }
    for (alias = 0; alias < MAX_DUPES; alias++)
    {
	free_string(pcdata->dupes[alias]);
    }
    INVALIDATE(pcdata);
    pcdata->next = pcdata_free;
    pcdata_free = pcdata;

    return;
}

	


/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void)
{
    int val;

    val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
    last_pc_id = val;
    return val;
}

long get_mob_id(void)
{
    last_mob_id++;
    return last_mob_id;
}

MEM_DATA *mem_data_free;

/* procedures and constants needed for buffering */

BUFFER *buf_free;

MEM_DATA *new_mem_data(void)
{
    MEM_DATA *memory;
  
    if (mem_data_free == NULL)
	memory = alloc_mem(sizeof(*memory));
    else
    {
	memory = mem_data_free;
	mem_data_free = mem_data_free->next;
    }

    memory->next = NULL;
    memory->id = 0;
    memory->reaction = 0;
    memory->when = 0;
    VALIDATE(memory);

    return memory;
}

void free_mem_data(MEM_DATA *memory)
{
    if (!IS_VALID(memory))
	return;

    memory->next = mem_data_free;
    mem_data_free = memory;
    INVALIDATE(memory);
}

/* buffer sizes */
const int buf_size[MAX_BUF_LIST] =
{
16,32,64,128,256,1024,2048,4096,8192,16384,32768,65536,131072,262144
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val)
{
/*    int bfs; */

    if (buf_size[0] >= val)
	return buf_size[0];
    else if (buf_size[1] >= val)
	return buf_size[1];
    else if (buf_size[2] >= val)
	return buf_size[2];
    else if (buf_size[3] >= val)
	return buf_size[3];
    else if (buf_size[4] >= val)
	return buf_size[4];
    else if (buf_size[5] >= val)
	return buf_size[5];
    else if (buf_size[6] >= val)
	return buf_size[6];
    else if (buf_size[7] >= val)
	return buf_size[7];
    else if (buf_size[8] >= val)
	return buf_size[8];
    else if (buf_size[9] >= val)
	return buf_size[9];
    else if (buf_size[10] >= val)
	return buf_size[10];
    else if (buf_size[11] >= val)
	return buf_size[11];
    else
	return -1;

/*

    for (bfs = 0; bfs < MAX_BUF_LIST; bfs++)
    {
	if (buf_size[bfs] >= val)
	{
	    return buf_size[bfs];
	}
	if (bfs >= MAX_BUF_LIST)
	{
	    return -1;
	}
    }
    
*/
    return -1;
}
#define EMEM_SIZE -1 /* find_mem_size returns this when block is too large */
#define NUL '\0'

extern const int rgSizeList [MAX_MEM_LIST];

/* Find in rgSizeList a memory size at least this long */
int find_mem_size (int min_size)
{
	int i;
	
	for (i = 0; i < MAX_MEM_LIST; i++)
		if (rgSizeList[i] >= min_size)
			return rgSizeList[i];
	
	/* min_size is bigger than biggest allowable size! */
	
	return EMEM_SIZE;
}

BUFFER * __new_buf (int min_size, const char *file, const char *function, int line) 
{
	int size;
	BUFFER *buffer;
	char buf[200]; /* for the bug line */
	
	size = find_mem_size (min_size);
	
	if (size == EMEM_SIZE)
	{
		log_string(LOG_ERR, "Buffer size too big: %d bytes (%s: %s - %u).", min_size, file, function, line);
		abort();
	}
	
	if (buf_free == NULL) 
	buffer = alloc_perm(sizeof(*buffer));
    else
    {
	buffer = buf_free;
	buf_free = buf_free->next;
    }
	buffer->next	= NULL;
    buffer->state	= BUFFER_SAFE;
	
	buffer->size = size;
	 /*For debugging purposes*/
    buffer->file    = str_dup(file);
    buffer->function= str_dup(function);
    buffer->line    = line;
	buffer->data = alloc_mem (size);
	buffer->data[0] = '\0';
		top_buffer++;
    LINK_SINGLE(buffer, next, buffer_list);
	buffer->overflowed = FALSE;
	VALIDATE(buffer);
	
	buffer->len = 0;
	
	return buffer;
} /* __buf_new */

/* Add a string to a buffer. Expand if necessary */
				 /* debugging - expect filename and line */
bool __add_buf (BUFFER *buffer, const char *text, const char * file, const char *function, unsigned line)
{
	int new_size;
	int text_len;
	char *new_data;
	char *old_data;
	int old_size;
	char buf[200];
	buffer->file    = str_dup(file);
    buffer->function= str_dup(function);
    buffer->line    = line;
	
	old_data = buffer->data;
	old_size = buffer->size;
	
	if (buffer->overflowed) /* Do not attempt to add anymore if buffer is already overflowed */
		return FALSE;

	if (!text) /* Adding NULL string ? */
		return FALSE;
	
	text_len = strlen(text);
	
	if (text_len == 0) /* Adding empty string ? */
		return FALSE;
		
	/* Will the combined len of the added text and the current text exceed our buffer? */

	if ((text_len+buffer->len+1) > buffer->size) /* expand? */
	{
		buffer->size = find_mem_size (old_size + text_len + 1);
		if (new_size == EMEM_SIZE) /* New size too big ? */
		{
			log_string(LOG_ERR, "Buffer overflow, wanted %d bytes (%s: %s - %u).", text_len+buffer->len, file, function, line);				
			buffer->overflowed = TRUE;
			return FALSE;
		}

		/* Allocate the new buffer */
		
		buffer->data = alloc_mem (buffer->size);		
		
		/* Copy the current buffer to the new buffer */
		
		strcpy (buffer->data, old_data);
		free_mem (old_data, old_size);

	} /* if */

	strcat (buffer->data, text);	/* Start copying */
	buffer->len += text_len;	/* Adjust length */
	buffer->data[buffer->len] = NUL; /* Null-terminate at new end */
	return TRUE;
	
} /* __buf_strcat */

int BufPrintf (BUFFER *buffer, char *fmt, ...)
{
	char buf[MSL];
	va_list va;
	int res;
	
	va_start (va, fmt);
	res = vsnprintf (buf, MSL, fmt, va);
	va_end (va);
	add_buf (buffer, buf);

	return res;	
}

/* Free a buffer */
void free_buf (BUFFER *buffer)
{
	/* Free data */
	free_mem (buffer->data, buffer->size);
	
	buffer->data = NULL;
    buffer->size   = 0;
    buffer->state  = BUFFER_FREED;
	free_string(buffer->file);
	free_string(buffer->function);
	top_buffer--;
	UNLINK_SINGLE(buffer, next, BUFFER, buffer_list);
	INVALIDATE(buffer);
    buffer->next  = buf_free;
    buf_free      = buffer;
	
	free_mem (buffer, sizeof(BUFFER));
}

/* Clear a buffer's contents, but do not deallocate anything */

void clear_buf (BUFFER *buffer)
{
	 buffer->data[0] 		= '\0';
    buffer->state     		= BUFFER_SAFE;
	buffer->overflowed 	= FALSE;
	buffer->len 				= 0;
}


char *buf_string(BUFFER *buffer)
{
    return buffer->data;
}

 /* stuff for recycling mobprograms */
PROG_LIST *mprog_free;
PROG_LIST *oprog_free;
PROG_LIST *rprog_free;

PROG_LIST *new_mprog(void)
 {
    static PROG_LIST mp_zero;
    PROG_LIST *mp = malloc(sizeof(*mp));
   
    *mp 								= mp_zero;
    mp->vnum             		= 0;
    mp->trig_type       	= 0;
    mp->code             		= str_dup("");
	mp->trig_phrase		= str_dup("");
	VALIDATE(mp);
    return mp;
 }
 
 void free_mprog(PROG_LIST *mp)
 {
    if (!IS_VALID(mp))
       return;
 
    INVALIDATE(mp);
    free_string(mp->code);
	free_string(mp->trig_phrase);
	free(mp);
 } 

 PROG_LIST *new_oprog(void)
{
   static PROG_LIST op_zero;
   PROG_LIST *op = malloc(sizeof(*op));

   *op 								= op_zero;
   op->vnum             		= 0;
   op->trig_type        	= 0;
   op->code             		= str_dup("");
   op->trig_phrase		= str_dup("");
   VALIDATE(op);
   return op;
}

void free_oprog(PROG_LIST *op)
{
   if (!IS_VALID(op))
      return;

   INVALIDATE(op);
   free_string(op->code);
   free_string(op->trig_phrase);
   free(op);
}

PROG_LIST *new_rprog(void)
{
   static PROG_LIST rp_zero;
   PROG_LIST *rp = malloc(sizeof(*rp));
   
   *rp 								= rp_zero;
   rp->vnum             		= 0;
   rp->trig_type      	= 0;
   rp->trig_phrase		= str_dup("");
   rp->code             		= str_dup("");
   VALIDATE(rp);
   return rp;
}

void free_rprog(PROG_LIST *rp)
{
   if (!IS_VALID(rp))
      return;

   INVALIDATE(rp);
   free_string(rp->trig_phrase);
   free_string(rp->code);
   free(rp);
}
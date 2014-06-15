/*Broken Dreams
Syn - imported utilties and functions Either written by me, or others, credit shown where known*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "interp.h"
#include "syn_mysql.h"
#include "olc.h"
#include "recycle.h"

void stc(const char *txt, CHAR_DATA * ch)
	{
	if (txt != NULL && ch->desc != NULL)
	write_to_buffer(ch->desc, txt, strlen(txt));
	return;}
/*UWID Creator(s)
* UWID - Unique World I-Dentifier
* constructor map:
	sections separate by . except location vnum which is @
	O=Obj
	C=Char
	d1=area vnum
	s1=area name
	d2=entity vnum
	s2=entity name
	s3=random-key
	d3=vnum.location
	
	
*/
char *uwid_key_gen()
{
		struct timeval tv;
        char seq[20];
        int i;

        for (i=0;i<10;i++)
        {
                gettimeofday (&tv, NULL);
                srand(tv.tv_usec);
                sprintf (seq,"%X%X", tv.tv_usec, rand());
        }
		return seq;
}
char *time_stamp(int *i)
{
		static char zs[1000];
		time_t t = time(NULL);
		struct tm * p = localtime(&t);
		if(i == 1) //take what I learned and manipulate the time_t struct to provide a time string of my choosing -Syn
		strftime(zs, 1000, "{C%B{8-{C%d {G%H{8:{G%M{8:{G%S{8", p);
		else if(i == 2)
		strftime(zs, 1000, "{G%H{8:{G%M{8:{G%S{8", p);
		return zs;
}

char     *get_curdate(void)
{
        static char buf[128];
        struct tm *datetime;

        datetime = localtime(&current_time);
        strftime(buf, sizeof(buf), "%m-%d-%Y", datetime);
        return buf;
}

int imm_editor_check(CHAR_DATA *ch)
{
			if(ch->desc->editor == ED_ROOM)
			{
				return 1;
			}
			if(ch->desc->editor == ED_MOBILE)
			{
				return 2;
			}
			if(ch->desc->editor == ED_OBJECT)
			{
				return 3;
			}
			if(ch->desc->editor == ED_AREA)
			{
				return 4;
			}
			if(ch->desc->editor == ED_OPCODE)
			{
				return 5;
			}
			if(ch->desc->editor == ED_MPCODE)
			{
				return 6;
			}
			if(ch->desc->editor == ED_RPCODE)
			{
				return 7;
			}
			else
				return 0;
}

int obj_uwid_constructor(	OBJ_DATA *uObj)
{
	char buf[2*MSL];
	char *sbuf;
	char *next;

	next = strdup(uObj->pIndexData->area->file_name);
	int i;
	for(i = 0; i < strlen(next); i++)
	{
		if(next[i] == '.')
		{
			next[i] = '\0';
			break;
		}
	}
	
	sprintf(buf, "O%lu.%s.%s@%d", uObj->pIndexData->vnum, next, uObj->name, obj_count);
	uObj->ud->uwid = strdup(buf);
	uObj->ud->sh_uwid = obj_count;
	log_string(LOG_UWID,"UWID created : %s Short UWID: %d", buf, obj_count);
	return 0;
}

int char_uwid_constructor(CHAR_DATA *uCh)
{
	char buf[2*MSL];
	
	sprintf(buf, "C%s.%s.%s.%d",uCh->name, race_table[uCh->race].name, class_table[uCh->class].name, char_count);
	uCh->ud->uwid = strdup(buf);
	uCh->ud->sh_uwid = char_count;
	log_string(LOG_UWID,"UWID created : %s Short UWID: %d", buf, char_count);
	return buf;
}

int mob_uwid_constructor(CHAR_DATA *uMob)
{
	char buf[2*MSL];
	char *next;

	next = strdup(uMob->pIndexData->area->file_name);
	
	int i;
	for(i = 0; i < strlen(next); i++)
	{
		if(next[i] == '.')
		{
			next[i] = '\0';
			break;
		}
	}
	
	sprintf(buf, "M%lu.%s.%s@%d", uMob->pIndexData->vnum, next, uMob->name, mobile_count);
	uMob->ud->uwid = strdup(buf);
	uMob->ud->sh_uwid = mobile_count;
	log_string(LOG_UWID,"UWID created : %s Short UWID: %d", buf, mobile_count);
	return 0;
}

char *room_uwid_constructor(ROOM_INDEX_DATA *uRoom)
{
	char buf[2*MSL];
	char sbuf[MSL];
	
	sprintf(buf, "R.%d.%s.%lu.%s.%s",uRoom->area->vnum, uRoom->area->file_name, uRoom->vnum, uRoom->name, uwid_key_gen());
	return buf;
}

int strcasecmp(const char *s1, const char *s2)
{
  int i = 0;

  while (s1[i] != '\0' && s2[i] != '\0' && toupper(s1[i]) == toupper(s2[i]))
    i++;

  /* if they matched, return 0 */
  if (s1[i] == '\0' && s2[i] == '\0')
    return 0;

  /* is s1 a prefix of s2? */
  if (s1[i] == '\0')
    return -110;

  /* is s2 a prefix of s1? */
  if (s2[i] == '\0')
    return 110;

  /* is s1 less than s2? */
  if (toupper(s1[i]) < toupper(s2[i]))
    return -1;

  /* s2 is less than s1 */
  return 1;
}

void ssmash_tilde (char *str)
{
    for (; *str != '\0'; str++)
    {
        if (*str == '~')
            *str = '-';
    }

    return;
}

void ssmash_quote (char *str)
{
    for (; *str != '\0'; str++)
    {
        if (*str == '`')
            *str = '-';
    }

    return;
}

void smash_quote2 (char *str)
{
    for (; *str != '\0'; str++)
    {
        if (*str == '\'')
            *str = '-';
    }

    return;
}

void convert_pound (char *str)
{
    for (; *str != '\0'; str++)
    {
        if (*str == '{')
            *str = '{';
    }

    return;
}


char     *bash_color(const char *txt)
{
        const char *point;
        char     *point2;
        static char buf[MSL * 4];

        buf[0] = '\0';
        point2 = buf;

        for (point = txt; *point; point++)
        {
                if (*point == '{' && *(point + 1) != '\0')
                {
                        point++;
                        continue;
                }
                *point2 = *point;
                *++point2 = '\0';
        }
        *point2 = '\0';
        return buf;
}

char *replace(const char *s, char ch, const char *repl) {
    int count = 0;
    const char *t;
	char *output;
    for(t=s; *t; t++)
        count += (*t == ch);

    size_t rlen = strlen(repl);
    char *res = malloc(strlen(s) + (rlen-1)*count + 1);
    char *ptr = res;
    for(t=s; *t; t++) {
        if(*t == ch) {
            memcpy(ptr, repl, rlen);
            ptr += rlen;
        } else {
            *ptr++ = *t;
        }
    }
    *ptr = 0;
	//strcpy(output, res);
	//free_mem(res, sizeof(res));
    return res;
}

void log_string(int type, const char *fmt, ...)
{
        DESCRIPTOR_DATA *d;
        va_list   args;
        char     *strtime;
        char      buf[MSL];
        char      bufew[2 * MSL];
        char      bufee[4 * MSL];
        FILE     *log_file;

        buf[0] = '\0';
        log_file = NULL;

        // Get the wanted text
        va_start(args, fmt);
        vsprintf(bufew, fmt, args);
        va_end(args);

        if (type & LOG_CRIT)
        {
                sprintf(buf, "../log/%s.critical", get_curdate());
                log_file = fopen(buf, "a");
				if ( log_file == NULL)
				{
				perror("fopen");
				return(-1);
				}
                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                fprintf(log_file, "%s :: %s\n", strtime, bash_color(bufew));
                fflush(log_file);
                fclose(log_file);
                for (d = descriptor_list; d != NULL; d = d->next)
                        if (d->connected == CON_PLAYING
                            && IS_IMMORTAL(d->character))
                                printf_to_char(d->character, "Critical: %s\n\r", bufew);
                sprintf(bufee, "{RC{rritical{8: {W%s{8", bufew);
				// SQL DB Log insert
				char values[MAX_STRING_LENGTH];        
				sprintf(values,"\'%s\', \'%s\'", time_stamp(1), bufee);
				mysql_insert("log_crit", "timestamp, message", values);
        }
        if (type & LOG_ERR)
        {
                sprintf(buf, "../log/%s.error", get_curdate());
                log_file = fopen(buf, "a");
					if ( log_file == NULL)
				{
				perror("fopen");
				return(-1);
				}
                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                fprintf(log_file, "%s :: %s\n", strtime, bash_color(bufew));
                fprintf(stderr, "%s :: %s\n", strtime, bash_color(bufew));
                fflush(log_file);
                fclose(log_file);
                sprintf(bufee, "{RE{rrror{8: {W%s{8", bufew);
				// SQL DB Log insert
				char values[MAX_STRING_LENGTH];        
				sprintf(values,"\'%s\', \'%s\'", time_stamp(1), bufee);
				mysql_insert("log_err", "timestamp, message", values);
        }
        if (type & LOG_BUG)
        {
                sprintf(buf, "../log/%s.bug", get_curdate());
                log_file = fopen(buf, "a");
							if ( log_file == NULL)
				{
				perror("fopen");
				return(-1);
				}
                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                fprintf(log_file, "%s :: %s\n", strtime, bash_color(bufew));
                fflush(log_file);
                fclose(log_file);
                sprintf(bufee, "{RB{ru{Rg{8: {W%s{8", bufew);
				char values[MAX_STRING_LENGTH];        
				sprintf(values,"\'%s\', \'%s\'", time_stamp(1), bufee);
				mysql_insert("log_bug", "timestamp, message", values);
        }
		 if (type & LOG_OLC)
        {
                sprintf(buf, "../log/%s.olc", get_curdate());
                log_file = fopen(buf, "a");
							if ( log_file == NULL)
				{
				perror("fopen");
				return(-1);
				}
                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                fprintf(log_file, "%s :: %s\n", strtime, bash_color(bufew));
                fflush(log_file);
                fclose(log_file);
                sprintf(bufee, "{PO{plc{8: {W%s{8", bufew);
				char values[MAX_STRING_LENGTH];        
				sprintf(values,"\'%s\', \'%s\'", time_stamp(1), bufee);
				mysql_insert("log_olc", "timestamp, message", values);
        }
		if (type & LOG_HELP)
        {
                sprintf(buf, "../log/%s.help", get_curdate());
                log_file = fopen(buf, "a");
							if ( log_file == NULL)
				{
				perror("fopen");
				return(-1);
				}
                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                fprintf(log_file, "%s :: %s\n", strtime, bash_color(bufew));
                fflush(log_file);
                fclose(log_file);
                sprintf(bufee, "{YH{yelp{8: {w%s{8", bufew);
				char values[MAX_STRING_LENGTH];        
				sprintf(values,"\'%s\', \'%s\'", time_stamp(1), bufee);
				mysql_insert("log_help", "timestamp, message", values);
        }
        if (type & LOG_SECURITY)
        {
                sprintf(buf, "../log/%s.security", get_curdate());
                log_file = fopen(buf, "a");
						if ( log_file == NULL)
				{
				perror("fopen");
				return(-1);
				}
                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                fprintf(log_file, "%s :: %s\n", strtime, bash_color(bufew));
                fflush(log_file);
                fclose(log_file);
                sprintf(bufee, "{PS{pecurity{8: {W%s{8", bufew);
				// SQL DB Log insert
				char values[MAX_STRING_LENGTH];        
				char *as = replace(bufee, '\'', "`");
				sprintf(values,"\'%s\', \'%s\'", time_stamp(1), as);
				mysql_insert("log_security", "timestamp, message", values);
        }
        if (type & LOG_CONNECT)
        {
                sprintf(buf, "../log/%s.connect", get_curdate());
                log_file = fopen(buf, "a");
							if ( log_file == NULL)
				{
				perror("fopen");
				return(-1);
				}
                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                fprintf(log_file, "%s :: %s\n", strtime, bash_color(bufew));
                fflush(log_file);
                fclose(log_file);
                sprintf(bufee, "{GC{gonnect{8: {W%s{8", bufew);
				// SQL DB Log insert
				char values[MAX_STRING_LENGTH];        
				sprintf(values,"\'%s\', \'%s\'", time_stamp(1), bufee);
				mysql_insert("log_connect", "timestamp, message", values);
        }
        if (type & LOG_GAME)
        {
                sprintf(buf, "../log/%s.game", get_curdate());
                log_file = fopen(buf, "a");
					if ( log_file == NULL)
				{
				perror("fopen");
				return(-1);
				}
                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                fprintf(log_file, "%s :: %s\n", strtime, bash_color(bufew));
                fprintf(stderr, "%s :: %s\n", strtime, bash_color(bufew));
                fflush(log_file);
                fclose(log_file);
                sprintf(bufee, "{GG{game{8: {W%s{8", bufew);
				// SQL DB Log insert
				char values[MAX_STRING_LENGTH];        
				sprintf(values,"\'%s\', \'%s\'", time_stamp(1), bufee);
				mysql_insert("log_game", "timestamp, message", values);
        }
		if (type & LOG_SQL)
        {
                sprintf(buf, "../log/%s.msql", get_curdate());
                log_file = fopen(buf, "a");
				if ( log_file == NULL)
				{
				perror("fopen");
				return(-1);
				}
                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                fprintf(log_file, "%s :: %s\n", strtime, bash_color(bufew));
                fprintf(stderr, "%s :: %s\n", strtime, bash_color(bufew));
                fflush(log_file);
                fclose(log_file);
                sprintf(bufee, "{GS{gQL{8: {W%s{8", bufew);

        }
        if (type & LOG_COMMAND)
        {
                sprintf(buf, "../log/%s.comm", get_curdate());
                log_file = fopen(buf, "a");
				if ( log_file == NULL)
				{
				perror("fopen");
				return(-1);
				}
                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                fprintf(log_file, "%s :: %s\n", strtime, bash_color(bufew));
                fflush(log_file);
                fclose(log_file);
                sprintf(bufee, "{LC{lommand{8: {W%s{8", bufew);
				// SQL DB Log insert
				char values[MAX_STRING_LENGTH];        
				char *as = replace(bufee, '\'', "`");
				sprintf(values,"\'%s\', \'%s\'", time_stamp(1), as);
				mysql_insert("log_command", "timestamp, message", values);
        }
		if (type & LOG_UWID)
        {
                sprintf(buf, "../log/%s.uwid", get_curdate());
                log_file = fopen(buf, "a");
				if ( log_file == NULL)
				{
				perror("fopen");
				return(-1);
				}
                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                fprintf(log_file, "%s :: %s\n", strtime, bash_color(bufew));
                fflush(log_file);
                fclose(log_file);
                sprintf(bufee, "#YU#yWID#0: #W%s#n", bufew);
				// SQL DB Log insert
				char values[MAX_STRING_LENGTH];        
				char *as = replace(bufee, '\'', "`");
				sprintf(values,"\'%s\', \'%s\'", time_stamp(1), as);
				mysql_insert("log_uwid", "timestamp, message", values);
        }
        wiznet(bufee,NULL,NULL, WIZ_DEBUG, 0, 7);
}
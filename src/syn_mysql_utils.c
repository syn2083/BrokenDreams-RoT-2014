/* Broken Dreams Mud - Syn's MySQL Utilities and functions. */

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>


#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "syn_mysql.h"


/*
 * INSERT Function, to allow call of SQL Insert without rebuilding the SQL INSERT/code in every place I decide to do one
 * Format is mysql_insert(target_table, target_columns, target_values);
 * Note that this obviously requires YOU to know what table, and columns to target, and if the data is appropriate.
 * SQL libs apparently do not tolerate bad data AT ALL and bad infos will likely cause a SIGSEGV, be warned.
 * -Syn : Broken Dreams
 */
void mysql_insert(char *table_into, char *columns_into, char *values_into)
{
		MYSQL_RES *res;

		char query[4096*2];
		snprintf(query, sizeof(query) -1, "INSERT INTO %s(%s) VALUES( %s)", table_into, columns_into, values_into);
		mysql_safe_query(query);

		if(query != NULL)
		{
			res = mysql_store_result(&db);
			mysql_free_result(res);
			return;
		}
		else
		return;
}

/*for the SQL result formatter*/
void print_dashes (CHAR_DATA *ch, MYSQL_RES *res)
{
	MYSQL_FIELD *field;
	unsigned int i, j;
	char sbuf[MSL];
	int jic;


	mysql_field_seek (res, 0);
	sprintf (sbuf,"#G+#0");
	stc(sbuf, ch);
	for (i = 0; i < mysql_num_fields (res); i++)
	{
		field = mysql_fetch_field (res);
		if(!strcmp(field->name,"message_num"))
		{
			for (j = 0; j < 8 ; j++)
			{
				sprintf (sbuf,"#8-#0");
				stc(sbuf, ch);
			}
		}
		else if(!strcmp(field->name,"log_num"))
		{
			for (j = 0; j < strlen(field->name) +2; j++)
			{
				sprintf (sbuf,"#8-#0");
				stc(sbuf, ch);
			}
		}
		else if(!strcmp(field->name,"timestamp"))
		{
			for (j = 0; j < 17 ; j++)
			{
				sprintf (sbuf,"#8-#0");
				stc(sbuf, ch);
			}
		}
		else if(!strcmp(field->name,"entry_num"))
		{
			for (j = 0; j < 11 ; j++)
			{
				sprintf (sbuf,"#8-#0");
				stc(sbuf, ch);
			}
		}
		else if(field->max_length > 55)
		{
			for (j = 0; j < 55; j++)
			{
				sprintf (sbuf,"#8-#0");
				stc(sbuf, ch);
			}
		}
		else
		{
			for (j = 0; j < field->max_length + 2; j++)
			{
				sprintf (sbuf,"#8-#0");
				stc(sbuf, ch);
			}
		}
		sprintf (sbuf,"#G+#0");
		stc(sbuf, ch);
	}
	sprintf (sbuf,"\n");
	stc(sbuf, ch);
}

void
process_result_set (CHAR_DATA *ch, MYSQL *db, MYSQL_RES *res)
{
	MYSQL_FIELD *field;
	MYSQL_ROW row;
	unsigned int i, c, col_len;
	
	char sbuf[MSL];
	/* determine column display widths */
	mysql_field_seek (res, 0);
	unsigned int clen[1000];
	for (i = 0; i < mysql_num_fields (res); i++)
	{	
		field = mysql_fetch_field (res);
		clen[i] = strlen (field->name);
		if (clen[i] < field->max_length)
			clen[i]=field->max_length;
		if (clen[i] < 4 && !IS_NOT_NULL (field->flags))
			clen[i] = 4; /* 4 = length of the word “NULL” */
		if(!strcmp(field->name,"timestamp"))
			clen[i] -= 18;
		
	}
	print_dashes (ch, res);
	mysql_field_seek (res, 0);
	for (i = 0; i < mysql_num_fields (res); i++)
	{
		field = mysql_fetch_field (res);
		sprintf (sbuf,"#8| #C%-*s #0", clen[i], field->name);
		stc(sbuf, ch);
	}
	sprintf (sbuf,"\n");
	stc(sbuf, ch);
	print_dashes (ch, res);
	while ((row = mysql_fetch_row (res)) != NULL)
	{
		mysql_field_seek (res, 0);
		for (i = 0; i < mysql_num_fields (res); i++)
		{
			field = mysql_fetch_field (res);
			if (row[i] == NULL)
			{
				sprintf (sbuf,"#8| %-*s #0", clen[i], "NULL");
				stc(sbuf, ch);
			}
			else if (IS_NUM (field->type))
			{	
				sprintf (sbuf,"#8| #R%*s #0", clen[i], row[i]);
				stc(sbuf, ch);
			}
			else
			{
					c = strlen(field->name);
					sprintf (sbuf,"#8| #W%-*s #0", clen[i], row[i]);
				
				stc(sbuf, ch);
			}
		}
		sprintf (sbuf,"\n");
		stc(sbuf, ch);
	}
	print_dashes (ch, res);
	sprintf (sbuf,"\n\r#R%lu #8Records returned#0\n\r", (unsigned long) mysql_num_rows (res));
	stc(sbuf, ch);
}

/*
 * I wanted a function that would allow me to easily make a select statement from inside the MUD, so here it is.. 
 * It will mostly format the output in a boxed setup. Todo - break this out so you can use select, insert, update, or create
 * effectively becoming a remote interface to run queries of any type..
 * -Syn
 */
void do_sqlgenselect(CHAR_DATA *ch, char *argument)
{
	MYSQL_RES *res;
	char query[4096*2];
	snprintf(query, sizeof(query) -1, argument);
	if(!mysql_safe_query(query))
	{
		stc("#W\n\rSomething went wrong, check the string and try again.\n\r#0",ch);
		return;
	}
	res = mysql_store_result(&db);	
	unsigned int numfields = mysql_num_fields(res);	
	if((numfields == 0) || (numfields == NULL))
	{
		stc("#W\n\rSomething went wrong, check the string and try again.\n\r#0",ch);
		return;
	}
	if(argument == NULL)
	{
		send_to_char("SQL SELECT query.. general syntax: SELECT <stuff> FROM <someplace> ORDER BY <root-order>\n\r",ch);
		return;
	}
	if((res == NULL) || (!res))
	{
		send_to_char("It might help if you entered a valid SQL SELECT query.. general syntax: SELECT <stuff> FROM <someplace> ORDER BY <root-order>\n\r",ch);
		return;
	}
	process_result_set(ch, &db, res);
	mysql_free_result(res);
	return;
}
	
/*
 * SELECT for channel history, provides a SELECT function to call the database based on channel argument, and result limit.
 * We are also passing char_data ch so we could in theory use/manipulate that within this function, but that shouldnt be necesarry.
 * -Syn : Broken Dreams
 */
void mysql_chan_select(CHAR_DATA *ch, char *channel, char *reslimit)
	{
		char buf[MAX_STRING_LENGTH];
		MYSQL_RES *res;
		char query[4096*2];
		char bufch[MAX_STRING_LENGTH];
		snprintf(query, sizeof(query) -1, "SELECT q.message_num, q.player, q.timestamp, q.message FROM (SELECT message_num, player, timestamp, message FROM channel_%s ORDER BY message_num DESC LIMIT %s) q ORDER BY q.message_num ASC", channel, reslimit);
		mysql_safe_query(query);
		res = mysql_store_result(&db);


		if(!db_connect)
		{
			send_to_char("There has been an error with the database connection. Please try again later.\n\r", ch);
			return;
		}

		log_string(LOG_SQL,"Loading %s History upon request!", channel);
		sprintf(bufch,"#wHere are the last #R%s #C%s #wrecords from the #CD#ca#Ct#ca#Cb#ca#Cs#ce #G:#0\n\r", reslimit, channel);
		send_to_char( bufch, ch);
		
		if(!strcmp("chat",channel))
		{
		while ((row = mysql_fetch_row( res )))
			{
				sprintf(buf, "#8[#G%s#8] #CC#cha#Ct #8- #W%s #8: #w%s#0 \n\r", row[2], row[1], row[3]);
				send_to_char(buf, ch);
			}
		mysql_free_result(res);
		return;
		}
		else if(!strcmp("bug",channel))
		{
		while ((row = mysql_fetch_row( res )))
			{
				sprintf(buf, "#8[#G%s#8] #CB#cu#Cg #8- #W%s #8: #w%s#0 \n\r", row[2], row[1], row[3]);
				send_to_char(buf, ch);
			}
		mysql_free_result(res);
		return;
		}
		else if(!strcmp("yell",channel))
		{
		while ((row = mysql_fetch_row( res )))
			{
				sprintf(buf, "#8[#G%s#8] #CY#cel#Cl #8- #W%s #8: #w%s#0 \n\r", row[2], row[1], row[3]);
				send_to_char(buf, ch);
			}
		mysql_free_result(res);
		return;
		}
		else if(!strcmp("group",channel))
		{
		while ((row = mysql_fetch_row( res )))
			{
				sprintf(buf, "#8[#G%s#8] #CG#crou#Cp #8- #W%s #8: #w%s#0 \n\r", row[2], row[1], row[3]);
				send_to_char(buf, ch);
			}
		mysql_free_result(res);
		return;
		}
		else if(!strcmp("say",channel) )
		{
		while ((row = mysql_fetch_row( res )))
			{
				sprintf(buf, "#8[#G%s#8] #CS#ca#Cy #8- #W%s #8: #w%s#0 \n\r", row[2], row[1], row[3]);
				send_to_char(buf, ch);
			}
			mysql_free_result(res);
		return;
		}
		else if(!strcmp("sing",channel))
		{
		while ((row = mysql_fetch_row( res )))
			{
				sprintf(buf, "#8[#G%s#8] #CS#cin#Cg #8- #W%s #8: #w%s#0 \n\r", row[2], row[1], row[3]);
				send_to_char(buf, ch);
			}
			mysql_free_result(res);
		return;
		}
		else if(!strcmp("trivia",channel))
		{
		while ((row = mysql_fetch_row( res )))
			{
				sprintf(buf, "#8[#G%s#8] #CT#crivi#Ca #8- #W%s #8: #w%s#0 \n\r", row[2], row[1], row[3]);
				send_to_char(buf, ch);
			}
			mysql_free_result(res);
		return;
		}
		else if(!strcmp("quest",channel))
		{
		while ((row = mysql_fetch_row( res )))
			{
				sprintf(buf, "#8[#G%s#8] #CQ#cues#Ct #8- #W%s #8: #w%s#0 \n\r", row[2], row[1], row[3]);
				send_to_char(buf, ch);
			}
			mysql_free_result(res);
		return;
		}
		else if(!strcmp("immtalk",channel))
		{
		while ((row = mysql_fetch_row( res )))
			{
				sprintf(buf, "#8[#G%s#8] #CI#cmmtal#Ck #8- #W%s #8: #w%s#0 \n\r", row[2], row[1], row[3]);
				send_to_char(buf, ch);
			}
			mysql_free_result(res);
		return;
		}
		else if(!strcmp("flame",channel))
		{
		while ((row = mysql_fetch_row( res )))
			{
				sprintf(buf, "#8[#G%s#8] #CF#clam#Ce #8- #W%s #8: #w%s#0 \n\r", row[2], row[1], row[3]);
				send_to_char(buf, ch);
			}
			mysql_free_result(res);
		return;
		}	
		else
		log_string(LOG_SQL,"SQL Select Failed %s %s", channel, reslimit);
		return;
	}


	

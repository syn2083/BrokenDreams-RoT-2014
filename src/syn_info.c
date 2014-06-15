#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "syn_mysql.h"
/* Broken Dreams informational commands */

/*
 * Retooled Help command based off work by Jobo of SocketMUD
 * Had to re-engineer for non threaded environment, and tailor to the MUD
 * Will find help (but help position still counts) and related partial matches 
 * returning first match as main result, and subsequent as 
 * potentially related headers..
 * -Syn
 */
void do_help(CHAR_DATA * ch, char * argument)
{
    MYSQL_RES *res;
    bool found = FALSE;
	bool extra = FALSE;
	char query[MSL];
	char buf[MSL];
	int len;
	int f;
	int i;
	int e;
	
	e = 0; //we will use this to mark first match of result set, so we can truncate results of subsequent matches
	i = 0; //int in the for loop to determine if any special characters so we dont break SQL SELECT query, and remove them
	f = 0; // bit used to tell us yep, we had specials, lets shut er down. Prefered this option vs modifying string ourselves
	len = strlen(argument); //used in for loop for special char detection

	// no argument? lets explain what this is..
	 if ((ch->level < 7) && (argument[0] == '\0'))
    {
		stc("{WWhat would you like to find a help for?\n{GH{gelp {W<{rkeyword{W>\n{GH{gelp{W all game{8/{Wcommands{8/{Wspells{8/{Wcredits{0\n\r",ch);
        return;
    }
    else if ((ch->level >= 7) && (argument[0] == '\0'))
    {
		stc("{WWhat would you like to find a help for?\n{GH{gelp {W<{rkeyword{W>\n{GH{gelp {Wall \n{GH{gelp {Wall game{8/{Wcommands{8/{Wspells{8/{Wcredits{8/{Wimmortals{8/{Wolc{0\n\r",ch);
        return;
    }
	if (!strcasecmp(argument,"all saves"))
	{
		int g = save_mud_helps();
		return;
	}
	//lets dump a list!
	if (!strcasecmp(argument,"all"))
	{
		if(ch->level < 109)
		{
			do_help(ch, "");
			return;
		}
		help_slist(ch,0);
		return;
	}
	if (!strcasecmp(argument,"all game"))
	{
		help_slist(ch,1);
		return;
	}
	if (!strcasecmp(argument,"all commands"))
	{
		help_slist(ch,2);
		return;
	}
	if (!strcasecmp(argument,"all spells"))
	{
		help_slist(ch,3);
		return;
	}
	if (!strcasecmp(argument,"all immortals"))
	{
		if(ch->level < 102)
		{
			do_help(ch, "");
			return;
		}
		help_slist(ch,4);
		return;
	}
	if (!strcasecmp(argument,"all olc"))
	{
		if(ch->level < 102)
		{
			do_help(ch, "");
			return;
		}
		help_slist(ch,5);
		return;
	}
	if (!strcasecmp(argument,"all credits"))
	{
		help_slist(ch,6);
		return;
	}
	// lets see what the string consists of, f = 1 if it finds any special character
	f = sanitize_arg(argument);
	// did we find any specials? if so, lets cancel the function and send a message about why..
	if(f==1)
	{
		stc("{WPlease remove any special characters from your search string {8({RE{rG{8:{4{W!@{{$%^&*()_+':><,./?\|{{}[]=-`~{0{8){W.{0\n\r",ch);
		return;
	}
	 // this is where we set the query string with our argument, and do our partial search as well in the form of %s = search %% = wildcard
	 // eg who or who whois or whois who or whois who whom or whois
	snprintf(query, sizeof(query) -1, "SELECT entry_num, level, category, keywords, text FROM bd_help WHERE keywords=\'%s\' OR keywords LIKE \'%s %%\' OR keywords LIKE \'%% %s\' OR keywords LIKE \'%% %s %%\' OR keywords LIKE \'%% %%%s %%\'OR keywords LIKE \'%% %s%% %%\' OR keywords LIKE \'%s%%\' LIMIT 10", argument, argument, argument,argument, argument, argument, argument);
	//sanitize and send the query off to SQL, then set the result string to res
	mysql_safe_query(query);
    res = mysql_store_result(&db);
	
    while ((row = mysql_fetch_row(res))) // while row (data row from SQL) has a row result from res (result)...
    {
		int level; 
		level = atoi(row[1]);// lets find level of helpfile to check if the requester can view!
		if((ch->level < level) || (ch->trust < level))
		{
			sprintf(buf, "{WNo help entry found for '%s'{0\n", argument); //sorry bub, no way!
			stc(buf,ch);
			return;
		}
        if(e == 0) //are we at base initialization?
		{
			e = 1; //set our bit to 1 to mark a successful find
			stc("                        {8-{w={W[{CB{croke{Cn {CD{cream{Cs {CH{cel{Cp {CS{cyste{Cm{W]{w={8-{0\n\r",ch);
			stc("{C+{8---------------------------------------------------------------------------{C+{0\n\r",ch);
			sprintf(buf,"{WH{welp {WF{wile {WF{wound {R- {8[{RE{rntry{8: {R%d{8] [{CK{ceyword{8({Cs{8): {W%s{8] [{CL{cevel{8: {C%d{8]{0\n\r", atoi(row[0]), row[3], level);
			stc(buf,ch);
			stc("{C+{8---------------------------------------------------------------------------{C+{0\n\r",ch);
			sprintf(buf, "{W%s{0\n\r", row[4]);
			stc(buf,ch);
			stc("{C+{8---------------------------------------------------------------------------{C+{0\n\r",ch);
			found = TRUE; //set our flag to true
			extra = TRUE;
		}
		else //this kicks in after the first loop, so following found entries will show up as related helpfiles instead of dumping each helpfile that partially matches search string
		{
			if(extra == TRUE)
			{
			sprintf(buf, " {RT{rhe following were also found with partially matching keyword{W({8s{W){R:{0\n\r");
			stc(buf,ch);
			stc("{C+{8---------------------------------------------------------------------------{C+{0\n\r",ch);
			extra = FALSE;
			}
			sprintf(buf, "{8({GK{geyword {GM{gatch {8- {R%s {8){R:{0", argument);
			stc(buf, ch);
			sprintf(buf,"{8[{RE{rntry{8: {R%d{8] [{CK{ceyword{8({Cs{8): {W%s{8] [{CL{cevel{8: {C%d{8]{0\n\r", atoi(row[0]), row[3], level);
			stc(buf,ch);
		}        
    }
	mysql_free_result(res); //free the result/memory
    if (!found) //didnt find any entries
    {
        sprintf(buf, "{WNo help entry found for {8'{C%s{8'{W. The Immortals will be notified. Thank you.{0\n", argument);
		log_string(LOG_HELP,"Missing help for: %s",argument);
		stc(buf,ch);
    }
    return; //done.
}

/*Lets make a help list*/
void help_slist(CHAR_DATA *ch, int type)
{
    char query[4096 * 2];
	char cbuf[MSL];
    MYSQL_RES *res;
    int col = 0;
	int e = 0;	
	//This is likely a very inefficient way to do this, but its what i know so, here we go.. ps, works perfect. int is passed to type, determines which helps to serve up.
	snprintf(query, sizeof(query) -1, "SELECT entry_num, level, keywords, category FROM bd_help ORDER BY entry_num");
	mysql_safe_query(query);
    res = mysql_store_result(&db);
	stc("                        {8-{w={W[{CB{croke{Cn {CD{cream{Cs {CH{cel{Cp {CS{cyste{Cm{W]{w={8-{0\n\r",ch);
	 if(type == 0)
	 {
		 while ((row = mysql_fetch_row(res)))
		{
				if(e==0)
				{
					stc("{C+{8----------------------------------{CA{cLL{8-------------------------------------{C+{0\n\r",ch);
					e = 1;
				}
				sprintf(cbuf, "{4{W[{0{8%-3d {W%-30s {4{W]{0", atoi(row[0]), row[2]);
				stc(cbuf, ch);
				if (++col % 2 == 0)
					stc("\n",ch);
			
		}
	}
	 if(type == 1)
	 {
		 while ((row = mysql_fetch_row(res)))
		{
			if(!strcmp(row[3],"game"))
			{
				if(e==0)
				{
					stc("{C+{8----------------------------------{CG{cAME{8-----------------------------------{C+{0\n\r",ch);
					e = 1;
				}
				sprintf(cbuf, "{4{W[{0{8%-3d {W%-30s {4{W]{0", atoi(row[0]), row[2]);
				stc(cbuf, ch);
				if (++col % 2 == 0)
					stc("\n",ch);
			}
		}
	}
	 if(type == 2)
	 {
		 while ((row = mysql_fetch_row(res)))
		{
			if(!strcmp(row[3],"commands"))
			{
				if(e==0)
				{
					stc("{C+{8----------------------------------{RC{rommands{8---------------------------------{C+{0\n\r",ch);
					e = 1;
				}
				sprintf(cbuf, "{4{W[{0{8%-3d {W%-30s {4{W]{0", atoi(row[0]), row[2]);
				stc(cbuf, ch);
				if (++col % 2 == 0)
					stc("\n",ch);
			}
		}
	}
	 if(type == 3)
	 {
		 while ((row = mysql_fetch_row(res)))
		{
			if(!strcmp(row[3],"spells"))
			{
				if(e==0)
				{
					stc("{C+{8----------------------------------{RS{Yp{Pe{Ll{Gl{gs{8-----------------------------------{C+{0\n\r",ch);
					e = 1;
				}
				sprintf(cbuf, "{4{W[{0{8%-3d {W%-30s {4{W]{0", atoi(row[0]), row[2]);
				stc(cbuf, ch);
				if (++col % 2 == 0)
					stc("\n",ch);
			}
		}
	}
	if((type == 4) && (ch->level >= 7))
	 {
		 while ((row = mysql_fetch_row(res)))
		{
			if(!strcmp(row[3],"immortals"))
			{
				if(e==0)
				{
					stc("{C+{8----------------------------------{GI{gmmortals{8--------------------------------{C+{0\n\r",ch);
					e = 1;
				}
				sprintf(cbuf, "{4{W[{0{8%-3d {W%-30s {4{W]{0", atoi(row[0]), row[2]);
				stc(cbuf, ch);
				if (++col % 2 == 0)
					stc("\n",ch);
			}
		}
	}
	if((type == 5) && (ch->level >= 7))
	 {
		 while ((row = mysql_fetch_row(res)))
		{
			if(!strcmp(row[3],"olc"))
			{
				if(e==0)
				{
					stc("{C+{8----------------------------------{GO{gLC{8--------------------------------------{C+{0\n\r",ch);
					e = 1;
				}
				sprintf(cbuf, "{4{W[{0{8%-3d {W%-30s {4{W]{0", atoi(row[0]), row[2]);
				stc(cbuf, ch);
				if (++col % 2 == 0)
					stc("\n",ch);
			}
		}
	}
	 if(type == 6)
	 {
		 while ((row = mysql_fetch_row(res)))
		{
			if(!strcmp(row[3],"credits"))
			{
				if(e==0)
				{
					stc("{C+{8----------------------------------{RC{rredits{8----------------------------------{C+{0\n\r",ch);
					e = 1;
				}
				sprintf(cbuf, "{4{W[{0{8%-3d {W%-30s {4{W]{0", atoi(row[0]), row[2]);
				stc(cbuf, ch);
				if (++col % 2 == 0)
					stc("\n",ch);
			}
		}
	}
    stc("\n",ch);
	stc("{C+{8---------------------------------------------------------------------------{C+{0\n\r",ch);
	mysql_free_result(res);
    return;
}

int save_mud_helps( )
{
	char query[5*MSL];
	MYSQL_RES *res;
	int control = 0;
	int count = 0;
	char *k;
	char *t;

	int cnt = 0;
        HELP_DATA *help;
        for (cnt = 0, help = help_first; cnt < top_help; help = help->next)
               {

					t = strdup(help->keyword);
					smash_singleq(t);
					long size = strlen(t);
					char tesc[(2*size)+1];
					mysql_real_escape_string(&db, tesc, t, size);
					
					k = strdup(help->text);
					smash_singleq(k);
					long size1 = strlen(k);
					char tesc1[(2*size1)+1];
					mysql_real_escape_string(&db, tesc1, k, size1);
					snprintf( query, sizeof(query)-1, "INSERT INTO bd_nhelp (level, keywords, text) VALUES( \'%d\', \'%s\', \'%s\')",help->level, tesc, tesc1);
					
					if(!mysql_safe_query(&query))
					{
						log_string(LOG_BUG,"{W\n\rSomething went wrong, help files not inserted for save.\n\r{0");
						//log_string(LOG_SQL, query);
						return 1;
					}
					res = mysql_store_result(&db);	
					mysql_free_result( res );
					free_string(t);
					free_string(k);
								++cnt;
				}
   log_string(LOG_GAME,"%d helps saved.",cnt);
  return 2;
}

void unboxed_help(CHAR_DATA * ch, char * argument)
{
    MYSQL_RES *res;
    bool found = FALSE;
	bool extra = FALSE;
	char query[MSL];
	char buf[MSL];

	 // this is where we set the query string with our argument, and do our partial search as well in the form of %s = search %% = wildcard
	 // eg who or who whois or whois who or whois who whom or whois
	snprintf(query, sizeof(query) -1, "SELECT text FROM bd_help WHERE keywords=\'%s\'", argument);
	//sanitize and send the query off to SQL, then set the result string to res
	mysql_safe_query(query);
    res = mysql_store_result(&db);
	
    while ((row = mysql_fetch_row(res))) // while row (data row from SQL) has a row result from res (result)...
    {
			stc("                        {8-{w={W[{CB{croke{Cn {CD{cream{Cs {CH{cel{Cp {CS{cyste{Cm{W]{w={8-{0\n\r",ch);
			stc("{C+{8---------------------------------------------------------------------------{C+{0\n\r",ch);
			sprintf(buf, "{W%s{0\n\r", row[0]);
			stc(buf,ch);
    }
	mysql_free_result(res); //free the result/memory
    return; //done.
}
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>

#include "merc.h"
#include "syn_mysql.h"

void connect_db(void)
{
 char buf[MAX_STRING_LENGTH];
	if (!mysql_init(&db))
	{
		log_string(LOG_SQL,"connect_db: error on initialize");
		return;
	}
	mysql_options(&db,MYSQL_OPT_COMPRESS,0);
	if ((mysql_real_connect(&db, DB_HOST, DB_USER, DB_PASS,DB_DB, DB_PORT, DB_SOCKET, CLIENT_MULTI_STATEMENTS)) ==0)
	{
		log_string(LOG_SQL,"connect_db: error on connect");
		sprintf(buf,"Error: %s", mysql_error(&db));
                log_string(LOG_SQL,buf);
 		return;
	}
        db_connect = TRUE;
        db_last_action = 0;
		log_string(LOG_SQL,"SQL Database Connection Opened.");
	return;
}

void disconnect_db(void)
{
	if(!db_connect)
	return;
	mysql_close(&db);
	log_string(LOG_SQL, "Closing SQL connection");
	db_connect = FALSE;
	return;
}
void check_db_disconnect(void)
{
	/* first check if we're even connected */
	if( !db_connect)
		return;
	/* No we see if we have been connected for too long,
	   if we have, lets disconnect and save resources
        */
	if( db_last_action >= DB_CONNECT_TIME_LIMIT)
	{
		disconnect_db();
		return;
	}
	/* at this point we must still be connected, so lets increment our time */
	db_last_action++;
	return;
}

/*
 * mysql_safe_query()
 *
 * Sanitizes Sql Querys
 */
int mysql_safe_query (char *query)
{
    char buf[4*MAX_STRING_LENGTH];
	buf[0] = '\0';

	if( !db_connect )
		connect_db();
		
	if( &db == NULL )
	{
		log_string(LOG_SQL,"NULL db in query_db.");
		return 0;
	}
	if( query == NULL || query[0] == '\0')
	{
		log_string(LOG_SQL,"Null query in query_db()");
		return 0;
	}
 	if (mysql_real_query(&db, query, strlen(query)))
	{
		//log_string(LOG_SQL,"query_db(): error on query");
		sprintf(buf,"Error: %s", mysql_error(&db));
               log_string(LOG_SQL,buf);
		return 0;
	}
 	return 1;
}

char *sql_escape_string( char *source ) 
{ 
 char *buf = '\0';  
 char *ret = '\0';  
 int len = 0; 
  
 len = strlen( source );  
 buf = malloc( (2 *len) + 1); 

 mysql_real_escape_string( &db, buf, source, len);  
 ret = str_dup( buf );  
 free( buf );  
 return ret; 
}
/* Broken Dreams Mud - Syn's MySQL Utilities and functions. */


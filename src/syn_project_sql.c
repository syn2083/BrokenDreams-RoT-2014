/*BDreams Project Code*/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "merc.h"
#include "syn_mysql.h"
#include "interp.h"

/**********************************************
* Load, alloc, realloc, and save project data
* This section sets up the project data to be used.. 
* I went with a kind of split system that may seem a bit strange

* Project data is loaded from the DB into MUD memory
* as a table struct, edits are done in memory, changes
* can then be saved to the DB 

* This is done so that the edits, creation of a project
* do not tie up the SQL interface. Unfortunately even though
* the codebase in general is non-blocking, the MySQL
* C API is blocking, eg request -> wait -> get results

* Log data is stored in SQL, and edited directly to SQL
* logs are not loaded into memory

* relying on in game loads, it was very difficult keeping
* logs per project accounted for and usable
* allowing logs to reside only in SQL made this much 
* easier

*Much of the project loading and saving code was taken
*or based off The Vortex's Sociallib SQL code by Kline
*Many portions had to be, or were, changed to update
*and fix issues that were present.
*The rest is all mine - Syn
********************************************/
void alloc_mud_projects(void)
{
	int i;

	mud_project_table = (MUD_PROJECT_DATA **)malloc( mud_project_count * sizeof( MUD_PROJECT_DATA *));
	for( i = 0; i < mud_project_count; i++)
		{
		mud_project_table[i] = (MUD_PROJECT_DATA  *) malloc(sizeof(MUD_PROJECT_DATA ));
		}
	return;
}

//this is the load function present in DB.c
void load_projects()
{
	MYSQL_RES *res;
	int control = -1;
	char query[MSL];
	int count = 0;

	snprintf( query, sizeof(query) -1, "SELECT * FROM bd_projects ORDER BY proj_id");
	log_string(LOG_GAME,"Initializing project data...");
	alloc_mud_projects();
	if(!mysql_safe_query(query))
	{
		log_string(LOG_BUG,"#W\n\rSomething went wrong, projects not loaded.\n\r#n");
		return;
	}
	res = mysql_store_result(&db);	
	log_string(LOG_GAME, "Loading project data...");
	while( (row = mysql_fetch_row( res )) )
	{
		control++;
		if( control > 0 )
			refactor_mud_projects(1);
		mud_project_table[control]->project_id          = atoi( row[PROJ_COL_ID]);
		mud_project_table[control]->created_by   		= str_dup( row[PROJ_COL_CB] );
		mud_project_table[control]->type 					= str_dup( row[PROJ_COL_TY] );
		mud_project_table[control]->assigned_to    	= str_dup( row[PROJ_COL_AT]  );
		mud_project_table[control]->name  				= str_dup( row[PROJ_COL_PN]  );
		mud_project_table[control]->description   		= str_dup( row[PROJ_COL_PD]  );
		mud_project_table[control]->created_on     	= str_dup( row[PROJ_COL_CO]  );
		mud_project_table[control]->log_count   		= str_dup( row[PROJ_COL_LC]  );
		mud_project_table[control]->status   				= str_dup( row[PROJ_COL_PS]  );
		mud_project_table[control]->completed   		= str_dup( row[PROJ_COL_PC]  );
		mud_project_table[control]->completed_on   	= str_dup( row[PROJ_COL_FO]  );
		mud_project_table[control]->changed      	 	= FALSE;
		count++;
	}
	sprintf(query,"%d projects loaded.", count);
	log_string(LOG_GAME,query);
	mysql_free_result( res );
	return;
}

//when we add a project we need to refactor that memory allocation, hence:
void refactor_mud_projects( int amt)
{
	int i;
	int total = amt + mud_project_count;

	mud_project_table = (MUD_PROJECT_DATA **)realloc(mud_project_table, total * sizeof(MUD_PROJECT_DATA *));

	for( i = mud_project_count; i < (mud_project_count + amt); i++)
		mud_project_table[i] = (MUD_PROJECT_DATA *) malloc(sizeof(MUD_PROJECT_DATA));
	mud_project_count = mud_project_count + amt;
	return;
}

// Save it up, ditched the crazy malloc setup vortex used, instead I will take care of data sanitization BEFORE we do any saving
// The malloc code was buggy, and not optimal for the siutation
int save_mud_projects( )
{
	char query[MSL];
	MYSQL_RES *res;
	int control = 0;
	int count = 0;
	char convers[200];

	for( control = 0; control < mud_project_count; control++)
	{
		if( mud_project_table[control]->changed == FALSE )
			continue;
		if( !str_cmp( mud_project_table[control]->name, "NULL") )
			continue;
		count = count_project_plogs(mud_project_table[control]->name);
		sprintf(convers, "%d",count);
		mud_project_table[control]->log_count = str_dup(convers);
		//lets see if this little dog is barkin
		snprintf( query, sizeof(query), "SELECT * from bd_projects WHERE project_name = '%s'", mud_project_table[control]->name);
		if(!mysql_safe_query(query))
		{
			log_string(LOG_BUG,"#W\n\rSomething went wrong, mud_project_data not queried for save.\n\r#n");
			return 1;
		}
		res = mysql_store_result(&db);	

		/* update if found */
		if( mysql_num_rows(res))
		{
			mysql_free_result( res);
			snprintf( query, sizeof(query) -1, "UPDATE bd_projects SET created_by = '%s', project_type = '%s', project_assigned_to = '%s', project_name = '%s', project_description = '%s', project_created_on = '%s', project_log_count = '%s', project_status = '%s', project_completed = '%s', project_completed_on = '%s' WHERE project_name = '%s'", 
                        mud_project_table[control]->created_by, mud_project_table[control]->type, mud_project_table[control]->assigned_to, mud_project_table[control]->name, mud_project_table[control]->description, mud_project_table[control]->created_on, mud_project_table[control]->log_count, mud_project_table[control]->status, mud_project_table[control]->completed, mud_project_table[control]->completed_on,  mud_project_table[control]->name);
			if(!mysql_safe_query(query))
			{
				log_string(LOG_BUG,"#W\n\rSomething went wrong, mud_project_data not updated for save.\n\r#n");
				return 1;
			}
			res = mysql_store_result(&db);
			mysql_free_result( res );
		}
		/* insert if not */
		else
		{
			mysql_free_result( res );
			snprintf( query, sizeof(query) -1, "INSERT INTO bd_projects (created_by, project_type, project_assigned_to, project_name, project_description, project_created_on, project_log_count, project_status, project_completed, project_completed_on) VALUES( '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",mud_project_table[control]->created_by, mud_project_table[control]->type, mud_project_table[control]->assigned_to, mud_project_table[control]->name, mud_project_table[control]->description, mud_project_table[control]->created_on, mud_project_table[control]->log_count, mud_project_table[control]->status, mud_project_table[control]->completed, mud_project_table[control]->completed_on);
			if(!mysql_safe_query(query))
					{
						log_string(LOG_BUG,"#W\n\rSomething went wrong, mud_project_data not inserted for save.\n\r#n");
						return 1;
					}
			res = mysql_store_result(&db);	
			mysql_free_result( res );
		}
	}
   log_string(LOG_GAME,"%d projects saved.",mud_project_count);
  return 2;
}
/*************************************
* End loads, saves and re/allocs for projects
*************************************/

//I wanted the log count to be determined dynamically to avoid some issues i ran into, so:
int count_project_plogs(char *pname)
{
	int i;
	MYSQL_RES *res;
	int count;
	char query[MSL];
	
	snprintf( query, sizeof(query), "SELECT projectname from bd_projects_log WHERE projectname='%s'", pname);
	if(!mysql_safe_query(query))
		{
			log_string(LOG_BUG,"#W\n\rSomething went wrong, mud_project_data not queried for save.\n\r#n");
			return -1;
		}
	res = mysql_store_result(&db);
	count = mysql_num_rows(res);
	mysql_free_result(res);
	return count;
}

// verify creator of plog for deletion/edit reasons
int is_plog_owner(CHAR_DATA *ch, char *ref)
{
	MYSQL_RES *res;
	char query[MSL];
	
	snprintf(query, sizeof(query) -1, "SELECT ref, created_by FROM bd_projects_log WHERE ref='%s'", ref);
	if(!mysql_safe_query(query))
	{
		log_string(LOG_SQL,"Verify PLOG failed");
		return 0;
	}
	
	res = mysql_store_result(&db);
	while(row = mysql_fetch_row(res))
	{
		if(!is_prefix(row[1], ch->name))
		{
			mysql_free_result(res);
			return 1;
		}
		mysql_free_result(res);
		return 0;
	}
}

// verify plog exists for plog commands
int is_plog_real(CHAR_DATA *ch, char *ref)
{
	MYSQL_RES *res;
	char query[MSL];
	
	snprintf(query, sizeof(query) -1, "SELECT ref FROM bd_projects_log WHERE ref='%s'", ref);
	if(!mysql_safe_query(query))
	{
		log_string(LOG_SQL,"Verify PLOG failed");
		return 0;
	}
	
	res = mysql_store_result(&db);
	while(row = mysql_fetch_row(res))
	{
			mysql_free_result(res);
			return 1;
	}
	mysql_free_result(res);
	return 0;
}

//verify project creator by plog ref id and ch data
int is_proj_plog_owner(char *ch, char *ref)
{
	MYSQL_RES *res;
	char query[MSL];
	int k = 0;
	int l = 0;
	int f = 0;
	char *pname;
	
	snprintf(query, sizeof(query) -1, "SELECT ref, projectname, created_by FROM bd_projects_log WHERE ref='%s'", ref);
	if(!mysql_safe_query(query))
	{
		log_string(LOG_SQL,"Verify PLOG pname failed");
		k = 2;
	}
	res = mysql_store_result(&db);
	while(row = mysql_fetch_row(res))
	{
		pname = str_dup(row[1]);
		if(is_prefix(ch,row[2]))
		k = 1;
	}
	
	mysql_free_result(res);
	if (k == 2)
	{
		stc("#RCould not find Project Name..\n\r",ch);
		return 0;
	}
	snprintf(query, sizeof(query) -1, "SELECT project_name, created_by FROM bd_projects WHERE project_name='%s'", pname);
	if(!mysql_safe_query(query))
	{
		log_string(LOG_SQL,"Verify Project Name failed");
		l = 2;
	}
	res = mysql_store_result(&db);
	while(row = mysql_fetch_row(res))
	{
		if(is_prefix(row[1], ch))
		{
			l = 1;
		}
	}
	mysql_free_result(res);
	f = k+l;
		return f;
}

// ensure a particular project exists, so a plog can 'attach'
int find_mud_project(char *projectname)
{
	int count;
	int fail = -1;
	if(projectname == NULL)
	return fail;
	
	for(count = 0; count < mud_project_count; count++)
	{
		if(!str_prefix(projectname, mud_project_table[count]->name))
		{
			return count;
		}	
	}
	return fail;
}

//clean up arg strings to remove ' and other odd input, most special characters are ok
int sanitize_arg(char *string)
{
	int i = 0;
	int z = 0;
	int len = 0;
	len = strlen(string);
	for(i =0; i < len; i++)
	{
		if (!isalnum(string[i]) && string[i] != ' ' && string[i] != '%' 
		&& string[i] != '@'&& string[i] != '(' && string[i] != ')' 
		&& string[i] != '-' && string[i] != '+' && string[i] != '=' 
		&& string[i] != '#' && string[i] != '.'&& string[i] != ',' 
		&& string[i] != '/' && string[i] != '\\' && string[i] != ';' 
		&& string[i] != ':' && string[i] != '[' && string[i] != ']' 
		&& string[i] != '|' && string[i] != '!' && string[i] != '?' 
		&& string[i] != '<' && string[i] != '>' && string[i] != '"'
		&& string[i] != '&' && string[i] != '$' && string[i] != '^'
		&& string[i] != '`' )
		{
			z = 1;
		}
		
	}
	if(z==1)
	return 1;
	else
	return 0;
}

/*And now, the show*/
void do_project( CHAR_DATA *ch, char *argument)
{
	int value;
	char arg1[MSL];
	char arg2[MSL];
	char arg3[MSL];
	char arg4[MSL];
	int project;
	int completion = 0;
	char buf[MSL];
	char query[MSL];
	int i =0;
	int g =0;
	int f =0;
	
		smash_tilde(argument);
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);
        strcpy(arg4, argument);
	/*lets clear out any characters we cant have in our SQL stuff*/
	f = sanitize_arg(arg1);
	i = sanitize_arg(arg4);
	if(f==1 || i==1)
	{
		stc("#WPlease remove the following special characters from your string #0(#RE#rG#0:{4#W'~#n#0)#W.#n\n\r",ch);
		return;
	}
	if( arg1[0] == '\0')
	{
		stc("                            #0-#w=#W[#CB#croke#Cn #CD#cream#Cs #CP#crojec#Ct #CS#cyste#Cm#W]#w=#0-#n\n\r",ch);
		stc("#W.#0------------------------------------#0---#c=#C=#0||#C=#c=#0---#0------------------------------------#W.\n\r",ch);
		stc("#W| #GC#greated #GB#gy  #W|     #CP#croject #CN#came          #0||      #RT#rype    #W| #PL#pog #PC#pount #W|   #YC#yompleted  #W|   \n\r",ch);
		stc("#W|#0____________________________________#0_#C/#0-#w--#0||#w--#0-#C\\#0_#0____________________________________#W|\n\r",ch);
		for(project =0; project<mud_project_count; project++)
		{
			mud_project_table[project]->log_count = count_project_plogs(mud_project_table[project]->name);
			stc("#W|#0--------------------------------------#0-#C\\#w_#0||#w_#C/#0-#0--------------------------------------#W|\n\r",ch);
			sprintf(buf,  "#W| #G%-11s ", mud_project_table[project]->created_by);
			stc(buf, ch);
			sprintf(buf,  "#W| #C%-20s      #0|", mud_project_table[project]->name);
			stc(buf, ch);
			sprintf(buf,  "#0|    #R%-10s", mud_project_table[project]->type);
			stc(buf, ch);
			sprintf(buf,  "#W| #P%-9d ", mud_project_table[project]->log_count);
			stc(buf, ch);
			sprintf(buf, "#W| #Y%-12s #W|#n\n\r", mud_project_table[project]->completed);
			stc(buf, ch);
			stc("#W|#0______________________________________#0_#C/#w-#0||#w-#C\\#0_#0______________________________________#W|\n\r",ch);
			
		}
		stc("#W`#0------------------------------------#0-#C\\#0_#w__#0||#w__#0_#C/#0-#0------------------------------------#W`\n\r",ch);
		sprintf(buf,"\n\r#R%d #WProjects displayed.#n\n\r", project);
		stc(buf, ch);
		
		stc("#0\n\r#WYou can use the following subcommands with no arguments for a description.\n\r",ch);
		stc("#0#CC#create#0, #GE#gdit#0, #RD#relete#0, #PS#pave#0 - saves all active projects, #YP#ylog#n\n\r",ch);
		return;
	}
	
	if(!strcasecmp(arg1, "edit"))
	{
		stc("#RTo edit a project type project <name> edit <field> <text>\n\r",ch);
		stc("#WValid fields are: type, status, description, assigned, completed.\nFor long names surround with \' #Rexample \'Long Project Name\' #W\nAssigned being the person the project is assigned to.\nText at the end needs no \' #REG:\nProject OLC edit description this is some long text that needs no \'s\n\r#n", ch);
		return;
	}
	if(!strcasecmp(arg1, "create"))
	{
		stc("#0To create a project type #Rproject <name> create\n\r",ch);
		stc("#WThis will create a new project.#n\n\r", ch);
		return;
	}
	if(!strcasecmp(arg1, "delete"))
	{
		stc("#0To delete a project type #Rproject <name> delete\n\r",ch);
		stc("#WDeletes the named project. - #RBy Creator only#n\n\r", ch);
		return;
	}
	if(!strcasecmp(arg1, "save"))
	{
		stc("\n\r#RSaving projects.#n\n\r",ch);
		save_mud_projects();
		return;
	}
	//validate whether the project we want to edit/delete/attach plog to etc exists
	if((strcmp(arg1,"plog"))&&(strcmp(arg2,"delete")) && (strcmp(arg2,"create")) && (value = find_mud_project( arg1)) == -1)
	{
		//ruh roh!
		sprintf(buf, "\n\r#RNo project named #C%s#R found.#n\n\r", arg1);
		stc(buf, ch);
		return;
	}
	//Project proper commands, not PLOG sub commands --
	if(!strcasecmp(arg2,"show"))
			{
				snprintf(query, sizeof(query) -1, "SELECT * FROM bd_projects WHERE project_name = '%s'", arg1);
				if(!mysql_safe_query(query))
					{
						log_string(LOG_SQL, query);
						return;
					}
				res = mysql_store_result(&db);
				
					stc("                          #0-#w=#W[#CB#croke#Cn #CD#cream#Cs #CP#crojec#Ct #CS#cyste#Cm#W]#w=#0-#n\n\r",ch);
					stc("#W.#0------------------------------------#0-#C\\#0_#w__#0||#w__#0_#C/#0-#0------------------------------------#W.\n\r",ch);
					while(row = mysql_fetch_row(res))
					{
						sprintf(buf,"#WP#wroject #WI#wnfo \n#RN#rame#0:          #W%-21s#RC#rreated #RO#rn#0:      #W%s\n#RC#rreated #RB#ry#0:    #W%-21s#RA#rssigned #RT#ro#0:     #W%s\n#RL#rog #RC#rount#0:     #W%-21d#RS#rtatus#0:          #W%s\n#RC#rompleted#0:     #W%-21s#RC#rompleted #RO#rn#0:    #W%s#n\n\r", row[4], row[6], row[1], row[3], atoi(row[7]), row[8], row[9], row[10]);
						stc(buf,ch);
						stc("#W'#0____________________________________#0_#C/#0-#w--#0||#w--#0-#C\\#0_#0____________________________________#W'\n\r",ch);
						stc("#W.#0------------------------------------#0-#C\\#0_#w__#0||#w__#0_#C/#0-#0------------------------------------#W.\n\r",ch);
						sprintf(buf, "#RD#resription#0:\n\r\n\r#W%s#n\n\r\n\r", row[5]);
						stc(buf,ch);
						stc("#W'#0____________________________________#0_#C/#0-#w--#0||#w--#0-#C\\#0_#0____________________________________#W'\n\r",ch);
					}
				
			mysql_free_result(res);
			return;
			}
	if(!strcasecmp(arg2, "edit"))
	{
		if(str_prefix(ch->name,mud_project_table[value]->created_by) && str_prefix(ch->name, mud_project_table[value]->assigned_to))
		{
			stc("#RYou can only edit a project you have created or are assigned to!#n\n\r",ch);
			return;
		}
		else if( !strcasecmp( arg3, "status"))
		{
			mud_project_table[value]->status = strdup( arg4 );
			mud_project_table[value]->changed = TRUE;
			sprintf(buf, "#WProject #R%s#W has been updated. Status is now #R%s#W.#n \n\r", mud_project_table[value]->name, mud_project_table[value]->status);
			stc(buf, ch);
			completion = save_mud_projects();
			return;
		}
		else if( !strcasecmp( arg3, "description"))
		{
			mud_project_table[value]->description = str_dup( arg4 );
			mud_project_table[value]->changed = TRUE;
			sprintf(buf, "#WProject #R%s#W description set to #R%s#W.#n\n\r", mud_project_table[value]->name, mud_project_table[value]->description);
			stc(buf, ch);
			completion = save_mud_projects();
			return;
		}
		else if( !strcasecmp( arg3, "type"))
		{
			mud_project_table[value]->type = strdup( arg4 );
			mud_project_table[value]->changed = TRUE;
			sprintf(buf, "#WProject #R%s#W type set to #R%s#w.#n \n\r",mud_project_table[value]->name, mud_project_table[value]->type);
			stc(buf, ch);
			completion = save_mud_projects();
			return;
		}
		else if( !strcasecmp( arg3, "assign"))
		{
			mud_project_table[value]->assigned_to = strdup( arg4 );
			mud_project_table[value]->changed = TRUE;
			sprintf(buf, "#WProject #R%s#W assigned to #R%s#W.#n\n\r",mud_project_table[value]->name, mud_project_table[value]->assigned_to);
			stc(buf, ch);
			completion = save_mud_projects();
			return;
		}
		else if( !strcasecmp( arg3, "completed"))
		{
			if(!strcasecmp(arg4,"yes"))
			{
				completion = 1;
				mud_project_table[value]->completed_on = time_stamp(1);
				mud_project_table[value]->completed = strdup("#GY#ges#n         ");
				mud_project_table[value]->changed = TRUE;
				sprintf(buf, "#WProject #R%s #Whas been marked as completed on %s#W.#n \n\r", mud_project_table[value]->name, mud_project_table[value]->completed_on);
				stc(buf, ch);
				completion = save_mud_projects();
			}
			else if(!strcasecmp(arg4,"no"))
			{
				completion = 0;
				mud_project_table[value]->completed_on = strdup("Incomplete");
				mud_project_table[value]->completed = strdup("#YN#yo#n");
				mud_project_table[value]->changed = TRUE;
				sprintf(buf, "#WProject #R%s #Whas been marked as #R%s#W.#n\n\r", mud_project_table[value]->name, mud_project_table[value]->completed_on);
				stc(buf, ch);
				completion = save_mud_projects();
			}
			else 
			stc("#0Please choose #Ryes #0or #Rno#0.#n\n\r",ch);
			mud_project_table[value]->changed = FALSE;
			return;
		}
		completion = save_mud_projects();
		stc("\n#RSaving projects..#n\n\r",ch);
		return;
	}
	if(!strcasecmp(arg2, "save"))
	{
		completion = save_mud_projects();
		if(completion == 1)
			stc("#RSomething went wrong with the save.#n\n\r",ch);
		else if(completion == 2)
			stc("#0Everything went well with the save.#n\n\r",ch);
		return;
	}
	if(!strcasecmp(arg2, "delete"))
	{
		completion = -1;
		if((value = find_mud_project( arg1)) == -1)
		{
			sprintf(buf, "#0No project named #R%s#0 was found.#n\n\r", arg1);
			stc(buf, ch);
			return;
		}
		if(str_prefix(ch->name,mud_project_table[value]->created_by))
		{
			stc("#ROnly the original project creator may delete it.#n\n\r",ch);
			return;
		}
		snprintf( query, sizeof(query), "DELETE FROM bd_projects WHERE project_name = '%s'", mud_project_table[value]->name);
		if(!mysql_safe_query(query))
		{
			log_string(LOG_SQL,"#W\n\rSomething went wrong, mud_project_data not queried for deletion.\n\r#n", query);
			return;
		}
		log_string(LOG_GAME,"Project %s was deleted by %s.",mud_project_table[value]->name, ch->name);
		res = mysql_store_result(&db);
		mysql_free_result(res);
		snprintf( query, sizeof(query), "DELETE FROM bd_projects_log WHERE projectname = '%s'", mud_project_table[value]->name);
		if(!mysql_safe_query(query))
		{
			log_string(LOG_BUG,"#W\n\rSomething went wrong, mud_project_data not queried for create.\n\r#n");
			return 1;
		}
		res = mysql_store_result(&db);	
		free_string(mud_project_table[value]->created_by);
		free_string(mud_project_table[value]->type);
		free_string(mud_project_table[value]->assigned_to);
		free_string(mud_project_table[value]->name);
		free_string(mud_project_table[value]->description);
		free_string(mud_project_table[value]->created_on);
		free_string(mud_project_table[value]->status);
		free_string(mud_project_table[value]->completed_on);
		free_string(mud_project_table[value]->completed);
		mud_project_table[value]->changed = TRUE;
		mud_project_table[mud_project_count-1]->name  				= str_dup( "NULL" );
		mud_project_table[value]->project_id = completion;
		--mud_project_count;
		save_mud_projects();
		sprintf(buf,"#0Project #R%s#0 was deleted successfully.#n\n\r", arg1);
		stc(buf, ch);
		return;		
	}
	if(!strcasecmp(arg2, "create"))
	{
		snprintf( query, sizeof(query), "SELECT * from bd_projects WHERE project_name = '%s'", arg1);
		log_string(LOG_SQL,query);
		if(!mysql_safe_query(query))
		{
			log_string(LOG_BUG,"#W\n\rSomething went wrong, mud_project_data not queried for create.\n\r#n");
			return 1;
		}
		res = mysql_store_result(&db);	

		/* this secion of code is to update an entry that already exists */
		if( mysql_num_rows(res))
		{
			stc("#RThis project already exists, please select another project name\n\r",ch);
			return;
		}
		mysql_free_result(res);
		refactor_mud_projects(1);
		char sname[50];
		sprintf(sname,"%s",ch->name);
		mud_project_table[mud_project_count-1]->project_id   		= mud_project_count-1;
		mud_project_table[mud_project_count-1]->created_by   	= strdup( sname );
		mud_project_table[mud_project_count-1]->type 				= strdup( "Not Set" );
		mud_project_table[mud_project_count-1]->assigned_to    	= strdup( "Not Set"  );
		mud_project_table[mud_project_count-1]->name  				= strdup( arg1 );
		mud_project_table[mud_project_count-1]->description   		= strdup( "Not Set"  );
		mud_project_table[mud_project_count-1]->created_on     	= strdup( time_stamp(1)  );
		mud_project_table[mud_project_count-1]->log_count   		= strdup("0");
		mud_project_table[mud_project_count-1]->status   			= strdup( "Not Set"  );
		mud_project_table[mud_project_count-1]->completed   		= strdup("No");
		mud_project_table[mud_project_count-1]->completed_on   = strdup( "Not Set"  );
		mud_project_table[mud_project_count-1]->changed      	 	= TRUE;
		sprintf( buf, "#0New project: #R%s created.#n\n\r", arg1);
		stc(buf,ch);
		sprintf(buf,"New project: %s created by %s.", arg1, ch->name);
		log_string(LOG_GAME, buf);
		completion = save_mud_projects();
	}
	//now the nitty gritty plog sub command..
	if(!strcasecmp(arg1, "plog"))
	{
		if(arg2[0]=='\0')
		{
		stc("#RProject Plog <Project Name> Create, Project Plog <Project Name> List\nProject Plog <number> Delete, Project Plog <num> <field> <text>\n\r",ch);
		stc("Create - make a new log for a specified project\nList shows a list of logs for that project\nDelete - delete a specified log\n<num> <field> <text> - Valid fields are subject, message, by log number\n\r#n ",ch);
		return;
		}
		if( is_number(arg2))
		{
			int g;
			g	= is_plog_real(ch, arg2);
			if(g !=1)
			{
				stc("\n\r#RPlog entry not found!#n\n\r",ch);
				return;
			}
			if(!strcasecmp(arg3,"delete"))
			{
				int p = 0;
				char *name;
				name = strdup(ch->name);
				p = is_proj_plog_owner(name, arg2);
				if((p == 2) || (p == 1))
				{	
					snprintf( query, sizeof(query) -1, "DELETE FROM bd_projects_log WHERE ref = '%s'", arg2);
					if(!mysql_safe_query(query))
					{
						log_string(LOG_BUG,"#W\n\rSomething went wrong, plog not deleted.\n\r#n", query);
						return 1;
					}
					res = mysql_store_result(&db);
					log_string(LOG_SQL,"Plog %s deleted by %s", arg2, ch->name);
					mysql_free_result( res );
					sprintf(buf,"#R\n\rPlog %s has been deleted.#n\n\r", arg2);
					stc(buf,ch);
					return;
				}
				else
				{
					stc("#RYou must be the message creator or project creator to delete it!#n\n\r",ch);
					return;
				}
			}
			if(!strcasecmp(arg3,"subject"))
			{
				int p = 0;
				char *name;
				name = strdup(ch->name);
				p = is_proj_plog_owner(name, arg2);
				if((p == 2) || (p == 1))
				{	
					snprintf( query, sizeof(query) -1, "UPDATE bd_projects_log SET subject = '%s' WHERE ref = '%s'", arg4, arg2);
					if(!mysql_safe_query(query))
					{
						log_string(LOG_BUG,"#W\n\rSomething went wrong, plog subject not updated.\n\r#n", query);
						return 1;
					}
					res = mysql_store_result(&db);
					log_string(LOG_SQL,query);
					mysql_free_result( res );
					return;
				}
				else
				{
					stc("#RYou must be the message creator or project creator to edit it!#n\n\r",ch);
					return;
				}
			}
			if(!strcasecmp(arg3,"message"))
			{
				int p = 0;
				char *name;
				name = strdup(ch->name);
				p = is_proj_plog_owner(name, arg2);
				if((p == 2) || (p == 1))
				{	
					if(arg4[0] == '\0')
					{
						string_edit(ch,&ch->plogstring);
						stc("\n\n\n\n#0Use #RProject plog <num> message save#0 to save this message!#n\n\r",ch);
						return;
					}
					if(!strcasecmp(arg4,"save"))
					{
						snprintf( query, sizeof(query) -1, "UPDATE bd_projects_log SET message = '%s' WHERE ref = '%s'", ch->plogstring, arg2);
						log_string(LOG_SQL, query);
						if(!mysql_safe_query(query))
						{
							log_string(LOG_BUG,"#W\n\rSomething went wrong, plog message not updated.\n\r#n",query);
							return;
						}
						res = mysql_store_result(&db);
						mysql_free_result( res );
						stc("#RLog message saved!#n\n",ch);
						return;
					}
				}
				else
				{
					stc("#RYou must be the message creator or project creator to edit it!#n\n\r",ch);
					return;
				}
			}
			if(!strcasecmp(arg3,"show"))
			{
				snprintf(query, sizeof(query) -1, "SELECT * FROM bd_projects_log WHERE ref = '%s'", arg2);
				if(!mysql_safe_query(query))
					{
						log_string(LOG_SQL, query);
						return;
					}
				res = mysql_store_result(&db);
				while(row = mysql_fetch_row(res))
				{
					stc("                          #0-#w=#W[#CB#croke#Cn #CD#cream#Cs #CP#crojec#Ct #CS#cyste#Cm#W]#w=#0-#n\n\r",ch);
					stc("#W.#0--------------------------------------#0-#C\\#w_#0||#w_#C/#0-#0--------------------------------------#W.\n\r",ch);
					sprintf(buf,"#WP#wroject #WL#wog \n#RN#rumber#0:         #W%d \n#RP#rroject #RN#rame#0:   #W%s \n#RC#rreated #RB#ry#0:     #W%s \n#RC#rreated #RO#rn#0:     %s\n#RS#rubject#0:        #W%s#n\n\r", atoi(row[0]), row[1], row[2], row[3], row[4]);
					stc(buf,ch);
					stc("#W'#0______________________________________#0_#C/#w-#0||#w-#C\\#0_#0______________________________________#W'\n\r",ch);
					stc("#W.#0------------------------------------#0-#C\\#0_#w__#0||#w__#0_#C/#0-#0------------------------------------#W.\n\r",ch);
					sprintf(buf, "#RM#ressage#0:\n\r\n\r#W%s#n\n\r\n\r", row[5]);
					stc(buf,ch);
					stc("#W'#0______________________________________#0_#C/#w-#0||#w-#C\\#0_#0______________________________________#W'\n\r",ch);
				}
				mysql_free_result(res);
				return;
			}
		}
		if(!strcasecmp(arg3,"create"))
			{
				snprintf( query, sizeof(query) -1, "INSERT INTO bd_projects_log (projectname, created_by, created_on, subject, message) VALUES('%s', '%s', '%s', 'No Subject', 'No Message')", arg2, ch->name, time_stamp(1));
				if(!mysql_safe_query(query))
						{
							log_string(LOG_SQL, query);
							log_string(LOG_BUG,"#W\n\rSomething went wrong, mud_project_data not inserted for save.\n\r#n");
							return;
						}
				if((res = mysql_store_result(&db)) == 0 &&  mysql_field_count(&db) == 0 &&    mysql_insert_id(&db) != 0)
				{
					int t;
					t = mysql_insert_id(&db);
					sprintf(buf,"#RLog #C%d#R created for project #C%s#R.#n\n\r", t, arg2);
					stc(buf,ch);
				}
				mysql_free_result( res );
				return;
			}
		if( !strcasecmp(arg3,"list"))
		{
			int c = 0;
			c = count_project_plogs(arg2);
			if (c == 0)
			{
				stc("#RThere are no logs for this project.#n\n\r",ch);
				return;
			}
			snprintf( query, sizeof(query) -1, "SELECT ref, projectname, created_by, created_on, subject, message FROM bd_projects_log where projectname = '%s'", arg2);
			if(!mysql_safe_query(query))
			{
				log_string(LOG_SQL, query);
				log_string(LOG_BUG,"#W\n\rSomething went wrong, no plog list.\n\r#n");
				return;
			}
			res = mysql_store_result(&db);
			
			if(res)
			{
				stc("                          #0-#w=#W[#CB#croke#Cn #CD#cream#Cs #CP#crojec#Ct #CS#cyste#Cm#W]#w=#0-#n\n\r",ch);
				stc("#W.#0------------------------------------#0---#c=#C=#0||#C=#c=#0---#0------------------------------------#W.\n\r",ch);
				stc("#0( #GL#gog #G## #W|     #CP#croject #CN#came    #W|  #RA#rdded #RB#ry #0||     #PD#pate       #W|        #YS#yubject         #0)   \n\r",ch);
				stc("#W'#0______________________________________#0_#C/#w-#0||#w-#C\\#0_#0______________________________________#W'\n\r",ch);
				while(row = mysql_fetch_row(res))
				{
						stc("#W.#0--------------------------------------#0-#C\\#w_#0||#w_#C/#0-#0--------------------------------------#W.\n\r",ch);
						sprintf(buf,  "#0) #G%-5d ", atoi(row[0]));
						stc(buf, ch);
						sprintf(buf,  "#W| #C%-19s ", row[1]);
						stc(buf, ch);
						sprintf(buf,  "#W| #R%-10s", row[2]);
						stc(buf, ch);
						sprintf(buf,  "#0||%-9s ", row[3]);
						stc(buf, ch);
						sprintf(buf, "#W| #Y%-22s #0(#n\n\r", row[4]);
						stc(buf, ch);
						stc("#W'#0______________________________________#0_#C/#w-#0||#w-#C\\#0_#0______________________________________#W'\n\r",ch);
				}
				stc("#W.#0------------------------------------#0-#C\\#0_#w__#0||#w__#0_#C/#0-#0------------------------------------#W.\n\r",ch);
				mysql_free_result( res );
				return;
			}
			
		}
	}
	return;
}
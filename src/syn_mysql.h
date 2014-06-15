/* Broken Dreams SQL Core
*  This .h file contains the Defines
*  and base structure
*  -Syn
*/
#include <mysql/mysql.h>
#define DB_HOST "10.1.1.3"
#define DB_USER "someuser"
#define DB_PASS "somepassword"
#define DB_DB   "somedb"
#define DB_PORT        3306
#define DB_SOCKET      0
#define DB_CLIENT_FLAG 0
#define DB_CONNECT_TIME_LIMIT 5

MYSQL db;
MYSQL_RES  *res;
MYSQL_ROW  row;

void   connect_db(void);
void   disconnect_db(void);

bool   db_connect;
sh_int db_last_action;
void   db_check_disconnect();





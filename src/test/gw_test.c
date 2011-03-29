/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#include "gw_client.h"
#include "gw_cmds_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>

#define TEST_VERSION "0.2"

// Utils
int  check_proxy();
void exit_fail(char *reason);
void exit_gwd();
void display_help();
void change_jts(char *server_hostname);

extern char *optarg;
extern int   optind, opterr, optopt;

struct test_result {
   char *test_name;       /* name of the test                                    */
   int  success;          /* 0 = success, 1 = failure                            */
   char *reason;
   int  execute;          /* 0 = don't, 1 = do                                   */
};

struct test_result test_matrix[] =
{
	{"Normal Execution (SINGLE)", 1, "",0}, 							// 1
	{"Normal Execution (BULK)", 1, "",0},								// 2
	{"Pre Wrapper", 1, "",0},											// 3	
	{"Prolog Fail (Fake Stdin) No Reschedule", 1, "",0},			    // 4
	{"Prolog Fail (Fake Stdin) Reschedule", 1, "",0},					// 5
	{"Prolog Fail (Fake Input File) No Reschedule", 1, "",0},			// 6
	{"Prolog Fail (Fake Executable) No Reschedule", 1, "",0},	    	// 7
	{"Prolog Fail (Fake Stdin) No Reschedule (BULK)", 1, "",0},		    // 8
	{"Execution Fail No Reschedule", 1, "",0},							// 9
	{"Execution Fail Reschedule", 1, "",0},								// 10
	{"Hold Release", 1, "",0},					  	                    // 11
	{"Stop Resume", 1, "",0},						  	                // 12
	{"Kill Sync", 1, "",0},	                                            // 13
	{"Kill Async", 1, "",0},	                                        // 14
	{"Kill Hard", 1, "",0},	                                            // 15
	{"Migrate", 1, "",0},    	                                        // 16
	{"Checkpoint local", 1, "",0},                                      // 17
	{"Checkpoint remote server", 1, "",0},                              // 18
	{"Wait Timeout", 1, "",0},                                          // 19
	{"Wait ZeroTimeout", 1, "",0},                                      // 20
	{"Input Output files", 1, "",0},                                    // 21
	{"Epilog Fail (Fake Output) No Reschedule", 1, "",0},			    // 22
	{"Epilog Fail (Fake Output) Reschedule", 1, "",0},				    // 23
	{"Epilog Fail (Fake Output) No Reschedule (BULK)", 1, "",0}	        // 24	
};

gw_client_t * gw_session;

// Tests
char * normal_execution(int prewrapper);
char * normal_execution_bulk();
char * prolog_fail(int which_fail);
char * prolog_fail_bulk();
char * execution_fail(int which_fail);
char * hold_release();
char * stop_resume();
char * kill_sync();
char * kill_async();
char * kill_hard();
char * migrate();
char * checkpoint(int which_jt);
char * wait_timeout(int _timeout);
char * input_output();
char * epilog_fail(int which_fail);
char * epilog_fail_bulk();

int main(int argc, char **argv)
{
	char lock[PATH_MAX];
	char *reason;
	char *jt_dir;
	char *server_hostname;
  	char opt;
  	char *GW_LOCATION;
	
	int test_index = 0;
	int failed = 0;
	int i,j=0;      // loop indexes
	int check_gwd  = 1;
	int params;
	
	struct stat buf;
	
	
	printf("GridWay gwtest v%s\n",TEST_VERSION);
    fflush(NULL);
    
    opterr = 0;
    optind = 1;
    
    j++;
    params = argc;
    
    while((opt = getopt(argc, argv, "chs:")) != -1)
        switch(opt)
        {
        	case 'h':
        		display_help();
        		exit(0);
        		break;
        	case 'c':
        		check_gwd = 0;
        		j++;
        		params--;
        		break;
        	case 's':
            	server_hostname = strdup(optarg);
         		change_jts(server_hostname);
        		j+=2;
        		params-=2;
        		break;           	          		
        }      
    
    // Check args
    
    if(params>1)
    	for(;j<argc;j++)
    	{
    	  i=atoi(*(argv+j));	
    	  if(i<=24 && i>0)
    	  {
    	  	test_matrix[i-1].execute=1;
    	  	printf("Test %d \"%s\" enabled.\n",i,test_matrix[i-1].test_name);
    	  }
    	 }
    else
    	for(i=0;test_matrix[i].test_name;i++)
    		test_matrix[i].execute=1;
	
	printf("Checking system and environment.");
	fflush(NULL);
	// Check proxy
	if(check_proxy()!=0)
	{
		printf("\nCould not find a valid proxy.\n");
		exit(-1);
	} 
	
	printf(".");
    fflush(NULL);
	
	// Check environment	
	GW_LOCATION=getenv("GW_LOCATION");
	
	if(GW_LOCATION==NULL)
	{
		printf("GW_LOCATION is not set.\n");
		exit(-1);
	}
	
	printf(".");
    fflush(NULL);
    	
	// Start gwd, fails if already started	
    snprintf(lock, PATH_MAX - 1, "%s/" GW_VAR_DIR "/.lock", GW_LOCATION);
    
   	printf(".");
    fflush(NULL);
  
    if(check_gwd==1)
	    if( stat(lock,&buf) == 0 )
	    {
	        fprintf(stderr,"\nError! Lock file %s exists. Please ensure gwd is not running before attempting the tests (or use the -c option).\n",lock);
	        exit(-1);
	    }
    
    printf(".");
    fflush(NULL);
    
    // TODO -- gwd.conf
    if(check_gwd==1)
       system("gwd -c");
       
    sleep(1);
    
    printf(".");
    fflush(NULL);
    

	// Change to job template dir
	jt_dir = (char *) malloc (sizeof(char)*(strlen(GW_LOCATION) + sizeof(GW_TEST_DIR) + 6));
    sprintf(jt_dir, "%s/" GW_TEST_DIR "/jt", GW_LOCATION);
    chdir(jt_dir);
    
    printf(".");
    fflush(NULL);
    
    // connect to gwd
    gw_session = gw_client_init();
    
    printf(".done\n");
    fflush(NULL);
    
    
    printf("gwd started, starting tests ... \n");
    fflush(NULL);
   
    /**------------------------------------------
     *      1.Normal Execution (SINGLE)
     * ------------------------------------------*/ 
    if(test_matrix[test_index++].execute) 
    {
    
    printf("Beggining of test 1: Normal Execution (SINGLE)\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=normal_execution(0),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 1 finished. Result = %s\n",reason);
    fflush(NULL);

    free(reason);

    }
	
    /**------------------------------------------
     *      2.Normal Execution (BULK)
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 2: Normal Execution (BULK)\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=normal_execution_bulk(),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 2 finished. Result = %s\n",reason);
    fflush(NULL);
   
    free(reason);

    }

    /**------------------------------------------
     *      3.Pre Wrapper
     * ------------------------------------------*/
    if(test_matrix[test_index++].execute) 	
    {
    
    printf("Beggining of test 3: Pre Wrapper\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=normal_execution(1),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 3 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    }
    
    /**------------------------------------------
     *      4.Prolog Fail (Fake Stdin) No Reschedule
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 4: Prolog Fail (Fake Stdin) No Reschedule\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=prolog_fail(0),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 4 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);    

    }   

    /**------------------------------------------
     *      5.Prolog Fail (Fake Stdin) Reschedule
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 5: Prolog Fail (Fake Stdin) Reschedule\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=prolog_fail(1),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 5 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);
    
    }   
    
    /**------------------------------------------
     *      6.Prolog Fail (Fake Input) No Reschedule
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 6: Prolog Fail (Fake Input) No Reschedule\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=prolog_fail(2),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 6 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    }    

    /**------------------------------------------
     *      7.Prolog Fail (Fake Executable) No Reschedule
     * ------------------------------------------*/	
   if(test_matrix[test_index++].execute)
   {
    
    printf("Beggining of test 7: Prolog Fail (Fake Executable) No Reschedule\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=prolog_fail(3),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason); 

    printf("Test 7 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    }  
  
   /**------------------------------------------
     *      8.Prolog Fail (Fake Stdin) No Reschedule (BULK)
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 8: Prolog Fail (Fake Stdin) No Reschedule (BULK)\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=prolog_fail_bulk(),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 8 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    }   
    
   /**------------------------------------------
     *      9.Execution Fail No Reschedule
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 9: Execution Fail No Reschedule\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=execution_fail(0),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 9 finished. Result = %s\n",reason);
    fflush(NULL);
    
	free(reason);

    }   
    
   /**------------------------------------------
     *      10.Execution Fail Reschedule
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 10: Execution Fail Reschedule\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=execution_fail(1),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 10 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    } 
    
   /**------------------------------------------
     *      11.Hold Release
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 11: Hold Release\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=hold_release(),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 11 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    }  
  
   /**------------------------------------------
     *      12.Stop Resume
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 12: Stop Resume\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=stop_resume(),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 12 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    } 
    
   /**------------------------------------------
     *      13.Kill Sync
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 13: Kill Sync\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=kill_sync(),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 13 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    } 
    
    /**------------------------------------------
     *      14.Kill Async
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 14: Kill Async\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=kill_async(),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 14 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    } 
 
   /**------------------------------------------
     *      15.Kill Hard 
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 15: Kill Hard\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=kill_hard(),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 15 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    } 
    
   /**------------------------------------------
     *      16.Migrate 
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 16: Migrate\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=migrate(),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 16 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    }
    
   /**------------------------------------------
     *      17.Checkpoint local
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 17: Checkpoint local\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=checkpoint(0),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 17 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    } 
    
   /**------------------------------------------
     *      18.Checkpoint remote server
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 18: Checkpoint remote server\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=checkpoint(1),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 18 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    } 
    
    /**------------------------------------------
     *      19.Wait timeout
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 19: Wait timeout\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=wait_timeout(0),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 19 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    } 
    
    /**------------------------------------------
     *      20.Wait zero timeout
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 20: Wait zero timeout\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=wait_timeout(1),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 20 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    } 
    
    /**------------------------------------------
     *      21.Input Output files
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 21: Input Output files\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=input_output(),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 21 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    } 
      

    /**------------------------------------------
     *      22.Epilog Fail (Fake Output) No Reschedule
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 22: Epilog Fail (Fake Output) No Reschedule\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=epilog_fail(0),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 22 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);
    
    }   
    
    /**------------------------------------------
     *      23.Epilog Fail (Fake Output) Reschedule
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 23: Epilog Fail (Fake Output) Reschedule\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=epilog_fail(1),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 23 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    }    
  
   /**------------------------------------------
     *      24.Epilog Fail (Fake Output) No Reschedule (BULK)
     * ------------------------------------------*/	
    if(test_matrix[test_index++].execute)
    {
    
    printf("Beggining of test 24: Epilog Fail (Fake Output) No Reschedule (BULK)\n");
    fflush(NULL);
    
    // Test normal execution
    if(strcmp(reason=epilog_fail_bulk(),"OK")==0)
		test_matrix[test_index-1].success = 0;
	else
	    test_matrix[test_index-1].reason = strdup(reason);  

    printf("Test 24 finished. Result = %s\n",reason);
    fflush(NULL);
    
    free(reason);

    }   


    // Disconnect from gwd
    gw_client_finalize();
    
    
    // Get rid of output clutter
    system("rm std* > /dev/null 2>&1");
    system("rm ee*  > /dev/null 2>&1");
    system("rm oo*  > /dev/null 2>&1");
    system("rm env*  > /dev/null 2>&1");
    unlink("outfile");
    
    
    // Display results 

	for(i=0; i < test_index; i++)
    {
    	if(test_matrix[i].execute)
    	{
	    	if(!test_matrix[i].success)
	    		printf("[OK] [%d] Test %s was successful.\n",i+1,test_matrix[i].test_name);
	    	else
	    	{
	    		printf("[ER] [%d] Test %s failed, reason = %s\n",
	    		        i+1,test_matrix[i].test_name,test_matrix[i].reason);
	    		failed++;
	    	}
    	}
    }   
 
	if(!failed)
    	printf("All GridWay tests successful.\n");
    else
    	printf("%d test(s) failed, please report the bug to gridway-user@globus.org.\n",failed);
	
	
	if(check_gwd==1)
		exit_gwd();
	exit(0);
	
}

// Util functions

int check_proxy()
{
	int rc;
	rc = system("grid-proxy-info > /dev/null 2>&1");
	return rc;
}

void exit_fail(char *reason)
{
	printf("%s\n",reason);
	exit_gwd();
	exit(1);
}

void exit_gwd()
{
	char *GW_LOCATION;
	char lock[PATH_MAX];
    
    printf("Killing gwd ...");
	system("pkill -9 gwd > /dev/null 2>&1");
	GW_LOCATION=getenv("GW_LOCATION");
    snprintf(lock, PATH_MAX - 1,"%s/" GW_VAR_DIR "/.lock", GW_LOCATION);
    unlink(lock);
	printf("done\n");
}

void display_help()
{
	printf("usage: gwtest [-h] [-c] [id1,id2,id3...]\n");
	printf("        -h              display this help and exit\n");
	printf("        -c              disable gwd checking (use this if gwd is already running)\n");
	printf("	-s server_name  use server_name for checkpointing and staging\n");
	printf("	id1,id2,.. 	test identifiers (if present, only this tests will be perform)\n\n");
	printf("id	Test Name\n");
	printf("---------------\n");
	printf("1	Normal Execution (SINGLE)\n");
    printf("2	Normal Execution (BULK)\n");	
    printf("3	Pre Wrapper\n");
	printf("4	Prolog Fail (Fake Stdin) No Reschedule\n");
	printf("5	Prolog Fail (Fake Stdin) Reschedule\n");
    printf("6	Prolog Fail (Fake Input File) No Reschedule\n");	
    printf("7	Prolog Fail (Fake Executable) No Reschedule\n");
	printf("8	Prolog Fail (Fake Stdin) No Reschedule (BULK)\n");
	printf("9	Execution Fail No Reschedule\n");
    printf("10	Execution Fail Reschedule\n");	
    printf("11	Hold Release\n");
	printf("12	Stop Resume\n");
	printf("13	Kill Sync\n");
	printf("14	Kill Async\n");
	printf("15	Kill Hard\n");
	printf("16	Migrate\n");
	printf("17	Checkpoint local\n");
	printf("18	Checkpoint remote server\n");
	printf("19	Wait Timeout\n");
	printf("20	Wait ZeroTimeout\n");
	printf("21	Input Output files\n");
	printf("22	Epilog Fail (Fake Output) No Reschedule\n");
	printf("23	Epilog Fail (Fake Output) Reschedule\n");
	printf("24	Epilog Fail (Fake Output) No Reschedule (BULK)\n");   
}

void change_jts(char *server_hostname)
{
	char *cmd;
	char *jt_dir;
	char *GW_LOCATION;
	
	GW_LOCATION=getenv("GW_LOCATION");	
	cmd = (char *) malloc (sizeof(char)*1024);
	
	// Change to job template dir
	jt_dir = (char *) malloc (sizeof(char)*(strlen(GW_LOCATION) + sizeof(GW_TEST_DIR) + 6));
    sprintf(jt_dir, "%s/" GW_TEST_DIR "/jt", GW_LOCATION);
    chdir(jt_dir);
    
    // checkpoint_gsiftp.jt
    sprintf(cmd,"sed 's/gsiftp:\\/\\/.*\\//gsiftp:\\/\\/%s\\/var\\/tmp\\//g' checkpoint_gsiftp.jt > tmp ; mv tmp checkpoint_gsiftp.jt", server_hostname);
    system(cmd);
    
    //input_output_files.jt
    sprintf(cmd,"sed '/INPUT/s/gsiftp:\\/\\/.*\\//gsiftp:\\/\\/%s\\/etc\\//g' input_output_files.jt > tmp ; mv tmp input_output_files.jt", server_hostname);
    system(cmd);
    sprintf(cmd,"sed '/OUTPUT/s/gsiftp:\\/\\/.*\\//gsiftp:\\/\\/%s\\/tmp\\//g' input_output_files.jt > tmp ; mv tmp input_output_files.jt", server_hostname);
    system(cmd);    
    

    free(cmd);
    return;
	
}

// Tests
     /** prewrapper != 0 then use prewrapper.jt **/
char * normal_execution(int prewrapper)
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	signed long        timeout = -1;
	int                exit_code;
	gw_msg_job_t      job_status;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	if(prewrapper)
		sprintf(jt,"pre_wrapper.jt");
	else
		sprintf(jt,"normal_execution.jt");
		
	if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	}  
	
	free(jt); 
	
	
	// Wait for the job
	if((rc = gw_client_wait(job_id, &exit_code, timeout)!=GW_RC_SUCCESS))
	{
	     sprintf(reason,"[gw_client_wait] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	
	if(exit_code!=0)
	{
	     sprintf(reason,"Wrong exit code %d",exit_code);
	     return reason; 		
	}
	
	
	if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	else if(strcmp(gw_job_state_string(job_status.job_state),"done")!=0)
		{
	   	  	sprintf(reason,"Wrong job state %s",gw_job_state_string(job_status.job_state));
	     	return reason; 			
		}
	
	sprintf(reason,"OK");
	
	return reason;	
	
}

char * normal_execution_bulk()
{
	char *reason;
	int  *job_ids;
	int  *ot_ids;
	int              deps[GW_JT_DEPS];
	int              rc;
	int              *exit_codes;
	gw_msg_job_t     job_status;
	int              array_id;
	int              i;
	int              num_of_tasks = 5;
	signed long      timeout = -1;
	    
	reason = malloc(sizeof(char)*200);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	
	// Submit jobs
	if((rc = gw_client_array_submit("normal_execution.jt",num_of_tasks,GW_JOB_STATE_PENDING,&array_id,&job_ids,deps,0,1,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_array_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	}   
	

	/*realloc(job_ids,sizeof(int)*num_of_tasks+1);
	job_ids[num_of_tasks] = -1;*/
	ot_ids=malloc(sizeof(int)*num_of_tasks+1);
	for(i=0;i<num_of_tasks;i++)
	    *(ot_ids+i)=*(job_ids+i);
	 ot_ids[num_of_tasks] = -1; 
		
	if((rc = gw_client_wait_set(ot_ids,&exit_codes,GW_FALSE,timeout)!=GW_RC_SUCCESS))
    {
	        sprintf(reason,"[gw_client_wait_set] %s",gw_ret_code_string(rc));
	        fflush(NULL); 
	        return reason; 	
	 }    	
	
	 for(i=0;i<num_of_tasks;i++)
	 {
	 	if(exit_codes[i]!=0)
	 	{
	      sprintf(reason,"Wrong exit code %d in task number %i",exit_codes[i],i);
	      return reason; 		
		}

		if((rc = gw_client_job_status(ot_ids[i], &job_status))!=GW_RC_SUCCESS)
		{
		     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
		     return reason; 		
		}
		else if(strcmp(gw_job_state_string(job_status.job_state),"done")!=0)
			 {
		   	  	sprintf(reason,"Wrong job state %s for job %d",gw_job_state_string(job_status.job_state),i);
		     	return reason; 			
			 }
			 
		printf(" Task %d finished with code %d and state \"done\"\n",ot_ids[i],exit_codes[i]);

	 }
	
	sprintf(reason,"OK");

	return reason;	
	
}

/**
 * which_fail:
 *              0 - prolog failed stdin no reschedule
 * 				1 - prolog failed stdin reschedule
 * 				2 - prolog failed input no reschedule
 * 				3 - prolog failed executable no reschedule
 **/

char * prolog_fail(int which_fail)
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	signed long        timeout = -1;
	int                exit_code;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	switch(which_fail)
	{
		case 0:
			sprintf(jt,"prolog_stdin_nr.jt");	
			break;
		case 1:
			sprintf(jt,"prolog_stdin_r.jt");	
			break;
		case 2:
			sprintf(jt,"prolog_input_nr.jt");	
			break;
		case 3:
			sprintf(jt,"prolog_ex_nr.jt");	
			break;
	}
		
	if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	}  
	
	free(jt); 
	
	rc = gw_client_wait(job_id, &exit_code, timeout);
	
	if(rc==GW_RC_FAILED_JOB_FAIL)
	{
		sprintf(reason,"OK");
	}
	else
	{
	     sprintf(reason,"[gw_client_wait] %s",gw_ret_code_string(rc));		
	}	
	return reason;
}

char * prolog_fail_bulk()
{
	char *reason;
	int  *job_ids;
	int              deps[GW_JT_DEPS];
	int              rc;
	int              *exit_codes;
	gw_msg_job_t     job_status;
	int              array_id;
	int              i;
	int              num_of_tasks = 5;
	signed long      timeout = -1;
	int *ot_ids;
	
	reason = malloc(sizeof(char)*200);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;	
	
	// Submit jobs
	if((rc = gw_client_array_submit("prolog_stdin_nr.jt",num_of_tasks,GW_JOB_STATE_PENDING,&array_id,&job_ids,deps,0,1,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_array_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	}   
	
	ot_ids=malloc(sizeof(int)*num_of_tasks+1);
	for(i=0;i<num_of_tasks;i++)
	    *(ot_ids+i)=*(job_ids+i);
	 ot_ids[num_of_tasks] = -1;   
	
		
	// Wait for the array (all of them)
	if((rc = gw_client_wait_set(ot_ids,&exit_codes,GW_FALSE,timeout))!=GW_RC_FAILED_JOB_FAIL)
	{
	     sprintf(reason,"[gw_client_wait_set] %s",gw_ret_code_string(rc));
	     fflush(NULL);
	     return reason; 	
	}
    	
	
	 for(i=0;i<num_of_tasks;i++)
	 {

		if((rc = gw_client_job_status(ot_ids[i], &job_status))!=GW_RC_SUCCESS)
		{
		     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
		     return reason; 		
		}
	 
	    if(job_status.job_state==GW_JOB_STATE_FAILED)
			printf(" Task %d finished with state failed.\n",ot_ids[i]);
		else
		{
		     sprintf(reason,"Wrong job state for task %d (job state = %s)",ot_ids[i],gw_job_state_string(job_status.job_state));
		     return reason; 		
		}
	 }
	
	sprintf(reason,"OK");
	
	return reason;	
	
}


/**
 * which_fail:
 *              0 - execution failed no reschedule
 * 				1 - execution failed reschedule
 **/

char * execution_fail(int which_fail)
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	signed long        timeout = -1;
	int                exit_code;
	gw_msg_job_t      job_status;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	switch(which_fail)
	{
		case 0:
			sprintf(jt,"failed_execution_no_reschedule.jt");	
			break;
		case 1:
			sprintf(jt,"failed_execution_reschedule.jt");	
			break;
	}
		
	if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	}  
	
	free(jt); 
	
	
	// Wait for the job
	if((rc = gw_client_wait(job_id, &exit_code, timeout))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_wait] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	
	if(exit_code!=1)
	{
	     sprintf(reason,"Wrong exit code %d",exit_code);
	     return reason; 		
	}
	
	if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	else if(strcmp(gw_job_state_string(job_status.job_state),"done")!=0)
		{
	   	  	sprintf(reason,"Wrong job state %s",gw_job_state_string(job_status.job_state));
	     	return reason; 			
		}
	
	sprintf(reason,"OK");
	
	return reason;	
	
}

char * hold_release()
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	signed long        timeout = -1;
	int                exit_code;
	gw_msg_job_t      job_status;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	sprintf(jt,"normal_execution.jt");
	
    // Submit job in hold
	
    if((rc=gw_client_job_submit(jt,GW_JOB_STATE_HOLD,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	}  
	
	// Check that it's being submitted in hold
	
    if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	else if(strcmp(gw_job_state_string(job_status.job_state),"hold")!=0)
		{
	   	  	sprintf(reason,"Wrong job state %s",gw_job_state_string(job_status.job_state));
	     	return reason; 			
		}
		
	// Release it
	rc = gw_client_job_signal (job_id, GW_CLIENT_SIGNAL_RELEASE, GW_TRUE);
	
	// Wait for the job to finish
	if((rc = gw_client_wait(job_id, &exit_code, timeout))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_wait] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	
	if(exit_code!=0)
	{
	     sprintf(reason,"Wrong exit code %d",exit_code);
	     return reason; 		
	}
	
	// Check that it finished in the correct state
	
    if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	else if(strcmp(gw_job_state_string(job_status.job_state),"done")!=0)
		{
	   	  	sprintf(reason,"Wrong job state %s",gw_job_state_string(job_status.job_state));
	     	return reason; 			
		}
		
    sprintf(reason,"OK");
	
	return reason;	
			
}

char * stop_resume()
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	signed long        timeout = -1;
	int                exit_code;
	gw_msg_job_t      job_status;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	sprintf(jt,"normal_execution.jt");
	
    // Submit job 
	printf("    Submitting job\n");
	fflush(NULL);
    if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	} 
	
	// Wait for wrapper state
	do
	{
	    if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
		{
		     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
		     return reason; 		
		}
		sleep(1);
	}while(strcmp(gw_job_state_string(job_status.job_state),"wrap")!=0);

	// Stop the job
	rc = gw_client_job_signal (job_id, GW_CLIENT_SIGNAL_STOP, GW_TRUE);
	
	printf("    Job stopped\n");
	fflush(NULL);	
	
	// Wait for stop state
	do
	{
	    if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
		{
		     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
		     return reason; 		
		}
	}while(strcmp(gw_job_state_string(job_status.job_state),"stop")!=0);

	// Resume the job
	rc = gw_client_job_signal (job_id, GW_CLIENT_SIGNAL_RESUME, GW_TRUE);
	
	printf("    Job resumed\n");
	fflush(NULL);

	// Wait for the job to finish
	if((rc = gw_client_wait(job_id, &exit_code, timeout))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_wait] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	
	if(exit_code!=0)
	{
	     sprintf(reason,"Wrong exit code %d",exit_code);
	     return reason; 		
	}
	
	// Check that it finished in the correct state
	
    if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	else if(strcmp(gw_job_state_string(job_status.job_state),"done")!=0)
		{
	   	  	sprintf(reason,"Wrong job state %s",gw_job_state_string(job_status.job_state));
	     	return reason; 			
		}
		
    sprintf(reason,"OK");
	
	return reason;	
}

char * kill_sync()
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	gw_msg_job_t      job_status;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	sprintf(jt,"normal_execution.jt");
	
    // Submit job for synchronous kill
    if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	} 
	
	// Blocking kill 
	rc = gw_client_job_signal (job_id, GW_CLIENT_SIGNAL_KILL, GW_TRUE);
	
	// Check that it finished in the correct state
	
    if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_FAILED_BAD_JOB_ID)
	{
	   	  	sprintf(reason,"Wrong job state %s, the job shouldn't exist",gw_job_state_string(job_status.job_state));
	     	return reason; 		
	}

		
    sprintf(reason,"OK");
	
	return reason;			
}

char * kill_async()
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	gw_msg_job_t      job_status;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	sprintf(jt,"normal_execution.jt");
	
    // Submit job for asynchronous kill
    if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	} 
	
	
	// Non Blocking kill 
	rc = gw_client_job_signal (job_id, GW_CLIENT_SIGNAL_KILL, GW_FALSE);
	
	
	// Wait for job to disappear from GW
	do
	{
		rc = gw_client_job_status(job_id, &job_status);
		
	}while(rc!=GW_RC_FAILED_BAD_JOB_ID);
		
    sprintf(reason,"OK");
	
	return reason;			
}

char * kill_hard()
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	gw_msg_job_t      job_status;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	sprintf(jt,"normal_execution.jt");
	
    // Submit job for hard kill
    if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	} 
	
	
	// Non Blocking kill 
	rc = gw_client_job_signal (job_id, GW_CLIENT_SIGNAL_KILL_HARD, GW_FALSE);
	
	
	// Wait for job to disappear from GW
	do
	{
		rc = gw_client_job_status(job_id, &job_status);
		
	}while(rc!=GW_RC_FAILED_BAD_JOB_ID);
		
    sprintf(reason,"OK");
	
	return reason;			
}

char * migrate()
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	gw_msg_job_t      job_status;
	int                exit_code;
	signed long        timeout = -1;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	sprintf(jt,"migration.jt");
	
    // Submit job for migration
    if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	} 
	
	// Wait for wrapper state
	do
	{
	    if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
		{
		     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
		     return reason; 		
		}
		sleep(1);
	}while(strcmp(gw_job_state_string(job_status.job_state),"wrap")!=0);
	
	
	// Send migrate signal
	rc = gw_client_job_signal (job_id, GW_CLIENT_SIGNAL_RESCHEDULE, GW_FALSE);
    printf("    Migrating job %d\n",job_id);
	fflush(NULL);
	
	// Wait for the job
	if((rc = gw_client_wait(job_id, &exit_code, timeout)!=GW_RC_SUCCESS))
	{
	     sprintf(reason,"[gw_client_wait] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	
	if(exit_code!=0)
	{
	     sprintf(reason,"Wrong exit code %d",exit_code);
	     return reason; 		
	}
	
	
	if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	else if(strcmp(gw_job_state_string(job_status.job_state),"done")!=0)
		{
	   	  	sprintf(reason,"Wrong job state %s",gw_job_state_string(job_status.job_state));
	     	return reason; 			
		}
	
    sprintf(reason,"OK");
	
	return reason;			
}

char * checkpoint(int which_jt)
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	gw_msg_job_t      job_status;
	struct stat buf;
	int                exit_code;
	signed long        timeout = -1;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	switch(which_jt)
	{
		case 0:
			sprintf(jt,"checkpoint.jt");	
			break;
		case 1:
			sprintf(jt,"checkpoint_gsiftp.jt");	
			break;
	}
	
    // Submit job
    if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	} 
	
	// Wait for the job
	if((rc = gw_client_wait(job_id, &exit_code, timeout)!=GW_RC_SUCCESS))
	{
	     sprintf(reason,"[gw_client_wait] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	
	if(exit_code!=0)
	{
	     sprintf(reason,"Wrong exit code %d",exit_code);
	     return reason; 		
	}
	
	
	if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	else if(strcmp(gw_job_state_string(job_status.job_state),"done")!=0)
		{
	   	  	sprintf(reason,"Wrong job state %s",gw_job_state_string(job_status.job_state));
	     	return reason; 			
		}
		
    if( stat("outfile",&buf) != 0 )
    {
	   	  	sprintf(reason,"Checkpointing file not found");
	     	return reason; 	
    }
    
    unlink("outfile");
	
    sprintf(reason,"OK");
	
	return reason;	
			
}


char * wait_timeout(int _timeout)
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	signed long        timeout=0;
	int                exit_code;
	gw_msg_job_t      job_status;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;

	sprintf(jt,"normal_execution.jt");
		
	if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	}  
	
	free(jt); 

    // by default, zerotimeout	
	if(_timeout==0)
		timeout = 5;
		
	while((rc = gw_client_wait(job_id, &exit_code, timeout)!=GW_RC_SUCCESS))
	{
		printf("	Still waiting for job %d to finish.\n",job_id);
		fflush(NULL);
		sleep(2);
	}
	
	if(exit_code!=0)
	{
	     sprintf(reason,"Wrong exit code %d",exit_code);
	     return reason; 		
	}
	
	
	if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	else if(strcmp(gw_job_state_string(job_status.job_state),"done")!=0)
		{
	   	  	sprintf(reason,"Wrong job state %s",gw_job_state_string(job_status.job_state));
	     	return reason; 			
		}
	
	sprintf(reason,"OK");
	
	return reason;	
	
}

char * input_output()
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	int                exit_code;
	gw_msg_job_t      job_status;
	struct stat buf;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;

	sprintf(jt,"input_output_files.jt");
		
	if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	}  
	
	free(jt); 

		
	while((rc = gw_client_wait(job_id, &exit_code, -1)!=GW_RC_SUCCESS))
	{
		printf("	Still waiting for job %d to finish.\n",job_id);
		fflush(NULL);
		sleep(2);
	}
	
	
	if((rc = gw_client_job_status(job_id, &job_status))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
	     return reason; 		
	}
	else if(strcmp(gw_job_state_string(job_status.job_state),"done")!=0)
		{
	   	  	sprintf(reason,"Wrong job state %s",gw_job_state_string(job_status.job_state));
	     	return reason; 			
		}
		
	// Check files were created correctly
	
	  // Absolut path output
    if( stat("/tmp/test.txt.out",&buf) != 0 )
    {
	   	  	sprintf(reason,"Absolut path output file not found");
	     	return reason; 	
    }
    unlink("/tmp/test.txt.out");
    
	  // Relative path  output
    if( stat("passwd.out",&buf) != 0 )
    {
	   	  	sprintf(reason,"Relative path output file not found");
	     	return reason; 	
    }
    unlink("passwd.out");    
    
	  // gsiftp output
    if( stat("/tmp/passwd.gsi",&buf) != 0 )
    {
	   	  	sprintf(reason,"Gsiftp output file not found");
	     	return reason; 	
    }
    unlink("/tmp/passwd.gsi");	
	
	sprintf(reason,"OK");
	
	return reason;	
	
}

/**
 * which_fail:
 *              0 - epilog failed stdout no reschedule
 * 				1 - epilog failed stdout reschedule
 * 				2 - epilog failed output no reschedule
 **/

char * epilog_fail(int which_fail)
{
	char *reason;
	char *jt;
	int job_id;
	int              deps[GW_JT_DEPS];
	int rc;
	signed long        timeout = -1;
	int                exit_code;
	
	reason = malloc(sizeof(char)*200);
	jt = malloc(sizeof(char)*50);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;
	switch(which_fail)
	{
		case 0:
			sprintf(jt,"epilog_output_nr.jt");	
			break;
		case 1:
			sprintf(jt,"epilog_output_r.jt");	
			break;
	}
		
	if((rc=gw_client_job_submit(jt,GW_JOB_STATE_PENDING,&job_id,deps,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_job_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	}  
	
	free(jt); 
	
	rc = gw_client_wait(job_id, &exit_code, timeout);
	
	unlink("doesnt_exist.txt");
	
	if(rc==GW_RC_FAILED_JOB_FAIL)
	{
		sprintf(reason,"OK");
	}
	else
	{
	     sprintf(reason,"[gw_client_wait] %s",gw_ret_code_string(rc));		
	}	
	return reason;
}

char * epilog_fail_bulk()
{
	char *reason;
	int  *job_ids;
	int  *ot_ids;
	int              deps[GW_JT_DEPS];
	int              rc;
	int              *exit_codes;
	gw_msg_job_t     job_status;
	int              array_id;
	int              i;
	int              num_of_tasks = 5;
	signed long      timeout = -1;
	
	reason = malloc(sizeof(char)*200);
        
	if ( gw_session == NULL )
	{
		sprintf(reason,"Could not connect to gwd");
		return reason;	
	}
	
	deps[0]=-1;	
	
	// Submit jobs
	if((rc = gw_client_array_submit("epilog_output_nr.jt",num_of_tasks,GW_JOB_STATE_PENDING,&array_id,&job_ids,deps,0,1,GW_JOB_DEFAULT_PRIORITY))!=GW_RC_SUCCESS)
	{
	     sprintf(reason,"[gw_client_array_submit] %s",gw_ret_code_string(rc));
	     return reason; 
	}   
	
	/*realloc(job_ids,sizeof(int)*num_of_tasks+1);
	job_ids[num_of_tasks] = -1;*/
	ot_ids=malloc(sizeof(int)*num_of_tasks+1);
	for(i=0;i<num_of_tasks;i++)
	    *(ot_ids+i)=*(job_ids+i);
	 ot_ids[num_of_tasks] = -1; 
	
		
	// Wait for the array (all of them)
	if((rc = gw_client_wait_set(ot_ids,&exit_codes,GW_FALSE,timeout))!=GW_RC_FAILED_JOB_FAIL)
	{
	     sprintf(reason,"[gw_client_wait_set] %s",gw_ret_code_string(rc));
	     fflush(NULL);
	     return reason; 	
	}
    	
	
	 for(i=0;i<num_of_tasks;i++)
	 {

		if((rc = gw_client_job_status(ot_ids[i], &job_status))!=GW_RC_SUCCESS)
		{
		     sprintf(reason,"[gw_client_job_status] %s",gw_ret_code_string(rc));
		     return reason; 		
		}
	 
	    if(job_status.job_state==GW_JOB_STATE_FAILED)
			printf(" Task %d finished with state failed.\n",ot_ids[i]);
		else
		{
		     sprintf(reason,"Wrong job state for task %d (job state = %s)",ot_ids[i],gw_job_state_string(job_status.job_state));
		     return reason; 		
		}
	 }
	 
	unlink("doesnt_exist.txt");
	
	sprintf(reason,"OK");
	
	return reason;	
	
}


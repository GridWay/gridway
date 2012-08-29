/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012, GridWay Project Leads (GridWay.org)                   */
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

#include <time.h>
#include <pwd.h>

#ifndef DRMAA2_H
#define DRMAA2_H

typedef enum drmaa2_jstate {
  DRMAA2_UNDETERMINED                =  0,
  DRMAA2_QUEUED                      =  1,
  DRMAA2_QUEUED_HELD                 =  2,
  DRMAA2_RUNNING                     =  3,
  DRMAA2_SUSPENDED                   =  4,
  DRMAA2_REQUEUED                    =  5,
  DRMAA2_REQUEUED_HELD               =  6,
  DRMAA2_DONE                        =  7,
  DRMAA2_FAILED                      =  8
} drmaa2_jstate;

typedef enum drmaa2_os {
  DRMAA2_OTHER_OS                    =  0,
  DRMAA2_AIX                         =  1,
  DRMAA2_BSD                         =  2,
  DRMAA2_LINUX                       =  3,
  DRMAA2_HPUX                        =  4,
  DRMAA2_IRIX                        =  5,
  DRMAA2_MACOS                       =  6,
  DRMAA2_SUNOS                       =  7,
  DRMAA2_TRUE64                      =  8,
  DRMAA2_UNIXWARE                    =  9,
  DRMAA2_WIN                         = 10,
  DRMAA2_WINNT                       = 11
} drmaa2_os;

typedef enum drmaa2_cpu {
  DRMAA2_OTHER_CPU                   =  0,
  DRMAA2_ALPHA                       =  1,
  DRMAA2_ARM                         =  2,
  DRMAA2_CELL                        =  3,
  DRMAA2_PARISC                      =  4,
  DRMAA2_X86                         =  5,
  DRMAA2_X64                         =  6,
  DRMAA2_IA64                        =  7,
  DRMAA2_MIPS                        =  8,
  DRMAA2_PPC                         =  9,
  DRMAA2_PPC64                       = 10,
  DRMAA2_SPARC                       = 11,
  DRMAA2_SPARC64                     = 12
} drmaa2_cpu;

typedef enum drmaa2_limit {
  DRMAA2_CORE_FILE_SIZE              =  0,
  DRMAA2_CPU_TIME                    =  1,
  DRMAA2_DATA_SEG_SIZE               =  2,
  DRMAA2_FILE_SIZE                   =  3,
  DRMAA2_OPEN_FILES                  =  4,
  DRMAA2_STACK_SIZE                  =  5,
  DRMAA2_VIRTUAL_MEMORY              =  6,
  DRMAA2_WALLCLOCK_TIME              =  7
} drmaa2_limit;

typedef enum drmaa2_jtemplate_placeholder {
  DRMAA2_HOME_DIRECTORY              =  0,
  DRMAA2_WORKING_DIRECTORY           =  1,
  DRMAA2_PARAMETRIC_INDEX            =  2
} drmaa2_jtemplate_placeholder;

typedef enum drmaa2_event {
  DRMAA2_NEW_STATE                   =  0,
  DRMAA2_MIGRATED                    =  1,
  DRMAA2_ATTRIBUTE_CHANGE            =  2
} drmaa2_event;

typedef enum {
  DRMAA2_ADVANCE_RESERVATION         =  0,
  DRMAA2_RESERVE_SLOTS               =  1,
  DRMAA2_CALLBACK                    =  2,
  DRMAA2_BULK_JOBS_MAXPARALLEL       =  3,
  DRMAA2_JT_EMAIL                    =  4,
  DRMAA2_JT_STAGING                  =  5,
  DRMAA2_JT_DEADLINE                 =  6,
  DRMAA2_JT_MAXSLOTS                 =  7,
  DRMAA2_JT_ACCOUNTINGID             =  8,
  DRMAA2_RT_STARTNOW                 =  9,
  DRMAA2_RT_DURATION                 = 10,
  DRMAA2_RT_MACHINEOS                = 11,
  DRMAA2_RT_MACHINEARCH              = 12
} drmaa2_capability;

typedef enum drmaa2_bool {
  DRMAA2_FALSE                       =  0,
  DRMAA2_TRUE                        =  1
} drmaa2_bool;

typedef enum drmaa2_error {
  DRMAA2_SUCCESS                 =  0,
  DRMAA2_DENIED_BY_DRMS          =  1,
  DRMAA2_DRM_COMMUNICATION       =  2,
  DRMAA2_TRY_LATER               =  3,
  DRMAA2_SESSION_MANAGEMENT      =  4,
  DRMAA2_TIMEOUT                 =  5,
  DRMAA2_INTERNAL                =  6,
  DRMAA2_INVALID_ARGUMENT        =  7,
  DRMAA2_INVALID_SESSION         =  8,
  DRMAA2_INVALID_STATE           =  9,
  DRMAA2_OUT_OF_RESOURCE         = 10,
  DRMAA2_UNSUPPORTED_ATTRIBUTE   = 11,
  DRMAA2_UNSUPPORTED_OPERATION   = 12,
  DRMAA2_IMPLEMENTATION_SPECIFIC = 13,
  DRMAA2_LASTERROR               = 14
} drmaa2_error;

drmaa2_error drmaa2_string_free(char*);

drmaa2_error drmaa2_lasterror(void);
char *       drmaa2_lasterror_text(void);

typedef struct node
{
   void *value;
   struct node *prev;
   struct node *next;
} Node;

struct drmaa2_list_s
{
   Node           *head;
   Node           *tail;
   Node           *current;
   size_t         valuesize;
   unsigned long  listsize;
   unsigned long  current_pos;
} drmaa2_list_s;

typedef struct drmaa2_list_s* drmaa2_list;
typedef struct drmaa2_list_s* drmaa2_string_list;
typedef struct drmaa2_list_s* drmaa2_j_list;
typedef struct drmaa2_list_s* drmaa2_queueinfo_list;
typedef struct drmaa2_list_s* drmaa2_machineinfo_list;
typedef struct drmaa2_list_s* drmaa2_slotinfo_list;
typedef struct drmaa2_list_s* drmaa2_r_list;

typedef enum drmaa2_listtype {
  DRMAA2_STRINGLIST,        
  DRMAA2_JOBLIST,
  DRMAA2_QUEUEINFOLIST,
  DRMAA2_MACHINEINFOLIST,
  DRMAA2_SLOTINFOLIST,
  DRMAA2_RESERVATIONLIST
} drmaa2_listtype;

typedef void (*drmaa2_list_entryfree)(void * value);
drmaa2_list  drmaa2_list_create (const drmaa2_listtype t, const drmaa2_list_entryfree callback);
drmaa2_error drmaa2_list_free   ( drmaa2_list l); 
void*        drmaa2_list_get    ( drmaa2_list l, int pos);
drmaa2_error drmaa2_list_add    ( drmaa2_list l, const void * value);
drmaa2_error drmaa2_list_del    ( drmaa2_list l, int pos);
int          drmaa2_list_size   (const drmaa2_list l);


typedef struct gw_dict_elem
{
   char * key;
   char * value;
} gw_dict_elem;

typedef struct dictentry_t
{
   struct gw_dict_elem* elem;
   struct dictentry_t* prev;  
   struct dictentry_t* next;  
} dictentry_t;
 
struct drmaa2_dict_s
{
   dictentry_t    *head;
   dictentry_t    *tail;
   dictentry_t    *current;
   size_t         valuesize;
   unsigned long  dictsize;
   unsigned long  current_pos;
} drmaa2_dict_s;

typedef struct drmaa2_dict_s * drmaa2_dict;

typedef void (*drmaa2_dict_entryfree)(char * value);
drmaa2_dict        drmaa2_dict_create (const drmaa2_dict_entryfree callback);
drmaa2_error       drmaa2_dict_free   (drmaa2_dict d); 
drmaa2_string_list drmaa2_dict_list   (const drmaa2_dict d);            
drmaa2_bool        drmaa2_dict_has    (const drmaa2_dict d, const char * key);
char *             drmaa2_dict_get    (const drmaa2_dict d, const char * key);
drmaa2_error       drmaa2_dict_del    (      drmaa2_dict d, const char * key);
drmaa2_error       drmaa2_dict_set    (      drmaa2_dict d, const char * key, const char * val);

#define  DRMAA2_ZERO_TIME       ((time_t)  0)
#define  DRMAA2_INFINITE_TIME   ((time_t) -1)
#define  DRMAA2_NOW             ((time_t) -2)

#define  DRMAA2_UNSET_BOOL      DRMAA2_FALSE
#define  DRMAA2_UNSET_STRING    NULL   
#define  DRMAA2_UNSET_NUM       -1     
#define  DRMAA2_UNSET_ENUM      -1
#define  DRMAA2_UNSET_LIST      NULL
#define  DRMAA2_UNSET_DICT      NULL
#define  DRMAA2_UNSET_TIME      ((time_t) -3)
#define  DRMAA2_UNSET_CALLBACK  NULL
#define  DRMAA2_UNSET_JINFO     NULL

typedef struct {
  char *             jobId;
  int                exitStatus;        
  char *             terminatingSignal;
  char *             annotation;
  drmaa2_jstate      jobState;
  char *             jobSubState;
  drmaa2_string_list allocatedMachines;
  char *             submissionMachine;
  char *             jobOwner;
  long long          slots;
  char *             queueName;
  time_t             wallclockTime;
  long long          cpuTime;
  time_t             submissionTime;
  time_t             dispatchTime;
  time_t             finishTime;
} drmaa2_jinfo_s;
typedef drmaa2_jinfo_s * drmaa2_jinfo;

drmaa2_jinfo drmaa2_jinfo_create (void);
drmaa2_error drmaa2_jinfo_free   (drmaa2_jinfo ji);

typedef struct {
  char *                machineName; 
  long long             slots;
} drmaa2_slotinfo_s;
typedef drmaa2_slotinfo_s * drmaa2_slotinfo;

drmaa2_error drmaa2_slotinfo_free   (drmaa2_slotinfo si);

typedef struct {
  char *               reservationId;
  char *               reservationName;
  time_t               reservedStartTime;
  time_t               reservedEndTime;
  drmaa2_string_list   usersACL;
  long long            reservedSlots;
  drmaa2_slotinfo_list reservedMachines;
} drmaa2_rinfo_s;
typedef drmaa2_rinfo_s * drmaa2_rinfo;

drmaa2_error drmaa2_rinfo_free   (drmaa2_rinfo ri);

typedef struct {
  char *             remoteCommand;          
  drmaa2_string_list args;          
  drmaa2_bool        submitAsHold;
  drmaa2_bool        rerunnable;
  drmaa2_dict        jobEnvironment;        
  char *             workingDirectory;        
  char *             jobCategory;            
  drmaa2_string_list email;            
  drmaa2_bool        emailOnStarted;
  drmaa2_bool        emailOnTerminated;
  char *             jobName;              
  char *             inputPath;            
  char *             outputPath;            
  char *             errorPath;            
  drmaa2_bool        joinFiles;
  char *             reservationId;          
  char *             queueName;            
  long long          minSlots;              
  long long          maxSlots;              
  long long          priority;              
  drmaa2_string_list candidateMachines;  
  long long          minPhysMemory;            
  drmaa2_os          machineOS;        
  drmaa2_cpu         machineArch;      
  time_t             startTime;          
  time_t             deadlineTime;        
  drmaa2_dict        stageInFiles;        
  drmaa2_dict        stageOutFiles;        
  drmaa2_dict        resourceLimits;      
  char *             accountingId;          
} drmaa2_jtemplate_s;
typedef drmaa2_jtemplate_s * drmaa2_jtemplate;

drmaa2_jtemplate drmaa2_jtemplate_create   (void);
drmaa2_error     drmaa2_jtemplate_free     (drmaa2_jtemplate jt);
char*            drmaa2_jtemplate_tostring (drmaa2_jtemplate jt);

typedef struct {
  char *             reservationName;          
  time_t             startTime;          
  time_t             endTime;          
  time_t             duration;          
  long long          minSlots;              
  long long          maxSlots;
  char *             jobCategory;
  drmaa2_string_list usersACL;            
  drmaa2_string_list candidateMachines;  
  long long          minPhysMemory;            
  drmaa2_os          machineOS;        
  drmaa2_cpu         machineArch;      
} drmaa2_rtemplate_s;
typedef drmaa2_rtemplate_s * drmaa2_rtemplate;

drmaa2_rtemplate     drmaa2_rtemplate_create (void);
drmaa2_error         drmaa2_rtemplate_free   (drmaa2_rtemplate rt);

typedef struct {
  drmaa2_event   event;
  char *         jobId;
  char *         sessionName;
  drmaa2_jstate  jobState;
} drmaa2_notification_s;
typedef drmaa2_notification_s * drmaa2_notification;

drmaa2_error drmaa2_notification_free   (drmaa2_notification n);

typedef struct {
  char *                       name;
} drmaa2_queueinfo_s;
typedef drmaa2_queueinfo_s * drmaa2_queueinfo;

drmaa2_error drmaa2_queueinfo_free   (drmaa2_queueinfo qi);

typedef struct {
  char *                       major; 
  char *                       minor;
} drmaa2_version_s;
typedef drmaa2_version_s * drmaa2_version;

drmaa2_error drmaa2_version_free   (drmaa2_version v);

typedef struct {
  char *          name;  
  drmaa2_bool     available;    
  long long       sockets;      
  long long       coresPerSocket;
  long long       threadsPerCore;  
  float           load;  
  long long       physMemory;
  long long       virtMemory;    
  drmaa2_os       machineOS;  
  drmaa2_version  machineOSVersion;
  drmaa2_cpu      machineArch;
} drmaa2_machineinfo_s;
typedef drmaa2_machineinfo_s * drmaa2_machineinfo;

drmaa2_error drmaa2_machineinfo_free   (drmaa2_machineinfo mi);

drmaa2_string_list drmaa2_jtemplate_impl_spec     (void);
drmaa2_string_list drmaa2_jinfo_impl_spec         (void);
drmaa2_string_list drmaa2_rtemplate_impl_spec     (void);
drmaa2_string_list drmaa2_rinfo_impl_spec         (void);
drmaa2_string_list drmaa2_queueinfo_impl_spec     (void);
drmaa2_string_list drmaa2_machineinfo_impl_spec   (void);
drmaa2_string_list drmaa2_notification_impl_spec  (void);

char *       drmaa2_get_instance_value (const void * instance, const char * name);
char *       drmaa2_describe_attribute (const void * instance, const char * name);
drmaa2_error drmaa2_set_instance_value (      void * instance, const char * name, const char * value);

typedef void (*drmaa2_callback)(drmaa2_notification * notification);


typedef struct drmaa2_jarray_s
{
    char *jarray_id;
    char *session_name;
    drmaa2_jtemplate template;
    drmaa2_j_list jobs;
} drmaa2_jarray_s;
typedef struct drmaa2_jarray_s   * drmaa2_jarray;

typedef struct drmaa2_jsession_s
{
    char *contact;
    char *name;
    drmaa2_j_list jobs;
    drmaa2_list jarray_list;
} drmaa2_jsession_s;
typedef struct drmaa2_jsession_s * drmaa2_jsession;


typedef struct drmaa2_rsession_s
{
    char *contact;
    char *name;
    drmaa2_r_list reservations;
} drmaa2_rsession_s;
typedef struct drmaa2_rsession_s * drmaa2_rsession;


typedef struct drmaa2_msession_s
{
    char *name;
} drmaa2_msession_s;
typedef struct drmaa2_msession_s * drmaa2_msession;


typedef struct drmaa2_j_s
{
    char *jid;
    char *session_name;
    drmaa2_jtemplate template;
    drmaa2_jinfo info;
} drmaa2_j_s;
typedef struct drmaa2_j_s        * drmaa2_j;


typedef struct drmaa2_r_s
{
    char *id;
    char *session_name;
    drmaa2_rtemplate template;
    drmaa2_rinfo info;
} drmaa2_r_s;
typedef struct drmaa2_r_s        * drmaa2_r;

void drmaa2_jsession_free(drmaa2_jsession js);
void drmaa2_rsession_free(drmaa2_rsession rs);
void drmaa2_msession_free(drmaa2_msession ms);
void drmaa2_j_free(drmaa2_j j);
void drmaa2_jarray_free(drmaa2_jarray ja);
void drmaa2_r_free(drmaa2_r r);

char *         drmaa2_rsession_get_contact          (const drmaa2_rsession rs);
char *         drmaa2_rsession_get_session_name     (const drmaa2_rsession rs); 
drmaa2_r       drmaa2_rsession_get_reservation      (const drmaa2_rsession rs, const char * reservation_id);
drmaa2_r       drmaa2_rsession_request_reservation  (const drmaa2_rsession rs, const drmaa2_rtemplate rt);
drmaa2_r_list  drmaa2_rsession_get_reservations     (const drmaa2_rsession rs);

char *            drmaa2_r_get_id                   (const drmaa2_r r);
char *            drmaa2_r_get_session_name         (const drmaa2_r r);
drmaa2_rtemplate  drmaa2_r_get_reservation_template (const drmaa2_r r);
drmaa2_rinfo      drmaa2_r_get_info                 (const drmaa2_r r);
drmaa2_error      drmaa2_r_terminate                (drmaa2_r r);

char *           drmaa2_jarray_get_id            (const drmaa2_jarray ja);
drmaa2_j_list    drmaa2_jarray_get_jobs          (const drmaa2_jarray ja); 
char *           drmaa2_jarray_get_session_name  (const drmaa2_jarray ja);
drmaa2_jtemplate drmaa2_jarray_get_job_template  (const drmaa2_jarray ja); 
drmaa2_error     drmaa2_jarray_suspend           (drmaa2_jarray ja); 
drmaa2_error     drmaa2_jarray_resume            (drmaa2_jarray ja); 
drmaa2_error     drmaa2_jarray_hold              (drmaa2_jarray ja); 
drmaa2_error     drmaa2_jarray_release           (drmaa2_jarray ja); 
drmaa2_error     drmaa2_jarray_terminate         (drmaa2_jarray ja); 

char *              drmaa2_jsession_get_contact         (const drmaa2_jsession js); 
char *              drmaa2_jsession_get_session_name    (const drmaa2_jsession js);
drmaa2_string_list  drmaa2_jsession_get_job_categories  (const drmaa2_jsession js); 
drmaa2_j_list       drmaa2_jsession_get_jobs            (const drmaa2_jsession js, 
                                                         const drmaa2_jinfo filter);
drmaa2_jarray       drmaa2_jsession_get_job_array       (const drmaa2_jsession js, 
                                                         const char * jobarray_id);
drmaa2_j            drmaa2_jsession_run_job             (const drmaa2_jsession js, 
                                                         const drmaa2_jtemplate jt);
drmaa2_jarray       drmaa2_jsession_run_bulk_jobs       (const drmaa2_jsession js, 
                                                         const drmaa2_jtemplate jt, 
                                                         unsigned long begin_index, 
                                                         unsigned long end_index, 
                                                         unsigned long step, 
                                                         unsigned long max_parallel);
drmaa2_j            drmaa2_jsession_wait_any_started    (const drmaa2_jsession js, 
                                                         const drmaa2_j_list l, 
                                                         const time_t timeout);
drmaa2_j            drmaa2_jsession_wait_any_terminated (const drmaa2_jsession js, 
                                                         const drmaa2_j_list l, 
                                                         const time_t timeout);

char *             drmaa2_j_get_id            (const drmaa2_j j);
char *             drmaa2_j_get_session_name  (const drmaa2_j j);
drmaa2_jtemplate   drmaa2_j_get_jt            (const drmaa2_j j);
drmaa2_error       drmaa2_j_suspend           (drmaa2_j j); 
drmaa2_error       drmaa2_j_resume            (drmaa2_j j); 
drmaa2_error       drmaa2_j_hold              (drmaa2_j j); 
drmaa2_error       drmaa2_j_release           (drmaa2_j j); 
drmaa2_error       drmaa2_j_terminate         (drmaa2_j j); 
drmaa2_jstate      drmaa2_j_get_state         (const drmaa2_j j, char ** substate); 
drmaa2_jinfo       drmaa2_j_get_info          (const drmaa2_j j);
drmaa2_j           drmaa2_j_wait_started      (const drmaa2_j j, const time_t timeout); 
drmaa2_j           drmaa2_j_wait_terminated   (const drmaa2_j j, const time_t timeout); 

drmaa2_r_list            drmaa2_msession_get_all_reservations  (const drmaa2_msession ms);
drmaa2_j_list            drmaa2_msession_get_all_jobs          (const drmaa2_msession ms, 
                                                                const drmaa2_jinfo filter);
drmaa2_queueinfo_list    drmaa2_msession_get_all_queues        (const drmaa2_msession ms, 
                                                                const drmaa2_string_list names);
drmaa2_machineinfo_list  drmaa2_msession_get_all_machines      (const drmaa2_msession ms, 
                                                                const drmaa2_string_list names);

char *              drmaa2_get_drms_name                (void);
drmaa2_version      drmaa2_get_drms_version             (void);
char *              drmaa2_get_drmaa_name               (void);
drmaa2_version      drmaa2_get_drmaa_version            (void);
drmaa2_bool         drmaa2_supports                     (const drmaa2_capability c);
drmaa2_jsession     drmaa2_create_jsession              (const char * session_name, const char * contact);
drmaa2_rsession     drmaa2_create_rsession              (const char * session_name, const char * contact);
drmaa2_jsession     drmaa2_open_jsession                (const char * session_name);
drmaa2_rsession     drmaa2_open_rsession                (const char * session_name);
drmaa2_msession     drmaa2_open_msession                (const char * session_name);
drmaa2_error        drmaa2_close_jsession               (drmaa2_jsession js);
drmaa2_error        drmaa2_close_rsession               (drmaa2_rsession rs);
drmaa2_error        drmaa2_close_msession               (drmaa2_msession ms);
drmaa2_error        drmaa2_destroy_jsession             (const char * session_name);
drmaa2_error        drmaa2_destroy_rsession             (const char * session_name);
drmaa2_string_list  drmaa2_get_jsession_names           (void);
drmaa2_string_list  drmaa2_get_rsession_names           (void);
drmaa2_error        drmaa2_register_event_notification  (const drmaa2_callback callback);


//=================================================================
//                 Implementation specifics
//=================================================================

/** Pre-defined buffer size for string variable. 
*/
#define STRING_BUFSIZE   100

/** 
 * String presentation of elements in the drmaa2_jstate enum struct. 
 */
const char *drmaa2_gw_strstatus(int status);

/* ----------------------------------------------------------
        Preprocessor Directives for Handling String 
        Output Arguments
 -----------------------------------------------------------*/ 
/** Pre-defined buffer size for attribute variables which may be used
 *  in DRMAA programs in the creation of char* variables. 
 */
#define DRMAA2_ATTR_BUFFER              1024 

/** Pre-defined buffer size for the job identification string variables. Job 
 *  identification should not be smaller than DRMAA2_JOBNAME_BUFFER. If the size
 *  of the string passed is smaller the resultant job id string will be 
 *  truncated.  
 */
#define DRMAA2_JOBNAME_BUFFER      	1024 

/* -----------------------------------------------------------
        Preprocessor Directives for Control Operations 
------------------------------------------------------------*/ 

/** Pre-defined string to refer to the user's home directory.
 */ 
#define DRMAA2_HOME_DIRECTORY           "$drmaa2_hd_ph$" 

/** Pre-defined string to be used in parametric jobs (bulk jobs). 
 *  DRMAA2_PARAMETRIC_INDEX will be available during job execution and can be
 *  used as an ARGUMENT for the REMOTE COMMAND, or to generate output filenames.
 *  Please note that this attribute name should be used ONLY  in conjuntion 
 *  with a drmaa2_jsession_run_bulk_jobs() function call. Use DRMAA2_GW_JOB_ID 
 *  for "stand-alone" jobs.
 */ 
#define DRMAA2_PARAMETRIC_INDEX         "$drmaa2_incr_ph$"

/** Pre-defined string constant to represent the current working
 *  directory when building paths for the input, output, and error path attribute values.
 *  Plase note that ALL FILES MUST BE NAMED RELATIVE TO THE WORKING DIRECTORY. 
 */
#define DRMAA2_WORKING_DIRECTORY        "$drmaa2_wd_ph$" 

/* -----------------------------------------------------------
        Gridway Specific Preprocessor Directives for 
        Job Template Compilation
------------------------------------------------------------*/ 

/** Pre-defined string to refer to the number of total tasks in a bulk job.
 *  DRMAA2_GW_TOTAL_TASKS will be available during job execution 
 *  and can be used as an ARGUMENT for the REMOTE COMMAND.
 *  This attribute name should be used ONLY in conjuntion with a 
 *  drmaa2_jsession_run_bulk_jobs() function call.
 */
#define DRMAA2_GW_TOTAL_TASKS    "${TOTAL_TASKS}"

/** Pre-defined string to refer to the job unique identification as provided by
 *  the GridWay system. DRMAA2_GW_JOB_ID will be available during job execution 
 *  and can be used as an ARGUMENT for the REMOTE COMMAND. It is also usefull 
 *  to generate output filenames, since it is available in the main DRMAA
 *  program as returned by drmaa2_jsession_run_bulk_jobs() and drmaa2_jsession_run_job() 
 *  function calls.
 */
#define DRMAA2_GW_JOB_ID         "${JOB_ID}"

/** Pre-defined string to refer to the task unique identification as provided by
 *  the GridWay system. DRMAA2_GW_TASK_ID will be available during job execution 
 *  and can be used as an ARGUMENT for the REMOTE COMMAND. It is also usefull 
 *  to generate output filenames of bulk jobs. DRMAA2_GW_TASK_ID ALWAYS ranges
 *  from 0 to DRMAA2_GW_TOTAL_TASKS -1. Please note that this attribute name
 *  should be used ONLY in conjuntion with a drmaa2_jsession_run_bulk_jobs()
 *  function call.
 */
#define DRMAA2_GW_TASK_ID        "${TASK_ID}"

/** Pre-defined string to refer to a custom parameter in bulk jobs. This value
 *  is equal to <begin_index> + <task_id> * <step>, where <begin_index> and <step> are 
 *  drmaa2_jsession_run_bulk_jobs() arguments. DRMAA2_PARAMETRIC_INDEX should be used for 
 *  portability reasons.
 */
#define DRMAA2_GW_PARAM          "${PARAM}"

/** Pre-defined string to refer to the max value of the custom parameter in bulk jobs. 
 *  This value  is equal to <begin_index> + <total_tasks> * <step>.
 */
#define DRMAA2_GW_MAX_PARAM      "${MAX_PARAM}"


#endif

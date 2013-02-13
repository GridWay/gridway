/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013, GridWay Project Leads (GridWay.org)                   */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gw_client.h"
#include "drmaa2.h"



//==================================================================
//                        Module Globals
//==================================================================
drmaa2_error lasterror = DRMAA2_SUCCESS;
//char *lasterror_text = NULL;
drmaa2_string lasterror_text = NULL;

// Additional list type
#define DRMAA2_ADDITIONAL_LIST 6
// Job categories supported
char *job_categories[]={"single", "mpi"};

drmaa2_list j_sessions = DRMAA2_UNSET_LIST;
drmaa2_list m_sessions = DRMAA2_UNSET_LIST;
drmaa2_list r_sessions = DRMAA2_UNSET_LIST;

int gw_num_of_hosts;

static gw_short_string_t drmaa2_gw_template_strs[] = { 
   "EXECUTABLE",        
   "ARGUMENTS",
   "ENVIRONMENT",
   "INPUT_FILES",
   "OUTPUT_FILES",
   "RESTART_FILES",
   "STDERR_FILE",
   "STDIN_FILE",
   "STDOUT_FILE",   
   "RANK",   
   "REQUIREMENTS",
   "RESCHEDULE_ON_FAILURE",
   "NUMBER_OF_RETRIES",
   "JOB_NAME",
   "JOB_WD",
   "JS_STATE",
   "DEADLINE",
   "TYPE",
   "NP",
   "----"
};


static char* gw_job_state_names[21] = {
   "INIT",
   "PENDING",
   "HOLD",
   "PROLOG",
   "PRE_WRAPPER",
   "WRAPPER",
   "EPILOG",
   "EPILOG_STD",
   "EPILOG_RESTART",
   "EPILOG_FAIL",
   "STOP_CANCEL",
   "STOP_EPILOG",
   "STOPPED",
   "KILL_CANCEL",
   "KILL_EPILOG",
   "MIGR_CANCEL",
   "MIGR_PROLOG",
   "MIGR_EPILOG",
   "DONE",
   "FAILED",
   "----"
};


typedef enum {
        D_EXECUTABLE,
        D_ARGUMENTS,
        D_ENVIRONMENT,
        D_INPUT_FILES,
        D_OUTPUT_FILES,
        D_RESTART_FILES,
        D_STDERR_FILE,
        D_STDIN_FILE,
        D_STDOUT_FILE,
        D_RANK,
        D_REQUIREMENTS,
        D_RESCHEDULE_ON_FAILURE,
        D_NUMBER_OF_RETRIES,
        D_JOB_NAME,
        D_JOB_WD,
        D_JS_STATE,
        D_DEADLINE,
        D_TYPE,
        D_NP,
        NONE
} drmaa2_gw_template_names_t;



//==================================================================
//               Auxilary functions
//==================================================================

char *gw_job_get_state_name(gw_job_state_t job_state)
{
    if(job_state >= 0 && job_state < GW_JOB_STATE_LIMIT)
        return gw_job_state_names[job_state];
    else 
        return gw_job_state_names[GW_JOB_STATE_LIMIT];  
}


//----------------------------------------------------------
//----------------------------------------------------------
char* gw_get_rand_str(int len)
{
        char str[36] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        char *s;
        int i;

        s= (char*) malloc(len);
        for(i=0;i<len-1;i++)
        {
           s[i]=str[rand()%36];
        }

        s[len-1]='\0';
        return s;
}


//----------------------------------------------------------
//----------------------------------------------------------
char* gw_get_rand_id(int len)
{
        char str[9] = "123456789";
        char *s;
        int i;

        s=malloc(len);
        for(i=0;i<len-1;i++)
        {
           s[i]=str[rand()%9];
        }

        s[len-1]='\0';
        return s;
}



//----------------------------------------------------------
//----------------------------------------------------------
int gw_drmaa2_total_jobs(void)
{
    int i, total_jobs;
    drmaa2_jsession js = (drmaa2_jsession) malloc(sizeof(drmaa2_jsession_s));
    
    total_jobs=0;
    if (j_sessions != DRMAA2_UNSET_LIST)
    {
        for(i=0;i<drmaa2_list_size(j_sessions);i++)
        {
           js=(drmaa2_jsession) drmaa2_list_get(j_sessions,i);
           if(js!=NULL)
             total_jobs= total_jobs+drmaa2_list_size(js->jobs);
        }
    }

    return total_jobs;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_bool gw_drmaa2_found_job(drmaa2_j job)
{
    int i,j;
    drmaa2_bool bool=DRMAA2_FALSE;
    drmaa2_jsession js = (drmaa2_jsession) malloc(sizeof(drmaa2_jsession_s));
    drmaa2_j jobtmp = (drmaa2_j)malloc(sizeof(drmaa2_j_s));

    if (j_sessions != DRMAA2_UNSET_LIST)
    {
        for(i=0;i<drmaa2_list_size(j_sessions);i++)
        {
              js=(drmaa2_jsession) drmaa2_list_get(j_sessions,i);
              if((js==NULL) || (drmaa2_list_size(js->jobs)<1))
                 continue;
              for(j=0;i<drmaa2_list_size(js->jobs);j++)
              {
                 jobtmp=(drmaa2_j) drmaa2_list_get(js->jobs,j);
                 if(strcmp(job->jid,jobtmp->jid)==0)
                    return DRMAA2_TRUE;
              }
        }
    }

    return bool;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error gw_drmaa2_remove_job(drmaa2_j job)
{
    int i,j;
    drmaa2_error error=DRMAA2_INTERNAL;
    drmaa2_jsession js = (drmaa2_jsession) malloc(sizeof(drmaa2_jsession_s));
    drmaa2_j jobtmp = (drmaa2_j)malloc(sizeof(drmaa2_j_s));

    if (j_sessions != DRMAA2_UNSET_LIST)
    {
        for(i=0;i<drmaa2_list_size(j_sessions);i++)
        {
           js=(drmaa2_jsession) drmaa2_list_get(j_sessions,i);
           if(js==NULL) continue;
           if(strcmp(job->session_name,js->name)==0)
           {
              if(drmaa2_list_size(js->jobs)<1)
                 return DRMAA2_INTERNAL;
              for(j=0;i<drmaa2_list_size(js->jobs);j++)
              {
                 jobtmp=(drmaa2_j) drmaa2_list_get(js->jobs,j);
                 if(strcmp(job->jid,jobtmp->jid)==0)
                 {
                    error=drmaa2_list_del(js->jobs,j);
                    return error;
                 }
              }
           }

        }
    }
    else
       error= DRMAA2_INVALID_SESSION;

    return error;

}

//==================================================================
//               List related functions
//==================================================================

drmaa2_list  drmaa2_list_create (const drmaa2_listtype t, const drmaa2_list_entryfree callback)
{
    drmaa2_list list=NULL;

    if((list = (drmaa2_list) malloc(sizeof(drmaa2_list_s))) == NULL)
    {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memeory allocation failure!";
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->current = NULL;
    list->valuesize = sizeof(char)*STRING_BUFSIZE;
    list->listsize = 0;
    list->current_pos = 0;
  
    switch (t) 
    {
      case DRMAA2_STRINGLIST:
        return (drmaa2_string_list)list;
        break;
      case DRMAA2_JOBLIST:
        return (drmaa2_j_list)list;
        break;
      case DRMAA2_QUEUEINFOLIST:
        return (drmaa2_queueinfo_list)list;
        break;
      case DRMAA2_MACHINEINFOLIST:
        return (drmaa2_machineinfo_list)list;
        break;
      case DRMAA2_SLOTINFOLIST:
        return (drmaa2_slotinfo_list)list;
        break;
      case DRMAA2_RESERVATIONLIST:
        return (drmaa2_r_list)list;
        break;
      default:
        return list;
        break;
    }

    return list;
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error MoveToHead(drmaa2_list list)
{
    if(list->head == NULL)
    {
        lasterror = DRMAA2_INTERNAL;
        lasterror_text = "List head is NULL!";
        return DRMAA2_INTERNAL;
    }

    list->current = list->head;
    list->current_pos = 0;

    return DRMAA2_SUCCESS;
}



//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_list_free(drmaa2_list* list)
{
    void *oldData;
    Node *oldNode;

    if(*list != NULL && (*list)->head != NULL)
    {
      do
      {
        oldData = (*list)->head->value;
        oldNode = (*list)->head;
        (*list)->head = (*list)->head->next;
        free(oldData);
        free(oldNode);
      } while((*list)->head != NULL);

      free(*list);
      *list = DRMAA2_UNSET_LIST;
    }
    else
    {
      if(*list == NULL)
      {
         lasterror = DRMAA2_INTERNAL;
         lasterror_text = "List is NULL!";
      }
      if((*list)->head == NULL)
      {
         lasterror = DRMAA2_INTERNAL;
         lasterror_text = "List head is NULL!";
      }
    }
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_list_add(drmaa2_list list, const void* value)
{
    Node *newNode = NULL, *old;
    void *newData = NULL;

    if((newNode = (Node *) malloc(sizeof(Node))) == NULL)
    {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memory allocation failure!";
        return DRMAA2_OUT_OF_RESOURCE;
    }

    if((newData = (void *) malloc(list->valuesize)) == NULL)
    {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memory allocation failure!";
        free(newNode);
        return DRMAA2_OUT_OF_RESOURCE;
    }

    memcpy(newData, value, list->valuesize);

    if(list->head == NULL)
    {
        (newNode)->value = newData;
        (newNode)->next = NULL;
        (newNode)->prev = NULL;
        list->head = newNode;
        list->tail = newNode;
        list->current = newNode;
        list->listsize = 1;
        list->current_pos = 0;
 
        return DRMAA2_SUCCESS;
    }

     old = list->tail;
     
     list->current_pos = list->listsize+1;
     newNode->value = newData;
     old->next = newNode;
     newNode->next = NULL;
     newNode->prev = old; 
     list->tail = newNode;
     list->current = newNode;
     list->listsize++;

     return DRMAA2_SUCCESS;
}


//----------------------------------------------------------
//----------------------------------------------------------
long drmaa2_list_size(const drmaa2_list list)
{
   return list->listsize;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error MoveToNext(drmaa2_list list)
{
    if(list->current == NULL)
    {
        lasterror = DRMAA2_INTERNAL;
        lasterror_text = "Current element of list is NULL!";
        return DRMAA2_INTERNAL;
    }

    if(list->current->next == NULL)
    {
        lasterror = DRMAA2_INTERNAL;
        lasterror_text = "Next element of list is NULL!";
        return DRMAA2_INTERNAL;
    }

    list->current = list->current->next;
    list->current_pos++;

    return(DRMAA2_SUCCESS);
}


//----------------------------------------------------------
//----------------------------------------------------------
void display_list(drmaa2_list list)
{    
    if(list->current == NULL)
        return;

    if(MoveToHead(list) != DRMAA2_SUCCESS)
    {    
        printf("List is empty!\n");
        return;
    }    

    do   
    {    
        printf("%s\n",(char*)(list->current->value));
    }    
    while(MoveToNext(list) == DRMAA2_SUCCESS);

}    




//----------------------------------------------------------
//----------------------------------------------------------
const void* drmaa2_list_get(const drmaa2_list list, long pos)
{
    Node *node=NULL;

    if(list->current == NULL)
    {
       lasterror = DRMAA2_INTERNAL;
       lasterror_text = "Current element of list is NULL!";
       return NULL;
    }
    if(pos<0 || pos>=list->listsize)
    {
       lasterror = DRMAA2_INTERNAL;
       lasterror_text = "Index of list is out of range!";
       return NULL;
    }

    if(MoveToHead(list) != DRMAA2_SUCCESS)
    {    
       lasterror = DRMAA2_INTERNAL;
       lasterror_text = "Moving to head of list failed!";
       return NULL;
    }    

    do   
    {    
        if(list->current_pos==pos)
        {
           node=list->current;
           return node->value;
        }
    }while(MoveToNext(list) == DRMAA2_SUCCESS);

    return NULL;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_list_del(drmaa2_list list, long pos)
{
    void *oldData;
    Node *oldNode;

    if(pos<0 || pos>=list->listsize)
    {
       lasterror = DRMAA2_INTERNAL;
       lasterror_text = "Index of list is out of range!";
       return DRMAA2_INTERNAL;
    }

    if(MoveToHead(list) != DRMAA2_SUCCESS)
    {    
       lasterror = DRMAA2_INTERNAL;
       lasterror_text = "Moving to head of list failed!";
       return DRMAA2_INTERNAL;
    }    

    do   
    {    
        if(list->current_pos==pos) break;
    }while(MoveToNext(list) == DRMAA2_SUCCESS);

    if(list->current == NULL)
    {
       lasterror = DRMAA2_INTERNAL;
       lasterror_text = "Current element of list is NULL!";
       return DRMAA2_INTERNAL;
    }

    oldData = list->current->value;
    oldNode = list->current;

    if(list->current == list->head)
    {
        if(list->current->next != NULL)
            list->current->next->prev = NULL;

        list->head = list->current->next;
        list->current = list->head;
    }
    else if(list->current == list->tail) 
    {
        list->current->prev->next = NULL;
        list->tail = list->current->prev;
        list->current = list->tail;
        list->current_pos--;  
    }
    else 
    {
        list->current->prev->next = list->current->next;
        list->current->next->prev = list->current->prev;
        list->current = list->current->next;
    }

    free(oldData);
    free(oldNode);
    list->listsize--;

    return DRMAA2_SUCCESS;
}

//==================================================================
//               Dict related functions
//==================================================================

drmaa2_dict drmaa2_dict_create(const drmaa2_dict_entryfree callback)
{
    drmaa2_dict dict=NULL;
    if((dict = (drmaa2_dict) malloc(sizeof(drmaa2_dict_s))) == NULL)
    {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memory allocation failure!";
        return NULL;
    }

    dict->head = NULL;
    dict->tail = NULL;
    dict->current = NULL;
    dict->valuesize = sizeof(drmaa2_dict_s);
    dict->dictsize = 0;
    dict->current_pos = 0;

    return dict;
}


//----------------------------------------------------------
//----------------------------------------------------------
int drmaa2_dict_size(const drmaa2_dict dict)
{
   return dict->dictsize;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error MoveToHead_Dict(drmaa2_dict dict)
{
    if(dict->head == NULL)
    {
        lasterror = DRMAA2_INTERNAL;
        lasterror_text = "Dict head is NULL!";
        return DRMAA2_INTERNAL;
    }

    dict->current = dict->head;
    dict->current_pos = 0;

    return DRMAA2_SUCCESS;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error MoveToNext_Dict(drmaa2_dict dict)
{
    if(dict->current == NULL)
    {
        lasterror = DRMAA2_INTERNAL;
        lasterror_text = "Current element of dict is NULL!";
        return DRMAA2_INTERNAL;
    }

    if(dict->current->next == NULL)
    {
        lasterror = DRMAA2_INTERNAL;
        lasterror_text = "Next element of dict is NULL!";
        return DRMAA2_INTERNAL;
    }

    dict->current = dict->current->next;
    dict->current_pos++;

    return(DRMAA2_SUCCESS);
}


//----------------------------------------------------------
//----------------------------------------------------------
dictentry_t* FindNode_dict(drmaa2_dict dict, void* data)
{
    int cmp;
    dictentry_t* entry=NULL;

    gw_dict_elem* tmp;

    if(dict->current == NULL)
    {
        lasterror = DRMAA2_INTERNAL;
        lasterror_text = "Current element of dict is NULL!";
        return NULL;
    }

    if(MoveToHead_Dict(dict) != DRMAA2_SUCCESS)
    {
        lasterror = DRMAA2_INTERNAL;
        lasterror_text = "Moving to head of dict failed!";
        return NULL;
    }

    do   
    {    
        tmp=(dict->current)->elem;
        cmp=memcmp(data,(void*)(tmp->key),strlen(tmp->key)>=strlen(data)?strlen(tmp->key):strlen(data));
        if(cmp==0) 
        {
          entry=dict->current;
          break;
        }
    }while(MoveToNext_Dict(dict) == DRMAA2_SUCCESS);

    return entry;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_dict_free(drmaa2_dict* dict)
{
    gw_dict_elem* oldData;
    dictentry_t*  oldNode;

    if(*dict != NULL && (*dict)->head != NULL)
    {
      do
      {
        oldData = (*dict)->head->elem;
        oldNode = (*dict)->head;
        (*dict)->head = (*dict)->head->next;
        free(oldData);
        free(oldNode);
      } while((*dict)->head != NULL);

      free(*dict);
      *dict = DRMAA2_UNSET_DICT;
    }
    else
    {
      if(*dict == NULL)
      {
         lasterror = DRMAA2_INTERNAL;
         lasterror_text = "Dict is NULL!";
      }
      if((*dict)->head == NULL)
      {
         lasterror = DRMAA2_INTERNAL;
         lasterror_text = "Dict head is NULL!";
      }
    }
}


//----------------------------------------------------------
//----------------------------------------------------------
void display_dict(drmaa2_dict dict)
{    
    if(dict->current == NULL)
        return;

    if(MoveToHead_Dict(dict) != DRMAA2_SUCCESS)
    {    
        printf("Dict is empty!\n");
        return;
    }    

    do   
    {    
        printf("key: %s  value: %s\n",dict->current->elem->key,dict->current->elem->value);
    }    
    while(MoveToNext_Dict(dict) == DRMAA2_SUCCESS);

}    


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string_list drmaa2_dict_list(const drmaa2_dict dict)
{
  
    drmaa2_list keys=NULL;
    char* key;

    if((keys = (drmaa2_list) malloc(sizeof(drmaa2_list_s))) == NULL)
    {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memory allocation failure!";
        return NULL;
    }

    keys->head = NULL;
    keys->tail = NULL;
    keys->current = NULL;
    keys->valuesize = sizeof(char)*STRING_BUFSIZE;
    keys->listsize = 0;
    keys->current_pos = 0;

    if(dict->current == NULL)
    {
        lasterror = DRMAA2_INTERNAL;
        lasterror_text = "Current element of dict is NULL!";
        return NULL;
    }

    if(MoveToHead_Dict(dict) != DRMAA2_SUCCESS)
    {
        lasterror = DRMAA2_INTERNAL;
        lasterror_text = "Moving to head of dict failed!";
        return NULL;
    }

    do   
    {            
        if((key=(char*) malloc(strlen(dict->current->elem->key)))==NULL)
        {
            lasterror = DRMAA2_OUT_OF_RESOURCE;
            lasterror_text = "Memory allocation failure!";
            return NULL;
        }
        key=strdup(dict->current->elem->key); 
        if(drmaa2_list_add(keys, key) == DRMAA2_OUT_OF_RESOURCE)
        {
           free(key);
           return NULL;
        }
        free(key);
    }while(MoveToNext_Dict(dict) == DRMAA2_SUCCESS);

    return (drmaa2_string_list) keys;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_bool drmaa2_dict_has(const drmaa2_dict dict, const char* key)
{
    int cmp;
    gw_dict_elem* tmp;

    if(dict->current == NULL)
    {
        lasterror = DRMAA2_INTERNAL;
        lasterror_text = "Current element of dict is NULL!";
        return DRMAA2_FALSE;
    }

    if(MoveToHead_Dict(dict) != DRMAA2_SUCCESS)
        return DRMAA2_FALSE;

    do   
    {    
        tmp=dict->current->elem;
        cmp=memcmp(key,(void*)tmp->key,strlen(tmp->key)>=strlen(key)?strlen(tmp->key):strlen(key));
        if(cmp==0) return DRMAA2_TRUE;
    }while(MoveToNext_Dict(dict) == DRMAA2_SUCCESS);

    return DRMAA2_FALSE;
}


//----------------------------------------------------------
//----------------------------------------------------------
char* drmaa2_dict_get(const drmaa2_dict dict, const char* key)
{
  dictentry_t* entry=NULL;


  if((entry=FindNode_dict(dict,(void*)key))!=NULL) 
    return entry->elem->value;
  else
  {
     lasterror = DRMAA2_INTERNAL;
     lasterror_text = "Finding element of dict failed!";
     return NULL;
  }
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_dict_del(drmaa2_dict dict, const char* key)
{
    dictentry_t* entry=NULL;

    dictentry_t* saved=NULL;
    unsigned long saved_index;

    gw_dict_elem *oldData;
    dictentry_t *oldNode;
 
    if((entry=FindNode_dict(dict,(void*)key))==NULL) 
    {
       lasterror = DRMAA2_INTERNAL;
       lasterror_text = "Finding element of dict failed!";
       return DRMAA2_INTERNAL;
    }
    if(dict->current == NULL)
    {
       lasterror = DRMAA2_INTERNAL;
       lasterror_text = "Current element of dict is NULL!";
       return DRMAA2_INTERNAL;
    }
    saved = dict->current;
    saved_index = dict->current_pos;

    dict->current=entry; 

    oldData = dict->current->elem;
    oldNode = dict->current;

    if(dict->current == dict->head) 
    {
        if(dict->current->next != NULL)
            dict->current->next->prev = NULL;

        dict->head = dict->current->next;
        dict->current = dict->head;
    }
    else if(dict->current == dict->tail) 
    {
        dict->current->prev->next = NULL;
        dict->tail = dict->current->prev;
        dict->current = dict->tail;
        dict->current_pos--;
     }
     else 
     {
        dict->current->prev->next = dict->current->next;
        dict->current->next->prev = dict->current->prev;
        dict->current = dict->current->next;
     }

    free(oldData);
    free(oldNode);
    dict->dictsize--;

    dict->current = saved;
    saved = NULL;
    dict->current_pos = saved_index;
    
    return DRMAA2_SUCCESS;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_dict_set(drmaa2_dict dict, const char* key, const char* value)
{
    gw_dict_elem dictentry;
    dictentry_t* newNode = NULL, *old;
    gw_dict_elem *newData = NULL;

    if(drmaa2_dict_has(dict,key)==DRMAA2_TRUE)
    {
       lasterror = DRMAA2_INTERNAL;
       lasterror_text = "Element of dict already existed!";
       return DRMAA2_INTERNAL;
    }

    dictentry.key = strdup(key);
    dictentry.value = strdup(value);

    if((newNode = (dictentry_t *) malloc(sizeof(dictentry_t))) == NULL)
    {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memory allocation failure!";
        return DRMAA2_OUT_OF_RESOURCE;
    }

    if((newData = (void *) malloc(dict->valuesize)) == NULL)
    {
       lasterror = DRMAA2_OUT_OF_RESOURCE;
       lasterror_text = "Memory allocation failure!";
       free(newNode);
       return DRMAA2_OUT_OF_RESOURCE;
    }

    memcpy((void*)newData, (void*)&dictentry, sizeof(gw_dict_elem));

    if(dict->head == NULL)
    {
        (newNode)->elem = newData;
        (newNode)->next = NULL;
        (newNode)->prev = NULL;
        dict->head = newNode;
        dict->tail = newNode;
        dict->current = newNode;
        dict->dictsize = 1;
        dict->current_pos = 0;
 
        return DRMAA2_SUCCESS;
    }

     old = dict->tail;
     
     dict->current_pos = dict->dictsize+1;
     newNode->elem = newData;
     old->next = newNode;
     newNode->next = NULL;
     newNode->prev = old; 
     dict->tail = newNode;
     dict->current = newNode;
     dict->dictsize++;

    return DRMAA2_SUCCESS;
}


//==================================================================
//               Impl_spec functions
//==================================================================

drmaa2_string_list drmaa2_jtemplate_impl_spec(void)
{
    drmaa2_string_list impl_list=NULL;

    impl_list=drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);   
    drmaa2_list_add(impl_list, "remoteCommand");
    drmaa2_list_add(impl_list, "args");
    drmaa2_list_add(impl_list, "submitAsHold");
    drmaa2_list_add(impl_list, "rerunnable");
    drmaa2_list_add(impl_list, "jobEnvironment");
    drmaa2_list_add(impl_list, "workingDirectory");
    drmaa2_list_add(impl_list, "jobName");
    drmaa2_list_add(impl_list, "inputPath");
    drmaa2_list_add(impl_list, "outputPath");
    drmaa2_list_add(impl_list, "errorPath");
    drmaa2_list_add(impl_list, "priority");
    drmaa2_list_add(impl_list, "stageInFiles");
    drmaa2_list_add(impl_list, "stageOutFiles");

    return (drmaa2_string_list) impl_list;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string_list drmaa2_jinfo_impl_spec(void)
{
    drmaa2_string_list impl_list=NULL;

    impl_list=drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    drmaa2_list_add(impl_list, "jobId");
    drmaa2_list_add(impl_list, "exitStatus");
    drmaa2_list_add(impl_list, "jobState");
    drmaa2_list_add(impl_list, "jobSubState");
    drmaa2_list_add(impl_list, "allocatedMachines");
    drmaa2_list_add(impl_list, "submissionMachine");
    drmaa2_list_add(impl_list, "jobOwner");
    drmaa2_list_add(impl_list, "slots");
    drmaa2_list_add(impl_list, "queuename");
    drmaa2_list_add(impl_list, "wallclockTime");
    drmaa2_list_add(impl_list, "cpuTime");
    drmaa2_list_add(impl_list, "submissionTime");
    drmaa2_list_add(impl_list, "dispatchTime");
    drmaa2_list_add(impl_list, "finishTime");

    return (drmaa2_string_list) impl_list;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string_list drmaa2_rtemplate_impl_spec(void)
{
    drmaa2_string_list impl_list=NULL;

    impl_list=drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    drmaa2_list_add(impl_list, "reservationName");
    drmaa2_list_add(impl_list, "startTime");
    drmaa2_list_add(impl_list, "endTime");
    drmaa2_list_add(impl_list, "duration");
    drmaa2_list_add(impl_list, "minSlots");
    drmaa2_list_add(impl_list, "maxSlots");
    drmaa2_list_add(impl_list, "jobCategory");
    drmaa2_list_add(impl_list, "usersACL");
    drmaa2_list_add(impl_list, "candidateMachines");
    drmaa2_list_add(impl_list, "minPhysMemory");
    drmaa2_list_add(impl_list, "machineOS");
    drmaa2_list_add(impl_list, "machineArch");

    return (drmaa2_string_list) impl_list;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string_list drmaa2_rinfo_impl_spec(void)
{
    drmaa2_string_list impl_list=NULL;

    impl_list=drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    drmaa2_list_add(impl_list, "reservationId");
    drmaa2_list_add(impl_list, "reservationName");
    drmaa2_list_add(impl_list, "reservedStartTime");
    drmaa2_list_add(impl_list, "reservedEndTime");
    drmaa2_list_add(impl_list, "usersACL");
    drmaa2_list_add(impl_list, "reservedSlots");
    drmaa2_list_add(impl_list, "reservedMachines");

    return (drmaa2_string_list) impl_list;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string_list drmaa2_queueinfo_impl_spec(void)
{
    drmaa2_string_list impl_list=NULL;

    impl_list=drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    drmaa2_list_add(impl_list, "name");

    return (drmaa2_string_list) impl_list;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string_list drmaa2_machineinfo_impl_spec(void)
{
    drmaa2_string_list impl_list=NULL;

    impl_list=drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    drmaa2_list_add(impl_list, "name");
    drmaa2_list_add(impl_list, "available");
//    drmaa2_list_add(impl_list, "sockets");
//    drmaa2_list_add(impl_list, "coresPerSocket");
//    drmaa2_list_add(impl_list, "threadsPerCore");
//    drmaa2_list_add(impl_list, "load");
    drmaa2_list_add(impl_list, "physMemory");
    drmaa2_list_add(impl_list, "virtMemory");
    drmaa2_list_add(impl_list, "machineOS");
    drmaa2_list_add(impl_list, "machineOSVersion");
    drmaa2_list_add(impl_list, "machineArch");

    return (drmaa2_string_list) impl_list;

}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_notification_free(drmaa2_notification* n)
{
   if(*n==NULL)
   {
       lasterror = DRMAA2_INVALID_ARGUMENT;
       lasterror_text = "Try to free a NULL pointer!";
   }
   else
   {
     if((*n)->jobId != DRMAA2_UNSET_STRING)
        drmaa2_string_free(&((*n)->jobId));
     if((*n)->sessionName != DRMAA2_UNSET_STRING)
        drmaa2_string_free(&((*n)->sessionName));

     free(*n);
     *n = NULL;
   }
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string_list drmaa2_notification_impl_spec(void)
{
    drmaa2_string_list impl_list=NULL;

    impl_list=drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    drmaa2_list_add(impl_list, "event");
    drmaa2_list_add(impl_list, "jobId");
    drmaa2_list_add(impl_list, "sessionName");
    drmaa2_list_add(impl_list, "jobState");

    return (drmaa2_string_list) impl_list;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string drmaa2_get_instance_value(const void* instance, const char* name)
{
   char* value = NULL;


   return value;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string drmaa2_describe_attribute(const void* instance, const char* name)
{
   char* attribute = NULL;

  
   return attribute;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_set_instance_value(void* instance, const char* name, const char* value)
{
   drmaa2_error error;

   error = DRMAA2_SUCCESS;

   return error;
}

//==================================================================
//             Job template manipulate functions                  
//==================================================================

drmaa2_jtemplate drmaa2_jtemplate_create(void)
{
   drmaa2_jtemplate jt=(drmaa2_jtemplate) malloc(sizeof(drmaa2_jtemplate_s));
   if (jt == NULL)
   {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memory allocation failure!";
        return NULL;
   }

   jt->remoteCommand=DRMAA2_UNSET_STRING;
   jt->args=DRMAA2_UNSET_LIST;
   jt->submitAsHold=DRMAA2_UNSET_BOOL;
   jt->rerunnable=DRMAA2_UNSET_BOOL;
   jt->jobEnvironment=DRMAA2_UNSET_DICT;
   jt->workingDirectory=DRMAA2_UNSET_STRING;   
   jt->jobCategory=DRMAA2_UNSET_STRING;   
   jt->email=DRMAA2_UNSET_LIST;
   jt->emailOnStarted=DRMAA2_UNSET_BOOL;
   jt->emailOnTerminated=DRMAA2_UNSET_BOOL;
   jt->jobName=DRMAA2_UNSET_STRING;
   jt->inputPath=DRMAA2_UNSET_STRING;
   jt->outputPath=DRMAA2_UNSET_STRING;
   jt->errorPath=DRMAA2_UNSET_STRING;
   jt->joinFiles=DRMAA2_UNSET_BOOL;
   jt->reservationId=DRMAA2_UNSET_STRING;
   jt->queueName=DRMAA2_UNSET_STRING;
   jt->minSlots=DRMAA2_UNSET_NUM;
   jt->maxSlots=DRMAA2_UNSET_NUM;
   jt->priority=DRMAA2_UNSET_NUM;
   jt->candidateMachines=DRMAA2_UNSET_LIST;
   jt->minPhysMemory=DRMAA2_UNSET_NUM;
   jt->machineOS=DRMAA2_UNSET_ENUM;
   jt->machineArch=DRMAA2_UNSET_ENUM;
   jt->startTime=DRMAA2_UNSET_TIME;
   jt->deadlineTime=DRMAA2_UNSET_TIME;
   jt->stageInFiles=DRMAA2_UNSET_DICT;
   jt->stageOutFiles=DRMAA2_UNSET_DICT;
   jt->resourceLimits=DRMAA2_UNSET_DICT;
   jt->accountingId=DRMAA2_UNSET_STRING;

   return jt;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_jtemplate_free(drmaa2_jtemplate* jt)
{
    if(*jt==NULL)
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
      if((*jt)->remoteCommand != DRMAA2_UNSET_STRING) drmaa2_string_free(&((*jt)->remoteCommand));
      if((*jt)->args != DRMAA2_UNSET_LIST) drmaa2_list_free(&((*jt)->args));
      if((*jt)->jobEnvironment != DRMAA2_UNSET_DICT) drmaa2_dict_free(&((*jt)->jobEnvironment));
      if((*jt)->workingDirectory != DRMAA2_UNSET_STRING) drmaa2_string_free(&((*jt)->workingDirectory));
      if((*jt)->jobCategory != DRMAA2_UNSET_STRING) drmaa2_string_free(&((*jt)->jobCategory));
      if((*jt)->email != DRMAA2_UNSET_LIST) drmaa2_list_free(&((*jt)->email));
      if((*jt)->jobName != DRMAA2_UNSET_STRING) drmaa2_string_free(&((*jt)->jobName));
      if((*jt)->inputPath != DRMAA2_UNSET_STRING) drmaa2_string_free(&((*jt)->inputPath));
      if((*jt)->outputPath != DRMAA2_UNSET_STRING) drmaa2_string_free(&((*jt)->outputPath));
      if((*jt)->errorPath != DRMAA2_UNSET_STRING) drmaa2_string_free(&((*jt)->errorPath));
      if((*jt)->reservationId != DRMAA2_UNSET_STRING) drmaa2_string_free(&((*jt)->reservationId));
      if((*jt)->queueName != DRMAA2_UNSET_STRING) drmaa2_string_free(&((*jt)->queueName));
      if((*jt)->candidateMachines != DRMAA2_UNSET_LIST) drmaa2_list_free(&((*jt)->candidateMachines));
      if((*jt)->stageInFiles != DRMAA2_UNSET_DICT) drmaa2_dict_free(&((*jt)->stageInFiles));
      if((*jt)->stageOutFiles!= DRMAA2_UNSET_DICT) drmaa2_dict_free(&((*jt)->stageOutFiles));
      if((*jt)->resourceLimits != DRMAA2_UNSET_DICT) drmaa2_dict_free(&((*jt)->resourceLimits));
      if((*jt)->accountingId != DRMAA2_UNSET_STRING) drmaa2_string_free(&((*jt)->accountingId));
    
      free(*jt);
      *jt = NULL;
    }
}



//----------------------------------------------------------
//----------------------------------------------------------
char* drmaa2_jtemplate_tostring(drmaa2_jtemplate jt)
{
  int pos;
  char *jt_string, *key, *value, *list_elem;
  drmaa2_string_list keys;
  char tmp[12];

  list_elem = malloc(sizeof(char)*STRING_BUFSIZE);
  jt_string = (char*)malloc(2000);
  if(jt_string==NULL);
  {
      lasterror = DRMAA2_OUT_OF_RESOURCE;
      lasterror_text = "Memory allocation failure!";
      return NULL;
  }

  jt_string=strdup("");
  if(jt->remoteCommand!=NULL)
  {
    jt_string=strncat(jt_string,"jt->remoteCommand:",strlen("jt->remoteCommand:"));
    jt_string=strncat(jt_string,jt->remoteCommand,strlen(jt->remoteCommand));
  }
  if(jt->args!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->args:",strlen("\njt->args:"));
    for(pos=0;pos<drmaa2_list_size(jt->args);pos++)
    {
       list_elem=(char*)drmaa2_list_get(jt->args, pos);
       if(list_elem==NULL) continue;
       jt_string=strncat(jt_string,list_elem,strlen(list_elem));
    }
  }

  jt_string=strncat(jt_string,"\njt->submitAsHold:",strlen("\njt->submitAsHold:"));
  sprintf(tmp,"%d",jt->submitAsHold);
  jt_string=strncat(jt_string,tmp,strlen(tmp));

  jt_string=strncat(jt_string,"\njt->rerunnable:",strlen("\njt->rerunnable:"));
  sprintf(tmp,"%d",jt->rerunnable);
  jt_string=strncat(jt_string,tmp,strlen(tmp));

  if(jt->jobEnvironment!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->jobEnvironment:",strlen("\njt->jobEnvironment:"));
    keys=drmaa2_dict_list(jt->jobEnvironment);
    for(pos=0;pos<drmaa2_dict_size(jt->jobEnvironment);pos++)
    {
       list_elem=(char*)drmaa2_list_get(keys, pos);
       if(list_elem==NULL) break;
       key=list_elem;
       value= drmaa2_dict_get(jt->jobEnvironment,key);
       jt_string=strncat(jt_string,key,strlen(key));
       jt_string=strncat(jt_string,value,strlen(value));
    }
    if(keys != DRMAA2_UNSET_LIST)
       drmaa2_list_free(&keys);
  }

  if(jt->workingDirectory!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->workingDirectory:",strlen("\njt->workingDirectory:"));
    jt_string=strncat(jt_string,jt->workingDirectory,strlen(jt->workingDirectory));
  }

  if(jt->jobCategory!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->jobCategory:",strlen("\njt->jobCategory:"));
    jt_string=strncat(jt_string,jt->jobCategory,strlen(jt->jobCategory));
  }

  if(jt->email!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->email:",strlen("\njt->email:"));
    for(pos=0;pos<drmaa2_list_size(jt->email);pos++)
    {
       list_elem=(char*)drmaa2_list_get(jt->email, pos);
       if(list_elem==NULL) break;
       jt_string=strncat(jt_string,list_elem,strlen(list_elem));
    }
  }

  jt_string=strncat(jt_string,"\njt->emailOnStarted:",strlen("\njt->emailOnStarted:"));
  sprintf(tmp,"%d",jt->emailOnStarted);
  jt_string=strncat(jt_string,tmp,strlen(tmp));

  jt_string=strncat(jt_string,"\njt->emailOnTerminated:",strlen("\njt->emailOnTerminated:"));
  sprintf(tmp,"%d",jt->emailOnTerminated);
  jt_string=strncat(jt_string,tmp,strlen(tmp));

  if(jt->jobName!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->jobName:",strlen("\njt->jobName:"));
    jt_string=strncat(jt_string,jt->jobName,strlen(jt->jobName));
  }

  if(jt->inputPath!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->inputPath:",strlen("\njt->inputPath:"));
    jt_string=strncat(jt_string,jt->inputPath,strlen(jt->inputPath));
  }

  if(jt->outputPath!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->outputPath:",strlen("\njt->outputPath:"));
    jt_string=strncat(jt_string,jt->outputPath,strlen(jt->outputPath));
  }

  if(jt->errorPath!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->errorPath:",strlen("\njt->errorPath:"));
    jt_string=strncat(jt_string,jt->errorPath,strlen(jt->errorPath));
  }

  jt_string=strncat(jt_string,"\njt->joinFiles:",strlen("\njt->joinFiles:"));
  sprintf(tmp,"%d",jt->joinFiles);
  jt_string=strncat(jt_string,tmp,strlen(tmp));

  if(jt->reservationId!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->reservationId:",strlen("\njt->reservationId:"));
    jt_string=strncat(jt_string,jt->reservationId,strlen(jt->reservationId));
  }

  if(jt->queueName!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->queueName:",strlen("\njt->queueName:"));
    jt_string=strncat(jt_string,jt->queueName,strlen(jt->queueName));
  }


  jt_string=strncat(jt_string,"\njt->minSlots:",strlen("\njt->minSlots:"));
  sprintf(tmp,"%lld",jt->minSlots);
  jt_string=strncat(jt_string,tmp,strlen(tmp));

  jt_string=strncat(jt_string,"\njt->maxSlots:",strlen("\njt->maxSlots:"));
  sprintf(tmp,"%lld",jt->maxSlots);
  jt_string=strncat(jt_string,tmp,strlen(tmp));

  jt_string=strncat(jt_string,"\njt->priority:",strlen("\njt->priority:"));
  sprintf(tmp,"%lld",jt->priority);
  jt_string=strncat(jt_string,tmp,strlen(tmp));

  if(jt->candidateMachines!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->candidateMachines:",strlen("\njt->candidateMachines:"));
    for(pos=0;pos<drmaa2_list_size(jt->candidateMachines);pos++)
    {
       list_elem=(char*)drmaa2_list_get(jt->candidateMachines, pos);
       if(list_elem==NULL) break;
       jt_string=strncat(jt_string,list_elem,strlen(list_elem));
    }
  }

  jt_string=strncat(jt_string,"\njt->minPhysMemory:",strlen("\njt->minPhysMemory:"));
  sprintf(tmp,"%lld",jt->minPhysMemory);
  jt_string=strncat(jt_string,tmp,strlen(tmp));

  jt_string=strncat(jt_string,"\njt->machineOS:",strlen("\njt->machineOS:"));
  sprintf(tmp,"%d",jt->machineOS);
  jt_string=strncat(jt_string,tmp,strlen(tmp));

  jt_string=strncat(jt_string,"\njt->machineArch:",strlen("\njt->machineArch:"));
  sprintf(tmp,"%d",jt->machineArch);
  jt_string=strncat(jt_string,tmp,strlen(tmp));

  //We do not provide startTime and deadlineTime

  if(jt->stageInFiles!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->stageInFiles:",strlen("\njt->stageInFiles:"));
    keys=drmaa2_dict_list(jt->stageInFiles);
    for(pos=0;pos<drmaa2_dict_size(jt->stageInFiles);pos++)
    {
       list_elem=(char*)drmaa2_list_get(keys, pos);
       if(list_elem==NULL) break;
       key=list_elem;
       value= drmaa2_dict_get(jt->stageInFiles,key);
       jt_string=strncat(jt_string,key,strlen(key));
       jt_string=strncat(jt_string,value,strlen(value));
    }
    if(keys != DRMAA2_UNSET_LIST)
       drmaa2_list_free(&keys);
  }

  if(jt->stageOutFiles!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->stageOutFiles:",strlen("\njt->stageOutFiles:"));
    keys=drmaa2_dict_list(jt->stageOutFiles);
    for(pos=0;pos<drmaa2_dict_size(jt->stageOutFiles);pos++)
    {
       list_elem=(char*)drmaa2_list_get(keys, pos);
       if(list_elem==NULL) break;
       key=list_elem;
       value= drmaa2_dict_get(jt->stageOutFiles,key);
       jt_string=strncat(jt_string,key,strlen(key));
       jt_string=strncat(jt_string,value,strlen(value));
    }
    if(keys != DRMAA2_UNSET_LIST)
       drmaa2_list_free(&keys);
  }

  if(jt->resourceLimits!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->resourceLimits:",strlen("\njt->resourceLimits:"));
    keys=drmaa2_dict_list(jt->resourceLimits);
    for(pos=0;pos<drmaa2_dict_size(jt->resourceLimits);pos++)
    {
       list_elem=(char*)drmaa2_list_get(keys, pos);
       if(list_elem==NULL) break;
       key=list_elem;
       value= drmaa2_dict_get(jt->resourceLimits,key);
       jt_string=strncat(jt_string,key,strlen(key));
       jt_string=strncat(jt_string,value,strlen(value));
    }
    if(keys != DRMAA2_UNSET_LIST)
       drmaa2_list_free(&keys);
  }

  if(jt->accountingId!=NULL)
  {
    jt_string=strncat(jt_string,"\njt->accountingId:",strlen("\njt->accountingId:"));
    jt_string=strncat(jt_string,jt->accountingId,strlen(jt->accountingId));
  }

  return jt_string;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_string_free(drmaa2_string* string)
{
    if(*string != DRMAA2_UNSET_STRING)
    {
      free(*string);
      *string = DRMAA2_UNSET_STRING;
    }
    else
    {
       lasterror = DRMAA2_INTERNAL;
       lasterror_text = "Trying to free a NULL pointer!";
    }
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_lasterror(void)
{
    return lasterror;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string drmaa2_lasterror_text(void)
{
    return lasterror_text ? strdup(lasterror_text) : NULL;
}



//==================================================================
//             Reservation session related functions                  
//==================================================================

drmaa2_rinfo drmaa2_rinfo_create(void)
{
    drmaa2_rinfo ri = (drmaa2_rinfo) malloc(sizeof(drmaa2_rinfo_s));
    ri->reservationId = DRMAA2_UNSET_STRING;
    ri->reservationName = DRMAA2_UNSET_STRING;
    ri->reservedStartTime = DRMAA2_UNSET_TIME;
    ri->reservedEndTime = DRMAA2_UNSET_TIME;
    ri->usersACL = DRMAA2_UNSET_LIST;
    ri->reservedSlots = DRMAA2_UNSET_NUM;
    ri->reservedMachines = DRMAA2_UNSET_LIST;

    return ri;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_rinfo_free(drmaa2_rinfo* ri)
{
    if(*ri==NULL)
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
      if((*ri)->reservationId != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*ri)->reservationId));
      if((*ri)->reservationName != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*ri)->reservationName));
      if((*ri)->usersACL != DRMAA2_UNSET_LIST)
         drmaa2_list_free(&((*ri)->usersACL));
      if((*ri)->reservedMachines != DRMAA2_UNSET_LIST)
         drmaa2_list_free(&((*ri)->reservedMachines));

      free(*ri);
      *ri = NULL;
    }
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_rtemplate drmaa2_rtemplate_create(void)
{
    drmaa2_rtemplate rt = (drmaa2_rtemplate)malloc(sizeof(drmaa2_rtemplate_s));
    rt->reservationName = DRMAA2_UNSET_STRING;
    rt->startTime = DRMAA2_UNSET_TIME;
    rt->endTime = DRMAA2_UNSET_TIME;
    rt->duration = DRMAA2_UNSET_TIME;
    rt->minSlots = DRMAA2_UNSET_NUM;
    rt->maxSlots = DRMAA2_UNSET_NUM;
    rt->jobCategory = DRMAA2_UNSET_STRING;
    rt->usersACL = DRMAA2_UNSET_LIST;
    rt->candidateMachines = DRMAA2_UNSET_LIST;
    rt->minPhysMemory = DRMAA2_UNSET_NUM;
    rt->machineOS = DRMAA2_OTHER_OS;
    rt->machineArch = DRMAA2_OTHER_CPU;

    return rt;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_rtemplate_free(drmaa2_rtemplate* rt)
{
    if(*rt==NULL)
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
      if((*rt)->reservationName != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*rt)->reservationName));
      if((*rt)->jobCategory != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*rt)->jobCategory));
      if((*rt)->usersACL != DRMAA2_UNSET_LIST)
         drmaa2_list_free(&((*rt)->usersACL));
      if((*rt)->candidateMachines != DRMAA2_UNSET_LIST)
         drmaa2_list_free(&((*rt)->candidateMachines));

      free(*rt);
      *rt = NULL;
    }
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string drmaa2_rsession_get_contact(const drmaa2_rsession rs)
{
   return (rs->contact != NULL) ? strdup(rs->contact) : DRMAA2_UNSET_STRING;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string drmaa2_rsession_get_session_name(const drmaa2_rsession rs)
{
   return (rs->name != NULL) ? strdup(rs->name) : DRMAA2_UNSET_STRING;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_r drmaa2_rsession_get_reservation(const drmaa2_rsession rs, const drmaa2_string reservation_id)
{
   int i;

   drmaa2_r res= (drmaa2_r) malloc(sizeof(drmaa2_r_s));
   if(res==NULL)
   {
      lasterror = DRMAA2_OUT_OF_RESOURCE;
      lasterror_text = "Memory allocation failure!";
      return NULL;
   }
   for(i=0;i<drmaa2_list_size(rs->reservations);i++)
   {
      res=(drmaa2_r) drmaa2_list_get(rs->reservations,i);
      if (strcmp(res->id, reservation_id) == 0)
           return res;
   }
  
   return NULL;
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_r drmaa2_rsession_request_reservation(const drmaa2_rsession rs, const drmaa2_rtemplate rt)
{
    int i;
    drmaa2_r r = (drmaa2_r)malloc(sizeof(drmaa2_r_s));
    r->id= malloc(6);
    r->id= gw_get_rand_id(5);

    r->session_name = (rs->name != NULL) ? strdup(rs->name) : DRMAA2_UNSET_STRING;

    drmaa2_rtemplate template = (drmaa2_rtemplate)malloc(sizeof(drmaa2_rtemplate_s));
    memcpy(template, rt, sizeof(drmaa2_rtemplate_s)); 
    template->reservationName = (rt->reservationName != NULL) ? strdup(rt->reservationName) : DRMAA2_UNSET_STRING;
    template->jobCategory = (rt->jobCategory != NULL) ? strdup(rt->jobCategory) : DRMAA2_UNSET_STRING;

    template->usersACL = (drmaa2_string_list) drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    for(i=0;i<drmaa2_list_size(rt->usersACL);i++)
    {
       drmaa2_list_add(template->usersACL,(void*) drmaa2_list_get(rt->usersACL,i));
    }

    template->candidateMachines = (drmaa2_string_list) drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    for(i=0;i<drmaa2_list_size(rt->candidateMachines);i++)
    {
       drmaa2_list_add(template->candidateMachines,(void*) drmaa2_list_get(rt->candidateMachines,i));
    }

    r->template = template;

    drmaa2_rinfo info = drmaa2_rinfo_create();
    info->reservationId = (r->id != NULL) ? strdup(r->id) : DRMAA2_UNSET_STRING;
    info->reservationName = (r->session_name != NULL) ? strdup(r->session_name) : DRMAA2_UNSET_STRING;
    info->reservedStartTime = rt->startTime;
    info->reservedEndTime = rt->endTime;
    info->usersACL = rt->usersACL;
    info->reservedSlots = rt->maxSlots;
    info->reservedMachines = rt->candidateMachines;

    r->info = info;
    drmaa2_list_add(rs->reservations, r);

    return r;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_r_list drmaa2_rsession_get_reservations(const drmaa2_rsession rs)
{
    int i;
    drmaa2_r_list rlist;

    rlist = (drmaa2_r_list) drmaa2_list_create(DRMAA2_RESERVATIONLIST, DRMAA2_UNSET_CALLBACK);
    for(i=0;i<drmaa2_list_size(rs->reservations);i++)
    {
       drmaa2_list_add(rlist,(void*) drmaa2_list_get(rs->reservations,i));
    }

   return rlist;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string drmaa2_r_get_id(const drmaa2_r r)
{
    return (r->id != NULL) ? strdup(r->id) : DRMAA2_UNSET_STRING;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string drmaa2_r_get_session_name(const drmaa2_r r)
{
    return (r->session_name != NULL) ? strdup(r->session_name) : DRMAA2_UNSET_STRING;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_rtemplate drmaa2_r_get_reservation_template(const drmaa2_r r)
{
   drmaa2_rtemplate template;
 
   template=(drmaa2_rtemplate) malloc(sizeof(drmaa2_rtemplate_s));
   if(template==NULL)
   {
      lasterror = DRMAA2_OUT_OF_RESOURCE;
      lasterror_text = "Memory allocation failure!";
      return NULL;
   }
   memcpy(template, r->template, sizeof(drmaa2_rtemplate_s));

   return template;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_rinfo drmaa2_r_get_info(const drmaa2_r r)
{
   drmaa2_rinfo info;

   if(r==NULL)
   {
      lasterror = DRMAA2_INTERNAL;
      lasterror_text = "Try to access a NULL reservation interface!";
      return NULL;
   }
   info=(drmaa2_rinfo) malloc(sizeof(drmaa2_rinfo_s));
   if(info==NULL)
   {
      lasterror = DRMAA2_OUT_OF_RESOURCE;
      lasterror_text = "Memory allocation failure!";
      return NULL;
   }
   memcpy(info, r->info, sizeof(drmaa2_rinfo_s));

   return info;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_r_terminate(drmaa2_r r)
{
    int i,j;
    drmaa2_error error;

    drmaa2_rsession rs= (drmaa2_rsession) malloc(sizeof(drmaa2_rsession_s));
    if(rs==NULL)
    {
      lasterror = DRMAA2_OUT_OF_RESOURCE;
      lasterror_text = "Memory allocation failure!";
      return DRMAA2_OUT_OF_RESOURCE;
    }

    drmaa2_r res= (drmaa2_r) malloc(sizeof(drmaa2_r_s));
    if(res==NULL)
    {
      lasterror = DRMAA2_OUT_OF_RESOURCE;
      lasterror_text = "Memory allocation failure!";
      return DRMAA2_OUT_OF_RESOURCE;
    }

    if (r_sessions != DRMAA2_UNSET_LIST)
    {
        for(i=0;i<drmaa2_list_size(r_sessions);i++)
        {
           rs=(drmaa2_rsession)drmaa2_list_get(r_sessions,i);
           for(j=0;j<drmaa2_list_size(rs->reservations);j++)
           {
              res=(drmaa2_r) drmaa2_list_get(rs->reservations,j);
              if (strcmp(r->session_name, res->session_name) == 0)
              {
                 error=drmaa2_list_del(rs->reservations, j);
                 return error;
              }
           }
        }
    }
    
    return DRMAA2_UNSUPPORTED_ATTRIBUTE;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_r_free(drmaa2_r* r)
{
    if(*r==NULL)
    {
       lasterror = DRMAA2_INVALID_ARGUMENT;
       lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
      if((*r)->id != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*r)->id));
      if((*r)->session_name != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*r)->session_name));
      if((*r)->info != NULL)
         drmaa2_rinfo_free(&((*r)->info));

      free(*r);
      *r = NULL;
    }
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_close_rsession(drmaa2_rsession rs)
{
    return DRMAA2_SUCCESS;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_destroy_rsession(const char * session_name)
{
    int i;
    drmaa2_rsession rs;
    drmaa2_error error;

    rs = (drmaa2_rsession) malloc(sizeof(drmaa2_rsession_s));
    if (r_sessions != DRMAA2_UNSET_LIST)
    {
        for(i=0;i<drmaa2_list_size(r_sessions);i++)
        {
           rs=(drmaa2_rsession)drmaa2_list_get(r_sessions,i);
           if (strcmp(rs->name, session_name) == 0)
           {
              error=drmaa2_list_del(r_sessions, i);
              if(rs->reservations != DRMAA2_UNSET_LIST)
                 drmaa2_list_free(&(rs->reservations));
              return error;
           }
        }
    }
    else
    {
        lasterror = DRMAA2_INVALID_SESSION;
        lasterror_text = "Reservation session does not exist!";
        return lasterror;
    }


    if(drmaa2_list_size(r_sessions)==0 && r_sessions != DRMAA2_UNSET_LIST)
    {
       drmaa2_list_free(&r_sessions);
    }


    return DRMAA2_SUCCESS;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string_list drmaa2_get_rsession_names(void)
{
    int i;
    drmaa2_rsession rs = (drmaa2_rsession) malloc(sizeof(drmaa2_rsession_s));
    drmaa2_string_list session_names = drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);

    if (r_sessions != DRMAA2_UNSET_LIST)
    {
        for(i=0;i<drmaa2_list_size(r_sessions);i++)
        {
           rs=(drmaa2_rsession)drmaa2_list_get(r_sessions,i);
           drmaa2_list_add(session_names, strdup(rs->name));
        }
    }

    return session_names;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_rsession_free(drmaa2_rsession* rs)
{
    if(*rs==NULL)
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
      if((*rs)->contact != DRMAA2_UNSET_STRING)
        drmaa2_string_free(&((*rs)->contact)); 
      if((*rs)->name != DRMAA2_UNSET_STRING)
        drmaa2_string_free(&((*rs)->name)); 

      free(*rs);
      *rs = NULL;
    }
}


//==================================================================
//             DRMAA2 general functions                  
//==================================================================

drmaa2_string drmaa2_get_drms_name(void)
{
  drmaa2_string drms_name;
  drms_name=strdup("GridWay");
  
  return drms_name;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string drmaa2_get_drmaa_name(void)
{
  drmaa2_string drmaa_name;
  drmaa_name=strdup("DRMAA-GridWay");
  
  return drmaa_name;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_version drmaa2_get_drms_version(void)
{
  char temp[10], *p;
  drmaa2_version drms_version = malloc(sizeof(drmaa2_version_s));

  char* buf = strstr(PACKAGE_STRING, " ");
  if(buf != NULL)
  {
    buf++;
    sprintf(temp, "%s", buf);
    p = strtok(temp, ".");

    drms_version->major = strdup(p);
    p = strtok(NULL, ".");
    drms_version->minor = strdup(p);  
  }
  else
  {
    drms_version->major = strdup("5");
    drms_version->minor = strdup("10");
  }

  return drms_version;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_version drmaa2_get_drmaa_version(void)
{
  drmaa2_version drmaa_version;

  drmaa_version=malloc(sizeof(drmaa2_version_s));
  drmaa_version->major=strdup("2");
  drmaa_version->minor=strdup("0");

  return drmaa_version;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_version_free(drmaa2_version* v)
{
   if(*v != NULL)
   {
       if((*v)->major != DRMAA2_UNSET_STRING)
          drmaa2_string_free(&((*v)->major));
       if((*v)->minor != DRMAA2_UNSET_STRING)
          drmaa2_string_free(&((*v)->minor));

       free(*v);
       *v = NULL;
  }
  else
  {
       lasterror = DRMAA2_INTERNAL;
       lasterror_text = "Try to free a NULL pointer!";
  }
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_bool drmaa2_supports(const drmaa2_capability c)
{
    drmaa2_bool bool;
    switch(c)
    {
       case DRMAA2_ADVANCE_RESERVATION:
          bool=DRMAA2_FALSE;
          break;
       case DRMAA2_RESERVE_SLOTS:
          bool=DRMAA2_FALSE;
          break;
       case DRMAA2_CALLBACK:
          bool=DRMAA2_FALSE;
          break;
       case DRMAA2_BULK_JOBS_MAXPARALLEL:
          bool=DRMAA2_TRUE;
          break;
       case DRMAA2_JT_EMAIL:
          bool=DRMAA2_FALSE;
          break;
       case DRMAA2_JT_STAGING:
          bool=DRMAA2_TRUE;
          break;
       case DRMAA2_JT_DEADLINE:
          bool=DRMAA2_TRUE;
          break;
       case DRMAA2_JT_MAXSLOTS:
          bool=DRMAA2_TRUE;
          break;
       case DRMAA2_JT_ACCOUNTINGID:
          bool=DRMAA2_FALSE;
          break;
       case DRMAA2_RT_STARTNOW:
          bool=DRMAA2_FALSE;
          break;
       case DRMAA2_RT_DURATION:
          bool=DRMAA2_FALSE;
          break;
       case DRMAA2_RT_MACHINEOS:
          bool=DRMAA2_FALSE;
          break;
       case DRMAA2_RT_MACHINEARCH:
          bool=DRMAA2_FALSE;
          break;
       default:
          bool=DRMAA2_FALSE;
          break;
    }

    return bool;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_queueinfo_free(drmaa2_queueinfo* qi)
{
    if(*qi==NULL)
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
      if((*qi)->name != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*qi)->name));

      free(*qi);
      *qi = NULL;
    }
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_machineinfo_free(drmaa2_machineinfo* mi)
{
    if(*mi==NULL)
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
       if((*mi)->name != DRMAA2_UNSET_STRING)
          drmaa2_string_free(&((*mi)->name));
       if((*mi)->machineOSVersion != NULL)
          drmaa2_version_free(&((*mi)->machineOSVersion));

       free(*mi);
       *mi = NULL;
    }
}


//==================================================================
//             GridWay template related functions                  
//==================================================================

char * drmaa2_gw_jt_file  (const drmaa2_jtemplate jt)
{
    char * jt_file;
    char * job_name;
    char * job_wd;
    int    file_len;
   
    if ( jt == NULL )
    {
      lasterror = DRMAA2_IMPLEMENTATION_SPECIFIC;
      lasterror_text = "Try to access a NULL job template!";
      return NULL;
    }
    		
    job_name = (char *) (jt->jobName);
    job_wd   = (char *) (jt->workingDirectory);
	
    if ( (job_name == NULL) || (job_wd == NULL) )
    {
      lasterror = DRMAA2_IMPLEMENTATION_SPECIFIC;
      lasterror_text = "Job name or job working directory is NULL!";
      return NULL;
    }
		    
    file_len = strlen(job_name) + strlen(job_wd) + 2;
    jt_file  = (char *) malloc(file_len*sizeof(char));
    
    if (jt_file == NULL)
    {
      lasterror = DRMAA2_IMPLEMENTATION_SPECIFIC;
      lasterror_text = "Path to job template file could not be determined!";
      return NULL;
    }
            	
    sprintf(jt_file,"%s/%s",job_wd,job_name);

    return jt_file;
}


//----------------------------------------------------------
//----------------------------------------------------------
char * drmaa2_gw_jt_parse(const char * value)
{
	char *          token_start;
	char *          token_end;
	char *          value_cp;
	char *          tmp;
	int             len;
	struct passwd * pw_ent;
	
	if ( value == NULL )
        {
           lasterror = DRMAA2_IMPLEMENTATION_SPECIFIC;
           lasterror_text = "Try to access a NULL pointer in drmaa2_gw_jt_parse()!";
           return NULL;
        }
		
	value_cp = strdup(value);

	token_start = strstr(value_cp,DRMAA2_PARAMETRIC_INDEX);
	while ( token_start != NULL )
	{
		*token_start ='\0';
		token_end    = token_start + strlen(DRMAA2_PARAMETRIC_INDEX);

		len = strlen(value_cp)+strlen(token_end)+strlen(DRMAA2_GW_PARAM)+1;
		tmp = (char *) malloc(sizeof(char)*len);

		sprintf(tmp,"%s%s%s",value_cp,DRMAA2_GW_PARAM,token_end);

		free (value_cp);
		value_cp = tmp;
		
		token_start = strstr(value_cp,DRMAA2_PARAMETRIC_INDEX);
	}

	token_start = strstr(value_cp,DRMAA2_WORKING_DIRECTORY);
	while ( token_start != NULL )
	{

		*token_start ='\0';
		token_end    = token_start + strlen(DRMAA2_WORKING_DIRECTORY);
		
		len = strlen(value_cp) + strlen(token_end) + 1;
		tmp = (char *) malloc(sizeof(char)*len);
		
		sprintf(tmp,"%s%s",value_cp,token_end);

		free (value_cp);
		value_cp = tmp;
		
		token_start = strstr(value_cp,DRMAA2_WORKING_DIRECTORY);
	}

	token_start = strstr(value_cp,DRMAA2_HOME_DIRECTORY);
	
	while ( token_start != NULL )
	{
		*token_start ='\0';
		token_end    = token_start + strlen(DRMAA2_HOME_DIRECTORY);
		pw_ent       = getpwuid(getuid());
		
		len = strlen(value_cp)+strlen(token_end)+strlen(pw_ent->pw_dir)+1;
		tmp = (char *) malloc(sizeof(char)*len);
		
		sprintf(tmp,"%s%s%s",value_cp,pw_ent->pw_dir,token_end);
		
		free (value_cp);
		value_cp = tmp;
		
		token_start = strstr(value_cp,DRMAA2_HOME_DIRECTORY);
	}
	
	return value_cp;
}


//----------------------------------------------------------
//----------------------------------------------------------
void gw_drmaa2_jt_w_var (FILE *fd, int var, const char *val)
{
	char * val_cp;
	
	if ( val != NULL )
	{
		val_cp  = drmaa2_gw_jt_parse(val);
		if ( val_cp != NULL )
		{
			fprintf(fd,"%s=%s\n",drmaa2_gw_template_strs[var],val_cp);
			free(val_cp);
		}
	}
}	



//----------------------------------------------------------
//----------------------------------------------------------
void gw_drmaa2_jt_w_vvar(FILE *        fd, 
                        int           var, 
                        char **       val,
                        int           num,
                        const char *  ifs)
{
	int    i;
	char * val_cp;
	
	if ((val != NULL) && (num > 0))
	{
		fprintf(fd,"%s= ",drmaa2_gw_template_strs[var]);
		
		for (i=0;i<num-1;i++)
		{
			val_cp = drmaa2_gw_jt_parse(val[i]);
			fprintf(fd,"%s%s",val_cp,ifs);
			free(val_cp);
		}
		
		val_cp = drmaa2_gw_jt_parse(val[num-1]);
		fprintf(fd,"%s\n",val_cp);
		free(val_cp);
	}
}	


//----------------------------------------------------------
//----------------------------------------------------------
static void gw_drmaa2_gw_str2drmaa_str(char ** str)
{
	char *filename;
	char *hostname;
	char *cp;
	int length;
	
	if ( str == NULL || *str == NULL)
        {
           lasterror = DRMAA2_IMPLEMENTATION_SPECIFIC;
           lasterror_text = "Try to access a NULL pointer in gw_drmaa2_gw_str2drmaa_str()!";
           return;
        }
	
	hostname = strdup(*str);
		
	filename = strchr(hostname, ':');
	
	if ( filename == NULL )
	{
           lasterror = DRMAA2_IMPLEMENTATION_SPECIFIC;
           lasterror_text = "Try to access a NULL pointer in gw_drmaa2_gw_str2drmaa_str()!";
	   free(hostname);
	   return;
	}

	
	*filename='\0';
	filename++;
	
	if (( strcmp(hostname,"gsiftp") == 0 )||
	    ( strcmp(hostname,"file") == 0 )  ||
	    ( strcmp(hostname,"http") == 0 )  ||	   
	    ( strcmp(hostname,"https") == 0 ) ) /* a gw filename */
	{
		free(hostname);
		return;
	}
	else if (strlen(hostname) == 0) /* no hostname */
	{
		cp = *str;				
		*str = strdup(cp+1);
		free(cp);
		free(hostname);
		return;
	}
	else /* hostname use gsiftp:// */
	{
		length = strlen(hostname) + strlen (filename) + 11;
		
		free(*str);
		*str = (char *) malloc (sizeof(char)*length);
		
		snprintf(*str,length,"gsiftp://%s/%s",hostname,filename);
		free(hostname);
		
		return;
	}
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error gw_drmaa2_jt_write (drmaa2_jtemplate jt)
{
    char *jt_file;    
    FILE *fp;
    char *jt_parse;

    int i;    
    char *key, *value;
    char deadline[10];
    drmaa2_list tmplist;

    if ( jt == NULL )
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to access a NULL job template!";
        return DRMAA2_INVALID_ARGUMENT;
    }
        
    jt_file = drmaa2_gw_jt_file(jt);

    if (jt_file == NULL)
    {
        lasterror = DRMAA2_IMPLEMENTATION_SPECIFIC;
        lasterror_text = "Try to access a non-existent job template file!";
        return DRMAA2_IMPLEMENTATION_SPECIFIC;    	
    }
   
    jt_parse = drmaa2_gw_jt_parse(jt_file);

    if (jt_parse == NULL)
    {
        lasterror = DRMAA2_IMPLEMENTATION_SPECIFIC;
        lasterror_text = "Parsing job template file name failed!";
    	free(jt_file);
        return DRMAA2_IMPLEMENTATION_SPECIFIC;    	
    }
    
    fp = fopen(jt_parse,"w");

    if ( fp == NULL )
    {
        lasterror = DRMAA2_IMPLEMENTATION_SPECIFIC;
        lasterror_text = "Failed to open the job template file to write!";
        free(jt_file);
        free(jt_parse);
        return DRMAA2_IMPLEMENTATION_SPECIFIC;    	
    }
    
    fprintf(fp,"#This file was automatically generated by the GridWay DRMAA Version 2 library\n");

// jobName
    if((jt->jobName) != DRMAA2_UNSET_STRING)
       fprintf(fp,"NAME=%s\n",jt->jobName);

// remoteCommand
    if((jt->remoteCommand) != DRMAA2_UNSET_STRING)
       fprintf(fp,"EXECUTABLE=%s \n",jt->remoteCommand);
    else
    {
       printf("Warning! jt->remoteCommand NOT SET, EXITING NOW!\n");
       exit(1);
    }

// args
    if((jt->args) != DRMAA2_UNSET_LIST)
    {
         fprintf(fp,"ARGUMENTS= ");
         for(i=0;i<drmaa2_list_size(jt->args);i++)
         {
            fprintf(fp,"%s ",(char*)drmaa2_list_get(jt->args,i));
         }
         fprintf(fp,"\n");
    }


// jobEnvironment
    if((jt->jobEnvironment) != DRMAA2_UNSET_DICT)
    {
         tmplist=drmaa2_dict_list(jt->jobEnvironment);
         fprintf(fp,"\nENVIRONMENT=");
         for(i=0;i<drmaa2_dict_size(jt->jobEnvironment);i++)
         {
            key=(char*)drmaa2_list_get(tmplist,i);
            value=drmaa2_dict_get(jt->jobEnvironment,key);
            if(i<drmaa2_dict_size(jt->jobEnvironment)-1)
              fprintf(fp,"%s=\"%s\",",key,value);
            else
              fprintf(fp,"%s=\"%s\"\n",key,value);
         }
         if(tmplist != DRMAA2_UNSET_LIST)
            drmaa2_list_free(&tmplist);
    }

// stageInFiles
    if((jt->stageInFiles) != DRMAA2_UNSET_DICT)
    {
         tmplist=drmaa2_dict_list(jt->stageInFiles);
         fprintf(fp,"INPUT_FILES=");
         for(i=0;i<drmaa2_dict_size(jt->stageInFiles);i++)
         {
            key=(char*)drmaa2_list_get(tmplist,i);
            value=drmaa2_dict_get(jt->stageInFiles,key);
            if(i<drmaa2_dict_size(jt->stageInFiles)-1)
              fprintf(fp,"%s %s,",key,value);
            else
              fprintf(fp,"%s %s\n",key,value);
         }
         if(tmplist != DRMAA2_UNSET_LIST)
            drmaa2_list_free(&tmplist);
    }

// stageOutFiles
    if((jt->stageOutFiles) != DRMAA2_UNSET_DICT)
    {
         tmplist=drmaa2_dict_list(jt->stageOutFiles);
         fprintf(fp,"OUTPUT_FILES=");
         for(i=0;i<drmaa2_dict_size(jt->stageOutFiles);i++)
         {
            key=(char*)drmaa2_list_get(tmplist,i);
            value=drmaa2_dict_get(jt->stageOutFiles,key);
            if(i<drmaa2_dict_size(jt->stageOutFiles)-1)
              fprintf(fp,"%s %s,",key,value);
            else
              fprintf(fp,"%s %s\n",key,value);
         }
         if(tmplist != DRMAA2_UNSET_LIST)
            drmaa2_list_free(&tmplist);
    }

// inputPath
    if((jt->inputPath) != DRMAA2_UNSET_STRING)
    {
       gw_drmaa2_gw_str2drmaa_str(&(jt->inputPath));	
       gw_drmaa2_jt_w_var (fp,D_STDIN_FILE , jt->inputPath);
    }
// outputPath
    if((jt->outputPath) != DRMAA2_UNSET_STRING)
    {
       gw_drmaa2_gw_str2drmaa_str(&(jt->outputPath));
       gw_drmaa2_jt_w_var (fp,D_STDOUT_FILE, jt->outputPath);
    }

// errorPath	
    if((jt->errorPath) != DRMAA2_UNSET_STRING)
    {
       gw_drmaa2_gw_str2drmaa_str(&(jt->errorPath));	
       gw_drmaa2_jt_w_var (fp,D_STDERR_FILE, jt->errorPath);
    }

// rerunnable
    if(jt->rerunnable==DRMAA2_TRUE)
       gw_drmaa2_jt_w_var (fp,D_RESCHEDULE_ON_FAILURE, (char*) strdup("yes"));
    else
       gw_drmaa2_jt_w_var (fp,D_RESCHEDULE_ON_FAILURE, (char*) strdup("no"));

// deadlineTime
    if(jt->deadlineTime != DRMAA2_UNSET_TIME)
    {
       sprintf(deadline,"%lld",(long long int)jt->deadlineTime);
       gw_drmaa2_jt_w_var (fp,D_DEADLINE, deadline);
    }
    else
       gw_drmaa2_jt_w_var (fp,D_DEADLINE, (char*) strdup("00"));

       
    fclose(fp);
    
    free(jt_parse);
    free(jt_file);
    
    return DRMAA2_SUCCESS;			
} 


//==================================================================
//             Job Session related functions                  
//==================================================================

drmaa2_string drmaa2_jsession_get_contact(const drmaa2_jsession js)
{
    if (js->contact) return strdup(js->contact);
    return DRMAA2_UNSET_STRING;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string drmaa2_jsession_get_session_name(const drmaa2_jsession js)
{
    if(js->name) return strdup(js->name);
    return DRMAA2_UNSET_STRING;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string_list drmaa2_jsession_get_job_categories(const drmaa2_jsession js)
{
    int i;

    i=0;
    drmaa2_string_list jc = drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    do
    {
        drmaa2_list_add(jc, strdup(job_categories[i]));
        i++;
    } while(job_categories[i]!=NULL);

    return jc;
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_bool gw_drmaa2_jinfo_compare(drmaa2_jinfo jinfo, drmaa2_jinfo filter)
{
   drmaa2_bool found = DRMAA2_TRUE;

   if(filter->jobId != DRMAA2_UNSET_STRING)
   {
      if(strcmp(jinfo->jobId, filter->jobId) != 0)
          found = DRMAA2_FALSE;
   }
   if(filter->exitStatus != DRMAA2_UNSET_NUM)
   {
      if(jinfo->exitStatus != filter->exitStatus)
          found = DRMAA2_FALSE;
   }
   if(filter->jobState != DRMAA2_UNDETERMINED)
   {
      if(jinfo->jobState != filter->jobState)
          found = DRMAA2_FALSE;
   }
   if(filter->jobSubState != DRMAA2_UNSET_STRING)
   {
      if(strcmp(jinfo->jobSubState, filter->jobSubState) != 0)
          found = DRMAA2_FALSE;
   }
   if(filter->submissionMachine != DRMAA2_UNSET_STRING)
   {
      if(strcmp(jinfo->submissionMachine, filter->submissionMachine) != 0)
          found = DRMAA2_FALSE;
   }
   if(filter->jobOwner != DRMAA2_UNSET_STRING)
   {
      if(strcmp(jinfo->jobOwner, filter->jobOwner) != 0)
          found = DRMAA2_FALSE;
   }
   if(filter->wallclockTime != DRMAA2_UNSET_TIME)
   {
      if(jinfo->wallclockTime < filter->wallclockTime)
          found = DRMAA2_FALSE;
   }
   if(filter->cpuTime != DRMAA2_UNSET_NUM)
   {
      if(jinfo->cpuTime < filter->cpuTime)
          found = DRMAA2_FALSE;
   }
   if(filter->submissionTime != DRMAA2_UNSET_TIME)
   {
      if(jinfo->submissionTime < filter->submissionTime)
          found = DRMAA2_FALSE;
   }
   if(filter->dispatchTime != DRMAA2_UNSET_TIME)
   {
      if(jinfo->dispatchTime < filter->dispatchTime)
          found = DRMAA2_FALSE;
   }
   if(filter->finishTime != DRMAA2_UNSET_TIME)
   {
      if(jinfo->finishTime < filter->finishTime)
          found = DRMAA2_FALSE;
   }

   return found;
} 

//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_j_list drmaa2_jsession_get_jobs (const drmaa2_jsession js, const drmaa2_jinfo filter)
{
    int i;
    drmaa2_error error;
    drmaa2_bool found;
    drmaa2_j_list jobs = drmaa2_list_create(DRMAA2_JOBLIST, DRMAA2_UNSET_CALLBACK);
    drmaa2_j job = (drmaa2_j) malloc(sizeof(drmaa2_j_s));
   
    if(drmaa2_list_size(js->jobs)<1) 
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to access a zero length list!";
        return NULL;
    }

    
    for(i=0;i<drmaa2_list_size(js->jobs);i++)
    {
       job = (drmaa2_j) drmaa2_list_get(js->jobs,i);
       if(filter != NULL)
       {
         found = gw_drmaa2_jinfo_compare(job->info, filter);
         if(found == DRMAA2_TRUE)
         {
           error=drmaa2_list_add(jobs,(void*) job);
           if(error!=DRMAA2_SUCCESS)
           {
              lasterror = DRMAA2_INTERNAL;
              lasterror_text = "Adding new element to list failed!";
              return NULL;
           }
         }
       }
       else
       {
           error=drmaa2_list_add(jobs,(void*) job);
           if(error!=DRMAA2_SUCCESS)
           {
              lasterror = DRMAA2_INTERNAL;
              lasterror_text = "Adding new element to list failed!";
              return NULL;
           }
       }
    }

    return jobs;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_jarray drmaa2_jsession_get_job_array(const drmaa2_jsession js, const char* jobarray_id)
{
   int i;
   drmaa2_jarray jarray=(drmaa2_jarray) malloc(sizeof(drmaa2_jarray_s));

   for(i=0;i<drmaa2_list_size(js->jarray_list);i++)
   {
      jarray = (drmaa2_jarray) drmaa2_list_get(js->jarray_list, i);
      if(strcmp(jarray->jarray_id, jobarray_id)==0)
      {
         return jarray;
      }
   }
   
   return NULL;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_jsession drmaa2_open_jsession(const char * session_name)
{
    int i;    
    drmaa2_jsession js = (drmaa2_jsession) malloc(sizeof(drmaa2_jsession_s));
    if(js==NULL)
    {
      lasterror = DRMAA2_OUT_OF_RESOURCE;
      lasterror_text = "Memory allocation failure!";
      return NULL;
    }

    if (j_sessions != DRMAA2_UNSET_LIST && session_name != DRMAA2_UNSET_STRING)
    {
        for(i=0;i<drmaa2_list_size(j_sessions);i++)
        {
           js=(drmaa2_jsession)drmaa2_list_get(j_sessions,i); 
           if (strcmp(js->name, session_name) == 0)
               return js;
        }    
    }

    lasterror = DRMAA2_INVALID_ARGUMENT;
    lasterror_text = "No session with the given name.";
    return NULL;
}
   


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_close_jsession(drmaa2_jsession js)
{
    return DRMAA2_SUCCESS;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string_list drmaa2_get_jsession_names(void)
{
    int i;
    drmaa2_jsession js = (drmaa2_jsession)malloc(sizeof(drmaa2_jsession_s));
    drmaa2_string_list session_names = drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);

    if (j_sessions != DRMAA2_UNSET_LIST)
    {
        for(i=0;i<drmaa2_list_size(j_sessions);i++)       
        {
           js=(drmaa2_jsession)drmaa2_list_get(j_sessions,i);
           drmaa2_list_add(session_names, strdup(js->name));
        }
    }

    return session_names;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_jsession_free(drmaa2_jsession* js)
{
   if(*js==NULL)
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Try to free a NULL pointer!";
   }
   else
   {
      if((*js)->contact != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*js)->contact)); 
      if((*js)->name != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*js)->name)); 
      if((*js)->jobs != DRMAA2_UNSET_LIST)
         drmaa2_list_free(&((*js)->jobs));
      if((*js)->jarray_list != DRMAA2_UNSET_LIST)
         drmaa2_list_free(&((*js)->jarray_list));

      free(*js);
      *js = NULL;
   }
}



//==================================================================
//             Job info related functions                  
//==================================================================

drmaa2_jinfo drmaa2_jinfo_create(void)
{
    drmaa2_jinfo ji = (drmaa2_jinfo)malloc(sizeof(drmaa2_jinfo_s));
    ji->jobId = DRMAA2_UNSET_STRING;
    ji->exitStatus = DRMAA2_UNSET_NUM;
    ji->terminatingSignal = DRMAA2_UNSET_STRING;
    ji->annotation = DRMAA2_UNSET_STRING;
    ji->jobState = DRMAA2_UNDETERMINED;
    ji->jobSubState = DRMAA2_UNSET_STRING;
    ji->allocatedMachines = drmaa2_list_create(DRMAA2_SLOTINFOLIST,DRMAA2_UNSET_CALLBACK);
    ji->submissionMachine = DRMAA2_UNSET_STRING;
    ji->jobOwner = DRMAA2_UNSET_STRING;
    ji->slots = DRMAA2_UNSET_NUM;
    ji->queueName = DRMAA2_UNSET_STRING;
    ji->wallclockTime = DRMAA2_UNSET_TIME;
    ji->cpuTime = DRMAA2_UNSET_NUM;
    ji->submissionTime = DRMAA2_UNSET_TIME;
    ji->dispatchTime = DRMAA2_UNSET_TIME;
    ji->finishTime = DRMAA2_UNSET_TIME;
    return ji;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_jinfo_free(drmaa2_jinfo* ji)
{
    if(*ji==NULL)
    {
       lasterror = DRMAA2_INVALID_ARGUMENT;
       lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
      if((*ji)->jobId != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*ji)->jobId));
      if((*ji)->terminatingSignal != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*ji)->terminatingSignal));
      if((*ji)->annotation != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*ji)->annotation));
      if((*ji)->jobSubState != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*ji)->jobSubState));
      if((*ji)->allocatedMachines != DRMAA2_UNSET_LIST)
         drmaa2_list_free(&((*ji)->allocatedMachines));
      if((*ji)->submissionMachine != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*ji)->submissionMachine));
      if((*ji)->jobOwner != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*ji)->jobOwner));
      if((*ji)->queueName != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*ji)->queueName));

      free(*ji);
      *ji = NULL;
   }
}



//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_slotinfo_free(drmaa2_slotinfo* si)
{
    if(*si==NULL)
    {
       lasterror = DRMAA2_INVALID_ARGUMENT;
       lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
      if((*si)->machineName != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*si)->machineName));

      free(*si);
      *si = NULL;
   }
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_jsession drmaa2_create_jsession(const char * session_name, const char * contact)
{
    int i;
    time_t t;
    char *name, *rand_str;
    gw_client_t * gw_client;

    drmaa2_jsession jsession= malloc(sizeof(drmaa2_jsession_s));

    if (j_sessions == DRMAA2_UNSET_LIST)
    {
        j_sessions = drmaa2_list_create(DRMAA2_ADDITIONAL_LIST, DRMAA2_UNSET_CALLBACK);
        if (j_sessions == NULL)
        {
           lasterror = DRMAA2_OUT_OF_RESOURCE;
           lasterror_text = "Job session list could not be created!";
           return NULL;
        }
 
        srand((unsigned) time(&t));
        gw_client = gw_client_init();
        if(gw_client == NULL)
        {
           lasterror = DRMAA2_DRM_COMMUNICATION;
           lasterror_text = "Initiating GridWay client failed!";
           return NULL;
        }
        else 
        {
           gw_num_of_hosts = gw_client->number_of_hosts;
           if(gw_num_of_hosts<=0) gw_num_of_hosts=1;
        }
    }


    if (session_name == DRMAA2_UNSET_STRING)
    {
        name = malloc(15);
        name=strdup("Jsession_");
        rand_str=gw_get_rand_str(5);
        name=strcat(name,rand_str);
    }
    else
    {
       for(i=0;i<drmaa2_list_size(j_sessions);i++)
       {
          jsession=(drmaa2_jsession)drmaa2_list_get(j_sessions,i);
          if(strcmp(session_name,jsession->name)==0)
          {
             lasterror = DRMAA2_INTERNAL;
             lasterror_text = "Try to create job session with an already existent session name!";
             return NULL;
          }
       }
    }

    drmaa2_jsession js = (drmaa2_jsession)malloc(sizeof(drmaa2_jsession_s));
    if(session_name != DRMAA2_UNSET_STRING)
       js->name = strdup(session_name);
    else
       js->name= strdup(name);
    if (contact) 
       js->contact = strdup(contact);
    else
       js->contact = DRMAA2_UNSET_STRING;

    js->jobs = drmaa2_list_create(DRMAA2_JOBLIST, DRMAA2_UNSET_CALLBACK);
    js->jarray_list = drmaa2_list_create(DRMAA2_JOBLIST, DRMAA2_UNSET_CALLBACK);    

    drmaa2_list_add(j_sessions, js);

    return js;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_destroy_jsession(const char * session_name)
{
    int i;
    drmaa2_jsession js = (drmaa2_jsession) malloc(sizeof(drmaa2_jsession_s));
    drmaa2_error error;

    if (j_sessions != DRMAA2_UNSET_LIST)
    {
        for(i=0;i<drmaa2_list_size(j_sessions);i++)
        {
           js=(drmaa2_jsession)drmaa2_list_get(j_sessions,i);
           if (strcmp(js->name, session_name) == 0)
           {
              error=drmaa2_list_del(j_sessions, i);
              return error;
           }
        }  
    }
    else
    {
        lasterror = DRMAA2_INVALID_SESSION;
        lasterror_text = "Job session does not exist!";
        return lasterror;
    }
 

    if( j_sessions != DRMAA2_UNSET_LIST && drmaa2_list_size(j_sessions)==0)
    {
       drmaa2_list_free(&j_sessions);
       gw_client_finalize();
    }

    return DRMAA2_SUCCESS;
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_rsession drmaa2_create_rsession(const char * session_name, const char * contact)
{
    int i;
    char *name, *rand_str;
    time_t t;

    drmaa2_rsession rsession= malloc(sizeof(drmaa2_rsession_s));

    if (r_sessions == DRMAA2_UNSET_LIST)
    {
        srand((unsigned) time(&t));
        r_sessions = drmaa2_list_create(DRMAA2_ADDITIONAL_LIST, DRMAA2_UNSET_CALLBACK);
        if (r_sessions == NULL)
        {
           lasterror = DRMAA2_OUT_OF_RESOURCE;
           lasterror_text = "Reservation session list could not be created!";
           return NULL;
        }
    }

    if (session_name == DRMAA2_UNSET_STRING)
    {   
        name=strdup("Rsession_");
        rand_str = (char*) malloc(6);
        rand_str = gw_get_rand_str(5);
        name=strcat(name,rand_str);
    }   
    else
    {
       for(i=0;i<drmaa2_list_size(r_sessions);i++)
       {
          rsession=(drmaa2_rsession)drmaa2_list_get(r_sessions,i);
          if(strcmp(session_name,rsession->name)==0)
          {
             lasterror = DRMAA2_INTERNAL;
             lasterror_text = "Try to create reservation session with an already existent session name!";
             return NULL;
          }
       }
    }

    drmaa2_rsession rs = (drmaa2_rsession)malloc(sizeof(drmaa2_rsession_s));
    if(session_name != DRMAA2_UNSET_STRING)
       rs->name = strdup(session_name);
    else
       rs->name= strdup(name);
    if (contact) rs->contact = strdup(contact);

    rs->reservations = drmaa2_list_create(DRMAA2_RESERVATIONLIST, DRMAA2_UNSET_CALLBACK);
    drmaa2_list_add(r_sessions, rs);

    return rs;
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_rsession drmaa2_open_rsession(const char * session_name)
{
    int i;
    drmaa2_rsession rs = (drmaa2_rsession) malloc(sizeof(drmaa2_rsession_s));
    if(rs==NULL)
    {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memeory allocation failure!";
        return NULL;
    }

    if (r_sessions != DRMAA2_UNSET_LIST && session_name != DRMAA2_UNSET_STRING)
    {    
        for(i=0;i<drmaa2_list_size(r_sessions);i++)
        {    
           rs=(drmaa2_rsession)drmaa2_list_get(r_sessions,i); 
           if (strcmp(rs->name, session_name) == 0)
               return rs;
        }    
    }    

    lasterror = DRMAA2_INVALID_ARGUMENT;
    lasterror_text = "No reservation session with the given name!";
    return NULL;
}




//==================================================================
//             Job manipulation related functions                  
//==================================================================

drmaa2_string drmaa2_j_get_id(const drmaa2_j j)
{
    return (j->jid != NULL) ? strdup(j->jid) : DRMAA2_UNSET_STRING;
}
   
 
//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string drmaa2_j_get_session_name(const drmaa2_j j)
{
    return (j->session_name != NULL) ? strdup(j->session_name) : DRMAA2_UNSET_STRING;
}

    
//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_jtemplate drmaa2_j_get_jt(const drmaa2_j j)
{
   drmaa2_jtemplate template = (drmaa2_jtemplate)malloc(sizeof(drmaa2_jtemplate_s));
   if(template==NULL)
   {
      lasterror = DRMAA2_OUT_OF_RESOURCE;
      lasterror_text = "Memory allocation failure!";
      return NULL;
   }
   memcpy(template, j->template, sizeof(drmaa2_jtemplate_s));

   return template;
}

   
//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_j_suspend(drmaa2_j j)
{
   int all_rc;
   int jid;
   drmaa2_error error;

   if(j->session_name==NULL) 
   {
      lasterror = DRMAA2_INVALID_SESSION;
      lasterror_text = "Job session name does not exist!";
      return DRMAA2_INVALID_SESSION;
   }

   if(j->jid==NULL) 
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Job id is NULL!";
      return DRMAA2_INVALID_ARGUMENT;
   }

   jid=atoi(j->jid);
   all_rc=gw_client_job_signal(jid,GW_CLIENT_SIGNAL_STOP,GW_TRUE);
  
   if ( all_rc == GW_RC_FAILED_BAD_JOB_STATE )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_BAD_JOB_ID )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_CONNECTION )
           error = DRMAA2_DRM_COMMUNICATION;
   else if ( all_rc == GW_RC_SUCCESS )
   {    
           error = DRMAA2_SUCCESS;
           j->info->jobState=DRMAA2_SUSPENDED;
   }
   else
           error = DRMAA2_INVALID_ARGUMENT;

   return error;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_j_resume(drmaa2_j j)
{
   int all_rc;
   int jid;
   drmaa2_error error;

   if(j->session_name==NULL) 
   {
      lasterror = DRMAA2_INVALID_SESSION;
      lasterror_text = "Job session name does not exist!";
      return DRMAA2_INVALID_SESSION;
   }

   if(j->jid==NULL) 
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Job id is NULL!";
      return DRMAA2_INVALID_ARGUMENT;
   }

   jid=atoi(j->jid);
   all_rc=gw_client_job_signal(jid,GW_CLIENT_SIGNAL_RESUME,GW_TRUE);
  
   if ( all_rc == GW_RC_FAILED_BAD_JOB_STATE )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_BAD_JOB_ID )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_CONNECTION )
           error = DRMAA2_DRM_COMMUNICATION;
   else if ( all_rc == GW_RC_SUCCESS )
   {
           error = DRMAA2_SUCCESS;
           j->info->jobState=DRMAA2_RUNNING;
   }
   else
           error = DRMAA2_INVALID_ARGUMENT;

   return error;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_j_hold(drmaa2_j j)
{
   int all_rc;
   int jid;
   drmaa2_error error;

   if(j->session_name==NULL) 
   {
      lasterror = DRMAA2_INVALID_SESSION;
      lasterror_text = "Job session name does not exist!";
      return DRMAA2_INVALID_SESSION;
   }

   if(j->jid==NULL) 
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Job id is NULL!";
      return DRMAA2_INVALID_ARGUMENT;
   }

   jid=atoi(j->jid);
   all_rc=gw_client_job_signal(jid,GW_CLIENT_SIGNAL_HOLD,GW_TRUE);
  
   if ( all_rc == GW_RC_FAILED_BAD_JOB_STATE )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_BAD_JOB_ID )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_CONNECTION )
           error = DRMAA2_DRM_COMMUNICATION;
   else if ( all_rc == GW_RC_SUCCESS )
   {
           error = DRMAA2_SUCCESS;
           j->info->jobState=DRMAA2_QUEUED_HELD;
   }
   else
           error = DRMAA2_INVALID_ARGUMENT;

   return error;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_j_release(drmaa2_j j)
{
   int all_rc;
   int jid;
   drmaa2_error error;

   if(j->session_name==NULL) 
   {
      lasterror = DRMAA2_INVALID_SESSION;
      lasterror_text = "Job session name does not exist!";
      return DRMAA2_INVALID_SESSION;
   }

   if(j->jid==NULL) 
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Job id is NULL!";
      return DRMAA2_INVALID_ARGUMENT;
   }

   jid=atoi(j->jid);
   all_rc=gw_client_job_signal(jid,GW_CLIENT_SIGNAL_RELEASE,GW_TRUE);
  
   if ( all_rc == GW_RC_FAILED_BAD_JOB_STATE )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_BAD_JOB_ID )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_CONNECTION )
           error = DRMAA2_DRM_COMMUNICATION;
   else if ( all_rc == GW_RC_SUCCESS )
   {
           error = DRMAA2_SUCCESS;
           j->info->jobState=DRMAA2_QUEUED;
   }
   else
           error = DRMAA2_INVALID_ARGUMENT;

   return error;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_j_terminate(drmaa2_j j)
{
   int all_rc;
   int jid;
   drmaa2_error error;

   if(j->session_name==NULL) 
   {
      lasterror = DRMAA2_INVALID_SESSION;
      lasterror_text = "Job session name does not exist!";
      return DRMAA2_INVALID_SESSION;
   }

   if(j->jid==NULL) 
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Job id is NULL!";
      return DRMAA2_INVALID_ARGUMENT;
   }

   jid=atoi(j->jid);
   all_rc=gw_client_job_signal(jid,GW_CLIENT_SIGNAL_KILL,GW_TRUE);
  
   if ( all_rc == GW_RC_FAILED_BAD_JOB_STATE )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_BAD_JOB_ID )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_CONNECTION )
           error = DRMAA2_DRM_COMMUNICATION;
   else if ( all_rc == GW_RC_SUCCESS )
   {
           error = DRMAA2_SUCCESS;
           j->info->jobState=DRMAA2_FAILED;
   }
   else
           error = DRMAA2_INVALID_ARGUMENT;

   return error;
}


//----------------------------------------------------------
//----------------------------------------------------------
const drmaa2_string drmaa2_gw_strstatus(int status)
{
    switch (status)
    {                                
        case DRMAA2_UNDETERMINED:
            return("DRMAA2_UNDETERMINED");
            break;
         
        case DRMAA2_QUEUED:
            return("DRMAA2_QUEUED");
            break;
                                             
        case DRMAA2_QUEUED_HELD:
            return("DRMAA2_QUEUED_HELD");
            break;
     
        case DRMAA2_RUNNING:
            return("DRMAA2_RUNNING");
            break;
     
        case DRMAA2_SUSPENDED:
            return("DRMAA2_SUSPENDED");
            break;
                                     
        case DRMAA2_REQUEUED:
            return("DRMAA2_REQUEUED");
            break;

        case DRMAA2_REQUEUED_HELD:
            return("DRMAA2_REQUEUED_HELD");
            break;

        case DRMAA2_DONE:
             return("DRMAA2_DONE");
             break;

        case DRMAA2_FAILED:
             return("DRMAA2_FAILED");
             break;

        default:
            return("DRMAA2_UNDETERMINED");
            break;
    }

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_jstate drmaa2_j_get_state(drmaa2_j j, drmaa2_string * substate)
{
   int jid;
   gw_return_code_t grc;
   gw_msg_job_t  status;
   drmaa2_jstate jstate;


   if(j->session_name==NULL) 
   {
      lasterror = DRMAA2_INVALID_SESSION;
      lasterror_text = "Job session name does not exist!";
      return DRMAA2_UNDETERMINED;
   }

   if(j->jid==NULL) 
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Job id is NULL!";
      return DRMAA2_UNDETERMINED;
   }

   jid=atoi(j->jid);
   grc=gw_client_job_status(jid,&status);
   if ( grc == GW_RC_FAILED_CONNECTION )
        return DRMAA2_UNDETERMINED;
   else if ( grc == GW_RC_FAILED_BAD_JOB_ID )
        return DRMAA2_FAILED;
   switch(status.job_state)
   {
        case GW_JOB_STATE_INIT:
            jstate = DRMAA2_UNDETERMINED;
            *substate=strdup("GW_JOB_STATE_INIT");
            break;

        case GW_JOB_STATE_PENDING:
            jstate = DRMAA2_QUEUED;
            *substate=strdup("GW_JOB_STATE_PENDING");
            break;

        case GW_JOB_STATE_HOLD:
            jstate = DRMAA2_QUEUED_HELD;
            *substate=strdup("GW_JOB_STATE_HOLD");
            break;

        case GW_JOB_STATE_WRAPPER:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_WRAPPER");
            break;
        case GW_JOB_STATE_PRE_WRAPPER:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_PRE_WRAPPER");
            break;
        case GW_JOB_STATE_PROLOG:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_PROLOG");
            break;
        case GW_JOB_STATE_EPILOG:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_EPILOG");
            break;
        case GW_JOB_STATE_EPILOG_STD:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_EPILOG_STD");
            break;
        case GW_JOB_STATE_EPILOG_RESTART:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_EPILOG_RESTART");
            break;
        case GW_JOB_STATE_EPILOG_FAIL:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_EPILOG_FAIL");
            break;
        case GW_JOB_STATE_STOP_CANCEL:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_STOP_CANCEL");
            break;
        case GW_JOB_STATE_STOP_EPILOG:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_STOP_EPILOG");
            break;
        case GW_JOB_STATE_KILL_CANCEL:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_KILL_CANCEL");
            break;
        case GW_JOB_STATE_KILL_EPILOG:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_KILL_EPILOG");
            break;
        case GW_JOB_STATE_MIGR_CANCEL:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_MIGR_CANCEL");
            break;
        case GW_JOB_STATE_MIGR_PROLOG:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_MIGR_PROLOG");
            break;
        case GW_JOB_STATE_MIGR_EPILOG:
            jstate = DRMAA2_RUNNING;
            *substate=strdup("GW_JOB_STATE_MIGR_EPILOG");
            break;

        case GW_JOB_STATE_STOPPED:
            jstate = DRMAA2_SUSPENDED;
            *substate=strdup("GW_JOB_STATE_STOPPED");
            break;

        case GW_JOB_STATE_ZOMBIE:
            jstate = DRMAA2_DONE;
            *substate=strdup("GW_JOB_STATE_ZOMBIE");
            break;

        case GW_JOB_STATE_FAILED:
            jstate = DRMAA2_FAILED;
            *substate=strdup("GW_JOB_STATE_FAILED");
            break;

        default:
            jstate = DRMAA2_UNDETERMINED;
            *substate=strdup("GW_JOB_STATE_LIMIT");
            break;
   }

   return jstate;
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_jinfo drmaa2_j_get_info(const drmaa2_j j)
{
    int i;
    if(j==NULL)
    {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Try to access a non-existent job!";
      return NULL;
    }

    drmaa2_jinfo info = j->info;
    drmaa2_jinfo jinfo=drmaa2_jinfo_create();

    memcpy(jinfo, info, sizeof(drmaa2_jinfo_s));
    jinfo->jobId = (info->jobId != NULL) ? strdup(info->jobId) : DRMAA2_UNSET_STRING;
    jinfo->terminatingSignal = (info->terminatingSignal != NULL) ? strdup(info->terminatingSignal) : DRMAA2_UNSET_STRING;
    jinfo->annotation = (info->annotation != NULL) ? strdup(info->annotation) : DRMAA2_UNSET_STRING;
    jinfo->jobSubState = (info->jobSubState != NULL) ? strdup(info->jobSubState) : DRMAA2_UNSET_STRING;
    jinfo->allocatedMachines = (drmaa2_string_list) drmaa2_list_create(DRMAA2_STRINGLIST, DRMAA2_UNSET_CALLBACK);
    for(i=0;i<drmaa2_list_size(info->allocatedMachines);i++)
    {
       drmaa2_list_add(jinfo->allocatedMachines,(void*) drmaa2_list_get(info->allocatedMachines,i));
    }

    jinfo->submissionMachine = (info->submissionMachine != NULL) ? strdup(info->submissionMachine) : DRMAA2_UNSET_STRING;
    jinfo->jobOwner = (info->jobOwner != NULL) ? strdup(info->jobOwner) : DRMAA2_UNSET_STRING;
    jinfo->queueName = (info->queueName != NULL) ? strdup(info->queueName): DRMAA2_UNSET_STRING;

    return jinfo;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_j_wait_started(const drmaa2_j j, const time_t timeout)
{
    return DRMAA2_SUCCESS;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_j_wait_terminated(const drmaa2_j j, const time_t timeout)
{

    int exit_code;
    int jid;
    drmaa2_error error;
    int num_records, ii;
    gw_msg_history_t * history_list;
    gw_return_code_t grc, grc2;
    gw_msg_job_t status;
    char* machine= malloc(GW_MSG_STRING_HOST);


    jid = atoi(j->jid);
    grc = gw_client_wait(jid, &exit_code,timeout);

    grc2 = gw_client_job_status(jid,&status);

    j->info->exitStatus = status.exit_code;
    j->info->jobState = DRMAA2_FAILED;
    j->info->jobSubState = strdup(gw_job_get_state_name(status.job_state));
    drmaa2_slotinfo slot=malloc(sizeof(drmaa2_slotinfo_s));
    slot->machineName=strdup(status.host);
    slot->slots=1;
    drmaa2_list_add(j->info->allocatedMachines, (void*)(strdup(status.host))); 
    
    exit_code=gethostname(machine,GW_MSG_STRING_HOST);
    if(exit_code==0)
       j->info->submissionMachine = strdup(machine);
    j->info->jobOwner = strdup(status.owner);
    j->info->slots = status.np;

    j->info->queueName = strdup("default");
    grc2= gw_client_job_history(jid, &history_list, &num_records);
    for (ii=0;ii<num_records;ii++)
    {    
       if((&(history_list[ii]))->queue!=NULL)
          j->info->queueName = strdup((&(history_list[ii]))->queue);
          break;
    }    

    j->info->wallclockTime = time(NULL) - j->info->submissionTime;
    j->info->cpuTime = status.cpu_time;
    j->info->dispatchTime = status.start_time;
    j->info->finishTime = status.exit_time;
   

    switch(grc)
    {    
            case GW_RC_FAILED_BAD_JOB_ID:
                 if(gw_drmaa2_found_job(j) && (jid<gw_drmaa2_total_jobs()))
                 {
                    gw_drmaa2_remove_job(j);
                    error = DRMAA2_SUCCESS;
                 }
                 else
                    error = DRMAA2_INVALID_STATE;
                 break;
             
            case GW_RC_FAILED_JOB_KILLED:
                 gw_drmaa2_remove_job(j);
                 error = DRMAA2_SUCCESS;
                 break;

            case GW_RC_FAILED_CONNECTION:
                 error = DRMAA2_DRM_COMMUNICATION;
                 break;

            case GW_RC_FAILED_TIMEOUT:
                 error = DRMAA2_TIMEOUT;
                 break;

            case GW_RC_FAILED_PERM:
                 error = DRMAA2_DENIED_BY_DRMS;
                 break;

            case GW_RC_SUCCESS:
                 j->info->jobState = DRMAA2_DONE;
                 j->info->jobSubState = strdup("DONE");
                 error = DRMAA2_SUCCESS;
                 break;

            default:
                 error = DRMAA2_INTERNAL;
                 break;
    }

    return error;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_j_free(drmaa2_j* j)
{
    if(*j==NULL)
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
       if((*j)->jid != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*j)->jid));   
       if((*j)->session_name != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*j)->session_name));   
       if((*j)->info != NULL)
         drmaa2_jinfo_free(&((*j)->info));

       free(*j);
       *j = NULL;
    }
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_j drmaa2_jsession_wait_any_started(const drmaa2_jsession js,
                                          const drmaa2_j_list l,
                                          const time_t timeout)
{
    drmaa2_j job, job0;

    job  = (drmaa2_j) malloc(sizeof(drmaa2_j_s));
    job0 = (drmaa2_j) malloc(sizeof(drmaa2_j_s));
    job0 = (drmaa2_j) drmaa2_list_get(l,0); 
    if(strcmp(job0->session_name,js->name)!=0)
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "The job is not in the job session!";
        return NULL;
    }

    memcpy(job, job0, sizeof(drmaa2_j_s));
    return job;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_j drmaa2_jsession_wait_any_terminated(const drmaa2_jsession js,
                                             const drmaa2_j_list l,
                                             const time_t timeout)
{
    int *            job_ids;
    int              i, exit_code;
    int *            exit_codes;
    drmaa2_j         job,job0;
    int              num_records;
    gw_msg_history_t *history_list;
    gw_msg_job_t     status;
    gw_return_code_t grc, grc2;
    char machine[GW_MSG_STRING_HOST];

    job_ids    = (int *) malloc(sizeof(int)*(drmaa2_list_size(l)+1));
    exit_codes = (int *) malloc(sizeof(int)* drmaa2_list_size(l));

    job =(drmaa2_j) malloc(sizeof(drmaa2_j_s));
    job0=(drmaa2_j) malloc(sizeof(drmaa2_j_s));
    for (i=0;i<drmaa2_list_size(l);i++)
    {
       if(i==0)
       {
          job0=(drmaa2_j) drmaa2_list_get(l,i); 
          if(strcmp(job0->session_name,js->name)!=0)
          {
             lasterror = DRMAA2_INVALID_ARGUMENT;
             lasterror_text = "The job is not in the job session!";
             return NULL;
          }
          else
             job_ids[i] = (atoi)(job0->jid);
       }
       else
       {
          job=(drmaa2_j)drmaa2_list_get(l,i); 
          if(strcmp(job->session_name,js->name)!=0)
          {
             lasterror = DRMAA2_INVALID_ARGUMENT;
             lasterror_text = "The job is not in the job session!";
             return NULL;
          }
          else
             job_ids[i] = (atoi)(job->jid);
       }
    }
    job_ids[i] = -1;
   
    grc = gw_client_wait_set(job_ids, &exit_codes,GW_TRUE,timeout);
    grc2 = gw_client_job_status(job_ids[0],&status);

    job0->info->exitStatus = status.exit_code;
    job0->info->jobState = DRMAA2_FAILED;
    job0->info->jobSubState = strdup(gw_job_get_state_name(status.job_state));
    drmaa2_slotinfo slot=malloc(sizeof(drmaa2_slotinfo_s));
    slot->machineName=strdup(status.host);
    slot->slots=1;
    drmaa2_list_add(job0->info->allocatedMachines, (void*)slot);

    exit_code=gethostname(machine,GW_MSG_STRING_HOST);
    if(exit_code==0)
       job0->info->submissionMachine = strdup(machine);
    job0->info->jobOwner = status.owner;
    job0->info->slots = status.np;

    job0->info->queueName = strdup((&(history_list[i]))->queue);
    grc2= gw_client_job_history(job_ids[0], &history_list, &num_records);
    for (i=0;i<num_records;i++)
    {
       if((&(history_list[i]))->queue!=NULL)
          job0->info->queueName = strdup((&(history_list[i]))->queue);
          break;
    }

    job0->info->wallclockTime = time(NULL) - job0->info->submissionTime;
    job0->info->cpuTime = status.cpu_time;
    job0->info->dispatchTime = status.start_time;
    job0->info->finishTime = status.exit_time;

    switch(grc)
    {
        case GW_RC_FAILED_BAD_JOB_ID:
        case GW_RC_FAILED_JOB_KILLED:
             gw_drmaa2_remove_job(job0);
             lasterror = DRMAA2_INTERNAL;
             lasterror_text = "Bad job id or the job has been killed already!";
             break;
        case GW_RC_SUCCESS:
             job0->info->jobState = DRMAA2_DONE;
             job0->info->jobSubState = strdup("DONE");
             return job0;
             break;
        default:
             return NULL;
             break;
    }

    return NULL;

}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_j drmaa2_jsession_run_job(const drmaa2_jsession js, const drmaa2_jtemplate jt)
{
   int rc;
   char* jt_file;
   char* jt_parse;
   int jid, prio;
   char jobid[DRMAA2_JOBNAME_BUFFER];
   gw_return_code_t grc;

   drmaa2_j job=(drmaa2_j) malloc(sizeof(drmaa2_j_s));
   job->session_name = (js->name != NULL) ? strdup(js->name) : DRMAA2_UNSET_STRING;
   job->jid = NULL;

   rc = gw_drmaa2_jt_write(jt);
   jt_file = drmaa2_gw_jt_file(jt);
   jt_parse = drmaa2_gw_jt_parse(jt_file);

   prio = GW_JOB_DEFAULT_PRIORITY;


   if ((jt->submitAsHold)== DRMAA2_TRUE)
       grc = gw_client_job_submit(jt_parse,               
                                  GW_JOB_STATE_HOLD, 
                                  &jid,
                                  NULL,
                                  prio);
   else 
       grc = gw_client_job_submit(jt_parse,
                                  GW_JOB_STATE_PENDING,
                                  &jid,
                                  NULL,
                                  prio);

   if(grc != GW_RC_SUCCESS)
   {
      if(grc==GW_RC_FAILED_NO_MEMORY)
      {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memeory allocation failure!";
      }
      else
      {
        lasterror = DRMAA2_DRM_COMMUNICATION;
        lasterror_text = "DRM communication failure!";
      }

      free(jt_file);
      free(jt_parse);

      return NULL;
   }

     
   snprintf(jobid, DRMAA2_JOBNAME_BUFFER-1,"%i",jid);
   job->jid=strdup(jobid);
   job->template = (drmaa2_jtemplate)malloc(sizeof(drmaa2_jtemplate_s));

   memcpy(job->template, jt, sizeof(drmaa2_jtemplate_s));

   drmaa2_jinfo info = drmaa2_jinfo_create();
   info->jobId = (job->jid != NULL) ? strdup(job->jid) : DRMAA2_UNSET_STRING;
   info->wallclockTime = 0;
   info->submissionTime = time(NULL);
   info->dispatchTime = time(NULL);

   job->info = info;

   drmaa2_list_add(js->jobs,job);

   free(jt_file);
   free(jt_parse);

   return job;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_jarray drmaa2_jsession_run_bulk_jobs (const drmaa2_jsession js, 
                                             const drmaa2_jtemplate jt, 
                                             unsigned long begin_index,
                                             unsigned long end_index,
                                             unsigned long step,
                                             unsigned long max_parallel)
{
   int i, rc;
   char* jt_file;
   char* jt_parse;
   int jid, prio;
   char jobid[DRMAA2_JOBNAME_BUFFER];
   gw_return_code_t grc;
   drmaa2_j job=NULL;
   drmaa2_listtype listtype=DRMAA2_JOBLIST;

   int  *job_ids;
   int  array_id;
   int  total_jobs;

   drmaa2_jarray jarray;

   rc = gw_drmaa2_jt_write(jt);
   jt_file = drmaa2_gw_jt_file(jt);
   jt_parse = drmaa2_gw_jt_parse(jt_file);

   prio = GW_JOB_DEFAULT_PRIORITY;
   total_jobs = (int) (((end_index - begin_index)/step)+1);

   if ((jt->submitAsHold)== DRMAA2_TRUE)
       grc = gw_client_array_submit(jt_parse,                
                                    total_jobs,
                                    GW_JOB_STATE_HOLD,
                                    &array_id,
                                    &job_ids,
                                    NULL,
                                    begin_index,
                                    step,
                                    prio);
   else
       grc = gw_client_array_submit(jt_parse,
                                    total_jobs,
                                    GW_JOB_STATE_PENDING,
                                    &array_id,
                                    &job_ids,
                                    NULL,
                                    begin_index,
                                    step,
                                    prio);


   if(grc != GW_RC_SUCCESS)
   {
      if(grc==GW_RC_FAILED_NO_MEMORY)
      {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memeory allocation failure!";
      }
      else
      {
        lasterror = DRMAA2_DRM_COMMUNICATION;
        lasterror_text = "DRM communication failure!";
      }

      free(jt_file);
      free(jt_parse);
      return NULL;
   }


   jarray=malloc(sizeof(drmaa2_jarray_s));
   jarray->jarray_id=malloc(DRMAA2_JOBNAME_BUFFER);
   sprintf(jarray->jarray_id,"%d",array_id);

   jarray->session_name = (js->name != NULL) ? strdup(js->name) : DRMAA2_UNSET_STRING;
   jarray->template = (drmaa2_jtemplate)malloc(sizeof(drmaa2_jtemplate_s));
   memcpy(jarray->template, jt, sizeof(drmaa2_jtemplate_s));

   jarray->jobs=drmaa2_list_create(listtype, DRMAA2_UNSET_CALLBACK);   //jobs in the job_array
   for ( i=0; i<total_jobs; i++)
   {
      job=(drmaa2_j)malloc(sizeof(drmaa2_j_s));

      jid=job_ids[i];
      snprintf(jobid, DRMAA2_JOBNAME_BUFFER-1,"%i",jid);
      job->jid=strdup(jobid);
      job->session_name = (js->name != NULL) ? strdup(js->name) : DRMAA2_UNSET_STRING;
      job->template = (drmaa2_jtemplate)malloc(sizeof(drmaa2_jtemplate_s));
      memcpy(job->template, jt, sizeof(drmaa2_jtemplate_s));

      drmaa2_jinfo info = drmaa2_jinfo_create();
      info->jobId = (job->jid != NULL) ? strdup(job->jid) : DRMAA2_UNSET_STRING;
      info->wallclockTime = 0;
      info->submissionTime = time(NULL);
      info->dispatchTime = time(NULL);

      job->info = info;

      drmaa2_list_add(jarray->jobs,job);
      drmaa2_list_add(js->jobs,job);
   }


   drmaa2_list_add(js->jarray_list,jarray);

   free(jt_file);
   free(jt_parse);

   return jarray;

}


//==================================================================
//             Job array manipulation related functions                  
//==================================================================

drmaa2_string drmaa2_jarray_get_id(const drmaa2_jarray ja)
{
   drmaa2_string jarray_id;

   jarray_id=strdup(ja->jarray_id);

   return jarray_id;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_j_list drmaa2_jarray_get_jobs(const drmaa2_jarray ja)
{
   int i;
   drmaa2_error error;
   drmaa2_j job = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
   drmaa2_j_list jobs=drmaa2_list_create(DRMAA2_JOBLIST, DRMAA2_UNSET_CALLBACK);;

   if(drmaa2_list_size(ja->jobs)<1) 
   {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to access a zero length list!";
        return NULL;
   }

   for(i=0;i<drmaa2_list_size(ja->jobs);i++)
   {    
      job=(drmaa2_j) drmaa2_list_get(ja->jobs,i);
      error=drmaa2_list_add(jobs,(void*) job);
      if(error!=DRMAA2_SUCCESS)
         return NULL;
   }    

   return jobs;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_string drmaa2_jarray_get_session_name(const drmaa2_jarray ja)
{
   drmaa2_string session_name;

   session_name=strdup(ja->session_name);

   return session_name;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_jtemplate drmaa2_jarray_get_job_template(const drmaa2_jarray ja)
{

   drmaa2_jtemplate template = (drmaa2_jtemplate)malloc(sizeof(drmaa2_jtemplate_s));
   if(template==NULL)
   {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memeory allocation failure!";
        return NULL;
   }
   memcpy(template, ja->template, sizeof(drmaa2_jtemplate_s));

   return template;
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_jarray_suspend(drmaa2_jarray ja)
{
   int i, all_rc;
   int array_id;
   drmaa2_error error;
   drmaa2_j job = (drmaa2_j) malloc(sizeof(drmaa2_j_s));

   if(ja->session_name==NULL)
   {
      lasterror = DRMAA2_INVALID_SESSION;
      lasterror_text = "Job session name does not exist!";
      return DRMAA2_INVALID_SESSION;
   }

   if(ja->jarray_id==NULL)
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Job array id is NULL!";
      return DRMAA2_INVALID_ARGUMENT;
   }

   array_id=atoi(ja->jarray_id);
   all_rc=gw_client_array_signal(array_id,GW_CLIENT_SIGNAL_STOP,GW_TRUE);

   if ( all_rc == GW_RC_FAILED_BAD_JOB_STATE )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_BAD_JOB_ID )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_CONNECTION )
           error = DRMAA2_DRM_COMMUNICATION;
   else if ( all_rc == GW_RC_SUCCESS )
   {
           error = DRMAA2_SUCCESS;
           for(i=0;i<drmaa2_list_size(ja->jobs);i++)
           {
              job = (drmaa2_j) drmaa2_list_get(ja->jobs,i);
              job->info->jobState=DRMAA2_SUSPENDED;
           }
   }
   else
           error = DRMAA2_INVALID_ARGUMENT;

   return  error;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_jarray_resume(drmaa2_jarray ja)
{
   int i, all_rc;
   int array_id;
   drmaa2_error error;
   drmaa2_j job = (drmaa2_j) malloc(sizeof(drmaa2_j_s));

   if(ja->session_name==NULL)
   {
      lasterror = DRMAA2_INVALID_SESSION;
      lasterror_text = "Job session name does not exist!";
      return DRMAA2_INVALID_SESSION;
   }

   if(ja->jarray_id==NULL)
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Job array id is NULL!";
      return DRMAA2_INVALID_ARGUMENT;
   }

   array_id=atoi(ja->jarray_id);
   all_rc=gw_client_array_signal(array_id,GW_CLIENT_SIGNAL_RESUME,GW_TRUE);

   if ( all_rc == GW_RC_FAILED_BAD_JOB_STATE )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_BAD_JOB_ID )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_CONNECTION )
           error = DRMAA2_DRM_COMMUNICATION;
   else if ( all_rc == GW_RC_SUCCESS )
   {
           error = DRMAA2_SUCCESS;
           for(i=0;i<drmaa2_list_size(ja->jobs);i++)
           {
              job = (drmaa2_j) drmaa2_list_get(ja->jobs,i);
              job->info->jobState=DRMAA2_RUNNING;
           }
   }
   else
           error = DRMAA2_INVALID_ARGUMENT;

   return error;
}



//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_jarray_hold(drmaa2_jarray ja)
{
   int i, all_rc;
   int array_id;
   drmaa2_error error;
   drmaa2_j job = (drmaa2_j) malloc(sizeof(drmaa2_j_s));

   if(ja->session_name==NULL)
   {
      lasterror = DRMAA2_INVALID_SESSION;
      lasterror_text = "Job session name does not exist!";
      return DRMAA2_INVALID_SESSION;
   }

   if(ja->jarray_id==NULL)
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Job array id is NULL!";
      return DRMAA2_INVALID_ARGUMENT;
   }

   array_id=atoi(ja->jarray_id);
   all_rc=gw_client_array_signal(array_id,GW_CLIENT_SIGNAL_HOLD,GW_TRUE);

   if ( all_rc == GW_RC_FAILED_BAD_JOB_STATE )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_BAD_JOB_ID )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_CONNECTION )
           error = DRMAA2_DRM_COMMUNICATION;
   else if ( all_rc == GW_RC_SUCCESS )
   {
           error = DRMAA2_SUCCESS;
           for(i=0;i<drmaa2_list_size(ja->jobs);i++)
           {
              job = (drmaa2_j) drmaa2_list_get(ja->jobs,i);
              job->info->jobState=DRMAA2_QUEUED_HELD;
           }
   }
   else
           error = DRMAA2_INVALID_ARGUMENT;

   return error;

}


 
//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_jarray_release(drmaa2_jarray ja)
{
   int i, all_rc;
   int array_id;
   drmaa2_error error;
   drmaa2_j job = (drmaa2_j) malloc(sizeof(drmaa2_j_s));

   if(ja->session_name==NULL)
   {
      lasterror = DRMAA2_INVALID_SESSION;
      lasterror_text = "Job session name does not exist!";
      return DRMAA2_INVALID_SESSION;
   }

   if(ja->jarray_id==NULL)
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Job array id is NULL!";
      return DRMAA2_INVALID_ARGUMENT;
   }

   array_id=atoi(ja->jarray_id);
   all_rc=gw_client_array_signal(array_id,GW_CLIENT_SIGNAL_RELEASE,GW_TRUE);

   if ( all_rc == GW_RC_FAILED_BAD_JOB_STATE )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_BAD_JOB_ID )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_CONNECTION )
           error = DRMAA2_DRM_COMMUNICATION;
   else if ( all_rc == GW_RC_SUCCESS )
   {
           error = DRMAA2_SUCCESS;
           for(i=0;i<drmaa2_list_size(ja->jobs);i++)
           {
              job = (drmaa2_j) drmaa2_list_get(ja->jobs,i);
              job->info->jobState=DRMAA2_QUEUED;
           }
   }
   else
           error = DRMAA2_INVALID_ARGUMENT;

   return error;

}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_jarray_terminate(drmaa2_jarray ja)
{
   int i, all_rc;
   int array_id;
   drmaa2_error error;
   drmaa2_j job = (drmaa2_j) malloc(sizeof(drmaa2_j_s));

   if(ja->session_name==NULL)
   {
      lasterror = DRMAA2_INVALID_SESSION;
      lasterror_text = "Job session name does not exist!";
      return DRMAA2_INVALID_SESSION;
   }

   if(ja->jarray_id==NULL)
   {
      lasterror = DRMAA2_INVALID_ARGUMENT;
      lasterror_text = "Job array id is NULL!";
      return DRMAA2_INVALID_ARGUMENT;
   }

   array_id=atoi(ja->jarray_id);
   all_rc=gw_client_array_signal(array_id,GW_CLIENT_SIGNAL_KILL,GW_TRUE);

   if ( all_rc == GW_RC_FAILED_BAD_JOB_STATE )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_BAD_JOB_ID )
           error = DRMAA2_INVALID_STATE;
   else if ( all_rc == GW_RC_FAILED_CONNECTION )
           error = DRMAA2_DRM_COMMUNICATION;
   else if ( all_rc == GW_RC_SUCCESS )
   {
           error = DRMAA2_SUCCESS;
           for(i=0;i<drmaa2_list_size(ja->jobs);i++)
           {
              job = (drmaa2_j) drmaa2_list_get(ja->jobs,i);
              job->info->jobState=DRMAA2_FAILED;
           }
   }
   else
           error = DRMAA2_INVALID_ARGUMENT;

   return error;

}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_jarray_free(drmaa2_jarray* ja)
{
    if(*ja==NULL)
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
       if((*ja)->jarray_id != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*ja)->jarray_id));
       if((*ja)->session_name != DRMAA2_UNSET_STRING)
         drmaa2_string_free(&((*ja)->session_name));
       if((*ja)->jobs != DRMAA2_UNSET_LIST)
          drmaa2_list_free(&((*ja)->jobs));

       free(*ja);
       *ja = NULL;
    }
}





//==================================================================
//             Monitoring manipulation related functions                  
//==================================================================

drmaa2_r_list drmaa2_msession_get_all_reservations(const drmaa2_msession ms)
{
    int i,j;
    drmaa2_r_list reservations = drmaa2_list_create(DRMAA2_RESERVATIONLIST, DRMAA2_UNSET_CALLBACK);

    drmaa2_rsession rs= (drmaa2_rsession) malloc(sizeof(drmaa2_rsession_s));
    if(rs==NULL)
    {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memeory allocation failure!";
        return NULL;
    }

    drmaa2_r res= (drmaa2_r) malloc(sizeof(drmaa2_r_s));
    if(res==NULL)
    {
        lasterror = DRMAA2_OUT_OF_RESOURCE;
        lasterror_text = "Memeory allocation failure!";
        return NULL;
    }

    if (r_sessions != DRMAA2_UNSET_LIST)
    {
        for(i=0;i<drmaa2_list_size(r_sessions);i++)
        {
           rs=(drmaa2_rsession)drmaa2_list_get(r_sessions,i);
           for(j=0;j<drmaa2_list_size(rs->reservations);j++)
           {
              res=(drmaa2_r) drmaa2_list_get(rs->reservations,j);
              drmaa2_list_add(reservations, res);
           }
        }
    }

    return reservations;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_j_list drmaa2_msession_get_all_jobs(const drmaa2_msession ms, const drmaa2_jinfo filter)
{
    int i,j;
    drmaa2_bool found;
    drmaa2_error error;
    drmaa2_j job = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
    drmaa2_j_list jobs = drmaa2_list_create(DRMAA2_JOBLIST, DRMAA2_UNSET_CALLBACK);
    drmaa2_jsession js = (drmaa2_jsession) malloc(sizeof(drmaa2_jsession_s));

    if (j_sessions != DRMAA2_UNSET_LIST)
    {
        for(i=0;i<drmaa2_list_size(j_sessions);i++)
        {
           js=(drmaa2_jsession)drmaa2_list_get(j_sessions,i);
           if(drmaa2_list_size(js->jobs)<1)
              continue;
           for(j=0;j<drmaa2_list_size(js->jobs);j++)
           { 
              job=(drmaa2_j) drmaa2_list_get(js->jobs,j);
              if(filter != NULL)
              {    
                 found = gw_drmaa2_jinfo_compare(job->info, filter);
                 if(found == DRMAA2_TRUE)
                 {    
                    error=drmaa2_list_add(jobs,(void*) job);
                    if(error!=DRMAA2_SUCCESS)
                    {    
                       lasterror = DRMAA2_INTERNAL;
                       lasterror_text = "Adding new element to list failed!";
                       return NULL;
                    }    
                 }    
              }    
              else 
              {    
                 error=drmaa2_list_add(jobs,(void*) job);
                 if(error!=DRMAA2_SUCCESS)
                 {    
                    lasterror = DRMAA2_INTERNAL;
                    lasterror_text = "Adding new element to list failed!";
                    return NULL;
                 }    
              }   
           } 

        }
    }

    return jobs;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_queueinfo_list drmaa2_msession_get_all_queues(const drmaa2_msession ms, const drmaa2_string_list names)
{
    int i,j,k,found=0;
    gw_return_code_t rc;

    drmaa2_msession msession = (drmaa2_msession)malloc(sizeof(drmaa2_msession_s));


    if(names != NULL)
    {
       for(i=0;i<drmaa2_list_size(m_sessions);i++)
       {
          msession=(drmaa2_msession) drmaa2_list_get(m_sessions,i);
          if(strcmp(ms->name,msession->name) ==0)
          {
             found=1;
             break;
          }
       }

       if(found==0)
       {
          lasterror = DRMAA2_INVALID_ARGUMENT;
          lasterror_text = "Monitoring session with the given name does not exist!";
          return NULL;
       }
    }

    rc = gw_client_host_status_all();

    drmaa2_queueinfo qinfo= (drmaa2_queueinfo)malloc(sizeof(drmaa2_queueinfo_s));
    drmaa2_queueinfo_list queues = drmaa2_list_create(DRMAA2_QUEUEINFOLIST,DRMAA2_UNSET_CALLBACK);
    
    gw_msg_host_t* host_msg = (gw_msg_host_t *) malloc (sizeof(gw_msg_host_t));

    for (i=0;i<gw_num_of_hosts;i++)
    {
        rc = gw_client_host_status(i, host_msg);
        if(rc==0)
        {
           for (j=0;j<host_msg->number_of_queues;j++)
           {
              qinfo->name=strdup(host_msg->queue_name[j]);
              if(names != DRMAA2_UNSET_LIST)
              {
                 for(k=0;k<drmaa2_list_size(names);k++)
                 {
                    if(strcmp(host_msg->queue_name[j], strdup((char*) drmaa2_list_get(names,k)))==0)
                       drmaa2_list_add(queues, (void*) qinfo);
                 }
              }
              else
              {
                 drmaa2_list_add(queues, (void*) qinfo);
              }  
           }
        }
    }

    return queues;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_machineinfo_list drmaa2_msession_get_all_machines(const drmaa2_msession ms, const drmaa2_string_list names)
{
    int i,j, found=0;
    gw_return_code_t rc;

    drmaa2_msession msession = (drmaa2_msession)malloc(sizeof(drmaa2_msession_s));
    if(names != NULL)
    {    
       for(i=0;i<drmaa2_list_size(m_sessions);i++)
       {    
          msession=(drmaa2_msession) drmaa2_list_get(m_sessions,i);
          if(strcmp(ms->name,msession->name) ==0) 
          {    
             found=1;
             break;
          }    
       }    

       if(found==0)
       {    
          lasterror = DRMAA2_INVALID_ARGUMENT;
          lasterror_text = "Monitoring session with the given name does not exist!";
          return NULL;
       }    
    }    


    rc = gw_client_host_status_all();
    drmaa2_machineinfo_list machines = drmaa2_list_create(DRMAA2_MACHINEINFOLIST, DRMAA2_UNSET_CALLBACK);
    drmaa2_machineinfo machineinfo = (drmaa2_machineinfo) malloc(sizeof(drmaa2_machineinfo_s));
    gw_msg_host_t* host_msg = (gw_msg_host_t *) malloc (sizeof(gw_msg_host_t));
    for (i=0;i<gw_num_of_hosts;i++)
    {
        rc = gw_client_host_status(i, host_msg);
        if(rc==0)
        {
           machineinfo->name=strdup(host_msg->hostname);
           machineinfo->available = DRMAA2_TRUE;
           machineinfo->sockets = 1;
           machineinfo->coresPerSocket = 2;
           machineinfo->threadsPerCore = 2;
           machineinfo->load = 80;
           machineinfo->physMemory = (host_msg->size_mem_mb)*1000;
           machineinfo->virtMemory = (host_msg->size_mem_mb)*1000*2;
//           machineinfo->machineOS = strdup(host_msg->os_name);
           machineinfo->machineOS = DRMAA2_LINUX;
//           machineinfo->machineArch = strdup(host_msg->arch);
           machineinfo->machineArch = DRMAA2_X64;
           machineinfo->machineOSVersion = (drmaa2_version) malloc(sizeof(drmaa2_version_s));
           machineinfo->machineOSVersion->major = strdup(host_msg->os_version);
           machineinfo->machineOSVersion->minor = strdup("0");
           if(names != DRMAA2_UNSET_LIST)
           { 
              for(j=0;j<drmaa2_list_size(names);j++)
              {
                 if(strcmp(machineinfo->name, strdup((char*) drmaa2_list_get(names,j)))==0)
                    drmaa2_list_add(machines, (void*) machineinfo);
              }
           }
           else
              drmaa2_list_add(machines, (void*) machineinfo);
        }
    }


    return machines;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_msession drmaa2_open_msession(const char * session_name)
{
    char *name, *rand_str;
    drmaa2_msession msession = (drmaa2_msession)malloc(sizeof(drmaa2_msession_s));

    if (m_sessions == DRMAA2_UNSET_LIST)
    {
       m_sessions = drmaa2_list_create(DRMAA2_ADDITIONAL_LIST, DRMAA2_UNSET_CALLBACK);
       if (m_sessions == NULL)
       {    
          lasterror = DRMAA2_INTERNAL;
          lasterror_text = "Monitoring session list could not be created!";
          return NULL;
       } 
    }

    srand((unsigned)time(0));

    if (session_name == DRMAA2_UNSET_STRING)
    {
        name = malloc(15);
        name=strdup("Msession_");
        rand_str=gw_get_rand_str(5);
        name=strcat(name,rand_str);
        session_name = strdup(name);
    }

    msession->name = strdup(session_name);
    drmaa2_list_add(m_sessions, (void*) msession);

    return msession;
}


//----------------------------------------------------------
//----------------------------------------------------------
drmaa2_error drmaa2_close_msession(drmaa2_msession ms)
{
    int i;
    drmaa2_msession ms2;
    drmaa2_error error;

    ms2 = (drmaa2_msession) malloc(sizeof(drmaa2_msession_s));
    if (m_sessions != DRMAA2_UNSET_LIST)
    {
        for(i=0;i<drmaa2_list_size(m_sessions);i++)
        {
           ms2=(drmaa2_msession)drmaa2_list_get(m_sessions,i);
           if (strcmp(ms->name, ms2->name) == 0)
           {
              error=drmaa2_list_del(m_sessions, i);
              return error;
           }
        }
    }
    else
    {
        lasterror = DRMAA2_INVALID_SESSION;
        lasterror_text = "Monitoring session does not exist!";
        return lasterror;
    }

    if( m_sessions != DRMAA2_UNSET_LIST && drmaa2_list_size(m_sessions)==0)
    {
       drmaa2_list_free(&m_sessions);
    }


    return DRMAA2_SUCCESS;
}


//----------------------------------------------------------
//----------------------------------------------------------
void drmaa2_msession_free(drmaa2_msession* ms)
{
    if(*ms==NULL)
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Try to free a NULL pointer!";
    }
    else
    {
      if((*ms)->name != DRMAA2_UNSET_STRING )
         drmaa2_string_free(&((*ms)->name));   

      free(*ms);
      *ms = NULL;
   }
}

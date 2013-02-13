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

#include <jni.h>
#include "DrmaaJNI.h"
#include "drmaa.h"
#include "gw_client.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DRMAA_FUNCTION_INIT                     0
#define DRMAA_FUNCTION_EXIT                     1
#define DRMAA_FUNCTION_CREATE_JOB_TEMPLATE      2
#define DRMAA_FUNCTION_DELETE_JOB_TEMPLATE      3
#define DRMAA_FUNCTION_RUN_JOB                  4
#define DRMAA_FUNCTION_RUN_BULK_JOBS            5
#define DRMAA_FUNCTION_CONTROL                  6
#define DRMAA_FUNCTION_SYNCHRONIZE              7
#define DRMAA_FUNCTION_WAIT                     8
#define DRMAA_FUNCTION_GET_JOB_PROGRAM_STATUS   9


void    generate_job_template(JNIEnv *env, drmaa_job_template_t *jt, jobject jobTemplate);

void    throw_exception(JNIEnv *env, int function, int rc);

JNIEXPORT void JNICALL Java_org_ggf_drmaa_DrmaaJNI_init(JNIEnv *env, jobject obj, jstring jcontact)
{
        char            error[DRMAA_ERROR_STRING_BUFFER];
        char            *contact;
        int             rc;

        if ( jcontact == NULL )
                contact = NULL;
        else
                contact = (char *) (*env)->GetStringUTFChars(env, jcontact, NULL);

        rc =  drmaa_init(contact, error, DRMAA_ERROR_STRING_BUFFER-1);

        if (rc != DRMAA_ERRNO_SUCCESS)
        {
                throw_exception(env, DRMAA_FUNCTION_INIT, rc);
                return;
        }
}

JNIEXPORT void JNICALL Java_org_ggf_drmaa_DrmaaJNI_exit(JNIEnv *env, jobject obj)
{
        int             rc;

        rc = drmaa_exit (NULL, 0);

        if (rc != DRMAA_ERRNO_SUCCESS)
        {
                throw_exception(env, DRMAA_FUNCTION_EXIT, rc);
                return;
        }
}

JNIEXPORT jobject JNICALL Java_org_ggf_drmaa_DrmaaJNI_createJobTemplate(JNIEnv *env, jobject obj)
{
        char                            error[DRMAA_ERROR_STRING_BUFFER];
        jclass                           classTemplate;
        jmethodID                    method;
        jobject                         jobTemplate;


        int                             	rc;
        drmaa_job_template_t    *jt;


        classTemplate   		= (*env)->FindClass(env,"org/ggf/drmaa/GridWayJobTemplate");
        method              	= (*env)->GetMethodID(env,classTemplate,"<init>","()V");


        if (method==NULL)
            return NULL;
        else
        {
           jobTemplate = (*env)->NewObject(env,classTemplate,method);
           if (jobTemplate == NULL)
                return NULL;
        }

        rc = drmaa_allocate_job_template (&(jt), error, DRMAA_ERROR_STRING_BUFFER);

        if (rc != DRMAA_ERRNO_SUCCESS)
        {
                throw_exception(env, DRMAA_FUNCTION_CREATE_JOB_TEMPLATE, rc);
                return NULL;
        }

        method  = (*env)->GetMethodID(env,classTemplate,"setJobTemplatePointer","(J)V");
        if (method==NULL)
            return NULL;
        else
            (*env)->CallVoidMethod(env,jobTemplate,method,(long)jt);

        return jobTemplate;
}

JNIEXPORT void JNICALL Java_org_ggf_drmaa_DrmaaJNI_deleteJobTemplate(JNIEnv *env, jobject obj, jobject jobTemplate)
{
        char                                    error[DRMAA_ERROR_STRING_BUFFER];
        jclass                                  classTemplate;
        jmethodID                               method;
        int                                     rc;
        drmaa_job_template_t                    *jt;


        classTemplate   = (*env)->FindClass(env,"org/ggf/drmaa/GridWayJobTemplate");
        method          = (*env)->GetMethodID(env,classTemplate,"getJobTemplatePointer","()J");
        jt = (drmaa_job_template_t *) (long int) (*env)->CallLongMethod(env,jobTemplate,method);

        rc = drmaa_delete_job_template (jt, error, DRMAA_ERROR_STRING_BUFFER);

        method          = (*env)->GetMethodID(env,classTemplate,"setJobTemplatePointer","(J)V");
        (*env)->CallVoidMethod(env,jobTemplate,method,0);

        if (rc != DRMAA_ERRNO_SUCCESS)
        {
                throw_exception(env, DRMAA_FUNCTION_DELETE_JOB_TEMPLATE, rc);
                return;
        }
}


JNIEXPORT jstring JNICALL Java_org_ggf_drmaa_DrmaaJNI_runJob(JNIEnv *env, jobject obj, jobject jobTemplate)
{
        char                                    error[DRMAA_ERROR_STRING_BUFFER];
        jclass                                  classTemplate;
        jmethodID                               getMethod;
        char                                    job_id[DRMAA_JOBNAME_BUFFER];      //  Job identification.
        int                                     rc;
        drmaa_job_template_t                    *jt;

        classTemplate           = (*env)->FindClass(env,"org/ggf/drmaa/GridWayJobTemplate");

        getMethod               = (*env)->GetMethodID(env,classTemplate,"getJobTemplatePointer","()J");
        if (getMethod==NULL)
            return NULL;
        else
           jt = (drmaa_job_template_t *) (long int)(*env)->CallLongMethod(env,jobTemplate,getMethod);

        generate_job_template(env, jt, jobTemplate);

        rc = drmaa_run_job (job_id, DRMAA_JOBNAME_BUFFER, jt, error, DRMAA_ERROR_STRING_BUFFER);
	
        if (rc != DRMAA_ERRNO_SUCCESS)
        {
                throw_exception(env, DRMAA_FUNCTION_RUN_JOB, rc);
                return  NULL;
        }

        return (*env)->NewStringUTF(env,job_id);
}


JNIEXPORT jobject JNICALL Java_org_ggf_drmaa_DrmaaJNI_runBulkJobs
                  (JNIEnv *env, jobject obj, jobject jobTemplate, jint beginIndex, jint endIndex, jint step)
{
        char                                    error[DRMAA_ERROR_STRING_BUFFER];
        jclass                                  classTemplate;
        jmethodID                               method;
        int                                     rc;
        drmaa_job_template_t                    *jt;
        drmaa_job_ids_t                         *jobids;
        char                                    value[DRMAA_ATTR_BUFFER];
        jobject                                 list;


        classTemplate   = (*env)->FindClass(env,"org/ggf/drmaa/GridWayJobTemplate");
        method          = (*env)->GetMethodID(env,classTemplate,"getJobTemplatePointer","()J");
        jt = (drmaa_job_template_t *) (long int) (*env)->CallLongMethod(env,jobTemplate,method);

        generate_job_template(env, jt, jobTemplate);
        rc = drmaa_run_bulk_jobs(&jobids, jt, beginIndex, endIndex, step, error, DRMAA_ERROR_STRING_BUFFER);

        if (rc != DRMAA_ERRNO_SUCCESS)
        {
                throw_exception(env, DRMAA_FUNCTION_RUN_BULK_JOBS, rc);
                return  NULL;
        }


        classTemplate   = (*env)->FindClass(env,"java/util/ArrayList");
        method          = (*env)->GetMethodID(env,classTemplate,"<init>","()V");

        if (method==NULL)
            return NULL;
        else
           list = (*env)->NewObject(env,classTemplate,method);

        method          = (*env)->GetMethodID(env,classTemplate,"add","(Ljava/lang/Object;)Z");
        if (method==NULL)
            return NULL;

    	do
        {
        	rc = drmaa_get_next_job_id(jobids, value, DRMAA_ATTR_BUFFER);

        	if ( rc == DRMAA_ERRNO_SUCCESS )
 				(*env)->CallBooleanMethod(env, list, method, (*env)->NewStringUTF(env, value));
    	}while (rc != DRMAA_ERRNO_NO_MORE_ELEMENTS );

        return list;
}


JNIEXPORT void JNICALL Java_org_ggf_drmaa_DrmaaJNI_control(JNIEnv *env, jobject obj, jstring jobId, jint operation)
{
        char                                    error[DRMAA_ERROR_STRING_BUFFER];
        int                                     rc;

        if (jobId!=NULL)
                rc = drmaa_control((*env)->GetStringUTFChars(env, jobId, NULL), operation, error, DRMAA_ERROR_STRING_BUFFER);

        if (rc != DRMAA_ERRNO_SUCCESS)
        {
                throw_exception(env, DRMAA_FUNCTION_CONTROL, rc);
                return;
        }
}

JNIEXPORT void JNICALL Java_org_ggf_drmaa_DrmaaJNI_synchronize(JNIEnv *env, jobject obj, jobject jobList, jlong timeout, jboolean dispose)
{
        jclass          classTemplate;
        jmethodID       method;
        jint            size;
        const char      **job_ids;
        int             i;
        int             rc;
        char            error[DRMAA_ERROR_STRING_BUFFER];

        classTemplate   = (*env)->FindClass(env,"java/util/ArrayList");
        method          = (*env)->GetMethodID(env,classTemplate,"size","()I");
        size            = (*env)->CallIntMethod(env, jobList, method);
        job_ids         = malloc (sizeof(char *) * size);

        method          = (*env)->GetMethodID(env,classTemplate,"get","(I)Ljava/lang/Object;");

        for (i=0; i<size; i++)
                job_ids[i] = (*env)->GetStringUTFChars(env, (*env)->CallObjectMethod(env,jobList,method,i), NULL);

        job_ids[i] = NULL;

        rc = drmaa_synchronize(job_ids, timeout, (int) dispose, error, DRMAA_ERROR_STRING_BUFFER);

        if (rc != DRMAA_ERRNO_SUCCESS)
        {
                throw_exception(env, DRMAA_FUNCTION_SYNCHRONIZE, rc);
                return;
        }
}

JNIEXPORT jobject JNICALL Java_org_ggf_drmaa_DrmaaJNI_wait(JNIEnv *env, jobject obj, jstring jobName, jlong timeout)
{
        jclass                  classTemplate;
        jmethodID               method;
        jmethodID               hashMethod;
        jobject                 hashTable;
        jobject                 jobInfo;
        int                     rc;
        char                    error[DRMAA_ERROR_STRING_BUFFER];
        int                     stat;
        drmaa_attr_values_t     *rusage;
        char                    value[DRMAA_ATTR_BUFFER];
        char                    job_id_out[DRMAA_JOBNAME_BUFFER];
        int                     ifExited;
        int                     ifSignaled;
        int                     core_dumped;
        int                     ifAborted;
        char                    signal[DRMAA_SIGNAL_BUFFER];
        char *                  job_str;
        
        job_str = (char *) (*env)->GetStringUTFChars(env,jobName, NULL);

        rc = drmaa_wait(job_str,  
                        job_id_out, 
                        DRMAA_JOBNAME_BUFFER, 
                        &stat, 
                        timeout, 
                        &rusage,  
                        error, 
                        DRMAA_ERROR_STRING_BUFFER);

        (*env)->ReleaseStringUTFChars(env,jobName,job_str);

        if (rc != DRMAA_ERRNO_SUCCESS)
        {
                throw_exception(env, DRMAA_FUNCTION_WAIT, rc);
                return  NULL;
        }

        classTemplate   = (*env)->FindClass(env,"java/util/Hashtable");
        if (classTemplate==NULL)
            return NULL;

        method          = (*env)->GetMethodID(env,classTemplate,"<init>","()V");
        hashMethod      = (*env)->GetMethodID(env,classTemplate,"hashCode","()I");

        hashTable = (*env)->NewObject(env,classTemplate,method);
        method  = (*env)->GetMethodID(env,classTemplate,"put","(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

        rc = drmaa_get_next_attr_value(rusage, value, DRMAA_ATTR_BUFFER);
        while (rc == DRMAA_ERRNO_SUCCESS )
        {
           char *key = strtok(value,"=");
           char *val = strtok(NULL,"=");
           (*env)->CallObjectMethod(env,hashTable,method, (*env)->NewStringUTF(env,key), (*env)->NewStringUTF(env,val));

           rc = drmaa_get_next_attr_value(rusage, value, DRMAA_ATTR_BUFFER);
         };

        drmaa_release_attr_values(rusage);

        drmaa_wifexited(&ifExited, stat, error, DRMAA_ERROR_STRING_BUFFER);

        classTemplate   = (*env)->FindClass(env,"org/ggf/drmaa/GridWayJobInfo");
        method          = (*env)->GetMethodID(env,classTemplate,"<init>","(Ljava/lang/String;ILjava/util/Map;)V");

        jobInfo         = (*env)->NewObject(env, classTemplate, method, (*env)->NewStringUTF(env,job_id_out), stat,hashTable);


        method          = (*env)->GetMethodID(env,classTemplate,"setHasExited","(Z)V");

        (*env)->CallVoidMethod(env, jobInfo, method, ifExited);

        drmaa_wifsignaled(&ifSignaled, stat, error, DRMAA_ERROR_STRING_BUFFER);
        method          = (*env)->GetMethodID(env,classTemplate,"setHasSignaled","(Z)V");

        (*env)->CallVoidMethod(env, jobInfo, method, ifSignaled);

        if (ifSignaled==1)
        {
                drmaa_wtermsig(signal, DRMAA_SIGNAL_BUFFER, stat, error, DRMAA_ERROR_STRING_BUFFER);
                method          = (*env)->GetMethodID(env,classTemplate,"setTerminationSignal","(Ljava/lang/String;)V");
                (*env)->CallObjectMethod(env, jobInfo, method, (*env)->NewStringUTF(env,signal));
        }

        drmaa_wcoredump(&core_dumped, stat, error, DRMAA_ERROR_STRING_BUFFER);

        method          = (*env)->GetMethodID(env,classTemplate,"setHasCoreDump","(Z)V");
        (*env)->CallVoidMethod(env, jobInfo, method, core_dumped);

        drmaa_wifaborted(&ifAborted, stat, error, DRMAA_ERROR_STRING_BUFFER);
        method          = (*env)->GetMethodID(env,classTemplate,"setWasAborted","(Z)V");
        (*env)->CallVoidMethod(env, jobInfo, method, ifAborted);

        return jobInfo;
}

JNIEXPORT jint JNICALL Java_org_ggf_drmaa_DrmaaJNI_getJobStatus(JNIEnv *env, jobject obj, jstring jobName)
{
        char                                    error[DRMAA_ERROR_STRING_BUFFER];
        int                                     rc;
        int                                     remote_ps;


        rc = drmaa_job_ps((*env)->GetStringUTFChars(env, jobName, NULL), &remote_ps, error, DRMAA_ERROR_STRING_BUFFER);

        if (rc != DRMAA_ERRNO_SUCCESS)
        {
                throw_exception(env, DRMAA_FUNCTION_GET_JOB_PROGRAM_STATUS, rc);
                return  -1;
        }

        return remote_ps;
}

JNIEXPORT jstring JNICALL Java_org_ggf_drmaa_DrmaaJNI_getContact(JNIEnv *env, jobject obj)
{
        char error[DRMAA_ERROR_STRING_BUFFER];
        char contact[DRMAA_ATTR_BUFFER];

        drmaa_get_contact(contact,DRMAA_ATTR_BUFFER,error,DRMAA_ERROR_STRING_BUFFER);

        if (contact!= NULL)
                return (*env)->NewStringUTF(env,contact);
        else
                return NULL;

}


JNIEXPORT jobject JNICALL Java_org_ggf_drmaa_DrmaaJNI_getVersion(JNIEnv *env, jobject obj)
{
        unsigned int    major;
        unsigned int    minor;
        char            error[DRMAA_ERROR_STRING_BUFFER];
        jclass          classVersion;
        jmethodID       method;


        drmaa_version(&major,&minor,error,DRMAA_ERROR_STRING_BUFFER);

        classVersion    = (*env)->FindClass(env,"org/ggf/drmaa/Version");
        method          = (*env)->GetMethodID(env,classVersion,"<init>","(II)V");


        return (*env)->NewObject(env, classVersion, method, major, minor);
}


JNIEXPORT jstring JNICALL Java_org_ggf_drmaa_DrmaaJNI_getDrmsInfo(JNIEnv *env, jobject obj)
{
        char error[DRMAA_ERROR_STRING_BUFFER];
        char drm[DRMAA_ATTR_BUFFER];

        drmaa_get_DRM_system(drm,DRMAA_ATTR_BUFFER,error,DRMAA_ERROR_STRING_BUFFER);

        if (drm!= NULL)
                return (*env)->NewStringUTF(env,drm);
        else
                return NULL;
}


JNIEXPORT jstring JNICALL Java_org_ggf_drmaa_DrmaaJNI_getDrmaaImplementation(JNIEnv *env, jobject obj)
{
        char error[DRMAA_ERROR_STRING_BUFFER];
        char impl[DRMAA_ATTR_BUFFER];

        drmaa_get_DRMAA_implementation(impl,DRMAA_ATTR_BUFFER,error,DRMAA_ERROR_STRING_BUFFER);

        if (impl!= NULL)
                return (*env)->NewStringUTF(env,impl);
        else
                return NULL;
}

void    generate_job_template(JNIEnv *env, drmaa_job_template_t *jt, jobject jobTemplate)
{
        char                                    error[DRMAA_ERROR_STRING_BUFFER];
        jclass                                  classProperties;
        jclass                                  classTemplate;
        jclass								  classList;
        jclass								  classPartialTimestampFormat;
        jmethodID                           getMethod;
        jobject                                args;
        jobject                                object;
        jobject                                properties;
        jobject								  partialTimestampFormat;
        jobject								  dataTime;
        const char                          *argsC[DRMAA_ATTR_BUFFER];
        char                                   *envC;
        char                                    *attribute;
        int                                      rc;
        int                                      ind;
        jint									  size;
        jint									  submissionState;

        classTemplate = (*env)->FindClass(env,"org/ggf/drmaa/GridWayJobTemplate");
        getMethod       = (*env)->GetMethodID(env,classTemplate,"getRemoteCommand","()Ljava/lang/String;");

        object = (*env)->CallObjectMethod(env,jobTemplate,getMethod);

        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env,object,NULL);
                rc        = drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }

        getMethod  = (*env)->GetMethodID(env,classTemplate,"getArgs","()Ljava/util/List;");
        args = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (args != NULL)
        {
        		classList 		= (*env)->FindClass(env,"java/util/ArrayList");
        		getMethod 	= (*env)->GetMethodID(env,classList,"size","()I");
        		size            	= (*env)->CallIntMethod(env, args, getMethod);

			    getMethod    = (*env)->GetMethodID(env,classList,"get","(I)Ljava/lang/Object;");

        		for (ind=0; ind<size; ind++)
                	argsC[ind] = (*env)->GetStringUTFChars(env, (*env)->CallObjectMethod(env,args,getMethod,ind), NULL);

                argsC[ind] = NULL;

                rc = drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, argsC ,error, DRMAA_ERROR_STRING_BUFFER);
	    }

        //classTemplate           = (*env)->FindClass(env,"org/ggf/drmaa/SimpleJobTemplate");

        getMethod = (*env)->GetMethodID(env,classTemplate,"getJobSubmissionState","()I");
        submissionState = (*env)->CallIntMethod(env,jobTemplate,getMethod);

        if (object != NULL)
        {
			
        		attribute = malloc (sizeof(char) * DRMAA_ATTR_BUFFER);
                
		if (submissionState==0)
			sprintf(attribute,"drmaa_hold");
		else
			sprintf(attribute,"drmaa_active");	
                rc        = drmaa_set_attribute(jt, DRMAA_JS_STATE, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }

        getMethod = (*env)->GetMethodID(env,classTemplate,"getJobEnvironment","()Ljava/util/Map;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);

        if (object != NULL)
        {
                classProperties = (*env)->FindClass(env,"java/util/Properties");
                getMethod       = (*env)->GetMethodID(env,classProperties,"toString","()Ljava/lang/String;");
                properties      = (*env)->CallObjectMethod(env,object,getMethod);

                classProperties = (*env)->FindClass(env,"java/lang/String");

                getMethod       = (*env)->GetMethodID(env,classProperties,"substring","(II)Ljava/lang/String;");

                properties      = (*env)->CallObjectMethod(env,properties,getMethod,1, (*env)->GetStringUTFLength(env, properties)-1);

                envC = (char *) (*env)->GetStringUTFChars(env, properties, NULL);
                ind = 0;

                char    *token;

                token = strtok(envC,", ");

                while (token!= NULL)
                {

                        argsC[ind] = malloc(strlen(token)+2);

                        if (token[0]!='\"')
                                sprintf((char *)argsC[ind],"\"%s\"",token);
                        else
                                sprintf((char *)argsC[ind],"%s",token);

                        ind++;
                        token = strtok(NULL,", ");
                }
                argsC[ind] = NULL;

                rc = drmaa_set_vector_attribute(jt, DRMAA_V_ENV, argsC ,error, DRMAA_ERROR_STRING_BUFFER);
        }

        getMethod = (*env)->GetMethodID(env,classTemplate,"getWorkingDirectory","()Ljava/lang/String;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env, object, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_WD, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }

        getMethod = (*env)->GetMethodID(env,classTemplate,"getJobName","()Ljava/lang/String;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env, object, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_JOB_NAME, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }

        getMethod = (*env)->GetMethodID(env,classTemplate,"getInputPath","()Ljava/lang/String;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env, object, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_INPUT_PATH, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }

        getMethod = (*env)->GetMethodID(env,classTemplate,"getOutputPath","()Ljava/lang/String;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env, object, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }

        getMethod = (*env)->GetMethodID(env,classTemplate,"getErrorPath","()Ljava/lang/String;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env, object, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_ERROR_PATH, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }

        getMethod  = (*env)->GetMethodID(env,classTemplate,"getInputFiles","()Ljava/util/List;");
        object = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
        		classList 		= (*env)->FindClass(env,"java/util/ArrayList");
        		getMethod 	= (*env)->GetMethodID(env,classList,"size","()I");
        		size            	= (*env)->CallIntMethod(env, object, getMethod);

			    getMethod    = (*env)->GetMethodID(env,classList,"get","(I)Ljava/lang/Object;");

        		for (ind=0; ind<size; ind++)
                	argsC[ind] = (*env)->GetStringUTFChars(env, (*env)->CallObjectMethod(env,object,getMethod,ind), NULL);

                argsC[ind] = NULL;

                rc = drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, argsC ,error, DRMAA_ERROR_STRING_BUFFER);
        }

     	getMethod  = (*env)->GetMethodID(env,classTemplate,"getOutputFiles","()Ljava/util/List;");
        object = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
        		classList 		= (*env)->FindClass(env,"java/util/ArrayList");
        		getMethod 	= (*env)->GetMethodID(env,classList,"size","()I");
        		size            	= (*env)->CallIntMethod(env, object, getMethod);

			    getMethod    = (*env)->GetMethodID(env,classList,"get","(I)Ljava/lang/Object;");

        		for (ind=0; ind<size; ind++)
                	argsC[ind] = (*env)->GetStringUTFChars(env, (*env)->CallObjectMethod(env,object,getMethod,ind), NULL);

                argsC[ind] = NULL;

                rc = drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, argsC ,error, DRMAA_ERROR_STRING_BUFFER);
        }

     	getMethod  = (*env)->GetMethodID(env,classTemplate,"getRestartFiles","()Ljava/util/List;");
        object = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
        		classList 		= (*env)->FindClass(env,"java/util/ArrayList");
        		getMethod 	= (*env)->GetMethodID(env,classList,"size","()I");
        		size            	= (*env)->CallIntMethod(env, object, getMethod);

			    getMethod    = (*env)->GetMethodID(env,classList,"get","(I)Ljava/lang/Object;");

        		for (ind=0; ind<size; ind++)
                	argsC[ind] = (*env)->GetStringUTFChars(env, (*env)->CallObjectMethod(env,object,getMethod,ind), NULL);

                argsC[ind] = NULL;

                rc = drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, argsC ,error, DRMAA_ERROR_STRING_BUFFER);
        }

        getMethod = (*env)->GetMethodID(env,classTemplate,"getRescheduleOnFailure","()Ljava/lang/String;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);

        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env, object, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_GW_RESCHEDULE_ON_FAILURE, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }

        getMethod = (*env)->GetMethodID(env,classTemplate,"getNumberOfRetries","()Ljava/lang/String;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env, object, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_GW_NUMBER_OF_RETRIES, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }


        //classTemplate           = (*env)->FindClass(env,"org/ggf/drmaa/JobTemplateImpl");
        getMethod = (*env)->GetMethodID(env,classTemplate,"getRank","()Ljava/lang/String;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env, object, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_GW_RANK, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }

        getMethod = (*env)->GetMethodID(env,classTemplate,"getRequirements","()Ljava/lang/String;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env, object, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_GW_REQUIREMENTS, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }
        
        getMethod = (*env)->GetMethodID(env,classTemplate,"getType","()Ljava/lang/String;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env, object, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_GW_TYPE, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }
        
        getMethod = (*env)->GetMethodID(env,classTemplate,"getNp","()Ljava/lang/String;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
                attribute = (char *) (*env)->GetStringUTFChars(env, object, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_GW_NP, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }
        
        getMethod = (*env)->GetMethodID(env,classTemplate,"getDeadlineTime","()Lorg/ggf/drmaa/PartialTimestamp;");
        object    = (*env)->CallObjectMethod(env,jobTemplate,getMethod);
        if (object != NULL)
        {
        	    classPartialTimestampFormat = (*env)->FindClass(env,"org/ggf/drmaa/PartialTimestampFormat");
        		getMethod 	= (*env)->GetMethodID(env,classPartialTimestampFormat,"<init>","()V");		
          		partialTimestampFormat = (*env)->NewObject(env,classPartialTimestampFormat, getMethod);
        		
        		getMethod		= (*env)->GetMethodID(env,classPartialTimestampFormat,"format","(Lorg/ggf/drmaa/PartialTimestamp;)Ljava/lang/String;");
        		dataTime      	= (*env)->CallObjectMethod(env,partialTimestampFormat,getMethod, object);
        		 	
                attribute = (char *) (*env)->GetStringUTFChars(env, dataTime, NULL);
                rc = drmaa_set_attribute(jt, DRMAA_DEADLINE_TIME, attribute, error, DRMAA_ERROR_STRING_BUFFER);
        }
        
}

void    throw_exception(JNIEnv *env, int function, int rc)
{
        jclass          classException;

        switch(function)
        {
                case DRMAA_FUNCTION_INIT:

                        if (rc == DRMAA_ERRNO_DRMS_INIT_FAILED)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DrmsInitException");
                                (*env)->ThrowNew(env, classException, "Fail while initializing the session");
                        }
                        else if(rc == DRMAA_ERRNO_INVALID_CONTACT_STRING)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/InvalidContactStringException");
                                if (classException == NULL) printf("La clase es NULA");
                                (*env)->ThrowNew(env, classException, "The contact parameter is invalid");
                        }
                        else if(rc == DRMAA_ERRNO_ALREADY_ACTIVE_SESSION)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/AlreadyActiveSessionException");
                                (*env)->ThrowNew(env, classException, "The session has already been initialized");

                        }
                        else if(rc == DRMAA_ERRNO_DEFAULT_CONTACT_STRING_ERROR)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DefaultContactStringException");
                                (*env)->ThrowNew(env, classException, "The contact string is null and the defaul contact string could not be used to connect to the DRMS");
                        }
                        else
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/NoDefaultContactStringSelectedException");
                                (*env)->ThrowNew(env, classException, "The contact parameter is null and more than one DRMAA implementation is available"
);
                        }

                        break;

                case DRMAA_FUNCTION_EXIT:

                        if (rc == DRMAA_ERRNO_DRMS_EXIT_ERROR)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DrmsExitException");
                                (*env)->ThrowNew(env, classException, "Fail while exiting the session");
                        }
                        else if(rc == DRMAA_ERRNO_NO_ACTIVE_SESSION)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/NoActiveSessionException");
                                (*env)->ThrowNew(env, classException, "The session has not been initialized or exit() has already been called");
                        }
                        break;

                case DRMAA_FUNCTION_CREATE_JOB_TEMPLATE:

                        classException = (*env)->FindClass(env,"org/ggf/drmaa/DrmCommunicationException");
                        (*env)->ThrowNew(env, classException, "Unable to communication with the DRMS");
                        break;

                case DRMAA_FUNCTION_DELETE_JOB_TEMPLATE:

                        if (rc == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DrmCommunicationException");
                                (*env)->ThrowNew(env, classException, "Unable to communication with the DRMS");
                        }

                case DRMAA_FUNCTION_RUN_JOB:

                        if (rc == DRMAA_ERRNO_TRY_LATER)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/TryLaterException");
                                (*env)->ThrowNew(env, classException, "The request could not be processed due to excessive system load");
                        }
                        else if(rc == DRMAA_ERRNO_DENIED_BY_DRM)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DeniedByDrmException");
                                (*env)->ThrowNew(env, classException, "Job rejected by DRMS. The job will never be accepted due to job template or DRMS configuration settings");
                        }
                        else if(rc == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DrmCommunicationException");
                                (*env)->ThrowNew(env, classException, "Unable to communication with the DRMS");

                        }
                        else if(rc == DRMAA_ERRNO_AUTH_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/AuthorizationException");
                                (*env)->ThrowNew(env, classException, "The user does not have permission to submit jobs");
                        }
                        else if (rc == DRMAA_ERRNO_NO_ACTIVE_SESSION)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/NoActiveSessionException");
                                (*env)->ThrowNew(env, classException, "The session has not been initialized");
                        }
                        break;

                case DRMAA_FUNCTION_RUN_BULK_JOBS:

                        if (rc == DRMAA_ERRNO_TRY_LATER)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/TryLaterException");
                                (*env)->ThrowNew(env, classException, "The request could not be processed due to excessive system load");
                        }
                        else if(rc == DRMAA_ERRNO_DENIED_BY_DRM)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DeniedByDrmException");
                                (*env)->ThrowNew(env, classException, "Job rejected by DRMS. The job will never be accepted due to job template or DRMS configuration settings");
                        }
                        else if(rc == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DrmCommunicationException");
                                (*env)->ThrowNew(env, classException, "Unable to communication with the DRMS");

                        }
                        else if(rc == DRMAA_ERRNO_AUTH_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/AuthorizationException");
                                (*env)->ThrowNew(env, classException, "The user does not have permission to submit jobs");
                        }

                        break;

                case DRMAA_FUNCTION_CONTROL:

                        if (rc == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DrmCommunicationException");
                                (*env)->ThrowNew(env, classException, "Unable to communication with the DRMS");
                        }
                        else if(rc == DRMAA_ERRNO_AUTH_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/AuthorizationException");
                                (*env)->ThrowNew(env, classException, "The user does not have permission to modify jobs");
                        }
                        else if(rc == DRMAA_ERRNO_RESUME_INCONSISTENT_STATE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/ResumeInconsistentStateException");
                                (*env)->ThrowNew(env, classException, "The job is not in a state from which it can be resumed");

                        }
                        else if(rc == DRMAA_ERRNO_SUSPEND_INCONSISTENT_STATE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/SuspendInconsistentStateException");
                                (*env)->ThrowNew(env, classException, "The job is not in a state from which it can be suspended");
                        }
                        else if (rc == DRMAA_ERRNO_HOLD_INCONSISTENT_STATE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/HoldInconsistentStateException");
                                (*env)->ThrowNew(env, classException, "The job is not in a state from which it can be held");
                        }
                        else if (rc == DRMAA_ERRNO_RELEASE_INCONSISTENT_STATE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/ReleaseInconsistentStateException");
                                (*env)->ThrowNew(env, classException, "The job is not in a state from which it can be released");
                        }
                        else if (rc == DRMAA_ERRNO_INVALID_JOB)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/InvalidJobException");
                                (*env)->ThrowNew(env, classException, "The job id does not represent a valid job");
                        }

                        break;

                case DRMAA_FUNCTION_SYNCHRONIZE:

                        if (rc == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DrmCommunicationException");
                                (*env)->ThrowNew(env, classException, "Unable to communication with the DRMS");
                        }
                        else if(rc == DRMAA_ERRNO_AUTH_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/AuthorizationException");
                                (*env)->ThrowNew(env, classException, "The user does not have permission to synchronize against jobs");
                        }
                        else if(rc == DRMAA_ERRNO_EXIT_TIMEOUT)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/ExitTimeoutException");
                                (*env)->ThrowNew(env, classException, "The call was interrupted before all given jobs finished");

                        }
                        else if (rc == DRMAA_ERRNO_INVALID_JOB)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/InvalidJobException");
                                (*env)->ThrowNew(env, classException, "The job id does not represent a valid job");
                        }

                        break;

                case DRMAA_FUNCTION_WAIT:

                        if (rc == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DrmCommunicationException");
                                (*env)->ThrowNew(env, classException, "Unable to communication with the DRMS");
                        }
                        else if(rc == DRMAA_ERRNO_AUTH_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/AuthorizationException");
                                (*env)->ThrowNew(env, classException, "The user does not have permission to wait for a job");
                        }
                        else if(rc == DRMAA_ERRNO_NO_RUSAGE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/InternalException");
                                (*env)->ThrowNew(env, classException, "The resource usage information for the given job is unavailable");
                        }
                        else if(rc == DRMAA_ERRNO_EXIT_TIMEOUT)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/ExitTimeoutException");
                                (*env)->ThrowNew(env, classException, "The call was interrupted before all given jobs finished");

                        }
                        else if (rc == DRMAA_ERRNO_INVALID_JOB)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/InvalidJobException");
                                (*env)->ThrowNew(env, classException, "The job id does not represent a valid job");
                        }
                        else if (rc == DRMAA_ERRNO_NO_ACTIVE_SESSION)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/NoActiveSessionException");
                                (*env)->ThrowNew(env, classException, "The session has not been initialized");
                        }
                        break;

                case DRMAA_FUNCTION_GET_JOB_PROGRAM_STATUS:

                        if (rc == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/DrmCommunicationException");
                                (*env)->ThrowNew(env, classException, "Unable to communication with the DRMS");
                        }
                        else if(rc == DRMAA_ERRNO_AUTH_FAILURE)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/AuthorizationException");
                                (*env)->ThrowNew(env, classException, "The user does not have permission to query for a job's status");
                        }
                        else if (rc == DRMAA_ERRNO_INVALID_JOB)
                        {
                                classException = (*env)->FindClass(env,"org/ggf/drmaa/InvalidJobException");
                                (*env)->ThrowNew(env, classException, "The job id does not represent a valid job");
                        }

                        break;

                default:
                        exit (0);
                        break;
        }
}

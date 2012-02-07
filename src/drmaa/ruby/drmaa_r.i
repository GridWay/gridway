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

%module "DRMAA"

%include "typemaps.i"

%{
#include "drmaa.h"
%}

%typemap(in,numinputs=1) (const char *contact) {
	if($input==Qnil)
		$1=0;
	else
		$1=StringValuePtr($input);
}

%typemap(in,numinputs=0) (char * error_diagnosis, size_t error_diag_len ) {
	$1 = (char*) malloc( sizeof(char) * DRMAA_ERROR_STRING_BUFFER );
	$2 = DRMAA_ERROR_STRING_BUFFER;
}
%apply (char * error_diagnosis, size_t error_diag_len ) { ( char *value, size_t value_len ) }
%typemap(in,numinputs=0) ( char *job_id, size_t job_id_len ) {
	$1 = (char*) malloc( sizeof(char) * DRMAA_JOBNAME_BUFFER );
	$2 = DRMAA_JOBNAME_BUFFER;
}
%apply (char *job_id, size_t job_id_len ) { ( char *job_id_out, size_t job_id_out_len ) }
%typemap(in,numinputs=0) ( char *drm_system, size_t drm_system_len ) {
	$1 = (char*) malloc( sizeof(char) * DRMAA_DRM_SYSTEM_BUFFER );
	$2 = DRMAA_DRM_SYSTEM_BUFFER;
}
%apply (char *drm_system, size_t drm_system_len ) { ( char *drmaa_impl, size_t drmaa_impl_len ) }
%typemap(in,numinputs=0) ( char *contact, size_t contact_len ) {
	$1 = (char*) malloc( sizeof(char) * DRMAA_CONTACT_BUFFER );
	$2 = DRMAA_CONTACT_BUFFER;
}
%typemap(in,numinputs=0) ( char *signal, size_t signal_len ) {
	$1 = (char*) malloc( sizeof(char) * DRMAA_SIGNAL_BUFFER );
	$2 = DRMAA_SIGNAL_BUFFER;
}

%typemap(argout) char *error_diagnosis {
	VALUE str, res, ary;
	res=$result;

	str=rb_str_new2($1);
	
	if(TYPE(res) == T_ARRAY) {
		ary=res;
	} else {
		ary=rb_ary_new();
		rb_ary_push(ary, res);
	}
	rb_ary_push(ary, str);
	
	$result=ary;
}

%apply char *error_diagnosis { char *value }
%apply char *error_diagnosis { char *job_id }
%apply char *error_diagnosis { char *job_id_out }
%apply char *error_diagnosis { char *contact }
%apply char *error_diagnosis { char *drm_system }
%apply char *error_diagnosis { char *drmaa_impl };

%typemap(argout) const char * "pass";

%typemap(in,numinputs=0) drmaa_job_template_t ** ( $*1_type keepme )  {
    keepme = NULL;
    
    $1 = &keepme;
}

%typemap(argout) drmaa_job_template_t ** {
	VALUE res, pointer;
	if(TYPE($result) != T_ARRAY) {
		res=$result;
		$result=rb_ary_new();
		rb_ary_push($result, res);
	}
	pointer=SWIG_NewPointerObj(*$1, $*1_descriptor, 0);
	rb_ary_push($result, pointer);
}

%apply drmaa_job_template_t ** { drmaa_job_ids_t ** }
%apply drmaa_job_template_t ** { drmaa_attr_names_t ** }
%apply drmaa_job_template_t ** { drmaa_attr_values_t ** }

%typemap(in) const char *[] {
  /* This is to eliminate the warning for non initialized variable */
  $1=0;
  /* Check if is an array */
  if (TYPE($input) == T_ARRAY) {
    int size = RARRAY($input)->len;
    int i = 0;
    $1 = (char **) malloc((size+1)*sizeof(char *));
    for (i = 0; i < size; i++) {
		VALUE o = rb_ary_entry($input,i);
      	if (TYPE(o) == T_STRING)
			$1[i] = STR2CSTR(rb_ary_entry($input,i));
      	else {
			//croak("array must contain strings");
			free($1);
			//return NULL;
      	}
    }
    $1[i] = 0;
  } else {
    //croak("not an array");
    //return NULL;
  }
}

%typemap(in,numinputs=0) int * {
	$1=($*1_type *)malloc(sizeof(int));
}
%typemap(argout) int * {
	VALUE num, res;
	num=INT2NUM(*$1);
	free($1);
	if(TYPE($result) != T_ARRAY) {
		res=$result;
		$result=rb_ary_new();
		rb_ary_push($result, res);
	}
	rb_ary_push($result, num);
}
%apply int * { unsigned int * }

%include "drmaa.h"


/* -------------------------------------------------------------------------- */
/* Copyright 2002-2006 GridWay Team, Distributed Systems Architecture         */
/* Group, Universidad Complutense de Madrid                                   */
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

import java.io.*;
import java.util.Collections;
import java.util.Map;
import java.util.HashMap;
import java.util.Date;
import java.net.URL;

import org.apache.axis.components.uuid.UUIDGen;
import org.apache.axis.components.uuid.UUIDGenFactory;
import org.apache.axis.message.addressing.EndpointReferenceType;

import org.globus.exec.client.GramJobListener;
import org.globus.exec.client.GramJob;
import org.globus.exec.utils.client.ManagedJobFactoryClientHelper;
import org.globus.exec.utils.ManagedJobFactoryConstants;
import org.globus.exec.utils.rsl.RSLHelper;
import org.globus.exec.generated.StateEnumeration;


class GW_mad_ws_seq implements GramJobListener {

  static String info = null;
  private Map job_pool = null; // Job pool
  private Map jid_pool = null; // JID pool

  public static void main(String args[]) {
    GW_mad_ws_seq gw_mad_ws = new GW_mad_ws_seq();

    gw_mad_ws.start();
  }

  // Constructor
  GW_mad_ws_seq() {
    // Create the job and JID pool
    job_pool = Collections.synchronizedMap(new HashMap());
    jid_pool = Collections.synchronizedMap(new HashMap());
  }

  void start() {
    String str;
    String action = null;
    String jid_str = null;
    String resource;
    String rsl;
    boolean fin = false;
    int status = 0;

    try
    {
      BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

      while (!fin) {
        // Read a line a parse it
        str = in.readLine();

        String str_split[] = str.split(" ", 4);

	if (str_split.length != 4)
        {
          status = 1;
	  info = "Error reading line";
	  fin = true;
	}
	else
        {
          action = str_split[0].toUpperCase();
          jid_str = str_split[1];
          resource = str_split[2];
          rsl = str_split[3];

          // Perform the action
          if (action.equals("INIT"))
            status = init();
          else if (action.equals("SUBMIT"))
            status = submit(jid_str, resource, rsl);
          else if (action.equals("CANCEL"))
            status = cancel(jid_str);
          else if (action.equals("POLL"))
            status = poll(jid_str);
          else if (action.equals("FINALIZE"))
          {
            status = finalize_mad();
            fin = true;
          }
          else
          {
            status = 1;
            info = "Not valid action";
          }
        }

        synchronized(this) {
          if (status == 0)
            System.out.println(action + " " + jid_str + " SUCCESS " + info);
          else
            System.out.println(action + " " + jid_str + " FAILURE " + info);
        }
      }
    }
    catch (IOException e)
    {
      System.out.println(action + " " + jid_str + " FAILURE " + e.getMessage());
    }
    catch (Exception e)
    {
      System.out.println(action + " " + jid_str + " FAILURE " + e.getMessage());
    }
  }

  int init () {
    // It's necessary to do nothing here
    int status = 0;

    info = "-";

    return status;
  }

  int submit (String jid_str, String resource, String rsl) {
    int status = 0;
    int index = 0;
    GramJob job = null;
    Integer jid = null;

    try
    {
      info = "-";

      // Default factory type is Fork
      String factoryType = ManagedJobFactoryConstants.DEFAULT_FACTORY_TYPE;

      // Parse resource string
      String resource_split[] = resource.split("/");
      String host = resource_split[0];

      if (resource_split.length == 2)
         factoryType = resource_split[1];

      // Create a job from its RSL
      job = new GramJob(rsl);
      jid = new Integer(jid_str);

      // Create Endpoint Reference to ManagedJobFactoryService with its
      //associated ManagedJobFactoryResource
      URL factoryURL =
          ManagedJobFactoryClientHelper.getServiceURL(host).getURL();
      EndpointReferenceType factoryEndpoint =
          ManagedJobFactoryClientHelper.getFactoryEndpoint(factoryURL,
          factoryType);

      // Add state listener
      job.addListener(this);

      // Submit the job
      job.submit(factoryEndpoint, false, false, null);

      // Add job and handle to the pools
      job_pool.put(jid, job);
      jid_pool.put(job.getHandle(), jid);
    }
    catch (NumberFormatException e)
    {
      info = e.getMessage();
      status = 1;
    }
    catch (Exception e)
    {
      info = e.getMessage();
      status = 1;
    }

    return status;
  }

  int cancel (String jid_str) {
    int status = 0;
    GramJob job = null;
    Integer jid = null;

    info = "-";

    try
    {
      // Get job from JID
      jid = new Integer(jid_str);
      job = (GramJob) job_pool.get(jid);

      if (job == null)
      {
        status = 1;
        info = "Job does not exist";
      }
      else
      {
        // Cancel the job
        job.cancel();
      }
    }
    catch (NumberFormatException e)
    {
      info = e.getMessage();
      status = 1;
    }
    catch (Exception e)
    {
      info = e.getMessage();
      status = 1;
    }

    return status;
  }

  int poll(String jid_str) {
    int status = 0;
    int index = 0;
    GramJob job = null;
    Integer jid = null;

    try
    {
      // Get the job from the JID
      jid = new Integer(jid_str);
      job = (GramJob) job_pool.get(jid);

      if (job == null)
      {
        status = 1;
        info = "Job " + jid + " does not exist";
      }
      else
      {
        // Get job status
        job.refreshStatus();
        StateEnumeration jobState = job.getState();
        info = jobState.getValue().toUpperCase();
	
	if (jobState == StateEnumeration.Unsubmitted)
          info = "PENDING";
      }
    }
    catch (NumberFormatException e)
    {
      info = e.getMessage();
      status = 1;
    }
    catch (Exception e)
    {
      info = e.getMessage();
      status = 1;
    }

    return status;
  }

  int finalize_mad() {
    int status = 0;

    info = "-";

    return status;
  }

  // Method from interface GramJobListener
  public void stateChanged(GramJob job) {
    int status;
    String info;
    
    // Get job state
    StateEnumeration jobState = job.getState();
    String jobState_str = jobState.getValue().toUpperCase();

    // Get the JID from the job
    Integer jid = (Integer) jid_pool.get(job.getHandle());

    if (jid == null)
    {
      status = 1;
      info = "Job " + job.getHandle() + " does not exist";
    }
    else
    {
      if (jobState == StateEnumeration.Unsubmitted)
        jobState_str = "PENDING";

      if (jobState == StateEnumeration.Failed)
      {
        // Check if the reason is "User cancelled"
        if (job.getFault() == null)
        {
          status = 0;
          info = StateEnumeration._Done.toUpperCase();
        }
        else
        {
          status = 1;
          info = job.getFault().getDescription(0).toString();
        }
      }
      else
      {
        status = 0;
        info = jobState_str;
      }

      // If the job is done, remove it from the pool
      if (jobState == StateEnumeration.Done
          || jobState == StateEnumeration.Failed)
      {
        job_pool.remove(jid);
        jid_pool.remove(job.getHandle());

        try
        {
          job.destroy();
        }
        catch (Exception e)
        {
          status = 1;
          info = e.getMessage();
        }
      }
    }

    synchronized(this) {
      if (status == 0)
        System.out.println("CALLBACK " + jid + " SUCCESS " + info);
      else
        System.out.println("CALLBACK " + jid + " FAILURE " + info);
    }
  }
}

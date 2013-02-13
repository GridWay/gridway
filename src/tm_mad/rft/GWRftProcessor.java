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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;


/**
 * RFT driver for GridWay. The commands are read from the standard input. 
 * Each line is a command.
 *
 */
public class GWRftProcessor{


	public static boolean end = false;
	

	/**
	 * Parses the line to execute the command (the sintax is the same that GW used in the GridFTP driver)
	 * @param line
	 */
	public static boolean processLine(String line) {
		boolean status;
		String info = "";
		
	    String action;
	    int xfr_id = 0;
	    String cp_xfr_id_s;
	    int cp_xfr_id;
	    char modex;
	    String src_url;
	    String dst_url;

		
		StringTokenizer st = new StringTokenizer(line, " ");
		List list = new ArrayList(); 
		int elem = 0;
		
		while (st.hasMoreElements()) {
			list.add(st.nextToken());
			elem++;
		}
		
		if (elem != 6) {
			System.out.println("FAILURE Not all four arguments defined");
			return false;
		}
		
		action = (String)list.get(0);
		try {
			xfr_id = Integer.parseInt( (String)list.get(1) );
		}catch (NumberFormatException nfe) {
			//System.out.println("FAILURE xfr_id must be an integer");
			//return false;
		}
		cp_xfr_id_s = (String)list.get(2);
		modex = ((String)list.get(3)).charAt(0);
		src_url = (String)list.get(4);
		dst_url = (String)list.get(5);
		
        if (action.equals("INIT")) {
            try{
            	status = GWRftClient.gw_init(xfr_id);
            }catch (Exception ex){
            	status = true;
            	//ex.printStackTrace();
            	info += ex.getMessage();
            }
            if (status) {
            	info += "Error delegating credentials";
            } else {
            	System.out.println("INIT - - SUCCESS -");
            }
        } else if (action.equals("START")) {
        	status = GWRftClient.gw_add_xfr(xfr_id);
            if (status) {
            	info += "Transfer " +  xfr_id + " , already started, or invalid id";
            } else {
            	System.out.println("START "+ xfr_id + " - SUCCESS -");
            }
        } else if (action.equals("END")) {
			status = GWRftClient.gw_del_xfr(xfr_id);
            if (status) {
            	info += "Transfer " + xfr_id + ", does not exists, or invalid id";
            }
        } else if (action.equals("RMDIR")) {
        	if (GWRftClient.gw_pool == null || GWRftClient.gw_pool.size() <= xfr_id || GWRftClient.gw_pool.get(xfr_id) == null)
			{
				status = true;
				info += "Transfer " + xfr_id + ", does not exists, or invalid id";
			}
			else
			{
				List list_xfr = (List)GWRftClient.gw_pool.get(xfr_id);
				
				GWRftResource xfr = new GWRftResource();
				list_xfr.add(xfr);
				xfr.setAction(action + " " + xfr_id + " - ");
				try{
					status = GWRftClient.gw_transfer_rmdir(xfr, src_url);
				}catch(Exception ex) {
	            	status = true;
	            	//ex.printStackTrace();
	            	info += ex.getMessage();
				}
				if (status) {
					info += "Error in MKDIR command, not supported by server?";
				}
			}
        }
//        else if (strcmp(action, "EXISTS") == 0 )
//        {
//			if (gw_tm_ftp_xfr_pool[xfr_id] == NULL)
//			{
//				status = 1;
//				sprintf(info,"Transfer %i, does not exists, or invalid id\n",xfr_id);
//			}
//			else
//			{
//				status = gw_tm_ftp_transfer_exists_dir(gw_tm_ftp_xfr_pool[xfr_id],src_url);	
//				if ( status == 1 )
//					sprintf(info,"Error in EXISTS command, not supported by server?\n");
//			}
//        }        
        else if (action.equals("MKDIR")) {
// 			if (gw_tm_ftp_xfr_pool[xfr_id] == NULL)
 			if (GWRftClient.gw_pool == null || GWRftClient.gw_pool.size() <= xfr_id || GWRftClient.gw_pool.get(xfr_id) == null)
			{
				status = true;
				info += "Transfer " + xfr_id + ", does not exists, or invalid id";
			}
			else
			{
				List list_xfr = (List)GWRftClient.gw_pool.get(xfr_id);

				GWRftResource xfr = new GWRftResource();
				list_xfr.add(xfr);
				xfr.setAction(action + " " + xfr_id + " - ");
				try {
					status = GWRftClient.gw_transfer_mkdir(xfr, src_url);	
				}catch(Exception ex) {
	            	status = true;
	            	//ex.printStackTrace();
	            	info += ex.getMessage();
				}
				if (status)
					info += "Error in MKDIR command, not supported by server?";
			}
        }                
        else if (action.equals("CP")) {
// 			if (gw_tm_ftp_xfr_pool[xfr_id] == NULL)
 			if (GWRftClient.gw_pool == null || GWRftClient.gw_pool.size() <= xfr_id || GWRftClient.gw_pool.get(xfr_id) == null)
			{
				status = true;
				info += "Transfer " + xfr_id + ", does not exists, or invalid id";
			}
			else
			{
				cp_xfr_id = Integer.parseInt(cp_xfr_id_s);
				
				List list_xfr = (List)GWRftClient.gw_pool.get(xfr_id);

				GWRftResource xfr = new GWRftResource();
				list_xfr.add(xfr);
				xfr.setAction(action + " " + xfr_id + " " + cp_xfr_id + " ");

				try {
					status = GWRftClient.gw_transfer_url_to_url(xfr,
									                cp_xfr_id, 
									                src_url,
									                dst_url,
									                modex);
				}catch(Exception ex) {
	            	status = true;
	            	//ex.printStackTrace();
	            	info += ex.getMessage();
				}
				if (status) {
					System.out.println("CP " + xfr_id + " " + cp_xfr_id + " FAILURE Error in GASS URL2URL copy command: "+info);
				}
				status = false;
			}
        } else if (action.equals("FINALIZE")) {
            end    = true;
            status = false;
        } else {
            status = true;
            info += "Not valid action";
        }

        if (status) {
            System.out.println(action + " " + xfr_id + " - FAILURE " + info);
		}

		
		return status;
	}
	


    
	/**
	 * @param args
	 */
	public static void main(String[] args) {

		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
		
		try {

			String line = null;
			do {
				line = in.readLine();
				if (!line.startsWith("#")) {
					processLine(line);
				}
			}while (!end && line != null);
		
			in.close();
		
		} catch (IOException ioe) {
			System.err.println("IOException: " + ioe.getMessage());
			System.exit(-1);
		}
	}

}

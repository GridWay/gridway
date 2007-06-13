/* -------------------------------------------------------------------------- */
/* Copyright 2002-2007 GridWay Team, Distributed Systems Architecture         														*/
/* Group, Universidad Complutense de Madrid                                   																	*/
/*                                                                            																							*/
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    														*/
/* not use this file except in compliance with the License. You may obtain    															*/
/* a copy of the License at                                                   																				*/
/*                                                                            																							*/
/* http://www.apache.org/licenses/LICENSE-2.0                                 																	*/
/*                                                                            																							*/
/* Unless required by applicable law or agreed to in writing, software        																*/
/* distributed under the License is distributed on an "AS IS" BASIS,          																*/
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   												*/
/* See the License for the specific language governing permissions and        															*/
/* limitations under the License.                                             																				*/
/* -------------------------------------------------------------------------- */


/*
 * 
 * Main Class: Only creates a JSDLTranslator object and execute the doTranslation method. 
 *
 */

public class JSDLParser 
{

	public static void main(String[] args) 
	{
		JSDLTranslator jsdlTranslator;
		try
		{
			if (args.length==0 || args.length > 2)
				throw new Exception("USE: JSDLParser JsdlFileName [GwjtFileName]");
			else if (args.length==1)
				jsdlTranslator = new JSDLTranslator(args[0]);
			else
				jsdlTranslator = new JSDLTranslator(args[0], args[1]);
					
			jsdlTranslator.doTranslation();
						    
		}
		catch(Exception e)
		{
			System.out.println(e.getMessage());	
		}
		    
	}

}

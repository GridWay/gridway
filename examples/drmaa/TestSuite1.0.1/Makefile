#
# Makefile
#

# -------------------------------------------------------------------------- 
# Copyright 2002-2011, GridWay Project Leads (GridWay.org)                   
#                                                                            
# Licensed under the Apache License, Version 2.0 (the "License"); you may    
# not use this file except in compliance with the License. You may obtain    
# a copy of the License at                                                   
#                                                                            
# http://www.apache.org/licenses/LICENSE-2.0                                 
#                                                                            
# Unless required by applicable law or agreed to in writing, software        
# distributed under the License is distributed on an "AS IS" BASIS,          
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   
# See the License for the specific language governing permissions and        
# limitations under the License.                                             
# -------------------------------------------------------------------------- 

# -----------------------------
# Directory definitions
# -----------------------------

DRMAA              = $(GW_LOCATION)/lib/drmaa.jar

TEST_SUITE_JAVA    = org/ggf/drmaa/testSuite
TEST_SUITE_PACKAGE = package
JAR_TARGET         = testDrmaa.jar
#--------------------
#  targets
#--------------------

TestSuite: 
	@mkdir -p $(TEST_SUITE_PACKAGE)
	javac $(TEST_SUITE_JAVA)/*.java -classpath $(DRMAA) -d $(TEST_SUITE_PACKAGE)
#	
	echo "Class-path: $(DRMAA)" > testSuiteManifest
	echo "Main-class: org.ggf.drmaa.testSuite.TestDrmaa" >> testSuiteManifest
#	
	jar cvfm $(JAR_TARGET) testSuiteManifest -C $(TEST_SUITE_PACKAGE)/ .
#
	@rm -rf $(TEST_SUITE_PACKAGE)
	@rm -rf testSuiteManifest

clean:
	@rm -f $(JAR_TARGET)

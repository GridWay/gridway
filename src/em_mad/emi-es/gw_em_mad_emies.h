/* -------------------------------------------------------------------------- */
/* Copyright 2002-2013 GridWay Project Leads (GridWay.org)                    */
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

#ifndef EMIESEMMAD_H_
#define EMIESEMMAD_H_
#define MAX_THREADS 1024

#include <string>
#include <EMIESClient.h>

struct EMIESOperation
{
    int code;
    std::string info;
    Arc::EMIESJob *job;
};

class EMIESService
{
    private:
        Arc::UserConfig usercfg;
        Arc::EMIESClients clients;

    public:
        EMIESService();
        EMIESOperation submit(std::string serviceAddress, std::string adlFile);
        EMIESOperation poll(std::string emiesJid, std::string serviceAddress);
        EMIESOperation cancel(std::string emiesJid, std::string serviceAddress);
};

class EMIESEmMad
{
    private:
        std::map <int, Arc::EMIESJob*> emiesJobs;
        pthread_mutex_t jobMutex;
        EMIESService *emiesService;

    public:
        EMIESEmMad();
        void init();
        void submit(int jid, std::string contact, std::string adlFile);
        void recover(int jid, std::string contact);
        void cancel(int jid);
        void poll(int jid);
        void finalize();
        std::string fileToString(std::string adlFileName);
};

#endif /*EMIESEMMAD_H_*/

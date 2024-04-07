/*

DISTRIBUTED SYSTEM ASSIGNMENT 5

AMAN UTKARSH

IIB2020027

*/

#include<iostream>

#include<vector>

#include<unordered_map>

#include<set>

#include <fstream>



using namespace std;

int main()

{

    

    /*User input*/

    int  processCount,maxEventCount;

    cout<<"Enter the number of processes\n";

    cin>>processCount;

    cout<<"Enter the max event count\n";

    cin>>maxEventCount;

    

    /*Type of events*/

    enum eventTypes{

            INTERNAL,

            SEND,

            RECIEVE

    };

    vector<string>type{

            "INTERNAL",

            "SEND",

            "RECIEVE"

    };



    

    /*determine number of events in processes*/

    vector<pair<eventTypes,int>>processFlow[processCount];

    vector<int>sz(processCount,0);

    for(int i = 0;i < processCount; i++)

    {

        sz[i] = 1+ (int)((rand()) % maxEventCount);

    }

    

    cout<<"Event count per process\n";

    for(int i=0;i<processCount;i++)

       cout<<(i+1)<<" -> "<<sz[i]<<"\n";

    

    /*intialize all events internal*/

    for(int i=0;i<processCount;i++)

    {

        processFlow[i] = vector<pair<eventTypes,int>>(sz[i],make_pair(INTERNAL,0));

    }



    

    /*under processing processes and their progress*/

    set<int>processes;

    unordered_map<int,int>progress;

    for(int i=0;i<processCount;i++)

    {

        if(sz[i] > 1)   processes.insert(i);

        progress[i] = 1;

    } 

    

    /*processing prcoesses until space for message passing left*/

    

    while(processes.size()>1)

    {

        set<int>workingSet = processes;

        for(int process : processes)

        {

            int jump = 0;

            if((sz[process] - progress[process] -1)>0)

                 jump = rand() % (sz[process] - progress[process] -1);

        

            int currEvent = progress[process] + jump; 

            progress[process] = currEvent + 1;

           

            int nodeType = rand() % 2;

            

            if(nodeType==1 && workingSet.size()>1) // not internal i.e send

            {

                workingSet.erase(process);

                auto it = workingSet.begin();

                int jump = rand()%workingSet.size();

                advance(it,jump);

                int reciever = *it;

                workingSet.insert(process);



                processFlow[process][currEvent] = {SEND,reciever};

                processFlow[reciever][progress[reciever]] = {RECIEVE,process};

                progress[reciever]++;

                

                if(progress[reciever]==sz[reciever])

                {

                    workingSet.erase(reciever);

                }



            }

            if(progress[process]==sz[process])

            {

                workingSet.erase(process);

            }

        }

        processes = workingSet;

    }

    cout<<"\n";

    ofstream outdata; 

    outdata.open("randomDis.txt");

    outdata<<processCount;

    outdata<<endl;

    for(int i = 0;i<processCount;i++)

    {   

        outdata<<sz[i]<<endl;

        for(auto k : processFlow[i])

        {

            outdata<<type[k.first];

            outdata<<" ";

            if(k.first) outdata<<k.second;

            outdata<<endl;

        }

        

    }

    outdata.close();



    

    for(int i = 0;i<processCount;i++)

    {   cout<<"process "<<i<<"    ";

        for(auto k : processFlow[i])

        {

            cout<< type[k.first]<<" ";

            if(k.first) cout<<k.second;

            cout<<" | ";

        }

        cout<<"\n";

    }





    

    return 0;

}
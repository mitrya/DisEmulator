/*
AMAN UTKARSH
IIB2020027

*/
#include <iostream>

#include <fstream>

#include<cstring>
#include<cctype>
#include<string>
#include <string>
#include <vector>
#include <queue>
#include <map>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <chrono>
#include <thread>

using namespace std;

const int PORT = 7000;
const int MAX_PROCESS_COUNT = 100;

struct Event {
    string type;
    int scalarTime;
    vector<int>vectorTime;
    int meta;   // destination if sending, sender if receiving
};

// Message structure for passing between processes
struct Message {
    int sender;
    int receiver;
    int scalarTime;
    char* vectorTime;
    char* body;
};

vector<queue<Message>>messageQueue(MAX_PROCESS_COUNT);

class Process{
    int id;
public:
    vector<Event> events;

    Process()
    {
        events = vector<Event>(0);
    }
    void setID(int _id)
    {
        id = _id;
    }
    int getID()
    {
        return id;
    }
};

// Function to delay for given amount of milliseconds
void delay(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}


void simulate_send_event(Process process, int event_num, int receiver, Message message) {
    int process_id = process.getID();
    cout << "P" << process_id << ": " << event_num << " - sending message to P" << receiver << ": " << message.body<<" "<< message.vectorTime<< endl;
    cout<<"send to port = "<<PORT + receiver<<"\n";
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT + receiver);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
    // Send message
    //int sock = process.getChannels()[receiver];
    cout<<"send f val = "<<send(sockfd, &message, sizeof(message), 0)<<"\n";
}

void simulate_receive_event(int process_id, int event_num, int sender) {
    cout << "P" << process_id << ": " << event_num << " - recieved message from P" << sender << endl;
}

void listenMessages(int reciever_id)
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[sizeof(Message)];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT + reciever_id);  

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
         
    cout<<reciever_id<<" listening to PORT"<<PORT + reciever_id<<"\n";

    while(1)
    {
        listen(server_fd, 1);
        // Wait for message from sender
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        
        int ret = read(new_socket, buffer, sizeof(buffer));
        if(ret==0 || ret ==-1)
            continue;
        Message message = *(reinterpret_cast<Message*>(buffer));
        messageQueue[reciever_id].push(message);
   }
    close(new_socket);
}

string serializeVectorTime(vector<int>&vectorTime)
{
    string res = "";
    res+='[';

    for(int t : vectorTime)
    {
        res+=to_string(t);
        res+=", ";
    }
    res.pop_back();
    res.pop_back();
    res+=']';
    return res;
}

vector<int> deserializeVectorTime(char* c)
{
    string s = string(c);
    //TIME [2, 0, 0, 0, 0, 0, 0]
    cout<<"for desez string s = "<<s<<"\n";
    vector<int>res;
    int i = 1;
    while(s[i]!=']')
    {
        string temp="";
        while(s[i]!=']' && isdigit(s[i]) )
        {
            temp+=s[i];
            i++;
        }
        if(temp.size())
        {
            res.push_back(stoi(temp));
        }
        while(s[i]!=']' && !isdigit(s[i]))
        {
            i++;
        }
    }
    for(int i : res)
    {
        cout<<i<<" ";
    }
    cout<<"end des\n";
    return res;
}
int main(int argc, char* argv[]) {

    
    // Read the input file
    ifstream input_file("randomDis.txt");
    int process_count, max_events;
    input_file >> process_count;


    // Create an array of processes
    vector<Process> processes(process_count);

    for (int i = 0; i < process_count; i++) {
        processes[i].setID(i);
    }
    
    // Parse the input file to initialize each process's events
    for (int i = 0; i < process_count; i++) {
        int event_count;
        input_file >> event_count;
        for (int j = 0; j < event_count; j++) {
            string event_type;
            input_file >> event_type;
            Event event;
            event.type = event_type;
            if (event_type == "SEND" || event_type == "RECIEVE")
            {
                int meta;
                input_file >> meta;
                event.meta = meta;
            }
            processes[i].events.push_back(event);
        }
    }
    
    // Choose the clock implementation
    int clockType;
    cout<<"Choose the logical clock\n\n";
    cout<<"1 : Scalar Clock\n";
    cout<<"2 : Vector Clock\n\n";
    cin>>clockType;    

    pid_t pid[process_count]; 
    
    for(int i = 0;i < process_count; i++)
    {
        pid[i] = fork();
        
        if(pid[i] == 0)
        {
            static int curr = i;
            
            // Delay for a random amount of time
            int delay_time = rand() % 1000;
            delay(delay_time);

            int eventCount = 1;
            thread t3(listenMessages,curr);
            delay(500);

            ofstream outdata; 
            string fileName = "log"+to_string(curr);
            outdata.open(fileName);
            
            //time for the process starting with zero;
            int scalarCurrTime = -1;
            vector<int>vectorCurrTime(process_count,0);

            for(Event event : processes[curr].events)
            {
                cout<<"PROCESS:"<<curr<<" EVENT:"<<event.type<<"\n\n";
                eventCount++;
                if(event.type=="INTERNAL")
                {
                    string timeOutput;
                    if(clockType==1)
                        {
                            scalarCurrTime++;
                            event.scalarTime = scalarCurrTime;
                            timeOutput = to_string(scalarCurrTime);
                        }
                    else 
                        {
                            vectorCurrTime[curr]++;
                            event.vectorTime = vectorCurrTime;
                            timeOutput = serializeVectorTime(vectorCurrTime);
                        }
                    cout<<"INTERNAL event in process "<<curr<<" at time: "<<timeOutput<<"\n\n";
                    outdata<<"INTERNAL event in process "<<curr<<endl;
                    outdata<<"TIME "<<timeOutput<<endl<<endl;
                }
                else if(event.type=="SEND")
                {
                    Message send_msg;
                    send_msg.sender=curr;
                    send_msg.receiver=event.meta;
                    
                    send_msg.body=new char[sizeof("message")];
                    sprintf(send_msg.body,"%s","message");
                    
                    string timeOutput;
                    
                    if(clockType==1)
                        {
                            scalarCurrTime++;
                            event.scalarTime = scalarCurrTime;
                            send_msg.scalarTime = scalarCurrTime;
                            timeOutput = to_string(scalarCurrTime);
                        }
                    else 
                        {
                            vectorCurrTime[curr]++;
                            event.vectorTime = vectorCurrTime;
                            
                            string serializedVectorTime = serializeVectorTime(vectorCurrTime);
                            cout<<"for sending ser vect time = "<<serializedVectorTime<<"\n";
                            send_msg.vectorTime = new char[serializedVectorTime.size()+1];
                            strcpy(send_msg.vectorTime,serializedVectorTime.c_str());
                            
                            timeOutput = serializedVectorTime;
                        }
                   
                    
                    
                    simulate_send_event(processes[curr],eventCount,event.meta,send_msg);
                    
                    cout<<"SEND event in process "<<curr<<"\n";
                    cout<<"FROM = "<<curr<<" TO = "<<event.meta<<"\n";
                    cout<<"TIME = "<<timeOutput<<"\n\n";

                    outdata<<"SEND event in process "<<curr<<endl;
                    outdata<<"FROM "<<curr<<endl;
                    outdata<<"TO "<<event.meta<<endl;
                    outdata<<"TIME "<<timeOutput<<endl<<endl;
                }
                else{
                    while(messageQueue[curr].size()==0)
                    {
                        delay(1000);
                    }
                    
                    if(clockType==2)cout<<"q val"<<messageQueue[curr].front().vectorTime<<"\n";
                    Message recv_msg = messageQueue[curr].front();
                    messageQueue[curr].pop();
                    
                    simulate_receive_event(curr,eventCount,event.meta);
                    string timeOutput;
                    
                    if(clockType==1)
                        {
                            scalarCurrTime = max(scalarCurrTime,recv_msg.scalarTime)+1;
                            event.scalarTime = max(scalarCurrTime,recv_msg.scalarTime)+1;
                            timeOutput = to_string(scalarCurrTime);
                        }
                    else 
                        {
                            cout<<"recv messg for desx = "<<recv_msg.vectorTime<<"\n";
                            vector<int> recvVectorTime = deserializeVectorTime(recv_msg.vectorTime);
                            
                            for(int h=0;h<process_count;h++)
                            {
                                vectorCurrTime[h] = max(vectorCurrTime[h],recvVectorTime[h]);
                            }
                            vectorCurrTime[curr]++;
                            event.vectorTime = vectorCurrTime;
                            timeOutput = serializeVectorTime(vectorCurrTime);
                        }
                    cout<<"RECV event in process "<<curr<<"\n";
                    cout<<"FROM ="<<event.meta<<" TO = "<<curr<<"\n";
                    cout<<"TIME = "<<timeOutput<<"\n\n";

                    outdata<<"RECV event in process "<<curr<<endl;
                    outdata<<"FROM "<<event.meta<<endl;
                    outdata<<"TO "<<curr<<endl;
                    outdata<<"TIME "<<timeOutput<<endl<<endl;
               }
                
                // Delay for a constant amount of time
                delay(500);
            }
            
            exit(0);
        }
        else if (pid[i] < 0){
            perror("Error while forking");
        exit(1);
    }

    }
return 0;
}



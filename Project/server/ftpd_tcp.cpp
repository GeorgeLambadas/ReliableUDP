//    SERVER TCP PROGRAM
// revised and tidied up by
// J.W. Atwood
// 1999 June 30
// There is still some leftover trash in this code.
 
/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma once
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <winsock.h> 
#include <iostream>
#include <windows.h>
#include <fstream>
#include <sstream>

using namespace std;
 
//port data types
 
#define REQUEST_PORT 0x7070

 
int port=REQUEST_PORT;
int max_packet_size = 128;
 
//socket data types
SOCKET s;
 
SOCKET s1;
SOCKADDR_IN sa;      // filled by bind
SOCKADDR_IN sa1;     // fill with server info, IP, port
union {struct sockaddr generic;
        struct sockaddr_in ca_in;}ca;
 
        int calen=sizeof(ca);
 
        //buffer data types
        char szbuffer[128];
 
        char *buffer;
        int ibufferlen;
        int ibytesrecv;
 
        int ibytessent;
 
        //host data types
        char localhost[21];
 
        HOSTENT *hp;
 
        //wait variables
        int nsa1;
        int r,infds=1, outfds=0;
        struct timeval timeout;
        const struct timeval *tp=&timeout;
 
        fd_set readfds;
 
        //others
        HANDLE test;
 
        DWORD dwtest;
 
        //reference for used structures
 
        /*  * Host structure
 
            struct  hostent {
            char    FAR * h_name;             official name of host *
            char    FAR * FAR * h_aliases;    alias list *
            short   h_addrtype;               host address type *
            short   h_length;                 length of address *
            char    FAR * FAR * h_addr_list;  list of addresses *
#define h_addr  h_addr_list[0]            address, for backward compat *
};
 
         * Socket address structure
 
         struct sockaddr_in {
         short   sin_family;
         u_short sin_port;
         struct  in_addr sin_addr;
         char    sin_zero[8];
         }; */
 
        int main(void){
 
                WSADATA wsadata;
 
                try{                     
                        if (WSAStartup(0x0202,&wsadata)!=0){  
                                cout<<"Error in starting WSAStartup()\n";
                        }else{
                                buffer="WSAStartup was suuccessful\n";  
                                WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL);
 
                                /* display the wsadata structure */
                                /*
                                cout<< endl
                                        << "wsadata.wVersion "       << wsadata.wVersion       << endl
                                        << "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
                                        << "wsadata.szDescription "  << wsadata.szDescription  << endl
                                        << "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
                                        << "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
                                        << "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl;
                                */
                        }  
 
                        //Create the server socket
                        if((s = socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET)
                                throw "can't initialize socket";
                        // For UDP protocol replace SOCK_STREAM with SOCK_DGRAM
 
 
                        //Fill-in Server Port and Address info.
                        sa.sin_family = AF_INET;
                        sa.sin_port = htons(port);
                        sa.sin_addr.s_addr = htonl(INADDR_ANY);
 
                        //Get host name
                        gethostname(localhost, 20);
 
                        //Bind the server port
 
                        if (bind(s,(LPSOCKADDR)&sa,sizeof(sa)) == SOCKET_ERROR)
                                throw "can't bind the socket";
                        else
                                cout << "ftpd_tcp starting at host: [" << localhost << "]" << endl;
 
                        //Successfull bind, now listen for client requests.
 
                        if(listen(s,10) == SOCKET_ERROR)
                                throw "couldn't  set up listen on socket";
                        else
                                cout << "waiting to be contacted for transferring files..." << endl;
 
                        if ((hp = gethostbyname(localhost)) == NULL) {
                                cout << "gethostbyname() cannot get local host info?"
                                        << WSAGetLastError() << endl;
                                exit(1);
                        }
 
                        FD_ZERO(&readfds);
 
                        //wait loop
 
						

                        while(1)
 
                        {
                            FD_SET(s,&readfds);  //always check the listener
 
                            if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}
 
                            else if (outfds == SOCKET_ERROR) throw "failure in Select";
 
                            else if (FD_ISSET(s,&readfds))  cout << "got a connection request" << endl;							

                            //Found a connection request, try to accept.
                            if((s1=accept(s,&ca.generic,&calen))==INVALID_SOCKET)
                                    throw "Couldn't accept connection\n";
 
                            //Fill in szbuffer from accepted request.
                            if ((ibytesrecv = recv(s1, szbuffer, 128, 0)) == SOCKET_ERROR)
                                    throw "Receive error in server program\n";
 
                            int currentPos = 0;
 
                            std::string tempBuffer(szbuffer);
 
                            std::string file_name = tempBuffer.substr(currentPos, tempBuffer.find(',') - currentPos);
                            currentPos = tempBuffer.find(',') + 1;
                            std::string client = tempBuffer.substr(currentPos, tempBuffer.find(',', currentPos) - currentPos);
                            currentPos = tempBuffer.find(',', currentPos)+1;
                            std::string command = tempBuffer.substr(currentPos, tempBuffer.find(',', currentPos) - currentPos);
                            currentPos = tempBuffer.find(',', currentPos) + 1;
                            std::string user_name = tempBuffer.substr(currentPos);
 
							if (command.compare("get") == 0) { //prepare to send file
								memset(szbuffer, '\0', max_packet_size);
								cout << "User " << user_name.c_str() << " requested file " << file_name.c_str() << " to be sent." << endl;

								//actually send the file
								string filename_string = "server/";
								filename_string.append(file_name);

								FILE *fs = fopen(filename_string.c_str(), "rb");
								if (fs != NULL) {
									cout << "Sending file to " << client.c_str() << ", waiting..." << endl;
									//let client know file was found and will be sent
									szbuffer[0] = 's';
									send(s1, szbuffer, max_packet_size, 0);
									memset(szbuffer, '\0', max_packet_size);
										
									//send file
									int file_size;
									while ((file_size = fread(szbuffer, sizeof(char), max_packet_size, fs)) > 0) {
										send(s1, szbuffer, file_size, 0);
										memset(szbuffer, '\0', max_packet_size);
									}
									fclose(fs);
										
									cout << "File " << file_name.c_str() << " has been sent." << endl;
								} else {
										
									cout << "File " << file_name.c_str() << " not found on server." << endl;
									//let client know file was not found
									szbuffer[0] = 'n';
									send(s1, szbuffer, max_packet_size, 0);
									memset(szbuffer, '\0', max_packet_size);
								}
                            } else if (command.compare("put") == 0) {
								memset(szbuffer, '\0', max_packet_size);
								cout << "User " << user_name.c_str() << " requested to upload file " << file_name.c_str() << "." << endl;
                                cout << "Receiving file from " << client.c_str() << ", waiting..." << endl;

								ibytesrecv=0;

								if ((ibytesrecv = recv(s1,szbuffer,128,0)) > 0) {
									char response = szbuffer[0];
									memset(szbuffer, 0, max_packet_size);
									if (response == 's') {
										string filename_string = "server/";
										filename_string.append(file_name);

										FILE *fs = fopen(filename_string.c_str(), "wb");

										while ((ibytesrecv = recv(s1,szbuffer,128,0)) > 0) {
											fwrite(szbuffer, sizeof(char), ibytesrecv, fs);
											memset(szbuffer, 0, max_packet_size);
										}
										fclose(fs);
										cout << "File " << file_name.c_str() << " received and saved successfully." << endl;
									} else if (response == 'n') {
										//file not found
										cout << "File " << file_name.c_str() << " not found on client." << endl;
									}
								}
							} else if (command.compare("list") == 0) { //send directory listing
								memset(szbuffer, '\0', max_packet_size);
							cout << "User " << user_name.c_str() << " requested directory listing." << endl;
							system("dir server /b >> server\\list.txt");     //get directory listing
                            std::string listing;
                            std::ifstream file("server\\list.txt");   //transmit directory listing
                            std::string line;
                            if (!file.eof()) {
                                    getline(file, line);
                                    listing = line;
                                    while (!file.eof()) {
                                            getline(file, line);
                                            if (line != "list.txt") {
                                                    listing = listing + "," + line;
                                            }
                                    }
                            }
                            sprintf_s(szbuffer, listing.c_str());
                            file.close();
 
							if ((ibytessent = send(s1, szbuffer, max_packet_size, 0)) == SOCKET_ERROR)
                                    throw "error in send in server program\n";
                            else
                                    cout << "Directory listing sent to " << client.c_str() << " successfully!" << endl;
 
                            system("del server\\list.txt");
							} else if (command.compare("delete") == 0) {
								cout << "User " << user_name.c_str() << " requested file " << file_name.c_str() << " to be deleted." << endl;

								std::string result;
								string filename_string = "server/";
								filename_string.append(file_name);
										
								if (FILE *fs = fopen(filename_string.c_str(), "rb")) {
									fclose(fs);
									string command = "del server\\" + file_name;
									cout << command << endl;
									system(command.c_str());
									result = "File successfully deleted.";
									cout << "User " << user_name.c_str() << " deleted file " << file_name.c_str() << "!" << endl;
								} else {
									result = "File not found.";
									cout << "Could not find file " << file_name.c_str() << " to be deleted." << endl;
								}
								 
								memset(szbuffer, '\0', max_packet_size);
								sprintf_s(szbuffer, result.c_str());
								send(s1, szbuffer, max_packet_size, 0);
							} else {
								//error message
							}
							cout << endl;
							closesocket(s1);
                        }//wait loop
 
                } //try loop
 
                //Display needed error message.
 
                catch(char* str) { cerr<<str<<WSAGetLastError()<<endl;}
 
                //close Client socket
                closesocket(s1);               
 
                //close server socket
                closesocket(s);
 
                /* When done uninstall winsock.dll (WSACleanup()) and exit */
                WSACleanup();
                return 0;
        }

		
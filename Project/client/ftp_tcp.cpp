// CLIENT TCP PROGRAM
// Revised and tidied up by
// J.W. Atwood
// 1999 June 30
 
 
 
char* getmessage(char *);
 
/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>
 
#include <winsock.h>
#include <stdio.h>
#include <iostream>
 
#include <string.h>	
 
#include <windows.h>
#include <conio.h>
#include "Thread.h"
 
using namespace std;
 
//user defined port number
#define REQUEST_PORT 0x7070;
 
int port=REQUEST_PORT;
int max_packet_size = 128;
 
 
 
//socket data types
SOCKET s;
SOCKADDR_IN sa;         // filled by bind
SOCKADDR_IN sa_in;      // fill with server info, IP, port
 
 
 
//buffer data types
char szbuffer[128];
 
char *buffer;
 
int ibufferlen=0;
 
int ibytessent;
int ibytesrecv=0;
 
 
 
//host data types
HOSTENT *hp;
HOSTENT *rp;
 
char localhost[21],
     remotehost[21],
         filename[64],
         direction[8];
 
 
//other
 
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
 
        try {
 
                if (WSAStartup(0x0202,&wsadata)!=0){  
                        cout<<"Error in starting WSAStartup()" << endl;
                } else {
                        buffer="WSAStartup was successful\n";  
                        WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL);
 
                        /* Display the wsadata structure */
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
 
 
                //Display name of local host.
 
                gethostname(localhost,20);
                cout<<"Local host name is \"" << localhost << "\"" << endl;
 
                if((hp=gethostbyname(localhost)) == NULL)
                        throw "gethostbyname failed\n";
 
                //Ask for name of remote server
				
                cout << "Type name of ftp server: " << flush ;  
                cin >> remotehost ;
				string hostname(remotehost);
				while(hostname.compare("quit") != 0) {
                cout << "Type name of file to be transferred: " << flush;
                cin >> filename;
                cout << "Type direction of transfer: " << flush;
                cin >> direction;
                cout << endl;
                cout << "Sent request to " << remotehost << ", waiting..." << endl;
 
                if((rp=gethostbyname(remotehost)) == NULL)
                        throw "remote gethostbyname failed\n";
 
                //Create the socket
                if((s = socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET)
                        throw "Socket failed\n";
                /* For UDP protocol replace SOCK_STREAM with SOCK_DGRAM */
 
                //Specify server address for client to connect to server.
                memset(&sa_in,0,sizeof(sa_in));
                memcpy(&sa_in.sin_addr,rp->h_addr,rp->h_length);
                sa_in.sin_family = rp->h_addrtype;  
                sa_in.sin_port = htons(port);
 
                //Display the host machine internet address
                /*
                cout << "Connecting to remote host:";
                cout << inet_ntoa(sa_in.sin_addr) << endl;
                */
 
                //Connect Client to the server
                if (connect(s,(LPSOCKADDR)&sa_in,sizeof(sa_in)) == SOCKET_ERROR)
                        throw "connect failed\n";
 
                char user_name[32];
                DWORD user_name_size = sizeof(user_name);
                if (!GetUserNameA(user_name, &user_name_size))
					cout << "Cannot find client user name!" << endl;
 
                /* Have an open connection, so, server is
 
                   - waiting for the client request message
                   - don't forget to append <carriage return>
                   - <line feed> characters after the send buffer to indicate end-of file */
 
                //append client message to szbuffer + send.
 
				//HACK
				string filename_2 = filename;
                sprintf_s(szbuffer,strcat(strcat(strcat(strcat(strcat(strcat(filename, ",") , localhost), ","), direction),","), user_name));

                ibytessent=0;    
                ibufferlen = strlen(szbuffer);
                ibytessent = send(s,szbuffer,ibufferlen,0);

                if (ibytessent == SOCKET_ERROR)
					throw "Send failed";
 
				string dir(direction);

				if (dir.compare("get") == 0) { //prepare to receive file
					ibytesrecv=0;

					if ((ibytesrecv = recv(s,szbuffer,128,0)) > 0) {
						char response = szbuffer[0];
						memset(szbuffer, 0, max_packet_size);

						if (response == 's') {
							string filename_string = "client/";
							filename_string.append(filename_2);

							FILE *fs = fopen(filename_string.c_str(), "wb");
							cout << "File transfer beginning." << endl;
							while ((ibytesrecv = recv(s,szbuffer,128,0)) > 0) {
								fwrite(szbuffer, sizeof(char), ibytesrecv, fs);
								memset(szbuffer, 0, max_packet_size);
							}
							cout << "File transfer complete." << endl;
							fclose(fs);
						} else if (response == 'n') {
							//file not found
							cout << "File " << filename_2.c_str() << " not found on server." << endl;
						}
					}
                } else if (dir.compare("put") == 0) { //prepare to send file
					string filename_string = "client/";
					filename_string.append(filename_2);

					FILE *fs = fopen(filename_string.c_str(), "rb");
					if (fs != NULL) {

						//let server know file was found and will  be sent
						szbuffer[0] = 's';
						send(s, szbuffer, max_packet_size, 0);
						memset(szbuffer, '\0', max_packet_size);

						//send file
						int file_size;
						cout << "File transmission beginning." << endl;

						while ((file_size = fread(szbuffer, sizeof(char), max_packet_size, fs)) > 0) {
							send(s, szbuffer, file_size, 0);
							memset(szbuffer, '\0', max_packet_size);
						}
						cout << "File transmission complete." << endl;
						fclose(fs);
					} else {
						cout << "File not found." << endl;
						//let server know file was not found
						szbuffer[0] = 'n';
						send(s, szbuffer, max_packet_size, 0);
							memset(szbuffer, '\0', max_packet_size);
					}
				} else if (dir.compare("list") == 0) {
					ibytesrecv = 0;
                    if ((ibytesrecv = recv(s, szbuffer, 128, 0)) == SOCKET_ERROR)
                            throw "Receive failed";
                    else {
                            cout << "Received file listing from the server!:" << endl;
 
                            int currentPos = 0;
 
                            std::string tempBuffer(szbuffer);
                            size_t pos;
 
                            while (pos != -1) {
                                    pos = tempBuffer.find(',');
                                    cout << tempBuffer.substr(0, pos).c_str() << endl;
                                    tempBuffer = tempBuffer.substr(pos + 1);
                            }
                    }
				} else if (dir.compare("delete") == 0) {
					memset(szbuffer, '\0', max_packet_size);

					ibytesrecv = 0;
                    if ((ibytesrecv = recv(s, szbuffer, 128, 0)) == SOCKET_ERROR) {
                            throw "Response from server failed.";
					}
                    else {
						cout << szbuffer << endl;
						memset(szbuffer, '\0', max_packet_size);

					}
				} else {
					cout << "command not recognized." << endl;
				}
                
                closesocket(s);
				cout << "\nType name of ftp server: " << flush ;  
                cin >> remotehost ;
				string hostname(remotehost);

				}
 
        } // try loop
 
        //Display any needed error response.
 
        catch (char *str) { cerr<<str<<":"<<dec<<WSAGetLastError()<<endl;}
 
        
 
        /* When done uninstall winsock.dll (WSACleanup()) and exit */
        WSACleanup();
        _getch();
        return 0;
}
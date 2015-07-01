
const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                              \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "IRCServer.h"

int QueueLength = 5;

struct User {
	char * name;
	char * password;
	struct User * next;
};

struct UserList {
	User * head;
};

struct Message {
	char * message;
	char * user;
	struct Message * next;
};

struct MessageList {
	Message * head;
};

//struct Room {
//	char * value;
//	UserList * members;
//	MessageList * message;
//	int messageCount;
//	struct Room * next;
//};
//
//struct RoomList {
//	Room * head;
//};

typedef struct Message Message;

typedef struct MessageList MessageList;

//typedef struct RoomList RoomList;
//
//typedef struct Room Room;

typedef struct UserList UserList;

typedef struct User User;

FILE * fpass;

int IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);
  
	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			    (char *) &optval, sizeof( int ) );
	
	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			  (struct sockaddr *)&serverIPAddress,
			  sizeof(serverIPAddress) );
	if (error) {
		perror("bind");
		exit(-1);
	}
	
	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if (error) {
		perror("listen");
		exit( 1);
	}

	return masterSocket;
}

void IRCServer::runServer (int port) {
	int masterSocket = open_server_socket(port);

	initialize();
	
	while (1) {
		
		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof(clientIPAddress);
		int slaveSocket = accept(masterSocket,
					  (struct sockaddr *)&clientIPAddress,
					  (socklen_t*)&alen);
		
		if (slaveSocket < 0) {
			perror("accept");
			exit(-1);
		}
		
		// Process request.
		processRequest(slaveSocket);		
	}
}

int main(int argc, char ** argv) {
	// Print usage if not enough arguments
	if (argc < 2) {
		fprintf(stderr, "%s", usage);
		exit(-1);
	}
	
	// Get the port from the arguments
	int port = atoi(argv[1]);

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);
}

void IRCServer::processRequest (int fd) {
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[MaxCommandLine + 1];
	//char * commandLine = (char *) malloc((MaxCommandLine + 1) * sizeof(char));
	int commandLineLength = 0;
	int n;
	
	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;
	
	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while (commandLineLength < MaxCommandLine && read(fd, &newChar, 1) > 0) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}
		
		commandLine[commandLineLength] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}
	
	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
    commandLine[commandLineLength] = 0;

	//char * copy = strdup(commandLine);

	printf("RECEIVED: %s\n", commandLine);

	printf("The commandLine has the following format:\n");
	printf("COMMAND <user> <password> <arguments>. See below.\n");
	printf("You need to separate the commandLine into those components\n");
	printf("For now, command, user, and password are hardwired.\n");

	char * command = (char *) malloc(1000 * sizeof(char));
	char * user = (char *) malloc(1000 * sizeof(char));
	char * password = (char *) malloc(1000 * sizeof(char));
	char * args = (char *) malloc(1000 * sizeof(char));

	int spaces[10];
	n = 0;
	int i = 0;
	
	for (i = 0; i < 10; i++) {
		spaces[i] = -10;
	}

	for (i = 0; commandLine[i] != '\0'; i++) {
		if (commandLine[i] == ' ')
			spaces[n++] = i;
	}

	char * p = commandLine;

	int j = 0;
	for (i = 0, j = 0; i < spaces[0]; i++, p++) {
		command[j++] = *p;
	}
	command[j] = '\0';

	p++;

	for (i = spaces[0] + 1, j = 0; i < spaces[1]; i++, p++) {
		user[j++] = *p;
	}
	user[j] = '\0';

	p++;

	for (i = spaces[1] + 1, j = 0; (spaces[2] > 0) ? (i < spaces[2]):(commandLine[i] != '\0'); i++, p++) {
		password[j++] = *p;
	}
	password[j] = '\0';

	p++;

	for (i = spaces[2] + 1, j = 0; i >= 0 && commandLine[i] != '\0'; i++, p++) {
		args[j++] = *p;
	}
	args[j] = '\0';
	
	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf("password=%s\n", password );
	printf("args=%s\n", args);

	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	}
	else if (!strcmp (command, "CREATE-ROOM")) {
		createRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	}
	else if (strcmp(command, "CHECK-LOGIN") == 0) { 
		checkLogin(fd, user, password, args);
	}
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	close(fd);	
}

void IRCServer::initialize() {
	// Open password file
	fpass = fopen("password.txt", "r+");  	

	// Initialize users in room
	//rlist->head = NULL;
	
	// Initalize message list

}

bool IRCServer::checkPassword(int fd, const char * user, const char * password) {
	// Here check the password	
	char * s = (char *) malloc(1000 * sizeof(char));
	char * p;
	char * prev = (char *) malloc (1000 * sizeof(char));

	int countu = 0;
	int countp = 0;
	
	fseek(fpass, 0, SEEK_SET);
	while((fscanf(fpass, "%s", s)) != 0) {
		if (strcmp(s, "") != 0 && strcmp (prev, s) != 0) {
			p = s;
		
			for (countu; countu != 0; countu--) {
				user--;
			}
	
			for (countp; countp != 0; countp--) {
				password--;
			}
		
			int flag = 0;
			countu = 0;
			countp = 0;

			while(*p != '/') {
				if (*p != *user) {
					flag = -1;
					break;
				}
				p++;
				user++;
				countu++;
			}
			if (flag == 0) {
				p++;
				while(*p != '\n' && *password != '\0') {
					if (*p != *password) {
						flag = -1;
						break;
					}
					p++;
					password++;
					countp++;
				}
				if (flag == 0) {
					return true;
				}
			}
			prev = strdup(s);
		}

		else {
			break;
		}
	}		
	return false;
}

void IRCServer::addUser(int fd, const char * user, const char * password, const char * args) {
	// Here add a new user. For now always return OK.
	
	User * u = (User *) malloc(sizeof(User *));
	
	u->name = strdup(user);
	u->password = strdup(password);

	char * output = (char *) malloc(100 * sizeof(char));
	

	int i = 0;
	for (i = 0; user[i] != '\0'; i++) {
		output[i] = user[i];
	}
		
	output[i++] = '/'; 	

	for (int j = 0; password[j] != '\0'; j++) {
		output[i++] = password[j];
	}

	output[i++] = '\n';
	output[i] = '\0';	
	
	char * s = (char *) malloc(1000 * sizeof(char));
	char * prev = (char *) malloc (1000 * sizeof(char));

	int flag  = 0;
	
	fseek(fpass, 0, SEEK_SET);
	while((fscanf(fpass, "%s", s)) != 0) {
		flag = 0;
		if (strcmp(s, "") != 0 && strcmp (prev, s) != 0) {
			if (strcmp(s, output) == 0) {
				flag = -1;
				return;
			}
		}
		else {
			break;
		}
		prev = strcpy(prev, s);
	}

	if (flag != -1) { 
		fseek(fpass, 0, SEEK_END);
		fputs(output, fpass);
		fflush(fpass);
	}
	const char * msg =  "OK\r\n";
	write(fd, msg, strlen(msg));
	
	return;		
}

void IRCServer::enterRoom(int fd, const char * user, const char * password, const char * args) {
	if (checkPassword(fd, user, password) == false) {
		write(fd, "ERROR (Wrong password)\r\n", 24);
		return;
	}
	
	else {
		
		User * u = (User *) malloc(sizeof(User));
		
		u->name = strdup(user);
		u->password = strdup(password);
		
		char * roomName = strdup(args); 
		Room * e = rlist->head;
		
		int flag = 0;
		while(e != NULL) {
			if (strcmp(e->value, roomName) == 0) {
				flag = 1;
				User * m = (User *) malloc(sizeof(User));
				m = e->members->head;
				User * mprev = (User *) malloc(sizeof(User));
				
				User * check = e->members->head;				
				while (check != NULL) {
					if (strcmp(check->name, u->name) == 0) {
						flag = -1;
						write(fd, "OK\r\n", 4);
						return;
					}
					check = check->next;
				}
				if (flag != -1) {
					if (m == NULL) {
		    				e->members->head = u;
	    					u->next = NULL;
					}		   	  
					else {
	    					mprev = e->members->head;
						e->members->head = u;
			    			u->next = mprev;
         				}
	
					break;
				}
			}
			e = e->next;
		}

		if (flag == 0) {
			write(fd, "ERROR (No room)\r\n", 17);
			return;
		}
		write(fd, "OK\r\n", 4);
	}
	return;
}

void IRCServer::checkLogin(int fd, const char * user, const char * password, const char * args) {
	if (checkPassword(fd, user, password) == true) { 
		write(fd, "Good", 4);
		return;
	}
	else {
		write(fd, "Bad", 3);
		return;
	}
}

int encryptMessage(int fd, const char * user, const char * password, const char * args) {
    if (checkPassword(fd, user, password) == false) {
        write(fd, "ERROR (Wrong password)\r\n", 24);
		return;
    }
    else { 
        int p = 0;
        int q = 0;
        char * message;
    
        char * command = (char *) malloc(1000 * sizeof(char));
        char * str_p = (char *) malloc(1000 * sizeof(char));
	    char * str_q = (char *) malloc(1000 * sizeof(char));
     	char * str_m = (char *) malloc(1000 * sizeof(char));

      	int spaces[10];
	    n = 0;
        int i = 0;
	
    	for (i = 0; i < 10; i++) {
    		spaces[i] = -10;
    	}
    	
    	char * commandLine = request;
    
    	for (i = 0; commandLine[i] != '\0'; i++) {
    		if (commandLine[i] == ' ')
    			spaces[n++] = i;
    	}
    	char * p = commandLine;
    
    	int j = 0;
    	for (i = 0, j = 0; i < spaces[0]; i++, p++) {
    		command[j++] = *p;
    	}
    	command[j] = '\0';
    
    	p++;
    
    	for (i = spaces[0] + 1, j = 0; i < spaces[1]; i++, p++) {
    		str_p[j++] = *p;
    	}
    	str_p[j] = '\0';
    
    	p++;
    
    	for (i = spaces[1] + 1, j = 0; (spaces[2] > 0) ? (i < spaces[2]):(commandLine[i] != '\0'); i++, p++) {
    		str_q[j++] = *p;
    	}
    	str_q[j] = '\0';
    
    	p++;
    
    	for (i = spaces[2] + 1, j = 0; i >= 0 && commandLine[i] != '\0'; i++, p++) {
    		str_m[j++] = *p;
    	}
    	str_m[j] = '\0';
    	
    	int p_len = strlen(str_p);
        int p = 0;
        int i = 0;
        while (i <= p_len - 1) {
              p += pow(10, i) * (str_p[i]);
              i++;
        }
        
        int q_len = strlen(str_q);
        int q = 0;
        i = 0;
        while (i <= q_len - 1) {
              q += pow(10, i) * (str_q[i]);
              i++;
        }
        
        if (p < 11 || p > 1000 || isPrime(p) != 0) {
              write(fd, "Incorrect Input for p", 22);
        }
        
        else if (q < 11 || q == p || q > 1000 || isPrime(q) != 0) {
              write(fd, "Incorrect Input for q", 22);
        }
        
        // Check other parameters.  
        
        
        // Encrypt message. Input for p and q is perfect
        else {
                  
        }
     } 
}

int isPrime(int num) {
    int i = 11;
    int rem = 0;
    while (i <= num/2) {
          rem = 0;
          temp = 0;
          temp = num/i;
          rem = num - (temp * i);
          if (rem == 0) {
              return -1;
          }
    }
    
    return 0;
}
	


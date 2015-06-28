
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

	/*if (strcmp(commandLine, copy) != 0) {
		commandLine = strcpy(commandLine, copy);
	}*/

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
	printf( "password=%s\n", password );
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
	else if (!strcmp(command, "SEND-MESSAGE")) {
		sendMessage(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-MESSAGES")) {
		getMessages(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
		getUsersInRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	}
	else if (strcmp(command, "LIST-ROOMS") == 0) {
		listRooms(fd, user, password);
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
	fpass = fopen("password.txt", "w+");  	

	// Initialize users in room
	rlist->head = NULL;
	
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

void IRCServer::createRoom(int fd, const char * user, const char * password, const char * args) {
	if (checkPassword(fd, user, password) != true) {
		write(fd, "ERROR (Wrong password)\r\n", 24);
		return;
	}

	char * roomName = strdup(args);
	
	Room * r = (Room *) malloc(sizeof(Room));
	r->value = strdup(roomName);
	r->members = (UserList *) malloc(sizeof(UserList));
	r->members->head = NULL;
	r->message = (MessageList *) malloc(sizeof(MessageList));
	r->message->head = NULL;
	r->next = NULL;

	Room * temp;
	
	if (rlist->head == NULL) {
		rlist->head = r;
	}

	else {
		Room * check = (Room *) malloc(sizeof(Room));
		check = rlist->head;
		while (check != NULL) {
			if (strcmp (check->value, r->value) == 0) {
				write(fd, "ERROR (Room already exists\r\n", 28);
				return;
			}
			check = check->next;
		}
		temp = rlist->head;
		rlist->head = r;
		r->next = temp;
	}

	write(fd, "OK\r\n", 4);

	return;
}

void IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * args) {
	if (checkPassword(fd, user, password) != true)  {
		write(fd, "ERROR (Wrong password)\r\n", 24);
		return;
	}

	else {
		char * userName = strdup(user);
		if (args != NULL) {
			char * roomName = strdup(args);

			Room * r = rlist->head;
			if (r == NULL) {
				write(fd, "DENIED\r\n", 7);
				return;
			}
			else {
				while (r != NULL) {
					if (strcmp(r->value, roomName) == 0) {
						User * u = r->members->head;
						User * prev;
						prev = NULL;
						while (u != NULL) {
							if (strcmp(u->name, userName) == 0) {
								if (prev == NULL) {
									r->members->head = u->next;
									u->next = NULL;
									free(u);
								}
								else {
									prev->next = u->next;
									u->next = NULL;
									free(u);
								}
								write(fd, "OK\r\n", 4);
								return;
							}
							prev = u;
							u = u->next;
						}
					}
					r = r->next; 
				}

				write(fd, "ERROR (No user in room)\r\n\n", 25);
				return;
			}
		}

		else {
			int flag = 0;
			Room * r = rlist->head;
			while (r != NULL) {
				User * u = r->members->head;
				User * prev;
				prev = NULL;
				while (u != NULL) {
					if (strcmp(u->name, userName) == 0) {
						if (prev == NULL) { 
							r->members->head = u->next;
							u->next = NULL;
							free(u);
						}
						else {
							prev->next = u->next;
							u->next = NULL;
							free(u);
						}
						flag = 1;
					}
					prev = u;
					u = u->next;
				}
				r = r->next;
			}
			if (flag == 0) {
				write(fd, "DENIED\r\n", 8);
				return;
			}
			else {
				write(fd, "OK\n", 3);
				return;
			}
		}
	}
}

void IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args) {
	int i = 0;
	int pos = 0; 
	
	if (checkPassword(fd, user, password) != true) {
		write(fd, "ERROR (Wrong password)\r\n", 24);
		return;
	}
	
	else {
		char * room = (char *) malloc(100 * sizeof(char));
		char * message = (char *) malloc(100 * sizeof(char));

		char * argdup = strdup(args);
		char * p = argdup;
		char * r = room;
		char * m = message;

		char * name = strdup(user);

		while (*p != '\000' && *p != ' ') {
			*r = *p;
			p++;
			r++;
		}
		*r = '\0';

		p++;
		while(*p != '\0') {
			*m = *p;
			p++;
			m++;
		}

		*m = '\0';
	
		Room * rm = rlist->head;
		
		
		while (rm != NULL) {
			if (strcmp(rm->value, room) == 0) {
				User * u = rm->members->head;

				while (u != NULL) {
					if(strcmp(u->name, name) == 0) {
						rm->messageCount++;
						if (rm->messageCount > 100) {
							Message * mOld = rm->message->head;
							rm->message->head = mOld->next;
							free(mOld);
						}

						Message * newMessage = (Message *) malloc(sizeof(Message));
						newMessage->message = strdup(message);
						newMessage->user = strdup(user);
						newMessage->next = NULL;

						Message * m = rm->message->head;
						if (m == NULL) {
							rm->message->head = newMessage;
						}

						else {
							while (m->next != NULL) {
								m = m->next;
							}
							m->next = newMessage;
						}

						write(fd, "OK\r\n", 4);
						return;
					}
					u = u->next;
				}

				write(fd, "ERROR (user not in room)\r\n", 26);
			}
			rm = rm->next;
		}
				
	}
}

void IRCServer::getMessages(int fd, const char * user, const char * password, const char * args) {
	if (checkPassword(fd, user, password) != true) {
		write(fd, "ERROR (Wrong password)\r\n", 24);
		return;
	}

	else {
		char * argsdup = strdup(args);
		char * roomName = (char *) malloc(100 * sizeof(char));
		char * endpos = (char *) malloc(100 * sizeof(char));

		char * r = roomName;
		char * p = endpos;
		char * s = argsdup;

		while (*s != ' ') {
			*p = *s;
			p++;
			s++;
		}
		*p = '\0';

		s++;

		while (*s != '\0') {
			*r = *s;
			r++;
			s++;
		}
		*r = '\0';

		int end = atoi(endpos);
		int endCpy =  0;
		int flag = 0;

		Room * rm = rlist->head;
		while (rm != NULL) {	
			if (strcmp(rm->value, roomName) == 0) {
				User * u = rm->members->head;
				char * name = strdup(user);

				while (u != NULL) {
					if(strcmp(u->name, name) == 0) {
						flag = 1;
						Message * m = rm->message->head;				
						int count = 0;
						
						if (end >= rm->messageCount) {
							write(fd, "NO-NEW-MESSAGES\r\n", 17);
							return;
						}

						while (count < end) {
							m = m->next;
							count++;
						}
				
						while (count < rm->messageCount && m != NULL) {
							char * output = (char *) malloc(1000 * sizeof(char));
							char * a = output;
						
							int k = 0;
							endCpy = end;
								
							sprintf(output, "%d", count);
							int olen = strlen(output);
							a += olen;
							*a = ' ';
							a++;

							char * b = m->user;
							while (*b != '\0') {
								*a = *b;
								a++;
								b++;
							}
	
							*a = ' ';
							a++;

							char * c= m->message;
							while (*c != '\0') {
								*a = *c;
								a++;
								c++;
							}
				
							*a++ = '\r';
							*a++ = '\n';
							*a = '\0';
							write(fd, output, strlen(output));

							m = m->next;
							count++;
							end++;
						}
					}
					u = u->next;
				}
			}
			rm = rm->next;
		}
		if (flag == 0) {
			write(fd, "ERROR (User not in room)\r\n", 26);
		}	
		else if (flag != 0) {
			write(fd, "\r\n", 2);
		}
	}
}

void IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * args) {
	if (checkPassword(fd, user, password) != true) {
		write(fd, "ERROR (Wrong password)\r\n", 24);
		return;
	}
	
	char * roomName = strdup(args);
	Room * r = rlist->head;

	char * array[5];

	for (int i = 0; i < 5; i++) {
		array[i] = (char *) malloc(100 * sizeof(char));	
	}
	
	int count = 0;

	while (r != NULL) {
		if (strcmp(r->value, roomName) == 0) {
			User * u = r->members->head;
			while (u != NULL) {
				char * name = strdup(u->name);
				char * p = name;
				while(*p != '\0') {
					p++;
				}
				*p++ = '\r';
				*p++ = '\n';
				*p = '\0';

				array[count] = (char *) malloc(100 * sizeof(char));
				array[count] = strdup(name);

				u = u->next;
				count++;
			}

			for (int j = 0; j < 4; j++) {
				char * p = array[j];
				char * q = array[j + 1];
				if ((strcmp(p, "") != 0) && (strcmp(q, "") != 0) && (strcmp(p, q) > 0)) {		
					char * temp = strdup(array[j]);
					array[j] = strdup(array[j + 1]);
					array[j + 1] = strdup(temp);
				}
			}

			for (int j = 0; j < 5;  j++) {
				write(fd, array[j], strlen(array[j]));
			}
			write(fd, "\r\n", 2);
			return;
		}
		r = r->next;
	}

	write(fd, "DENIED\r\n", 8);
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
void IRCServer::getAllUsers(int fd, const char * user, const char * password, const  char * args) {
	if (checkPassword(fd, user, password) != true) {
		write(fd, "ERROR (Wrong password)\r\n", 24);
		return;
	}

	else {
		char * s = (char *) malloc(1000 * sizeof(char));
		char * p;
		
		char * prev = (char *) malloc(1000 * sizeof(char));
		
		char * sort[15];

		for (int i = 0; i < 15; i++) {
			sort[i] = (char *) malloc(100 * sizeof(char));	
		}
	
		fseek(fpass, 0, SEEK_SET);
		int count = 0;
		while((fscanf(fpass, "%s", s)) != 0 ) {
			if (strcmp(s, "") != 0 && strcmp(prev, s) != 0) {
				p = s;

				char * output = (char *) malloc(1000 * sizeof(char));
				char * r = output;

				while(*p != '/') {
					*r = *p;
					p++;
					r++;
				}
			
				*r++ = '\r';
				*r++ = '\n';
				*r++ = '\0';
				prev = strcpy(prev, s);
				write(fd, output, strlen(output));
			}
	
			else {
				break;
			}
		}
		
		write(fd, "\r\n", 2);
		return;
	}
}

void IRCServer::listRooms(int fd, const char * user, const char * password) {
	if (checkPassword(fd, user, password) != true) {
		write(fd, "ERROR (Wrong password)\r\n", 24);
		return;
	}

	Room * r = rlist->head;
	char * name;
	while (r != NULL) {
		name = strdup(r->value);
		char * p = name;
		while (*p != '\0') {
			p++;
		}

		*p++ = '\r';
		*p++ = '\n';
		*p = '\0';

		write(fd, name, strlen(name));
		r = r->next;
	}

	return;
}
	


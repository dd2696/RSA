
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
#include <math.h>

#include "RSA.h"

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

int RSAServer::open_server_socket(int port) {

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

void RSAServer::runServer (int port) {
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

	RSAServer RSAServer;

	// It will never return
	RSAServer.runServer(port);
}

void RSAServer::processRequest (int fd) {
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
	else if (strcmp(command, "CHECK-LOGIN") == 0) { 
		checkLogin(fd, user, password, args);
	}
	else if (strcmp(command, "ENCRYPT-MESSAGE") == 0) {
        encryptMessage(fd, user, password, args);
    } 
    else if (strcmp(command, "DECRYPT-MESSAGE") == 0) {
        decryptMessage(fd, user, password, args);
    }
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	close(fd);	
}

void RSAServer::initialize() {
	// Open password file
	fpass = fopen("password.txt", "r+");  	
}

bool RSAServer::checkPassword(int fd, const char * user, const char * password) {
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

void RSAServer::addUser(int fd, const char * user, const char * password, const char * args) {
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

void RSAServer::checkLogin(int fd, const char * user, const char * password, const char * args) {
	if (checkPassword(fd, user, password) == true) { 
		write(fd, "Good", 4);
		return;
	}
	else {
		write(fd, "Bad", 3);
		return;
	}
}

void RSAServer::encryptMessage(int fd, const char * user, const char * password, const char * args) {
    if (checkPassword(fd, user, password) == false) {
         write(fd, "ERROR (Wrong password)\r\n", 24);
		 return;
    }
    else { 
        char * str_p = (char *) malloc(1000 * sizeof(char));
	    char * str_q = (char *) malloc(1000 * sizeof(char));
     	char * str_m = (char *) malloc(1000 * sizeof(char));
     	char * str_e = (char *) malloc(1000 * sizeof(char));

      	int spaces[10];
	    int n = 0;
        int i = 0;
	
    	for (i = 0; i < 10; i++) {
    		spaces[i] = -10;
    	}
    	
    	char * commandLine = strdup(args);
    
    	for (i = 0; commandLine[i] != '\0'; i++) {
    		if (commandLine[i] == ' ')
    			spaces[n++] = i;
    	}
    	char * a = commandLine;
    
    	int j = 0;
    	    
    	// First input is value of p
        for (i = 0, j = 0; i < spaces[0]; i++, a++) {
    		str_p[j++] = *a;
    	}
    	str_p[j] = '\0';
    	a++;
    
        // Next os value of q    
    	for (i = spaces[0] + 1, j = 0; (spaces[1] > 0) ? (i < spaces[1]):(commandLine[i] != '\0'); i++, a++) {
    		str_q[j++] = *a;
    	}
    	str_q[j] = '\0';
    	a++;
    
        // Third is value of e
    	for (i = spaces[1] + 1, j = 0; (spaces[2] > 0) ? (i < spaces[2]):(commandLine[i] != '\0'); i++, a++) {
    		str_e[j++] = *a;
    	}
    	str_e[j] = '\0';
    	a++;
    	
    	// Rest is message
    	for (i = spaces[2] + 1, j = 0; i >= 0 && commandLine[i] != '\0'; i++, a++) {
    		str_m[j++] = *a;
    	}
    	str_m[j] = '\0';
    	
    	int p_len = strlen(str_p);
        int p = 0;
        i = p_len - 1;
	j = 0;
        while (i >= 0) {
              p += pow(10, j++) * (str_p[i] - 48);
              i--;
        }
        
        int q_len = strlen(str_q);
        int q = 0;
        i = q_len - 1;
	j = 0;
        while (i >= 0) {
              q += pow(10, j++) * (str_q[i] - 48);
              i--;
        }
        
        int e_len = strlen(str_e);
        int e = 0;
        i = e_len - 1;
	j = 0;
        while (i >= 0) {
              e += pow(10, j++) * (str_e[i] - 48);
              i--;
        }
        
        int phi = (p - 1) * (q - 1);
        
        if (p < 11 || p > 1000 || isPrime(p) != 0) {
              write(fd, "Incorrect Input for p", 22);
              return;
        }
        
        else if (q < 11 || q == p || q > 1000 || isPrime(q) != 0) {
              write(fd, "Incorrect Input for q", 22);
              return;
        }
                
        else if (gcdCalculator(e, phi) != 1 || e <= 1 || e >= phi) {
              write(fd, "Incorrect Input for e", 22);
              return;
        }  
        
        else if (strcmp(str_m, "") == 0) {
             write (fd, "Error: Incorrect input for message", 34);
             return;
        }
        // Encrypt message. Input for p and q is perfect
        else {
              int n = p * q;   
              char * enc_string = (char *) malloc(strlen(str_m) * 2 * sizeof(char));
              int ctr_string = 0;
              int d = modInverse(e, phi);
              
              char * d_string = (char *) malloc (100 * sizeof(char));
	      sprintf(d_string, "%d", d);             
	
	      
	      ctr_string += strlen(d_string) + 1;
	      enc_string = strcpy(enc_string, d_string);
	      enc_string = strcat (enc_string, " ");
	      
	      for (int k = 0; k < strlen(str_m); k++) {               
                  int ascii = (int) str_m[k];
                  int cipher_value = generateCipherValue(ascii, e, n);
                  if (cipher_value < 10) {
                     enc_string[ctr_string] = '0';
                     enc_string[ctr_string + 1] = (char) (48 + cipher_value);
                     ctr_string += 2;
                  }
                  else {
                     enc_string[ctr_string] = (char) (48 + (cipher_value/10));
                     enc_string[ctr_string + 1] = (char) (48 + (cipher_value%10));  
                     ctr_string += 2;
                  }
              }
             
	      enc_string[ctr_string + 1] = '\0';
	      // Write the encrypted message into string
              write (fd, enc_string, strlen(enc_string));  
        }
     }    
}

int RSAServer::generateCipherValue(int m, int e, int n) {
    int rem = 0;
    double temp = 0;

    temp = pow (m, e);
    rem = fmod(temp, n);
    return rem;
}

int RSAServer::isPrime(int num) {
    int i = 2;
    int rem = 0;
    while (i <= num/2) {
          rem = 0;
          int temp = 0;
          temp = num/i;
          rem = num - (temp * i);
          if (rem == 0) {
              return -1;
          }
	  i++;
    }
    return 0;
}

int RSAServer::gcdCalculator (int x, int y) {
    int i = 2;
    int gcd = 1;
    while (i <= x/2 && i <= y/2) {
        if (isPrime(i) == 0) {
           if ((x % i == 0) && (y % i == 0)) {
                  gcd *= i;
           }
           else {
                i++;
           }
        }
        else {
             i++;
        }
    }
    return gcd; 
}

int RSAServer::modInverse (int e, int phi) {
    int d = 0;
    int rem = 0; 
    do {
    	d++;
        int quotient = (d * e)/phi;
        int c = quotient * phi;
        rem = (d * e) - c;
    } while (rem != 1);
    return d;
}

void RSAServer::decryptMessage (int fd, const char * user, const char * password, const char * args) {
    if (checkPassword(fd, user, password) == false) {
        write(fd, "ERROR (Wrong password)\r\n", 24);
	    return;
    }  
    else {
        char * str_d = (char *) malloc(1000 * sizeof(char));
	    char * str_n = (char *) malloc(1000 * sizeof(char));
        char * str_m = (char *) malloc(1000 * sizeof(char));
        
       	int spaces[10];
	    int n = 0;
        int i = 0;
	
    	for (i = 0; i < 10; i++) {
    		spaces[i] = -10;
    	}
    	
    	char * commandLine = strcpy(commandLine, args);
    
    	for (i = 0; commandLine[i] != '\0'; i++) {
    		if (commandLine[i] == ' ')
    			spaces[n++] = i;
    	}
    	char * a = commandLine;
    
    	int j = 0;
    	    
    	// First input is value of d
        for (i = spaces[0] + 1, j = 0; i < spaces[1]; i++, a++) {
    		str_d[j++] = *a;
    	}
    	str_d[j] = '\0';
    	a++;
    
        // Next os value of n    
    	for (i = spaces[1] + 1, j = 0; (spaces[2] > 0) ? (i < spaces[2]):(commandLine[i] != '\0'); i++, a++) {
    		str_n[j++] = *a;
    	}
    	str_n[j] = '\0';
    	a++;
    
        // Third is value of m
    	for (i = spaces[2] + 1, j = 0; i >= 0 && commandLine[i] != '\0'; i++, a++) {
    		str_m[j++] = *a;
    	}
    	str_m[j] = '\0';    
    	
    	int d_len = strlen(str_d);
        int d = 0;
        i = 0;
        while (i <= d_len - 1) {
              d += pow(10, i) * (str_d[i] - 48);
              i++;
        }
        
        int n_len = strlen(str_n);
        n = 0;
        i = 0;
        while (i <= n_len - 1) {
              n += pow(10, i) * (str_n[i] - 48);
              i++;
        }
        
        if (d <= 0 || n <= 1) {
              write(fd, "Incorrect Input for d/n", 24);
              return;
        }
        
        else if (strcmp(str_m, "") == 0) {
             write(fd, "Incorrect input for message", 27);
             return;
        }
        
        else {  
             char temp;
             int m_len = strlen(str_m)/2;
             char * message = (char *) malloc (m_len * sizeof(char));
             int ctr = 0;
             
             for (int k = 0; k < strlen(str_m); k+=2) {
                 int val1 = str_m[k] - 48;
                 int val2 = str_m[k + 1] - 48;
                 
                 if (val1 == 0) {
                      temp = decryptCipherValue(val2, d, n);
                 }
                 else {
                      int val = val1 * 10 + val2;
                      temp = decryptCipherValue(val, d, n);
                 }
                 message[ctr] = (char) temp;
                 ctr++;          
             }      
             
             //Return final string
             write(fd, message, strlen(message));
        }
    }
}

char RSAServer::decryptCipherValue(int val, int d, int n) {
     int temp = pow(val, d);
     int rem = temp - ((temp/n) * n);
     return rem;
}



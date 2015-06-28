#include "RSA.h"

int open_server_socket(int port) {
	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);
  
	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if (masterSocket < 0) {
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
	error = listen (masterSocket, QueueLength);
	if (error) {
		perror("listen");
		exit( 1);
	}

	return masterSocket;
}

void runServer(int port) {
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

void initialize () {
     int fd = fopen("passwords.txt", r+);
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        fprintf(stderr, "%s", "run command");
        exit(-1);
    }
    
    int port = atoi(argv[1]);
    
    runServer(port);
}

void processCommand(char * request) {
     if (request == NULL) {
         const char * msg =  "UNKNOWN COMMAND\r\n";
		 write(fd, msg, strlen(msg));
     }    
 
     if (strcmp(request, "ENCRYPT") >= 0) {
         encryptMessage(fd, request);
     }
     else if (strcmp(request, "DECRYPT") == 0) {
         decryptMessage();
     }
     else {
         const char * msg =  "UNKNOWN COMMAND\r\n";
		 write(fd, msg, strlen(msg));
     }      
}
    
int encryptMessage(int fd, char * request) {
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
    
    if (p < 11 || isPrime(p) != 0) {
          write(fd, "Incorrect Input for p", 22);
    }
    
    else if (q < 11 || q == p || isPrime(q) != 0) {
          write(fd, "Incorrect Input for q", 22);
    }
    
    // Check other parameters.  
    
    
    // Encrypt message. Input for p and q is perfect
    else {
              
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
              
    
	
    
    
    
    
    
       
        

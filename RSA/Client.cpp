#include <gtk/gtk.h>
#include <time.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

static char buffer[256];

char * rooms;
char * user_stream;
char * message_stream;
char * host;
char * user;
char * password;
char * sport;
char * roomsEntered = (char *) malloc (10000 * sizeof(char));
int port;
int messageCount = 0;
 
#define MAX_MESSAGES 100
#define MAX_MESSAGE_LEN 300
#define MAX_RESPONSE (20 * 1024)
 
GtkWidget * window1;
GtkWidget * window2;
GtkWidget * window3;
GtkWidget * window4;
GtkWidget * window5;

GtkWidget * p_entry;
GtkWidget * q_entry;
GtkWidget * e_entry;
GtkWidget * m_entry;
GtkWidget * c_entry;
GtkWidget * private_entry;
GtkWidget * public_entry;

GtkWidget * d_entry;
GtkWidget * n_entry;
GtkWidget * pln_entry;
GtkWidget * cry_entry;

GtkWidget * dialog;

const char * user_text;
const char * pass_text;
const char * room_text;

int open_client_socket(char * host, int port) {
       // Initialize socket address structure
       struct sockaddr_in socketAddress;

       // Clear sockaddr structure
       memset((char *)&socketAddress,0,sizeof(socketAddress));

       // Set family to Internet 
       socketAddress.sin_family = AF_INET;
	 
       // Set port
       socketAddress.sin_port = htons((u_short)port);
 
       // Get host table entry for this host
       struct  hostent  *ptrh = gethostbyname(host);
       if ( ptrh == NULL ) {
                 perror("gethostbyname");
	         exit(1);
       }

       // Copy the host ip address to socket address structure
       memcpy(&socketAddress.sin_addr, ptrh->h_addr, ptrh->h_length);
 
       // Get TCP transport protocol entry
       struct  protoent *ptrp = getprotobyname("tcp");
       if ( ptrp == NULL ) {
                 perror("getprotobyname");
                 exit(1);
       }
 
       // Create a tcp socket
       int sock = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
       if (sock < 0) {
                 perror("socket");
                 exit(1);
       }

       // Connect the socket to the specified server
       if (connect(sock, (struct sockaddr *)&socketAddress,
                 sizeof(socketAddress)) < 0) {
                 perror("connect");
                 exit(1);
       }
 
       return sock;
}

int sendCommand (char * host, int port, char * command, char * user, char * password, char * args, char * response) {
        int sock = open_client_socket(host, port);
 
        // Send command
        write(sock, command, strlen(command));
        write(sock, " ", 1);
        write(sock, user, strlen(user));
        write(sock, " ", 1);
        write(sock, password, strlen(password));
        write(sock, " ", 1);
        write(sock, args, strlen(args));
        write(sock, "\r\n",2);

        // Keep reading until connection is closed or MAX_REPONSE
        int n = 0;
        int len = 0;
        while ((n=read(sock, response+len, MAX_RESPONSE - len))>0) {
 	       len += n;
        }

	    int rlen = strlen(response);
	    response[rlen] = '\0';
	    printf("response:%s\n", response);
        close(sock);
}

static gboolean delete_event (GtkWidget *widget, GdkEvent  *event, gpointer data) {
	g_print ("delete event occurred\n");
	gtk_dialog_run (GTK_DIALOG (dialog));
	return FALSE;
}

static void destroy_event (GtkWidget * widget, gpointer data) {
	gtk_main_quit ();
}

static void enter_callback_user (GtkWidget * widget, GtkWidget * entry){
	user_text = gtk_entry_get_text (GTK_ENTRY (entry));
	printf ("User contents: %s\n", user_text);
}

static void enter_callback_pass (GtkWidget * widget, GtkWidget * entry) {
	pass_text = gtk_entry_get_text (GTK_ENTRY(entry));
	printf("Password contents: %s\n", pass_text);
}

/*static void send_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
	g_print ("Hello again - %s was pressed\n", (gchar *) data);
	char * user = strdup(user_text);
	char * pass = strdup(pass_text);
	char * response = (char *) malloc(1000 * sizeof(char));
	
	GtkTextIter start;
	GtkTextIter end;

	GtkTextBuffer * buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(sview));
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	gchar * message = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer), &start, &end, FALSE); 

	char * withRoom = (char *) malloc(10000*sizeof(char));
	withRoom = strcpy(withRoom, room);
	withRoom = strcat(withRoom, " ");
	withRoom = strcat(withRoom, message);

	int n = sendCommand ("localhost", 1991, "SEND-MESSAGE", user, pass, withRoom, response);  
	if(strcmp("OK", response) < 0) {
		gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), "", -1);		
	}
}*/

/* Add some text to our text widget - this is a callback that is invoked
when our window is realized. We could also force our window to be
realized with gtk_widget_realize, but it would have to be part of
a hierarchy first */

static void insert_text(GtkTextBuffer * mbuffer, const char * initialText) {
   GtkTextIter iter;

   message_stream = strdup(initialText);   

   gtk_text_buffer_set_text(GTK_TEXT_BUFFER(mbuffer), "", -1);   
   gtk_text_buffer_get_iter_at_offset (mbuffer, &iter, 0);
   gtk_text_buffer_insert (mbuffer, &iter, initialText,-1);
}

static void insert_text_enter (GtkTextBuffer * mbuffer, const char * initialText) {
	GtkTextIter iter;

   	message_stream = strdup(initialText);   
  
   	gtk_text_buffer_get_end_iter (mbuffer, &iter);
	gtk_text_buffer_insert (mbuffer, &iter, initialText,  -1);
}

/*static void process_message (char * input) {
	if (strcmp(input, "NO-NEW-MESSAGES\r\n") == 0) {
		GtkTextBuffer * mbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(mview));
		insert_text (mbuffer, "");
	}
	else {
		char * final_message = (char *) malloc(10000 * sizeof(char));
		
		char * p = input;
		char * q = final_message;
		char * prev = (char *) malloc(1 * sizeof(char));
		*prev = '\0';
		while (*p != '\0') {
			if (*prev == '\0') {
				while (*p != ' ') {
					p++;
				}
				p++;
				while (*p != '\0' && *p != ' ') {
					*q = *p;
					q++;
					p++;
				}
				*q = ':';
				q++;
				*q = ' ';
				q++;
				p++;
			}
			if (*prev != '\0' && (*prev == '\r' && *p == '\n')) {
				messageCount++;
				*q = '\n';
				*prev = *p;
				p++;
				q++;
				while (*p != '\0' && *p != ' ') {
					p++;
				}
				p++;

				while (*p != '\0' && *p != ' ') {
					*q = *p;
					q++;
					p++;
				}
				*q = ':';
				q++;
				*q = ' ';
				q++;
				p++;
			}
			*q = *p;
			q++;
			*prev = *p;
			p++;
		}
		*q = '\0';

 		GtkTextBuffer * mbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(mview));
		insert_text (mbuffer, final_message);
	}
}*/

static gboolean time_handler(GtkWidget * widget) {
	printf("Time\n");
	if (widget->window == NULL) {
		return FALSE;
	}

	printf("Timew\n");
	time_t curtime;
    struct tm *loctime;
	
	curtime = time(NULL);
	loctime = localtime(&curtime);
	strftime(buffer, 256, "%T", loctime);
	gtk_widget_show(window5);
	
	return TRUE;
}

static void login_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
	g_print ("Hello again - %s was pressed\n", (gchar *) data);	
	char * user = strdup (user_text);
	char * pass = strdup (pass_text);
	char * response = (char *) malloc(1000 * sizeof(char));
	int n = sendCommand ("localhost", 1991, "CHECK-LOGIN", user, pass, "", response);
	if(strcmp("OK", response) < 0) {
		gtk_widget_hide(window1);
		gtk_widget_show(window2);
	}	
}

static void add_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
	g_print ("Hello again - %s was pressed\n", (gchar *) data);
	char * user = strdup(user_text);
	char * pass = strdup(pass_text);
	char * response = (char *) malloc(1000 * sizeof(char));
	int n = sendCommand ("localhost", 1991, "ADD-USER", user, pass, "", response);  
	if(strcmp("OK", response) < 0) {
		gtk_widget_hide(window1);
		gtk_widget_show(window2);
	}
}

static void encrypt_event (GtkWidget * widget, GdkEvent * event, gpointer data) {     
    g_print ("Hello again - %s was pressed\n", (gchar *) data);
    gtk_entry_set_text(GTK_ENTRY(c_entry), "");
	char * user = strdup(user_text);
	char * pass = strdup(pass_text);
	char * response = (char *) malloc(1000 * sizeof(char));
	
    const gchar * p_text = gtk_entry_get_text(GTK_ENTRY(p_entry)); 
    const gchar * q_text = gtk_entry_get_text(GTK_ENTRY(q_entry)); 
    const gchar * e_text = gtk_entry_get_text(GTK_ENTRY(e_entry)); 
    const gchar * m_text = gtk_entry_get_text(GTK_ENTRY(m_entry)); 
    
    // Convert all elements into message
    char * args = strdup(p_text);
    args = strcpy(args, " ");
    args = strcpy(args, q_text);
    args = strcpy(args, " ");
    args = strcpy(args, e_text);
    args = strcpy(args, " ");
    args = strcpy(args, m_text);
    
    int n = sendCommand ("localhost", 1991, "ENCRYPT-MESSAGE", user, pass, args, response);
    
    if (strncmp(response, "ERROR", 5) == 0) {
        gtk_entry_set_text(GTK_ENTRY(c_entry), response);
    }
    else {
        // Get value of d - Private key from Server
        char * b = response;
        char d_string[100];
        int i = 0;
        while (*b != ' ') {
            d_string[i++] = *b;
            b++;
        }
        d_string[i] = '\0';
        
        gtk_entry_set_text(GTK_ENTRY(private_entry), d_string); 
        
        b++;
        // Set value for public, private key
        int p_len = strlen(p_text);
        int p = 0;
        i = 0;
        while (i <= p_len - 1) {
              p += pow(10, i) * (p_text[i] - 48);
              i++;
        }
        
        int q_len = strlen(q_text);
        int q = 0;
        i = 0;
        while (i <= q_len - 1) {
              q += pow(10, i) * (q_text[i] - 48);
              i++;
        }
        
        int n = p * q;
        char n_str[15];
        sprintf(n_str, "%d", n);
        gtk_entry_set_text(GTK_ENTRY(public_entry), n_str);
        
        gtk_entry_set_text(GTK_ENTRY(c_entry), b);
        time_handler(window3);
    }                    
}

static void decrypt_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
    g_print ("Hello again - %s was pressed\n", (gchar *) data);
    gtk_entry_set_text(GTK_ENTRY(pln_entry), "");
	char * user = strdup(user_text);
	char * pass = strdup(pass_text);
	char * response = (char *) malloc(1000 * sizeof(char));
	
	const gchar * d_text = gtk_entry_get_text(GTK_ENTRY(d_entry)); 
	const gchar * n_text = gtk_entry_get_text(GTK_ENTRY(n_entry));
	const gchar * cry_text = gtk_entry_get_text(GTK_ENTRY(cry_entry));

    // Convert to message
    char * args = strdup(d_text);
    args = strcpy(args, " ");
    args = strcpy(args, n_text);
    args = strcpy(args, " ");
    args = strcpy(args, cry_text);

    int n = sendCommand ("localhost", 1991, "DECRYPT-MESSAGE", user, pass, args, response);
    
    if (strncmp(response, "ERROR", 5) == 0) {
        gtk_entry_set_text(GTK_ENTRY(pln_entry), response);
    }
    else {
        gtk_entry_set_text(GTK_ENTRY(pln_entry), response);
        time_handler(window4);
    } 
}

static void encryption_selected (GtkWidget * widget, GdkEvent * event, gpointer data) {
	g_print ("Hello again - %s was pressed\n", (gchar *) data);
	
	gtk_widget_hide(window2);
	gtk_widget_show(window3);
}

static void decryption_selected (GtkWidget * widget, GdkEvent * event, gpointer data) {
	g_print ("Hello again - %s was pressed\n", (gchar *) data);
	
	gtk_widget_hide(window2);
	gtk_widget_show(window4);
}

static void yes_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
    g_print ("Hello again - %s was pressed\n", (gchar *) data);
    
    gtk_widget_hide (window5);
    gtk_widget_show (window2);
}    

static void no_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
    g_print ("Hello again - %s was pressed\n", (gchar *) data);
    
    gtk_widget_hide (window5);
} 

static void logout_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
	g_print ("Hello again - %s was pressed\n", (gchar *) data);
	char * user = strdup(user_text);
	char * pass = strdup(pass_text);
	
	char * response = (char *) malloc(1000 * sizeof(char));
	
	gtk_widget_hide(window2);
	gtk_widget_show(window1);
}
   
int main (int argc, char ** argv) {
    GtkWidget * button;
    GtkWidget * entry;
        
    gtk_init(*argc, argv);
    
    window1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window1), "LOGIN TO CLIENT");
    
    g_signal_connect(GTK_WINDOW(window1), "delete-event", G_CALLBACK (delete_event), NULL);
    g_signal_connect(GTK_WINDOW(window1), "destroy", G_CALLBACK (destroy_event), NULL);
    
    gtk_container_set_border_width (GTK_CONTAINER(window1), 10);
    
    GtkWidget * table = gtk_table_new(4, 3, TRUE);
    gtk_container_add(GTK_CONTAINER(window1), table);
    
    GtkWidget * label1 = gtk_label_new("USERNAME: ");
    gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 1, 0, 1);
    gtk_widget_show(label1); 
    
    GtkWidget * label2 = gtk_label_new("PASSWORD: ");
    gtk_table_attach_defaults(GTK_TABLE(table), label2, 0, 1, 1, 2);
    gtk_widget_show(label2); 
    
    entry = gtk_entry_new();
	g_signal_connect (entry, "activate", G_CALLBACK (enter_callback_user), entry);
	gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 4, 0, 1);
	gtk_widget_show (entry);
	
	entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
	g_signal_connect (entry, "activate", G_CALLBACK (enter_callback_pass), entry);
	gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 4, 1, 2);
	gtk_widget_show (entry);
    
   	button = gtk_button_new_with_label ("LOGIN");
	g_signal_connect (button, "clicked", G_CALLBACK (login_event), (gpointer) "Login Button");
	gtk_table_attach_defaults(GTK_TABLE(table), button, 1, 2, 3, 4);
	gtk_widget_show (button);
	
	button = gtk_button_new_with_label ("ADD NEW USER");
	g_signal_connect (button, "clicked", G_CALLBACK (add_event), (gpointer) "Add Button");
	gtk_table_attach_defaults(GTK_TABLE(table), button, 2, 3, 3, 4);
	gtk_widget_show (button);
	
	gtk_widget_show(table);
	gtk_widget_show(window1);
	
	//*********WINDOW 2********
	// Choice of encryption/ decryption
	
	window2 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window2), "ENCRYPTION/DECRYPTION");
    
    g_signal_connect(GTK_WINDOW(window2), "delete-event", G_CALLBACK (delete_event), NULL);
    g_signal_connect(GTK_WINDOW(window2), "destroy", G_CALLBACK (destroy_event), NULL);
    
    gtk_container_set_border_width (GTK_CONTAINER(window2), 10);
    
    table = gtk_table_new(3, 4, TRUE);
    gtk_container_add(GTK_CONTAINER(window2), table);
	
	GtkWidget * radio1;
	GtkWidget * radio2;
	
    radio1 = gtk_radio_button_new_with_label(NULL, "Encrypt a plaintext message");
    g_signal_connect (GTK_TOGGLE_BUTTON(radio1), "toggled", G_CALLBACK (encryption_selected), NULL);
    gtk_table_attach_defaults(GTK_TABLE(table), radio1, 0, 1, 0, 1);
    
	radio2 = gtk_radio_button_new_with_label(gtk_radio_button_group(GTK_RADIO_BUTTON(radio1)), "Decrypt a cipher text");
	g_signal_connect (GTK_TOGGLE_BUTTON(radio2), "toggled", G_CALLBACK (decryption_selected), NULL);
	gtk_table_attach_defaults(GTK_TABLE(table), radio2, 0, 1, 1, 2);
	
	gtk_widget_show(radio1);
	gtk_widget_show(radio2);
	
   	gtk_widget_show(table);
	
	//*********WINDOW 3********
	//Encryption
	
	window3 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window3), "ENCRYPTION");
    
    g_signal_connect(GTK_WINDOW(window3), "delete-event", G_CALLBACK (delete_event), NULL);
    g_signal_connect(GTK_WINDOW(window3), "destroy", G_CALLBACK (destroy_event), NULL);
    
    gtk_container_set_border_width (GTK_CONTAINER(window3), 10);
    
    table = gtk_table_new(8, 7, TRUE);
    gtk_container_add(GTK_CONTAINER(window3), table);
    
    label1 = gtk_label_new("p");
    gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 1, 0, 1);
    gtk_widget_show(label1);
    
    p_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(p_entry), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), p_entry, 1, 3, 0, 1);
	gtk_widget_show (p_entry);
    
    label2 = gtk_label_new("q");
    gtk_table_attach_defaults(GTK_TABLE(table), label2, 4, 5, 0, 1);
    gtk_widget_show(label2);
    
    q_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(q_entry), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), q_entry, 5, 7, 0, 1);
	gtk_widget_show (q_entry);
	
	GtkWidget * label3 = gtk_label_new("e");
    gtk_table_attach_defaults(GTK_TABLE(table), label3, 1, 2, 1, 2);
    gtk_widget_show(label3);
    
    e_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(e_entry), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), e_entry, 2, 5, 1, 2);
	gtk_widget_show (e_entry);
	
	GtkWidget * label4 = gtk_label_new("Message");
    gtk_table_attach_defaults(GTK_TABLE(table), label4, 0, 1, 2, 3);
    gtk_widget_show(label4);
    
    m_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(m_entry), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), m_entry, 1, 5, 2, 3);
	gtk_widget_show (m_entry);
	
	button = gtk_button_new_with_label ("Encrypt");
	g_signal_connect (button, "clicked", G_CALLBACK (encrypt_event), (gpointer) "Encrypt Button");
	gtk_table_attach_defaults(GTK_TABLE(table), button, 2, 3, 4, 5);
	gtk_widget_show (button);
	
	GtkWidget * label5 = gtk_label_new("Cipher Text");
    gtk_table_attach_defaults(GTK_TABLE(table), label5, 0, 1, 6, 7);
    gtk_widget_show(label5);
    
    c_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(c_entry), TRUE);
	gtk_entry_set_text(GTK_ENTRY(c_entry), "");
	gtk_table_attach_defaults(GTK_TABLE(table), c_entry, 1, 7, 6, 7);
	gtk_widget_show (c_entry);
	
	GtkWidget * label6 = gtk_label_new("Public Key");
    gtk_table_attach_defaults(GTK_TABLE(table), label6, 0, 1, 7, 8);
    gtk_widget_show(label6);
    
    public_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(public_entry), TRUE);
	gtk_entry_set_text(GTK_ENTRY(public_entry), "");
	gtk_table_attach_defaults(GTK_TABLE(table), public_entry, 1, 3, 7, 8);
	gtk_widget_show (public_entry);
	
	GtkWidget * label7 = gtk_label_new("Private Key");
    gtk_table_attach_defaults(GTK_TABLE(table), label7, 4, 5, 7, 8);
    gtk_widget_show(label7);
    
    private_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(private_entry), TRUE);
	gtk_entry_set_text(GTK_ENTRY(private_entry), "");
	gtk_table_attach_defaults(GTK_TABLE(table), private_entry, 5, 7, 7, 8);
	gtk_widget_show (private_entry);
	
	gtk_widget_show (table);
    
    //*******WINDOW 4**********
    //Decryption
    
   	window4 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window4), "DECRYPTION");
    
    g_signal_connect(GTK_WINDOW(window4), "delete-event", G_CALLBACK (delete_event), NULL);
    g_signal_connect(GTK_WINDOW(window4), "destroy", G_CALLBACK (destroy_event), NULL);
    
    gtk_container_set_border_width (GTK_CONTAINER(window4), 10);
    
    table = gtk_table_new(4, 7, TRUE);
    gtk_container_add(GTK_CONTAINER(window4), table);
    
    GtkWidget * label8 = gtk_label_new("d");
    gtk_table_attach_defaults(GTK_TABLE(table), label8, 0, 1, 0, 1);
    gtk_widget_show(label8);
    
    d_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(d_entry), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), d_entry, 1, 3, 0, 1);
	gtk_widget_show (d_entry);
    
    GtkWidget * label9 = gtk_label_new("n");
    gtk_table_attach_defaults(GTK_TABLE(table), label9, 4, 5, 0, 1);
    gtk_widget_show(label9);
    
    n_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(n_entry), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), n_entry, 5, 7, 0, 1);
	gtk_widget_show (n_entry);
	
	GtkWidget * label10 = gtk_label_new("Crypted Text");
    gtk_table_attach_defaults(GTK_TABLE(table), label10, 0, 1, 1, 2);
    gtk_widget_show(label10);
    
    cry_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(cry_entry), TRUE);
	gtk_entry_set_text(GTK_ENTRY(cry_entry), "");
	gtk_table_attach_defaults(GTK_TABLE(table), cry_entry, 1, 7, 1, 2);
	gtk_widget_show (cry_entry);
	
	GtkWidget * button3 = gtk_button_new_with_label ("DECRYPT");
	g_signal_connect (button3, "clicked", G_CALLBACK (decrypt_event), (gpointer) "Decrypt Button");
	gtk_table_attach_defaults(GTK_TABLE(table), button3, 3, 4, 2, 3);
	gtk_widget_show (button3);
	
	GtkWidget * label11 = gtk_label_new("Plain Text");
    gtk_table_attach_defaults(GTK_TABLE(table), label11, 0, 1, 3, 4);
    gtk_widget_show(label11);
    
    pln_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(pln_entry), TRUE);
	gtk_entry_set_text(GTK_ENTRY(pln_entry), "");
	gtk_table_attach_defaults(GTK_TABLE(table), pln_entry, 1, 7, 3, 4);
	gtk_widget_show (pln_entry);
	
	//*******WINDOW 5*********
	//Selection
	window5 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window5), "START AGAIN?");
    
    g_signal_connect(GTK_WINDOW(window5), "delete-event", G_CALLBACK (delete_event), NULL);
    g_signal_connect(GTK_WINDOW(window5), "destroy", G_CALLBACK (destroy_event), NULL);
    
    gtk_container_set_border_width (GTK_CONTAINER(window5), 10);
    
    table = gtk_table_new(3, 5, TRUE);
    gtk_container_add(GTK_CONTAINER(window5), table);
    
    label1 = gtk_label_new("Do you want to start again?");
    gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 1, 0, 1);
    gtk_widget_show(label1);
    
    button = gtk_button_new_with_label ("Yes");
	g_signal_connect (button, "clicked", G_CALLBACK (yes_event), (gpointer) "Yes Button");
	gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 2, 1, 3);
	gtk_widget_show (button);
	
	button = gtk_button_new_with_label ("No");
	g_signal_connect (button, "clicked", G_CALLBACK (no_event), (gpointer) "No Button");
	gtk_table_attach_defaults(GTK_TABLE(table), button, 3, 5, 1, 3);
	gtk_widget_show (button);
	    
	gtk_main ();
   
  	return 0;
}


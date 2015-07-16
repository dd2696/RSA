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

static void destroy (GtkWidget * widget, gpointer data) {
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

static void send_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
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
}

static void tree_selection_changed_cb (GtkTreeSelection *selection, gpointer data) {
        GtkTreeIter iter;
        GtkTreeModel *model;

        if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
                gtk_tree_model_get (model, &iter, 0, &room, -1);
                g_print ("You selected %s\n", room);
        }
}

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

static void process_message (char * input) {
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
}
	

/* Create the list of "messages" */
static GtkWidget * create_list (const char * titleColumn, GtkListStore *model) {
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;

    int i;
   
    /* Create a new scrolled window, with scrollbars only if needed */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				    GTK_POLICY_AUTOMATIC, 
				    GTK_POLICY_AUTOMATIC);
   
    tree_view = gtk_tree_view_new ();
    gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));

    GtkTreeSelection *select;

    select = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
    g_signal_connect (G_OBJECT (select), "changed", G_CALLBACK (tree_selection_changed_cb), NULL);    

    gtk_widget_show (tree_view);
   
    cell = gtk_cell_renderer_text_new ();

    column = gtk_tree_view_column_new_with_attributes (titleColumn,
                                                       cell,
                                                       "text", 0,
                                                       NULL);
  
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
	  		         GTK_TREE_VIEW_COLUMN (column));

    return scrolled_window;
}

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
	get_new_Rooms();
	
	return TRUE;
}

static gboolean time_handler_users(GtkWidget * widget) {
	printf("Time\n");
	if (widget->window == NULL) {
		return FALSE;
	}

	printf("Time2\n");
	time_t curtime;
        struct tm *loctime;
	
	curtime = time(NULL);
	loctime = localtime(&curtime);
	strftime(buffer, 256, "%T", loctime);
	get_users_in_current_room();
	get_messages_in_current_room();
	return TRUE;
}

static void login_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
	g_print ("Hello again - %s was pressed\n", (gchar *) data);	
	char * user = strdup (user_text);
	char * pass = strdup (pass_text);
	char * response = (char *) malloc(1000 * sizeof(char));
	int n = sendCommand ("localhost", 1991, "CHECK-LOGIN", user, pass, "", response);
	if(strcmp("Good", response) < 0) {
		gtk_widget_hide(window);
		gtk_widget_show(window2);
		g_timeout_add(5000, (GSourceFunc) time_handler, gpointer(window2));
  		time_handler(window2);
	}	
}

static void add_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
	g_print ("Hello again - %s was pressed\n", (gchar *) data);
	char * user = strdup(user_text);
	char * pass = strdup(pass_text);
	char * response = (char *) malloc(1000 * sizeof(char));
	int n = sendCommand ("localhost", 1991, "ADD-USER", user, pass, "", response);  
	if(strcmp("OK", response) < 0) {
		gtk_widget_hide(window);
		gtk_widget_show(window2);
		g_timeout_add(5000, (GSourceFunc) time_handler, gpointer(window2));
  		time_handler(window2);
	}
}

static void logout_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
	g_print ("Hello again - %s was pressed\n", (gchar *) data);
	char * user = strdup(user_text);
	char * pass = strdup(pass_text);
	
	char * response = (char *) malloc(1000 * sizeof(char));

	char * p = roomsEntered;
	while (*p != '\0') {
		char * leaveRoom = (char *) malloc (100 * sizeof(char));
		char * q = leaveRoom;

		while (*p != '\n') {
			*q = *p;
			q++;
			p++;
		}
		*q = '\0';
		p++;	
	
		char * leaveMessage = (char *) malloc (1000 * sizeof(char));
		leaveMessage = strcpy(leaveMessage, leaveRoom);
		leaveMessage = strcat(leaveMessage, " ");
		leaveMessage = strcat(leaveMessage, "LEFT: ");
		leaveMessage = strcat(leaveMessage, user);
		leaveMessage = strcat(leaveMessage, " has left the room!");
		int m = sendCommand ("localhost", 1991, "SEND-MESSAGE", user, pass, leaveMessage, response);
		int n = sendCommand ("localhost", 1991, "LEAVE-ROOM", user, pass, leaveRoom, response);
	}
	gtk_widget_hide(window2);
	gtk_widget_show(window);
}
   
int main (int argc, char *argv[]) {
    GtkWidget * button;
        
    gtk_init(argc, argv);
    
    window1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window1), "LOGIN TO CLIENT");
    
    g_signal_connect(GTK_WINDOW(window), "delete-event", G_CALLBACK (delete_event), NULL);
    g_signal_connect(GTK_WINDOW(window), "destroy", G_CALLBACK (destroy_event), NULL);
    
    gtk_container_set_border (GTK_CONTAINER(window), 10);
    
    GtkWidget * table = gtk_table_new(4, 3, TRUE);
    gtk_container_add(GTK_CONTAINER(window), table);
    
    GtkWidget * label1 = gtk_label_new("USERNAME: ");
    gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 1, 0, 1);
    gtk_widget_show(label1); 
    
    GtkWidget * label2 = gtk_label_new("PASSWORD: ");
    gtk_table_attach_defaults(GTK_TABLE(table), label2, 0, 1, 1, 2);
    gtk_widget_show(label2); 
    
   	button = gtk_button_new_with_label ("LOGIN");
	g_signal_connect (button, "clicked", G_CALLBACK (login_event), (gpointer) "Login Button");
	gtk_table_attach_defaults(GTK_TABLE(table), button, 1, 2, 3, 4);
	gtk_widget_show (button);
	
	button = gtk_button_new_with_label ("ADD NEW USER");
	g_signal_connect (button, "clicked", G_CALLBACK (add_event), (gpointer) "Add Button");
	gtk_table_attach_defaults(GTK_TABLE(table), button, 2, 3, 3, 4);
	gtk_widget_show (button);
	
	gtk_widget_show(table);
	gtk_widget_show(window);
	
	//*********WINDOW 2********
	
	
        
	gtk_main ();
   
  	return 0;
}


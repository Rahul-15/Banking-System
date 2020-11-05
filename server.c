#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/types.h>  
#include <stdbool.h>
#include <stdlib.h> 
#include <string.h>
#include "Account.c"
#include "Customer.c"
#include "Transaction.c"
#include <time.h>  

int getWords(char *base, char target[10][1024])
{
	int n=0,i,j=0;
	
	for(i=0;1;i++)
	{
		if(base[i]!='-'){
			target[n][j++]=base[i];
		}
		else{
			target[n][j++]='\0';//insert NULL
			n++;
			j=0;
		}
		if(base[i]=='\0')
		    break;
	}
	return n;
	
}

int main(){
    struct sockaddr_in server, client;
    int socket_desc, size_client, new_sd;
    char user_name[1024];char user_password[1024];
    
    socket_desc = socket(AF_INET,SOCK_STREAM,0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(1123);
    
    if(bind(socket_desc, (void *)(&server), sizeof(server)) < 0){
        perror("Error on binding:");
        exit(EXIT_FAILURE);
    }
    listen(socket_desc,5);

    while(1){
        size_client = sizeof(client);
        if(new_sd = accept(socket_desc, (struct sockaddr*)(&client),&size_client)<0){
            perror("Error on accept\n");
            exit(EXIT_FAILURE);
        }
        printf("..................Server is running now ...............\n");
        char buff[1024];
        if(fork() == 0){
            while(1){

                buff[0] = '\0';
                read(new_sd, user_name, sizeof(user_name));                                 // reading username from the client
                read(new_sd, user_password, sizeof(user_password));                         // reading password from the client
                printf("details recieved checking it... \n");
            
                struct Customer input;
                FILE *infile = fopen ("Customer_db", "r");                                  // checking if it is present or not
                if (infile == NULL){ 
                    fprintf(stderr, "\nError opening file\n"); 
                    exit (1); 
                }
                int find = 0;
                while(fread(&input, sizeof(struct Customer), 1, infile)) {                  // running through the database
                    if(strcmp(input.username, user_name) == 0 && strcmp(input.password, user_password) == 0){
                        send(new_sd,"authentic user", sizeof("authentic user"),0);
                        find = 1;
                        break;
                    }
                }
                if(find!=1){                                                                // if user is not found
                    send(new_sd,"user Not found", sizeof("user Not found"),0);              // telling the client
                    break;
                }
                printf("result send ..regarding authentication..\n");   


                char in[1024];                                                              // in - taking i/p from client
                char op[2048];                                                              // op - giving o/p to clinet

                if(strcmp(user_name,"admin") == 0){                                         // if user is admin.
                    
                    struct Customer input;                                                  // Open customer for reading 
                    
                    infile = fopen ("Customer_db", "r"); 
                    if (infile == NULL) { 
                        fprintf(stderr, "\nError opening file\n"); 
                        exit (1); 
                    }
                    int active = 0;
                    while(fread(&input, sizeof(struct Customer), 1, infile)){               // knowing the current admin status
                        if(strcmp(input.username, user_name) == 0 ){
                            active = input.status;
                            break;
                        }
                    }
                    
                    if(active ==  1){                                                       // telling to client if admin is already present
                        op[0] = '\0';
                        strcat(op,"cannot Enter as admin is already present\nBYE BYE");
                        send(new_sd,op, sizeof(op),0);
                        break;
                    }
                    
                    int no, found = 0;                                                      // setting admin status as active
                    infile = fopen("Customer_db", "rb+");
                    while ((fread(&input, sizeof(input), 1, infile)) > 0 && found == 0){
                        if (strcmp(input.username, user_name) == 0){
                            input.status = 1;
                            fseek(infile,  - (long)sizeof(input), 1);
                            fwrite(&input, sizeof(input), 1, infile);
                            found = 1;
                            break;
                        } 
                    }                    
                    printf("admin status changed to 1\n");
                    fclose(infile);


                    char buff[] = "Press 1 to add account\nPress 2 to delete account\nPress 3 to modify password\nPress 4 to search\nPress 5 to make joint account\nPress -1 to Exit\n";
                    
                    while(1){
                        send(new_sd,buff, sizeof(buff),0);                                  // printing admin menu
                        read(new_sd, in, sizeof(in));                                       // getting admin choice from client
                        printf("clinet choice: %s\n",in);

                        if(strcmp("1", in) == 0){                                           // add an account to file
                            
                            struct Customer input1; 

                            op[0] = '\0';
                            strcat(op,"Enter the credentials:\nEnter user name and password with a '-' in between\n");          // username as i/p
                            write(new_sd, op , sizeof(op));
                            in[0] = '\0';
                            read(new_sd, in, sizeof(in));  

                            int count; //number of words 
                            char arr[10][1024];
                            count=getWords(in,arr);

                            strcat(input1.username, arr[0]);
                            strcat(input1.password, arr[1]);
                            
                            struct Customer inp1;
                            int no, found = 0, f2 = 0;
                            infile = fopen("Customer_db", "rb+");
                            while ((fread(&inp1, sizeof(inp1), 1, infile)) > 0 && found == 0){
                                if (strcmp(inp1.username, input1.username) == 0){
                                    if(inp1.type == 1){
                                        f2 = 1;
                                        break;
                                    }
                                    inp1.type = 1;
                                    fseek(infile,  - (long)sizeof(inp1), 1);
                                    fwrite(&inp1, sizeof(inp1), 1, infile);
                                    found = 1;
                                    break;
                                } 
                            }
                            if(f2 == 1){
                                send(new_sd, "old user has already joint account", sizeof("old user has already joint account"),0);
                                break;
                            }

                            infile = fopen ("Customer_db", "ab");                           // creating entry in Customer_db
                            if (infile == NULL){  
                                fprintf(stderr, "\nError opend file\n"); 
                                exit (1); 
                            }

                            strcat(input1.account_number,input1.username);
                            strcat(input1.account_number,"2020");
                            input1.status = 0;
                            input1.type = 0;

                            fwrite (&input1, sizeof(struct Customer), 1, infile);
                            printf("Customer Created\n");
                            //write(new_sd,"Account Created ", sizeof("Account Created ")); 
                            fclose (infile);
                        
                            struct Account ac1;

                            infile = fopen ("Account_db", "ab");                            // creating entry in Account_db
                            if (infile == NULL){  
                                fprintf(stderr, "\nError opend file\n"); 
                                exit (1); 
                            }

                            ac1.account_number[0] = '\0';
                            strcat(ac1.account_number, input1.account_number);
                            ac1.balance = 0;
                            ac1.status  = 0;
                            
                            fwrite (&ac1, sizeof(struct Account), 1, infile);
                            printf("Account created\n");
                            fclose(infile);
                        
                        }
                        else if(strcmp("2",in)== 0){                                        // to delete account
                            op[0] = '\0';
                            in[0] = '\0';
                            strcat(op,"Enter username:");
                            send(new_sd, op , sizeof(op),0);
                            read(new_sd, in, sizeof(in));

                            printf("Username to be deleted: %s\n",in);
                            
                            struct Customer inp1;                                           // Open Customer_db for reading 
                            infile = fopen ("Customer_db", "rb+"); 

                            char accno[30];                                                                
                            while(fread(&inp1, sizeof(struct Customer), 1, infile)){
                                if(strcmp(inp1.username, in) == 0 ){
                                    strcat(accno, inp1.account_number);
                                    inp1.username[0] = '\0';
                                    strcat(inp1.username,"NONE");
                                    inp1.account_number[0] = '\0';
                                    strcat(inp1.account_number,"NONE");
                                    inp1.status = -1;
                                    inp1.type = -1;
                                    inp1.password[0] = '\0';
                                    strcat(inp1.password,"NONE");
                                    fseek(infile,  - (long)sizeof(inp1), 1);
                                    fwrite(&inp1, sizeof(inp1), 1, infile);
                                    printf("job done\n");
                                    break;
                                }
                            }
                            printf("Customer deleted by admin\n");                         
                            fclose (infile);
 

                            struct Account ac1;
                            int no=0, found = 0, rem_balance = 0;
                            infile = fopen("Account_db", "rb+");
                            while ((fread(&ac1, sizeof(ac1), 1, infile)) > 0 && found == 0){
                                if (strcmp(ac1.account_number, accno) == 0){
                                    rem_balance = ac1.balance;
                                    ac1.account_number[0] = '\0';
                                    strcat(ac1.account_number,"NONE");
                                    ac1.balance = -1;
                                    ac1.status = -1;
                                    fseek(infile,  - (long)sizeof(ac1), 1);
                                    fwrite(&ac1, sizeof(ac1), 1, infile);
                                    found = 1;
                                    break;
                                } 
                            }
                            printf("account deleted by admin\n");
                            fclose(infile);
                            //op[0] = '\0';
                            //strcat(op,""); // store rem_balance


                            struct Transaction tc1;
                            found = 0;
                            infile = fopen("Transaction_db", "rb+");
                            while ((fread(&tc1, sizeof(tc1), 1, infile)) > 0 && found == 0){
                                if (strcmp(tc1.account_number, accno) == 0){
                                    tc1.amount = -1;
                                    tc1.balance = -1;

                                    tc1.account_number[0] = '\0';
                                    strcat(tc1.account_number,"NONE");
                                    
                                    tc1.date[0] = '\0';
                                    strcat(tc1.date,"NONE");

                                    fseek(infile,  - (long)sizeof(tc1), 1);
                                    fwrite(&tc1, sizeof(tc1), 1, infile);
                                    found = 1;
                                    break;
                                } 
                            }
                            printf("transaction deleted by admin\n");
                            fclose(infile);

                        }
                        else if(strcmp("3",in) == 0){                                       // to modify password
                            
                            op[0] = '\0';
                            strcat(op,"Enter the username and the new password with '-' in between:\n");
                            send(new_sd,op, sizeof(op),0);
                            char help[1024];
                            in[0] = '\0';
                            read(new_sd, help, sizeof(help));
                            
                            printf("Input - %s\n",help);

                            int count; //number of words 
                            char arr[10][1024];
                            count=getWords(help,arr);
                       
                            char user_p[1024];
                            strcat(in,arr[0]);
                            user_p[0] = '\0';
                            strcat(user_p,arr[1]);
                            
                            printf("username - %s new_pass - %s\n",in,user_p);

                            FILE *file;
                            struct Customer data;
                            int no, found = 0;
                            file = fopen("Customer_db", "rb+");
                            while ((fread(&data, sizeof(data), 1, file)) > 0 && found == 0){
                                if (strcmp(data.username, in) == 0){
                                    data.password[0] = '\0';
                                    strcat(data.password,user_p);
                                    fseek(file,  - (long)sizeof(data), 1);
                                    fwrite(&data, sizeof(data), 1, file);
                                    found = 1;
                                    break;
                                } 
                            }
                            printf("password modified by admin\n");
                            fclose(file);
                        }
                        else if(strcmp("4", in) == 0){                                      // to search for user
                            op[0] = '\0';
                            strcat(op,"Enter username:");
                            send(new_sd, op , sizeof(op),0);
                            read(new_sd, in, sizeof(in)); 

                            struct Customer inp1;                                           // Open Customer_db for reading 
                            infile = fopen ("Customer_db", "r"); 
                            if (infile == NULL) { 
                                fprintf(stderr, "\nError opening file\n"); 
                                exit (1); 
                            } 
                                                                                            
                            while(fread(&inp1, sizeof(struct Customer), 1, infile)){
                                if(strcmp(inp1.username, in) == 0 ){
                                    op[0] = '\0';
                                    strcat(op,inp1.username);
                                    strcat(op," ");
                                    strcat(op,inp1.password);
                                    strcat(op," ");
                                    strcat(op,inp1.account_number);
                                    strcat(op,"\n");

                                    char *vOut = inp1.type ? "1" : "0";
                                    strcat(op,vOut);

                                    break;
                                }
                            }                         
                            fclose (infile);

                            struct Transaction inp2; 
                            infile = fopen ("Transaction_db", "r"); 
                            if (infile == NULL) { 
                                fprintf(stderr, "\nError opening file\n"); 
                                exit (1); 
                            } 
                            while(fread(&inp2, sizeof(struct Transaction), 1, infile)){
                                if(strcmp(inp1.account_number,inp2.account_number) == 0){
                                    strcat(op,inp2.date);
                                    strcat(op," ");

                                    char rupees[7];
                                    sprintf(rupees,"%d", inp2.amount);
                                    strcat(op,rupees);
                                    strcat(op," ");
                                    
                                    rupees[0] = '\0';
                                    sprintf(rupees,"%d", inp2.balance);
                                    strcat(op,rupees);
                                    strcat(op,"\n");
                                }
                            } 
                            fclose (infile);

                            write(new_sd,op, sizeof(op));
                            
                        }
                        else if(strcmp("5",in) == 0){                                       // to make joint account
                            
                            op[0] = '\0';
                            strcat(op,"By default the password will remain same\nEnter old bank holder username and new username with '-' in between\n");
                            send(new_sd, op, sizeof(op),0);
                            char help[1024];
                            help[0] = '\0';
                            read(new_sd, help, sizeof(help));
                            
                            /*
                            op[0] = '\0';
                            strcat(op, "Enter new user name :");
                            send(new_sd, op, sizeof(op),0);
                            read(new_sd, new_name, sizeof(new_name));
                            */
                            printf("i/p ----:%s\n",help);
                            char new_name[1024];new_name[0] = '\0';
                            char old_name[1024];old_name[0] = '\0';
                            
                            int count; //number of words 
                            char arr[10][1024];
                            count=getWords(help,arr);
                            strcat(old_name, arr[0]);
                            strcat(new_name, arr[1]);
                            
                            printf("old name :%s\n",old_name);
                            printf("new name: %s\n",new_name);
                            struct Customer inp1;
                            struct Customer new_inp;
                                          
                            int no, found = 0, f2 = 0;
                            infile = fopen("Customer_db", "rb+");
                            while ((fread(&inp1, sizeof(inp1), 1, infile)) > 0 && found == 0){
                                if (strcmp(inp1.username, old_name) == 0){
                                    if(inp1.type == 1){
                                        f2 = 1;
                                        break;
                                    }
                                    inp1.type = 1;
                                    fseek(infile,  - (long)sizeof(inp1), 1);
                                    fwrite(&inp1, sizeof(inp1), 1, infile);
                                    found = 1;
                                    break;
                                } 
                            }
                            if(f2 == 1){
                                send(new_sd, "old user has already joint account", sizeof("old user has already joint account"),0);
                                break;
                            }
                            if(found == 0){
                                send(new_sd, "not found old user in db", sizeof("not found old user in db"),0);
                                break;
                            }
                            printf("password modified by admin\n");
                            fclose(infile);

                            printf("new user name: %s\n", new_name);
                            new_inp.username[0] = '\0';
                            strcat(new_inp.username,new_name);
                            printf("stored new user name: %s\n", new_inp.username);

                            strcat(new_inp.password, inp1.password);
                            strcat(new_inp.account_number, inp1.account_number);
                            new_inp.type = 1;
                            new_inp.status = 0;; 
                            
                            infile = fopen ("Customer_db", "ab");                           // creating entry in Customer_db
                            if (infile == NULL){  
                                fprintf(stderr, "\nError opend file\n"); 
                                exit (1); 
                            }
                            fwrite (&new_inp, sizeof(struct Customer), 1, infile);
                            printf("Joint Customer Created\n");
                            //write(new_sd,"Account Created ", sizeof("Account Created ")); 
                            fclose (infile);

                        }
                        break;
                    }

                    infile = fopen ("Customer_db", "r"); 
                    if (infile == NULL) { 
                        fprintf(stderr, "\nError opening file\n"); 
                        exit (1); 
                    }
                    active = 0;
                    while(fread(&input, sizeof(struct Customer), 1, infile)){               // knowing the current admin status
                        if(strcmp(input.username, user_name) == 0 ){
                            active = input.status;
                            break;
                        }
                    }
                    
                    no = 0; found = 0;                                                      // setting admin status as inactive
                    infile = fopen("Customer_db", "rb+");
                    while ((fread(&input, sizeof(input), 1, infile)) > 0 && found == 0){
                        if (strcmp(input.username, user_name) == 0){
                            input.status = 0;
                            fseek(infile,  - (long)sizeof(input), 1);
                            fwrite(&input, sizeof(input), 1, infile);
                            found = 1;
                            break;
                        } 
                    }                    
                    printf("admin status changed to 0\n");
                    fclose(infile);                       
                }
                else{
                    struct Customer input;                                                  // Open customer for reading 
                    infile = fopen ("Customer_db", "r"); 
                    if (infile == NULL) { 
                        fprintf(stderr, "\nError opening file\n"); 
                        exit (1); 
                    }
                    int active = 0;
                    while(fread(&input, sizeof(struct Customer), 1, infile)){               // knowing the current user status
                        if(strcmp(input.username, user_name) == 0 ){
                            active = input.status;
                            break;
                        }
                    }
                    
                    if(active ==  1){                                                       // telling to client if user is already present
                        op[0] = '\0';
                        strcat(op,"cannot Enter as user is already present\n");
                        send(new_sd,op, sizeof(op),0);
                        break;
                    }
                    
                    int no, found = 0;                                                      // setting user status as active
                    infile = fopen("Customer_db", "rb+");
                    while ((fread(&input, sizeof(input), 1, infile)) > 0 && found == 0){
                        if (strcmp(input.username, user_name) == 0){
                            input.status = 1;
                            fseek(infile,  - (long)sizeof(input), 1);
                            fwrite(&input, sizeof(input), 1, infile);
                            found = 1;
                            break;
                        } 
                    }                    
                    printf("user status changed to 1\n");
                    fclose(infile);


                    char buff[] = "Press 1 to Deposite\nPress 2 to Withdrawl\nPress 3 for Balance Enquiry\nPress 4 to change password\nPress 5 to view transactions\nPress -1 to Exit\n";
                    while(1){
                        send(new_sd,buff, sizeof(buff),0);                                   // printing user menu.
                        in[0] = '\0';
                        read(new_sd, in, sizeof(in));
                        printf("clinet choice: %s\n",in);
                        
                        if(strcmp("1",in) == 0){                                            // to deposite
                            op[0] = '\0';
                            strcat(op,"Enter amount to deposite:\n");
                            send(new_sd,op, sizeof(op),0);
                            in[0] = '\0';
                            read(new_sd, in, sizeof(in));                            

                            int add;
                            sscanf(in, "%d", &add);

                            struct Customer inp1; 
                            infile = fopen ("Customer_db", "r"); 
                            if (infile == NULL) { 
                                fprintf(stderr, "\nError opening file\n"); 
                                exit (1); 
                            } 
                            // read file contents customer_db till end of file 
                            while(fread(&inp1, sizeof(struct Customer), 1, infile)){
                                if(strcmp(inp1.username, user_name) == 0 ){
                                    break;
                                }
                            }                        
                            fclose (infile);

                            infile = fopen("Account_db","rb+");
                            int fd = open("./Account_db", O_RDWR , 0664);
                            struct Account ac1;
                            int c = 0, found = 0;
    
                            struct flock lock;

                            while ((fread(&ac1, sizeof(ac1), 1, infile)) > 0 && found == 0){
                                if (strcmp(ac1.account_number, inp1.account_number) == 0){  // implementing locking on account
                                                                                            
                                    lock.l_type    = F_WRLCK; 
                                    lock.l_start   = c*sizeof(ac1);
                                    lock.l_whence  = SEEK_SET;
                                    lock.l_len     = sizeof(ac1);
                                    lock.l_pid     = getpid();
                                    printf("Write Lock Implemented, before enetering into the Critical section\n");
                                    printf("trying to get in cs\n");
                                    fcntl(fd, F_SETLKW, &lock);
                                    printf("Inside cs\n");
                                    ac1.balance += add;

                                    FILE *outfile;                                          // updateing the transaction logs
                                    outfile = fopen ("Transaction_db", "ab"); 
                                    if (outfile == NULL) { 
                                        fprintf(stderr, "\nError opend file\n"); 
                                        exit (1); 
                                    }
                                    
                                    char buf[100] = {0};
                                    char ans[100];ans[0] = '\0'; 
                                    
                                    time_t rawtime = time(NULL);
                                    if (rawtime == -1) {
                                        puts("The time() function failed");
                                        return 1;   
                                    }
                                    struct tm *ptm = localtime(&rawtime);
                                    if (ptm == NULL) {
                                        puts("The localtime() function failed");
                                        return 1;
                                    }
                                    memset(buf, 0, 100);
                                    strftime(buf, 100, "%T ", ptm);
                                    strcat(ans,buf);
                                    
                                    memset(buf, 0, 100);
                                    strftime(buf, 100, "%D", ptm);
                                    strcat(ans,buf);
                                    
                                    struct Transaction input2;
                                    strcat(input2.account_number, ac1.account_number);
                                    input2.amount = add;
                                    input2.balance = ac1.balance;
                                    strcat(input2.date,ans);
	                                fwrite (&input2, sizeof(struct Transaction), 1, outfile); 
                                    
                                    if(fwrite != 0) printf("contents to file written successfully !\n"); 
                                    else printf("error writing file !\n"); 
                                    
                                    fclose (outfile);                                       // closing the transactions

                                    fseek(infile,  - (long)sizeof(ac1), 1);
                                    fwrite(&ac1, sizeof(ac1), 1, infile);
                                    fcntl(fd, F_SETLK, &lock);
                                    close(fd);                                              // closeing lock and file
                                    break;
                                }
                                c++;
                            }
                            fclose(infile);
                        }
                        else if(strcmp("2",in) == 0){                                       // to withdrawl
                            op[0] = '\0';
                            strcat(op,"Enter amount to withdrawl:\n");
                            send(new_sd,op, sizeof(op),0);
                            in[0] = '\0';
                            read(new_sd, in, sizeof(in));                            

                            int add;
                            sscanf(in, "%d", &add);
                            add *= -1;

                            struct Customer inp1; 
                            infile = fopen ("Customer_db", "r"); 
                            if (infile == NULL) { 
                                fprintf(stderr, "\nError opening file\n"); 
                                exit (1); 
                            } 
                            // read file contents customer_db till end of file 
                            while(fread(&inp1, sizeof(struct Customer), 1, infile)){
                                if(strcmp(inp1.username, user_name) == 0 ){
                                    break;
                                }
                            }                        
                            fclose (infile);

                            infile = fopen("Account_db","rb+");
                            int fd = open("./Account_db", O_RDWR , 0664);
                            struct Account ac1;
                            int c = 0, found = 0;
    
                            struct flock lock;

                            while ((fread(&ac1, sizeof(ac1), 1, infile)) > 0 && found == 0){
                                if (strcmp(ac1.account_number, inp1.account_number) == 0){  // implementing locking on account
                                                                                            
                                    lock.l_type    = F_WRLCK; 
                                    lock.l_start   = c*sizeof(ac1);
                                    lock.l_whence  = SEEK_SET;
                                    lock.l_len     = sizeof(ac1);
                                    lock.l_pid     = getpid();
                                    printf("Write Lock Implemented, before enetering into the Critical section\n");
                                    printf("trying to get in cs\n");
                                    fcntl(fd, F_SETLKW, &lock);
                                    printf("Inside cs\n");
                                    ac1.balance += add;

                                    FILE *outfile;                                          // updateing the transaction logs
                                    outfile = fopen ("Transaction_db", "ab"); 
                                    if (outfile == NULL) { 
                                        fprintf(stderr, "\nError opend file\n"); 
                                        exit (1); 
                                    }
                                    
                                    char buf[100] = {0};
                                    char ans[100];ans[0] = '\0'; 
                                    
                                    time_t rawtime = time(NULL);
                                    if (rawtime == -1) {
                                        puts("The time() function failed");
                                        return 1;   
                                    }
                                    struct tm *ptm = localtime(&rawtime);
                                    if (ptm == NULL) {
                                        puts("The localtime() function failed");
                                        return 1;
                                    }
                                    memset(buf, 0, 100);
                                    strftime(buf, 100, "%T ", ptm);
                                    strcat(ans,buf);
                                    
                                    memset(buf, 0, 100);
                                    strftime(buf, 100, "%D", ptm);
                                    strcat(ans,buf);
                                    
                                    struct Transaction input2;
                                    strcat(input2.account_number, ac1.account_number);
                                    input2.amount = add;
                                    input2.balance = ac1.balance;
                                    strcat(input2.date,ans);
	                                fwrite (&input2, sizeof(struct Transaction), 1, outfile); 
                                    
                                    if(fwrite != 0) printf("contents to file written successfully !\n"); 
                                    else printf("error writing file !\n"); 
                                    
                                    fclose (outfile);                                       // closing the transactions

                                    fseek(infile,  - (long)sizeof(ac1), 1);
                                    fwrite(&ac1, sizeof(ac1), 1, infile);
                                    fcntl(fd, F_SETLK, &lock);
                                    close(fd);                                              // closeing lock and file
                                    break;
                                }
                                c++;
                            }
                            fclose(infile);
                        }
                        else if(strcmp("3", in) == 0){                                      // balance enquiry

                            // Customer database
                            struct Customer inp1; // Open person.dat for reading 
                            infile = fopen ("Customer_db", "r"); 
                            if (infile == NULL) { 
                                fprintf(stderr, "\nError opening file\n"); 
                                exit (1); 
                            } 
                            // read file contents customer_db till end of file 
                            while(fread(&inp1, sizeof(struct Customer), 1, infile)){
                                if(strcmp(inp1.username, user_name) == 0 ){
                                    break;
                                }
                            }                        
                        
                            fclose (infile);
 
                           //Account database
                           struct Account ac1;
                           infile = fopen ("Account_db", "r");
                           if (infile == NULL) { 
                               fprintf(stderr, "\nError opening file\n"); 
                               exit (1); 
                           }
                           while(fread(&ac1, sizeof(struct Account), 1, infile)){
                               if(strcmp(ac1.account_number, inp1.account_number) == 0 ){
                                   op[0] = '\0';
                                   strcat(op,"Balance: ");
                                   char rupees[7];
                                   sprintf(rupees,"%d", ac1.balance);
                                   strcat(op,rupees);
                                   send(new_sd,op, sizeof(op),0);
                                   break;
                               }
                            }
                            fclose (infile);
                        }
                        else if(strcmp("4",in) == 0){                                       // to modify password
                                                     
                            char user_p[1024];
                            op[0] = '\0';
                            strcat(op,"Enter the new password :\n");
                            send(new_sd,op, sizeof(op),0);
                            read(new_sd, user_p, sizeof(user_p));
                            

                            FILE *file;
                            struct Customer data;
                            int no, found = 0;
                            file = fopen("Customer_db", "rb+");
                            while ((fread(&data, sizeof(data), 1, file)) > 0 && found == 0){
                                if (strcmp(data.username, user_name) == 0){
                                    data.password[0] = '\0';
                                    strcat(data.password,user_p);
                                    fseek(file,  - (long)sizeof(data), 1);
                                    fwrite(&data, sizeof(data), 1, file);
                                    found = 1;
                                    break;
                                } 
                            }
                            printf("password modified by user\n");
                            fclose(file);
                        }
                        else if(strcmp("5", in) == 0){                                      // to view transactions
                            op[0] = '\0';
                            
                            struct Customer inp1; // Open person.dat for reading 
                            infile = fopen ("Customer_db", "r"); 
                            if (infile == NULL) { 
                                fprintf(stderr, "\nError opening file\n"); 
                                exit (1); 
                            }
                            // read file contents customer_db till end of file 
                            while(fread(&inp1, sizeof(struct Customer), 1, infile)){
                                if(strcmp(inp1.username, user_name) == 0 ){
                                    break;
                                }
                            }                        
                            // close file 
                            fclose (infile);
 
                            //Transaction database
                            struct Transaction tc1;
                            infile = fopen ("Transaction_db", "r");
                            if (infile == NULL) { 
                                fprintf(stderr, "\nError opening file\n"); 
                                exit (1); 
                            }
                            op[0] = '\0';
                            strcat(op,"date\t account_no\t balance\n");
                            while(fread(&tc1, sizeof(struct Transaction), 1, infile)){
                                if(strcmp(tc1.account_number, inp1.account_number) == 0 ){
                                    strcat(op,tc1.date);strcat(op," ");
                                    strcat(op, tc1.account_number);strcat(op," ");
                                    
                                    char rupees[7];
                                    sprintf(rupees,"%d", tc1.amount);
                                    strcat(op,rupees);strcat(op," ");
                                
                                    rupees[0] = '\0';
                                    sprintf(rupees,"%d", tc1.balance);
                                    strcat(op,rupees);strcat(op,"\n");
                                }
                            }
                            fclose (infile);
                            write(new_sd,op, sizeof(op));
                        }
                        
                        break; 
                    }
                    
                    no = 0; found = 0;                                                      // setting admin status as inactive
                    infile = fopen("Customer_db", "rb+");
                    while ((fread(&input, sizeof(input), 1, infile)) > 0 && found == 0){
                        if (strcmp(input.username, user_name) == 0){
                            input.status = 0;
                            fseek(infile,  - (long)sizeof(input), 1);
                            fwrite(&input, sizeof(input), 1, infile);
                            found = 1;
                            break;
                        } 
                    }                    
                    printf("user status changed to 0\n");
                    fclose(infile);
                }
                
                close(new_sd);
                exit(EXIT_SUCCESS);
            }
        }
        else{
            close(new_sd);
        }
    }
    return 0;
}
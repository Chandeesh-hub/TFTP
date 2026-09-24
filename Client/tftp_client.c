
/********************************************************************************************************************

    Name : Chandeesh K M
    Date : 13/09/2026

    Description : The TFTP Server and Client (Trivial File Transfer Protocol) is a file transfer protocol
    wokrs based on UDP (User Datagram Protocol) to transfer files between a client and server. In Client
    its a menu based interface with operation like Connect, Put, Get, Mode and Exit. In Server Create a 
    socket and bind with th server and run a while loop to recvfrom client. In Connect operation the user 
    will give a IP address and validate the ip address if the ip address is invalid print error. If the 
    ip address is valid then create a socket of ipv4 with UDP. In put operation the user enter a file name
    then check file is present or not. If file is not present in client print error and if file is present
    send the file name to Server with WRQ request. Then the server receive the RRQ request from client and
    and create the a file in server side and send back ack to the client. After Receving the ack from server
    the client will send the data to server untill end of file. The server will receive the data and write 
    into server file. In Get operation the user enter a filename then the file will send to server with RRQ
    request and the server will check the file is present in the server. If the file is not present send ack
    as failure tio client. In client print error and goto menu. If the file is present in server send ack 
    as sucess to client and in client create a file and send ack success to server. In Server after receive
    the ack, send the data to the client untill eof of file and in client write the data into the file. In
    Mode operation User are got 3 option in it default, octet and netascii. In default mode the client and
    the server will transfer data in 512 bytes. In octet mode the client and the server will transfer data 
    in byte. In netascii mode the client and the server will transfer data in 512 bytes. While transfering
    the data to server whenever newline is found '\n' add carrage return '\r' to the next byte. While 
    transfering the data to client whenerver carrage return and newline is found '\r''\n' remove the carrage
    return '\r' and keep newline only '\n' only. In Exit mode the socket will be disconnected from the
    server and exit from the client.

    
**********************************************************************************************************************/
#include "tftp.h"
#include "tftp_client.h"

// Declare varibale gobal for mode
int mode=1;
int data_size=512;

int main()
{
    // Declare the varibales
    tftp_client_t client;
    char inputipaddress[20];
    char filename[35];

    // Clear the client and make it as 0
    memset(&client, 0, sizeof(client));
    int choice;

    do{

        choice=5;
        // Display menu
        printf("\n\tMENU\n1. Connect\n2. Put\n3. Get\n4. Mode\n5. Exit\n\n");
        printf("Enter the option: ");
        scanf("%d",&choice);

        switch(choice)
        {
            case 1:
                    // Collect IP address from User
                    printf("Enter the IP Address to Connect: ");
                    scanf(" %[^\n]",inputipaddress);

                    // Validate IP address
                    if(validate_ip(inputipaddress)==FAILURE)
                    {
                        puts("Invalid IP Address\nUsage: 0-255.0-255.0-255.0-255");
                        break;
                    }

                    // Connect to server
                    connect_to_server(&client,inputipaddress,PORT);
                    break;

            case 2:
                    // Get filename from user
                    printf("Enter the file name: ");
                    scanf(" %[^\n]",filename);

                    // Function call to put file in server
                    put_file(&client,filename);
                    break;

            case 3:
                    // Get filename from user
                    printf("Enter the file name: ");
                    scanf(" %[^\n]",filename);

                    // Function call to get file from server
                    get_file(&client,filename);
                    break;

            case 4:
                    // Display transfer modes
                    printf("\n1. Default\n2. Octet\n3. Netascii\n");
                    printf("Enter the Mode: ");
                    scanf("%d",&mode);

                    // For default, netascii set 512byte
                    // Octet = 1 byte per packet, as requested
                    if(mode==1)
                    {
                        data_size=512;
                        printf("Default Mode Selected\n");
                    }
                    else if(mode==2)
                    {
                        data_size=1;
                        printf("Octet Mode Selected\n");
                    }
                    else if(mode==3)
                    {
                        data_size=512;
                        printf("Netascii Mode Selected\n");
                    }
                    else
                    {
                        puts("Invalid Mode");
                        mode=1;
                        data_size=512;
                    }
                    break;

            case 5:
                    // Exit
                    disconnect(&client);

            default:
                    // Invalid option
                    puts("Invalid Input!\n");
        }

    }while(choice!=5);

    return 0;
}

// Function to connect with server
void connect_to_server(tftp_client_t *client, char *ip, int port)
{
    // Create UDP socket
    client->sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(client->sockfd==-1)
    {
        perror("Socket");
        return;
    }

    // Set socket timeout
    // Set 5 second timeout for send and receive
    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;

    // Set receive timeout and send timeout
    setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO,(const char*)&tv, sizeof(tv));

    setsockopt(client->sockfd, SOL_SOCKET, SO_SNDTIMEO,(const char*)&tv, sizeof(tv));

    // Update the Client details
    client->server_addr.sin_family=AF_INET;
    client->server_addr.sin_port=htons(port);
    client->server_addr.sin_addr.s_addr=inet_addr(ip);
    client->server_len=sizeof(client->server_addr);

    // Connect UDP socket to server
    if(connect(client->sockfd,(struct sockaddr*)&(client->server_addr),client->server_len)==-1)
    {
        perror("Connect");
        return;
    }

    // Print server details
    printf("Connected to Server : %s : %d \n",ip,port);
}

// Put Function
void put_file(tftp_client_t *client, char *filename)
{
    // Check client is connected to server
    if(client->sockfd==0)
    {
        puts("Not Connected to server");
        return;
    }

    // Open the file to read data
    int file_fd=open(filename,O_RDONLY);
    if(file_fd==-1)
    {
        perror("File");
        return;
    }

    // Declare packet
    tftp_packet packet;
    memset(&packet, 0, sizeof(packet));

    // Set write request to Server
    packet.opcode=WRQ;
    strcpy(packet.body.request.filename,filename);

    // Set transfer mode
    if(mode==1)
    {
        strcpy(packet.body.request.mode,"default");
    }
    else if(mode==2)
    {
        strcpy(packet.body.request.mode,"octet");
    }
    else if(mode==3)
    {
        strcpy(packet.body.request.mode,"netascii");
    }

    printf("Transfer Mode: ");
    printf("%s\n",packet.body.request.mode);
    int byte;

    // Send write request to server (retry on timeout/failure)
    int retries=0,ack=0;
    while(retries< MAX_RETRIES && !ack)
    {
        byte=sendto(client->sockfd,&packet,BUFFER_SIZE,0,(struct sockaddr*)(&client->server_addr),client->server_len);
        if(byte==-1)
        {
            perror("Sendto");
            retries++;
            continue;
        }

        // Receive ACK from server
        byte=recvfrom(client->sockfd,&packet,sizeof(packet),0,(struct sockaddr*)&client->server_addr,&client->server_len);
        if(byte==-1)
        {
            printf("Timeout waiting for WRQ ACK\n");
            retries++;
            continue;
        }
        ack=1;
    }
    if(!ack)
    {
        puts("Server not responding, aborting transfer");
        close(file_fd);
        return;
    }

    // Check server ACK
    if(packet.opcode==ACK)
    {
        if(packet.body.ack_packet.block_number!=SUCCESS)
        {
            puts("ERROR: File cannot be transferred");
            close(file_fd);
            return;
        }
    }

    // Run loop to read data
    while(1)
    {
        int read_count=0;

        // Read data from file for octect and default mode
        if(mode==1 || mode==2)
        {
            read_count=read(file_fd,&packet.body.data_packet.data,data_size);
        }

        // Convert data to netascii
        else if(mode==3)
        {
            char ch;
            read_count=0;

            // Read file data
            while(read_count<512)
            {
                int ret=read(file_fd,&ch,1);

                // Check end of file
                if(ret==0)
                {
                    break;
                }

                // Check read error
                if(ret==-1)
                {
                    perror("read");
                    close(file_fd);
                    return;
                }

                // change \n to \r\n
                if(ch=='\n')
                {
                    if(read_count<511)
                    {
                        packet.body.data_packet.data[read_count]='\r';
                        read_count++;

                        packet.body.data_packet.data[read_count]='\n';
                        read_count++;
                    }
                    else
                    {
                        // Move file pointer back by one byte
                        lseek(file_fd,-1,SEEK_CUR);
                        break;
                    }
                }
                else
                {
                    packet.body.data_packet.data[read_count]=ch;
                    read_count++;
                }
            }
        }

        // Check read error
        if(read_count==-1)
        {
            perror("read");
            close(file_fd);
            return;
        }

        // Check end of file
        if(read_count==0)
        {
            // Send zero length packet in octet mode
            if(mode==2)
            {
                packet.opcode=DATAW;
                packet.body.data_packet.block_number=0;

                // Send last (zero-length) packet, with retry
                while(retries<MAX_RETRIES && !ack)
                {
                    byte=sendto(client->sockfd,&packet,BUFFER_SIZE,0,(struct sockaddr*)(&client->server_addr),client->server_len);
                    if(byte==-1)
                    {
                        perror("Sendto");
                        retries++;
                        continue;
                    }

                    // Receive final ACK
                    byte=recvfrom(client->sockfd,&packet,sizeof(packet),0,(struct sockaddr*)&client->server_addr,&client->server_len);
                    if(byte==-1)
                    {
                        printf("Timeout waiting for final ACK\n");
                        retries++;
                        continue;
                    }
                    ack=1;
                }
                if(!ack)
                {
                    puts("Failed to confirm end of transfer after max retries");
                    close(file_fd);
                    return;
                }
            }

            // Transfer completed
            printf("File: %s Transfer Completed\n",filename);
            break;
        }

        // Prepare data packet
        packet.opcode=DATAW;
        packet.body.data_packet.block_number=read_count;

        // Send data packet to server, retrying this SAME block on timeout
        retries=0;
        ack=0;
        while(retries<MAX_RETRIES && !ack)
        {
            byte=sendto(client->sockfd,&packet,BUFFER_SIZE,0,(struct sockaddr*)(&client->server_addr),client->server_len);
            if(byte==-1)
            {
                perror("Sendto");
                retries++;
                continue;
            }

            // Receive ACK from server
            byte=recvfrom(client->sockfd,&packet,sizeof(packet),0,(struct sockaddr*)&client->server_addr,&client->server_len);
            if(byte==-1)
            {
                printf("Timeout waiting for DATA ACK\n");
                retries++;
                continue;
            }

            // Check ACK packet
            if(packet.opcode!=ACK)
            {
                puts("Error while transferring data");
                close(file_fd);
                return;
            }
            ack=1;
        }
        if(!ack)
        {
            puts("Transfer failed after max retries");
            close(file_fd);
            return;
        }

        // Clear data buffer
        memset(&packet.body.data_packet.data,0,sizeof(packet.body.data_packet.data));
    }

    // Close file
    close(file_fd);
}

// Get function
void get_file(tftp_client_t *client, char *filename)
{
    // Check client connected to server
    if(client->sockfd==0)
    {
        puts("Not Connected to server");
        return;
    }

    tftp_packet packet;
    memset(&packet, 0, sizeof(packet));

    // Set Read request to Server
    packet.opcode=RRQ;
    strcpy(packet.body.request.filename,filename);

    // Update mode
    if(mode==1)
    {
        strcpy(packet.body.request.mode,"default");
    }
    else if(mode==2)
    {
        strcpy(packet.body.request.mode,"octet");
    }
    else if(mode==3)
    {
        strcpy(packet.body.request.mode,"netascii");
    }

    printf("Transfer Mode: ");
    printf("%s\n",packet.body.request.mode);
    int byte;

    // Send read request to server (retry on timeout/failure)
    int retries=0;
    int ack=0;
    while(retries<MAX_RETRIES && !ack)
    {
        byte=sendto(client->sockfd,&packet,BUFFER_SIZE,0,(struct sockaddr*)(&client->server_addr),client->server_len);
        if(byte==-1)
        {
            perror("Sendto");
            retries++;
            continue;
        }

        // Receive ACK from server
        byte=recvfrom(client->sockfd,&packet,sizeof(packet),0,(struct sockaddr*)&client->server_addr,&client->server_len);
        if(byte==-1)
        {
            printf("Timeout waiting for RRQ ACK\n");
            retries++;
            continue;
        }
        ack=1;
    }
    if(!ack)
    {
        puts("Server not responding, aborting transfer");
        return;
    }

    // Check file is present in server or not
    if(packet.body.ack_packet.block_number!=SUCCESS)
    {
        puts("File is Not Present in Server");
        return;
    }

    // Create file for received data
    int file_fd=open(filename,O_CREAT|O_TRUNC|O_WRONLY,0644);
    if(file_fd==-1)
    {
        perror("File");
        return;
    }

    // Update opcode to Read data
    packet.opcode=DATAR;
    strcpy(packet.body.request.filename,filename);

    while(1)
    {
        // Request data from server, retrying on timeout/failure
        retries=0;
        ack=0;
        while(retries<MAX_RETRIES && !ack)
        {
            byte=sendto(client->sockfd,&packet,BUFFER_SIZE,0,(struct sockaddr*)(&client->server_addr),client->server_len);
            if(byte==-1)
            {
                perror("Sendto");
                retries++;
                continue;
            }

            // Receive data from server
            byte=recvfrom(client->sockfd,&packet,sizeof(packet),0,(struct sockaddr*)&client->server_addr,&client->server_len);
            if(byte==-1)
            {
                printf("Timeout waiting for DATA\n");
                retries++;
                continue;
            }
            ack=1;
        }
        if(!ack)
        {
            puts("Transfer failed after max retries");
            close(file_fd);
            return;
        }

        // Get received byte count
        int char_count=packet.body.data_packet.block_number;

        // Write data into file for octect and default mode
        if(mode==1 || mode==2)
        {
            write(file_fd,packet.body.data_packet.data,char_count);
        }

        // Convert to netascii data
        else if(mode==3)
        {
            for(int i=0;i<char_count;i++)
            {
                // Adding \n to \r
                if(packet.body.data_packet.data[i]=='\r' && i+1<char_count && packet.body.data_packet.data[i+1]=='\n')
                {
                    char ch='\n';
                    write(file_fd,&ch,1);
                    i++;
                }
                else
                {
                    write(file_fd,&packet.body.data_packet.data[i],1);
                }
            }
        }

        // Check octet mode completion
        if(mode==2 && char_count==0)
        {
            printf("Transfer Completed\n");
            break;
        }

        // Check normal mode completion
        if(mode!=2 && char_count<data_size)
        {
            printf("Transfer Completed\n");
            break;
        }

        // Clear received packet and set opcode
        memset(&packet,0,sizeof(packet));
        packet.opcode=DATAR;
    }

    // Close received file
    close(file_fd);
}

void disconnect(tftp_client_t *client)
{
    // Close client socket
    close(client->sockfd);

    // Exit program
    printf("Exited!...\n");
    exit(0);
}

// Validate the ip address given by user
int validate_ip(char *ipaddress)
{
    int len=strlen(ipaddress);
    if(len>15)
    {
        return FAILURE;
    }

    // Declare the variable
    int i=0,dotcount=0,num=0;
    int arr[4]={0,0,0,0};

    // Run a loop and validate
    while(i<len+1)
    {
        if(ipaddress[i]!='.' && ipaddress[i]!='\0')
        {
            // Check the given ipaddress is present only number.
            if(ipaddress[i]<'0' || ipaddress[i]>'9')
            {
                return FAILURE;
            }

            num=(num*10)+(ipaddress[i]-'0');
        }

        // Store the number in array
        if(ipaddress[i]=='.' || ipaddress[i]=='\0')
        {
            if(i==0 || ipaddress[i-1]==':')
            {
                return FAILURE;
            }
            else
            {
                // Check if more than 3 dot
                if(dotcount>4)
                {
                    return FAILURE;
                }

                arr[dotcount]=num;
                dotcount++;
                num=0;
            }
        }

        i++;
    }

    // If dot is not 3 
    if(dotcount!=4)
    {
        return FAILURE;
    }

    // Check if the number is greater than 255
    for(int i=0;i<4;i++)
    {
        if(arr[i]>255)
        {
            return FAILURE;
        }
    }

    // If validate ipaddress return Success
    return SUCCESS;
}

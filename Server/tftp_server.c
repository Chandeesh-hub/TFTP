#include "tftp.h"

// Declare varibale gobal for mode
int mode=1;
int data_size=512;

// Function declaration to handle
void handle_client(int *,int ,int , struct sockaddr_in , socklen_t , tftp_packet *);

int main()
{
    // Declare the varibale
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    tftp_packet packet;
    int file_fd=0;
    int byte;

    // Create UDP socket
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd==-1)
    {
        perror("Socket");
        return 1;
    }

    // Set timeout for receive and send
    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

    // Update the server address
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    server_addr.sin_addr.s_addr=inet_addr("192.168.163.86");

    // Bind socket with server 
    if(bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1)
    {
        perror("Bind");
        return 1;
    }

    printf("TFTP Server listening on port %d...\n", PORT);
    printf("Waiting for File and Operation\n");

    while(1)
    {
        // Receive packet from client
        byte=recvfrom(sockfd,&packet,BUFFER_SIZE,0,(struct sockaddr*)&client_addr,&client_len);

        // Check receive error or timeout
        if(byte<0)
        {
            perror("Receive failed or timeout occurred");
            continue;
        }
        
        // Print client details
        printf("Client Details %s:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        
        // Handle client request
        handle_client(&file_fd,byte,sockfd,client_addr,client_len,&packet);
    }

    // Close server socket
    close(sockfd);

    return 0;
}

void handle_client(int *file_fd,int byte,int sockfd,struct sockaddr_in client_addr,socklen_t client_len,tftp_packet *packet)
{
    // Check receive error
    if(byte==-1)
    {
        perror("Recvfrom");
        return ;
    }

    // Check empty packet
    if(byte==0)
    {
        return;
    }

    // If opcode is WRQ
    if(packet->opcode==WRQ)
    {
        
        // Get mode and update
        if(strcmp(packet->body.request.mode,"octet")==0)
        {
            mode=2;
            data_size=1;
        }
        else if(strcmp(packet->body.request.mode,"netascii")==0)
        {
            mode=3;
            data_size=512;
        }
        else
        {
            mode=1;
            data_size=512;
        }

        // Print selected transfer mode
        printf("Transfer Mode : %s\n",packet->body.request.mode);
        printf("Transfer Size : %d bytes\n",data_size);

        // Create and Open file
        *file_fd=open(packet->body.request.filename,O_CREAT|O_WRONLY|O_TRUNC,0644);
        if(*file_fd==-1)
        {
            perror("OPEN");
            return ;
        }

        printf("File opened successfully for writing: %s\n",packet->body.request.filename);

        // For ACK packet
        packet->opcode=ACK;
        packet->body.ack_packet.block_number=SUCCESS;

        // Send ACK to client
        byte=sendto(sockfd,packet,BUFFER_SIZE,0,(struct sockaddr*)&client_addr,client_len);
        if(byte==-1)
        {
            perror("Sendto");
            return ;
        }
    }

    // If opcode is RRQ
    if(packet->opcode==RRQ)
    {
        
        // Get transfer mode from client
        if(strcmp(packet->body.request.mode,"octet")==0)
        {
            mode=2;
            data_size=1;
        }
        else if(strcmp(packet->body.request.mode,"netascii")==0)
        {
            mode=3;
            data_size=512;
        }
        else
        {
            mode=1;
            data_size=512;
        }

        // Print selected transfer mode
        printf("Transfer Mode : %s\n",packet->body.request.mode);
        printf("Transfer Size : %d bytes\n",data_size);

        // Open file 
        *file_fd=open(packet->body.request.filename,O_RDONLY);

        // If file is not present send failure
        // If file is present send success
        if(*file_fd==-1)
        {
            packet->opcode=ACK;
            packet->body.ack_packet.block_number=FAILURE;

            // Send failure ACK
            byte=sendto(sockfd,packet,BUFFER_SIZE,0,(struct sockaddr*)&client_addr,client_len);
            if(byte==-1)
            {
                perror("Sendto");
                return;
            }
            return ;
        }
        else
        {
            // Send success ACK
            packet->opcode=ACK;
            packet->body.ack_packet.block_number=0;

            printf("File opened successfully for Reading: %s\n",packet->body.request.filename);

            byte=sendto(sockfd,packet,BUFFER_SIZE,0,(struct sockaddr*)&client_addr,client_len);
            if(byte==-1)
            {
                perror("Sendto");
                return;
            }
            return;
        }
    }

    // If opcode is DATAW
    if(packet->opcode==DATAW)
    {
        // Get number of bytes received
        int char_count=packet->body.data_packet.block_number;
        printf("DATA received \n");

        // Convert netascii data
        if(mode==3)
        {
            for(int i=0;i<char_count;i++)
            {
                // Add \n to \r
                if(packet->body.data_packet.data[i]=='\r' && i+1<char_count && packet->body.data_packet.data[i+1]=='\n')
                {
                    char ch='\n';
                    write(*file_fd,&ch,1);
                    i++;
                }
                else
                {
                    write(*file_fd,&packet->body.data_packet.data[i],1);
                }
            }
        }
        else
        {
            // Write received data to file
            write(*file_fd,packet->body.data_packet.data,char_count);
        }

        // Prepare ACK packet
        packet->opcode=ACK;
        packet->body.ack_packet.block_number=char_count;

        // Send ACK to client
        printf("Sending ACK \n");
        int retries=0;
        int ack=-1;
        while(retries<MAX_RETRIES)
        {
            ack=sendto(sockfd,packet,BUFFER_SIZE,0,(struct sockaddr*)&client_addr,client_len);
            if(ack_sent!=-1) 
            {
                break;
            }
            perror("Sendto ACK");
            retries++;
        }

        // Check octet mode end of transfer
        if(mode==2 && char_count==0)
        {
            printf("Transfer Completed to file\n");
            close(*file_fd);
            return;
        }

        // Check normal end of transfer
        if(char_count<data_size)
        {
            printf("Transfer Completed to file\n");
            close(*file_fd);
            return;
        }
    }

    // Check data request from client
    if(packet->opcode==DATAR)
    {
        // Read data from file
        int read_count=read(*file_fd,packet->body.data_packet.data,data_size);

        // Check read error
        if(read_count==-1)
        {
            perror("read");
            close(*file_fd);
            return;
        }

        // Check end of file
        if(read_count==0)
        {
            // Send last packet
            packet->opcode=DATAR;
            packet->body.data_packet.block_number=0;

            sendto(sockfd,packet,BUFFER_SIZE,0,(struct sockaddr*)&client_addr,client_len);
            puts("Transfer Completed");
            close(*file_fd);
            return;
        }

        // Convert file data to netascii
        if(mode==3)
        {
            memset(packet->body.data_packet.data,0,sizeof(packet->body.data_packet.data));

            // Move file pointer back
            lseek(*file_fd,-read_count,SEEK_CUR);
            read_count=0;
            char ch;

            // Read and convert data
            while(read_count<512)
            {
                int ret=read(*file_fd,&ch,1);

                // Check end of file
                if(ret==0)
                {
                    break;
                }

                // Check read error
                if(ret==-1)
                {
                    perror("read");
                    close(*file_fd);
                    return;
                }

                // Add \n to \r
                if(ch=='\n')
                {
                    if(read_count<511)
                    {
                        packet->body.data_packet.data[read_count]='\r';
                        read_count++;

                        packet->body.data_packet.data[read_count]='\n';
                        read_count++;
                    }
                    else
                    {
                        // Move file pointer back by one byte
                        lseek(*file_fd,-1,SEEK_CUR);
                        break;
                    }
                }
                else
                {
                    packet->body.data_packet.data[read_count]=ch;
                    read_count++;
                }
            }
        }

        // Set opcode DATAR
        packet->opcode=DATAR;
        packet->body.data_packet.block_number=read_count;
        printf("Sending DATA \n");

        // Send data to client 
        int retries=0;
        byte=-1;
        while(retries<MAX_RETRIES)
        {
            byte=sendto(sockfd,packet,BUFFER_SIZE,0,(struct sockaddr*)&client_addr,client_len);
            if(byte!=-1) 
            {
                break;
            }
            perror("Sendto");
            retries++;
        }
        if(byte==-1)
        {
            puts("Failed to send DATA after max retries");
            return;
        }

        // Check end of transfer
        if(read_count<data_size)
        {
            puts("Transfer Completed");
            close(*file_fd);
        }
    }
}

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

int send_seq_number, ack_recv, last_check_sum, recv_seq_number, buffer_len, buffer_start, total_buffer, next_ack;
char last_sent_message[20];
struct msg buffer_struct[1000];

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
	struct pkt send_packet;
	int i=0,check_sum=0;
	
	if(total_buffer == 0 && ack_recv == 0)
	{		
		for(i=0;i<20;i++)
		{
			check_sum=check_sum+(int)(message.data[i]);
			
		}
		
		check_sum = check_sum + send_seq_number;
		
		send_packet.seqnum = send_seq_number;
		send_packet.acknum = 0;
		send_packet.checksum = check_sum;
		strcpy(send_packet.payload,message.data);
		
		last_check_sum = check_sum;
		
		for(int i=0;i<20;i++)
		{
			last_sent_message[i]=message.data[i];
		}
		last_sent_message[20] = '\0';
				
		tolayer3(0, send_packet);
		starttimer (0, 11.0);
		
		ack_recv = 1;
		send_seq_number = send_seq_number^1;
	}
	else
	{
		memcpy(&buffer_struct[buffer_len],&message.data,sizeof(buffer_struct[buffer_len]));
		buffer_len++;
		total_buffer++;

	}

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
  
{
	
	if((packet.checksum == (packet.seqnum*2)+(65)) && (strcmp(packet.payload,"A")==0) && (send_seq_number^1) == packet.seqnum && packet.acknum == next_ack)
	{
		stoptimer(0);
		ack_recv = 0;
		next_ack = next_ack^1;
		
		if(total_buffer > 0)
		{
			int i=0,check_sum=0;
			struct pkt send_packet;
					
			for(i=0;i<20;i++)
			{
				check_sum=check_sum+(int)(buffer_struct[buffer_start].data[i]);
				send_packet.payload[i]=buffer_struct[buffer_start].data[i];
				last_sent_message[i]=buffer_struct[buffer_start].data[i];
				
			}
			
			send_packet.payload[20]='\0';
			last_sent_message[20] = '\0';
			
			check_sum = check_sum + send_seq_number;
			
			send_packet.seqnum = send_seq_number;
			send_packet.acknum = 0;
			send_packet.checksum = check_sum;
				
			
			send_packet.payload[20]='\0';
			last_sent_message[20] = '\0';
			
			last_check_sum = check_sum;
			
			
			tolayer3(0, send_packet);
			starttimer (0, 11.0);
						
			ack_recv = 1;
			send_seq_number = send_seq_number^1;
			buffer_start++;
			total_buffer--;
			
			
		}
	}
	
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	struct pkt send_packet;
		
	send_packet.seqnum = send_seq_number^1;
	send_packet.acknum = 0;
	send_packet.checksum = last_check_sum;
	
	for(int i=0;i<20;i++)
	{
		send_packet.payload[i]=last_sent_message[i];
	}
	
	send_packet.payload[20]='\0';
		
	tolayer3(0, send_packet);
	starttimer (0, 11.0);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	send_seq_number=0;
	ack_recv = 0;
	last_check_sum = 0;
	buffer_start = 0;
	total_buffer = 0;
	next_ack = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
	int i=0, check_sum=0;
	struct pkt send_ack;
				
	for(i=0;i<20;i++)
	{
		check_sum=check_sum+(int)packet.payload[i]; 		
	}
	
	
	check_sum = check_sum+packet.seqnum+packet.acknum;
			
	if(check_sum == packet.checksum)
	{
		send_ack.seqnum = packet.seqnum;
		send_ack.acknum = packet.seqnum;
		send_ack.checksum = (packet.seqnum*2)+(65);
		strcpy(send_ack.payload,"A"); 
		
		
		tolayer3(1, send_ack);
		
		if(recv_seq_number == packet.seqnum)
		{
			recv_seq_number = recv_seq_number^1;
			tolayer5(1,packet.payload);
		}
	}
	

}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	recv_seq_number=0;
}

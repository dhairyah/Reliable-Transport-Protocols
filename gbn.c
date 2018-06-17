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

/*
References:
https://stackoverflow.com/questions/1693853/copying-arrays-of-structs-in-c
*/

int win_size, base, next_seq, seq_expected, timer_flag;
struct pkt buffer_struct[1000],last_received_packet;

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
	struct pkt send_packet;
	int i=0,check_sum=0, msg_len=0;
	
	
	if(next_seq < base+win_size)
	{
		
		msg_len = sizeof(message.data);
		
		for(i=0;i<msg_len;i++)
		{
			check_sum=check_sum+(int)(message.data[i]);
			
		}
		
		check_sum = check_sum+next_seq;
		
		
		send_packet.seqnum = next_seq;
		send_packet.acknum = 0;
		send_packet.checksum = check_sum;
		strcpy(send_packet.payload,message.data);
		
		memcpy(&buffer_struct[next_seq],&send_packet, sizeof(buffer_struct[next_seq]));
		
		tolayer3(0, send_packet);
				
		if(base==next_seq)
		{
		starttimer (0, 15.0);
		timer_flag = 1;
		}
		next_seq++;
	}
	else
	{
		msg_len = sizeof(message.data);
		
		for(i=0;i<msg_len;i++)
		{
			check_sum=check_sum+(int)(message.data[i]);
			
		}
		
		check_sum = check_sum+next_seq;
		
		send_packet.seqnum = next_seq;
		send_packet.acknum = 0;
		send_packet.checksum = check_sum;
		strcpy(send_packet.payload,message.data);
		
		memcpy(&buffer_struct[next_seq],&send_packet, sizeof(buffer_struct[next_seq]));
				
		next_seq++;
		
	}
	

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
	if((packet.checksum == (packet.seqnum+packet.acknum)+(65)) && (strcmp(packet.payload,"A")==0))
	{
		base = packet.acknum+1;
		
		if(base == next_seq)
		{
			if(timer_flag == 1)
			{
			stoptimer(0);
			timer_flag = 0;	
			}			
		}
		else
		{
			if(timer_flag == 1)
			{
			stoptimer(0);
			starttimer (0, 15.0);
			}
		}
		
						
	}
	
	

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	
	int to_val;
	if(base+win_size<next_seq)
	{
		to_val =base+win_size;
	}
	else
	{
		to_val = next_seq;
	}
	starttimer (0, 15.0);
	timer_flag = 1;
	for(int i=base;i<to_val;i++)
	{
		tolayer3(0, buffer_struct[i]);
	}
	
	

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	win_size = getwinsize();
	base =0;
	next_seq =0;
	
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
	
	if(packet.seqnum == seq_expected)
	{
		int i=0, check_sum=0, msg_len=0;
		struct pkt send_ack;
		
		msg_len = sizeof(packet.payload);
		
		for(i=0;i<msg_len;i++)
		{
			check_sum=check_sum+(int)(packet.payload[i]);
			
		}
		
		check_sum = check_sum+packet.seqnum+packet.acknum;
						
		if(check_sum == packet.checksum && check_sum!=0)
		{
			send_ack.seqnum = packet.seqnum;
			send_ack.acknum = packet.seqnum;
			send_ack.checksum = (packet.seqnum*2)+(65);
			strcpy(send_ack.payload,"A");
			
			memcpy(&last_received_packet,&send_ack,sizeof(last_received_packet));
			
			tolayer3(1, send_ack);
			tolayer5(1,packet.payload);
			seq_expected++;
		}
		
	}
	else
	{
		tolayer3(1, last_received_packet);
	}

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	seq_expected=0;
}

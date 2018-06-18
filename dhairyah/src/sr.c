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

int win_size, rec_win_size, base, next_seq, seq_expected, win_occupied, timer_seq_number[120000], timer_ptr,timer_base, timer_flag, rec_base, s, r, buffer_len;
float timer_start_time[120000], last_timer, current_timer;
struct pkt buffer_struct[1000], recv_buffer_struct[1000];
int packet_ack[1000],packet_rec[1000] = {0};

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
	struct pkt send_packet;
	int i=0,check_sum=0, msg_len=0;
		
	if(next_seq < (base+win_size))
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
		s++;
		packet_ack[next_seq] =0;
			
		if(timer_flag == 0)
		{
			last_timer = get_sim_time();
			current_timer = last_timer;
			timer_seq_number[timer_ptr]=next_seq;
			timer_start_time[timer_ptr]=(current_timer-last_timer);
			timer_ptr++;
			starttimer (0, 42.0);
			timer_flag = 1;
		}
		else
		{
			current_timer = get_sim_time();
			timer_seq_number[timer_ptr]=next_seq;
			timer_start_time[timer_ptr]=(current_timer-last_timer);
			timer_ptr++;
			last_timer = current_timer;
			
		}
		
	//	win_occupied++;
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
		
		buffer_len++;
				
		next_seq++;
		
	}

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
	int i,j;
	
	
	
	if((packet.checksum == (packet.seqnum+packet.acknum)+(65)) && (strcmp(packet.payload,"A")==0))
	{
		
		if(packet.seqnum>=base && packet.seqnum<(base+win_size))
		{
			r++;
			if(packet.seqnum==base)
			{
				
				packet_ack[packet.seqnum]=1;
				//win_occupied--;
				if(buffer_len == 0 && timer_flag == 1)
				{
					
					stoptimer(0);
					timer_flag =0;
				}
				//base++;
				j= base+1;
				for(i=j-1;i<(j+win_size-1);i++)
				{
					if(packet_ack[i]==1)
					{
						base++;
						if(buffer_struct[i+win_size].seqnum == (i+win_size))
						{
							current_timer = get_sim_time();
							timer_seq_number[timer_ptr]=(i+win_size);
							timer_start_time[timer_ptr]=(current_timer-last_timer);
							timer_ptr++;
							last_timer = current_timer;

							tolayer3(0, buffer_struct[i+win_size]);
							s++;
							packet_ack[i+win_size] =0;
							buffer_len--;
							
						}
					}
					else
					{
						break;
					}			
				}
				
			}
			else
			{
				packet_ack[packet.seqnum]=1;
			}
				
						
		}
					
	}
	
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	
	

	if(packet_ack[timer_seq_number[timer_base]]==0)
	{

		tolayer3(0, buffer_struct[timer_seq_number[timer_base]]);
		
		
		current_timer = get_sim_time();
		
		
		timer_seq_number[timer_ptr] = timer_seq_number[timer_base];
		timer_start_time[timer_ptr] = (current_timer-last_timer);
		last_timer = current_timer;
		
		timer_ptr++;
		timer_base++;
		
		
	}
	else if((packet_ack[timer_seq_number[timer_base]]==1))
	{

		timer_base++;
	}

		
		if(s!=r)
		{

			starttimer (0, (timer_start_time[timer_base]));
		}

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	win_occupied=0;
	base=0;
	next_seq=0;
	timer_ptr=0;
	timer_base =0;
    last_timer =0.0;
	current_timer=0.0;
	timer_flag=0;
	win_size=getwinsize();	
	s=0;
	r=0;
	buffer_len=0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
	int i=0, j, check_sum=0, msg_len=0;
	struct pkt send_ack;
	
	msg_len = sizeof(packet.payload);
		
	for(i=0;i<msg_len;i++)
	{
		check_sum=check_sum+(int)(packet.payload[i]);
		
	}
		
	check_sum = check_sum+packet.seqnum+packet.acknum;
						
	if(check_sum == packet.checksum && check_sum!=0)
	{
	
		
		if(packet.seqnum < (rec_base+rec_win_size)) 
		{
			if(packet.seqnum == rec_base)
			{				
				memcpy(&recv_buffer_struct[packet.seqnum],&packet, sizeof(&recv_buffer_struct[packet.seqnum]));
								
				
				send_ack.seqnum = packet.seqnum;
				send_ack.acknum = packet.seqnum;
				send_ack.checksum = (packet.seqnum*2)+(65);
				strcpy(send_ack.payload,"A");
				
				tolayer3(1, send_ack);
				
				packet_rec[packet.seqnum]=1;
				
				tolayer5(1,packet.payload);
				
				rec_base++;
				j = rec_base;
				for(i=j;i<(j+rec_win_size-1);i++)
				{
					
					if(packet_rec[i]==1)
					{
						tolayer5(1,recv_buffer_struct[i].payload);
						rec_base++;
					}
					else
					{
						break;
					}
					
					
				}
	
			}
			else
			{
				
				if(packet_rec[packet.seqnum]!=1)
				{	
				
					memcpy(&recv_buffer_struct[packet.seqnum],&packet, sizeof(recv_buffer_struct[packet.seqnum]));		
				
				}
				
				
				send_ack.seqnum = packet.seqnum;
				send_ack.acknum = packet.seqnum;
				send_ack.checksum = (packet.seqnum*2)+(65);
				strcpy(send_ack.payload,"A");
				packet_rec[packet.seqnum]=1;	
				tolayer3(1, send_ack);
			}
		}
		else
		{
			send_ack.seqnum = packet.seqnum;
			send_ack.acknum = packet.seqnum;
			send_ack.checksum = (packet.seqnum*2)+(65);
			strcpy(send_ack.payload,"A");
						
			tolayer3(1, send_ack);
		}
	}
	

		
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	rec_base=0;
	rec_win_size=getwinsize();
}

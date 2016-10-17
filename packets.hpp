/*
 * packets.hpp
 *
 *  Created on: Feb 13, 2012
 *      Author: Zsolt Magyari
 */


//DEBUG INTEGRATION!!
#include "packets.h"
#include "openflow.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "config.hpp"
#include "debug.hpp"
#include <iostream>

#include <net/ethernet.h>
#include <boost/thread.hpp>
#include <sys/socket.h>

#include "topology.hpp"
#include "switch.hpp"

//const char packet_in_fake[] =
//		{
//			0x97,0x0a,0x00,0x52,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
//			0x01,0x00,0x40,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
//			0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x02,0x08,0x00,0x45,
//			0x00,0x00,0x32,0x00,0x00,0x00,0x00,0x40,0xff,0xf7,0x2c,
//			0xc0,0xa8,0x00,0x28,0xc0,0xa8,0x01,0x28,0x7a,0x18,0x58,
//			0x6b,0x11,0x08,0x97,0xf5,0x19,0xe2,0x65,0x7e,0x07,0xcc,
//			0x31,0xc3,0x11,0xc7,0xc4,0x0c,0x8b,0x95,0x51,0x51,0x33,
//			0x54,0x51,0xd5,0x00,0x36
//		};

const char packet_in_fake[] =            // 2012.03.22  The new cbench version
		{
			0x97,0x0a,0x00,0x52,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
			0x01,0x00,0x40,0x00,0x01,0x00,0x00,0x80,0x00,0x00,0x00,  //<
			0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x02,0x08,0x00,0x45,
			0x00,0x00,0x32,0x00,0x00,0x00,0x00,0x40,0xff,0xf7,0x2c,
			0xc0,0xa8,0x00,0x28,0xc0,0xa8,0x01,0x28,0x7a,0x18,0x58,
			0x6b,0x11,0x08,0x97,0xf5,0x19,0xe2,0x65,0x7e,0x07,0xcc,
			0x31,0xc3,0x11,0xc7,0xc4,0x0c,0x8b,0x95,0x51,0x51,0x33,
			0x54,0x51,0xd5,0x00,0x36
		};						//^

//const char make_features_reply_fake[] =     // stolen from wireshark
//		{
//			0x97,0x06,0x00,0xe0,0x04,0x01,0x00,0x00,0x00,0x00,0x76,0xa9,
//			0xd4,0x0d,0x25,0x48,0x00,0x00,0x01,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,
//			0x00,0x00,0x03,0xff,0x00,0x00,0x1a,0xc1,0x51,0xff,0xef,0x8a,0x76,0x65,0x74,0x68,
//			0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			0x00,0x00,0x00,0x00,0x00,0x01,0xce,0x2f,0xa2,0x87,0xf6,0x70,0x76,0x65,0x74,0x68,
//			0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			0x00,0x00,0x00,0x00,0x00,0x02,0xca,0x8a,0x1e,0xf3,0x77,0xef,0x76,0x65,0x74,0x68,
//			0x35,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			0x00,0x00,0x00,0x00,0x00,0x03,0xfa,0xbc,0x77,0x8d,0x7e,0x0b,0x76,0x65,0x74,0x68,
//			0x37,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			0x00,0x00,0x00,0x00
//		};

const char make_features_reply_fake[] =     // 2012.03.22  The new cbench version
{
	  0x97,0x06,0x00,0xe0,0x04,0x01,0x00,0x00,0x00,0x00,0x76,0xa9,
	  0xd4,0x0d,0x25,0x48,0x00,0x00,0x01,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	  0x00,0x00,0x07,0xff,0x00,0x01,0x1a,0xc1,0x51,0xff,0xef,0x8a,0x76,0x65,0x74,0x68,
	  0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	  0x00,0x00,0x00,0x00,0x00,0x02,0xce,0x2f,0xa2,0x87,0xf6,0x70,0x76,0x65,0x74,0x68,
	  0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	  0x00,0x00,0x00,0x00,0x00,0x03,0xca,0x8a,0x1e,0xf3,0x77,0xef,0x76,0x65,0x74,0x68,
	  0x35,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	  0x00,0x00,0x00,0x00,0x00,0x04,0xfa,0xbc,0x77,0x8d,0x7e,0x0b,0x76,0x65,0x74,0x68,
	  0x37,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	  0x00,0x00,0x00,0x00
};



static inline uint64_t htonll(uint64_t n)
{
    return htonl(1) == 1 ? n : ((uint64_t) htonl(n) << 32) | htonl(n >> 32);
}

struct timeval tvTimestamp;

/**
 * Create timestamp of us
 */
long double timestamp(long double *ldblPaTimestamp)
{
	gettimeofday(&tvTimestamp, NULL);
	long double ldblTimestampreturn;
	ldblTimestampreturn = (long double) tvTimestamp.tv_sec * 1000000;
	ldblTimestampreturn += tvTimestamp.tv_usec;

	if (ldblPaTimestamp != NULL) {
		*ldblPaTimestamp = ldblTimestampreturn;
	}

	return ldblTimestampreturn;
}


/***********************************************************************
 *  return 1 if the embedded packet in the packet_out is lldp
 *
 */

#ifndef ETHERTYPE_LLDP
#define ETHERTYPE_LLDP 0x88cc
#endif

/**
 * Recongnizing LLDP Packet
 */
static int packet_out_is_lldp(struct ofp_packet_out * po)
{
	char * ptr = (char *) po;
	ptr += sizeof(struct ofp_packet_out) + ntohs(po->actions_len);
	struct ether_header * ethernet = (struct ether_header *) ptr;
	unsigned short ethertype = ntohs(ethernet->ether_type);
	if (ethertype == ETHERTYPE_VLAN) {
		ethernet = (struct ether_header *) ((char *) ethernet) +4;
		ethertype = ntohs(ethernet->ether_type);
	}

	return ethertype == ETHERTYPE_LLDP;
}
/***********************************************************************/

/***********************************************************************
 *  return 1 if the embedded packet in the packet_out is lldp
 *
 */

/**
 * Recognizing OFDP packet
 */
static int packet_out_is_ofdp(struct ofp_packet_out * po)
{
	if (packet_out_is_lldp(po)){
		char * ptr = (char *) po;
		ptr += sizeof(struct ofp_packet_out) + ntohs(po->actions_len);
		struct ether_header * ethernet = (struct ether_header *) ptr;
		unsigned char * destination;
		destination = (unsigned char *) ethernet->ether_dhost;
		char *c = (char *) ethernet->ether_dhost;
//		int i = 6;
//		for(;i>0;i--){
//			fprintf(stdout, "0x%02X, ",(unsigned char )*c++);
//		}
//		c = (char *) ethernet->ether_dhost;
		return *(c+5) == 1;  //mac address last byte = 1 (broadcast)
	}else{
		return 0;
	}

}
/***********************************************************************/


/************************************************************************
 * The port from action
 */
static int packet_out_extract_port(struct ofp_packet_out *po)
{
	if (po->actions_len){
		uint16_t *action_type;
		action_type = (uint16_t*) po->actions;
		if (*action_type == OFPAT_OUTPUT){
			//printf("Action is output\n");
			struct ofp_action_output *action_o;
			action_o = (ofp_action_output *) po->actions;
			//printf("the Port is: %d\n", ntohs(action_o->port));
			//printf("the length is: %d\n", ntohs(action_o->len));
			if (ntohs(action_o->len) != 8){
				cerr << "action size != 8 , action size is always 8" << endl;
			}
			return ntohs(action_o->port);
		}else{
			cerr << "Can not extract the port from packet out. This function is for the first action.You must extend this function to other actions in the list!" << endl;
		}

	}
	return 0;
}

/************************************************************************/

/**
 * Create packets
 */
void packet_create(char *paBuf, int *paIntSize, ofp_type paOfpType, config* paConfig, debug *____debug)
{
	int mac_address = 100;
	//int switch_id = 1;
	switch (paOfpType) {
		case OFPT_HELLO:               /* Symmetric message */
			____debug->whisper("Creating OFPT_HELLO Packet.");

			struct ofp_header packet_hello;
			packet_hello.version = OFP_VERSION;
			packet_hello.type    = OFPT_HELLO;
			packet_hello.length  = htons(8);
			packet_hello.xid     = htonl(0);     //XID!
			memcpy(paBuf, &packet_hello, sizeof(packet_hello));
			*paIntSize = sizeof(packet_hello);

			____debug->all("Hello packet is created.");
			____debug->packet((char *)&packet_hello,sizeof(packet_hello),CREATE);
			break;

		case OFPT_ERROR:               /* Symmetric message */
			____debug->whisper("Creating OFPT_ERROR Packet.");

			struct ofp_error_msg packet_error;
			packet_error.header.type = OFPT_ERROR; //OFTP_ERROR
			packet_error.header.version = OFP_VERSION;
			packet_error.header.length = htons(sizeof(struct ofp_error_msg));
			packet_error.header.xid = htonl(1); //XID
			packet_error.type = htons(OFPET_BAD_REQUEST);
			packet_error.code = htons(OFPBRC_BAD_VENDOR);
			memcpy(paBuf, &packet_error, sizeof(packet_error));
			*paIntSize = sizeof(packet_error);

			____debug->all("OFPT_ERROR packet is created.");
			____debug->packet((char *)&packet_error,sizeof(packet_error),CREATE);
			break;

		case OFPT_ECHO_REQUEST:        /* Symmetric message */

			break;

		case OFPT_ECHO_REPLY:          /* Symmetric message */
			____debug->whisper("Creating OFPT_ECHO_REPLY Packet.");

			struct ofp_header packet_echo;
	    	packet_echo.version = OFP_VERSION;
	    	packet_echo.type    = OFPT_ECHO_REPLY;
	    	packet_echo.length  = htons(8);
	    	packet_echo.xid     = htonl(1);
	    	memcpy(paBuf, &packet_echo, sizeof(packet_echo));
	    	*paIntSize = sizeof(packet_echo);

	    	____debug->all("OFPT_ECHO_REPLY packet is created.");
	    	____debug->packet(paBuf,sizeof(packet_echo),CREATE);
			break;

		case OFPT_VENDOR:              /* Symmetric message */

			break;


		/* Switch configuration messages. */
		case OFPT_FEATURES_REQUEST:    /* Controller/switch message */

			break;

		case OFPT_FEATURES_REPLY:      /* Controller/switch message */
			____debug->whisper("Creating OFPT_FEATURES_REPLY Packet.");

			memcpy(paBuf, make_features_reply_fake, sizeof(make_features_reply_fake));  //copy fake features
			struct ofp_switch_features *features;
			features = (struct ofp_switch_features *) paBuf;
			features->header.version = OFP_VERSION;
			features->header.xid = htonl(1);             //XID
			features->datapath_id = htonll(paConfig->ID);
			//printf("dpid: %d\n", paConfig->ID);
			*paIntSize = sizeof(make_features_reply_fake);

			____debug->all("OFPT_FEATURES_REPLY packet is created.");
			____debug->packet(paBuf, sizeof(make_features_reply_fake), CREATE);
			break;


		case OFPT_GET_CONFIG_REQUEST:  /* Controller/switch message */

			break;

		case OFPT_GET_CONFIG_REPLY:    /* Controller/switch message */     //2012.03.19
			ofp_switch_config *switch_config_packet_to_send;
			switch_config_packet_to_send = (ofp_switch_config*) paBuf;
			switch_config_packet_to_send->header.length = sizeof(ofp_switch_config);
			switch_config_packet_to_send->header.type = OFPT_GET_CONFIG_REPLY;
			switch_config_packet_to_send->header.version = OFP_VERSION;
			switch_config_packet_to_send->header.xid = htonl(1);
			switch_config_packet_to_send->flags = paConfig->of_switch_config_to_send.flags;
			switch_config_packet_to_send->miss_send_len = paConfig->of_switch_config_to_send.miss_send_len;
			break;

		case OFPT_SET_CONFIG:          /* Controller/switch message */

			break;


		/* Asynchronous messages. */
		case OFPT_PACKET_IN:           /* Async message */
			struct ofp_packet_in * pi;
			struct ether_header * eth;

			//statistic

			int packet_id;
			packet_id = paConfig->SendPacketID();
			//paConfig->SendPacketID(packet_id); //the place of the timestamp    //2012.03.11 Neue Statistik

			//create packet
			____debug->whisper("Creating OFPT_PACKET_IN Packet.");
			memcpy(paBuf, packet_in_fake, sizeof(packet_in_fake));
			pi = (struct ofp_packet_in *) paBuf;
			pi->header.version = OFP_VERSION;
			pi->buffer_id = htonl(packet_id);  //2012.03.11 Neue Statistik   //2012.03.20 umstieg auf xid
			pi->header.xid = htonl(packet_id);//htonl(paConfig->ID);//nix //5

			eth = (struct ether_header * ) pi->data;
			//fake mac address
			mac_address = paConfig->PACKET_sent_count;//paConfig->ID;//(int)paConfig->PACKET_sent_count % paConfig->MacAdressPool;  //2012.03.21

			memcpy(&eth->ether_shost[1], &mac_address, sizeof(mac_address));
			eth->ether_dhost[5] = paConfig->ID;//switch_id;
			eth->ether_shost[5] = paConfig->ID;//switch_id;

			char strTMP[50];
			sprintf(strTMP,"PACKET_IN: %d",ntohl(pi->buffer_id));
			____debug->whisper(strTMP);
			//____debug->all("OFPT_PACKET_IN packet is created.");
			____debug->packet((char *)&packet_in_fake,sizeof(packet_in_fake),CREATE);
			break;

		case OFPT_FLOW_REMOVED:        /* Async message */

			break;

		case OFPT_PORT_STATUS:         /* Async message */

			break;


		/* Controller command messages. */
		case OFPT_PACKET_OUT:          /* Controller/switch message */

			break;

		case OFPT_FLOW_MOD:            /* Controller/switch message */

			break;

		case OFPT_PORT_MOD:            /* Controller/switch message */

			break;

		/* Statistics messages. */
		case OFPT_STATS_REQUEST:       /* Controller/switch message */

			break;

		case OFPT_STATS_REPLY:         /* Controller/switch message */

			break;

		/* Barrier messages. */
		case OFPT_BARRIER_REQUEST:     /* Controller/switch message */

			break;

		case OFPT_BARRIER_REPLY:       /* Controller/switch message */

			break;

		/* Queue Configuration messages. */
		case OFPT_QUEUE_GET_CONFIG_REQUEST:  /* Controller/switch message */

			break;

		case OFPT_QUEUE_GET_CONFIG_REPLY:     /* Controller/switch message */

			break;

		default:
			fprintf(stderr, "Unrecognized Packet");
			break;
	}
}

/**
 * Each pachet will be processed
 * 2012.03.19 socketSend
 */
void process_packet(char *paBuf, int paIntSize, debug *____debug, config* paConfig, boost::mutex* mutex,  int socketSend, void (*packet_sender)(char *po, int size, int switch_ID))  ////2012.02.04 (pthread_mutex_t* mutex)
{
	____debug->whisper("Packet to process:");
	____debug->packet(paBuf, paIntSize,IN);

	ofp_header *packetHeader;
	packetHeader = (ofp_header *) paBuf;

	char tempMessage[50];
	int i;
	int size;

	switch (packetHeader->type) {
			case OFPT_HELLO:               /* Symmetric message */
				____debug->gossip("received OFPT_HELLO, sending back");

				size = sizeof(ofp_hello);
				char buf_hello[size];
				packet_create(buf_hello, &size, OFPT_HELLO,paConfig,____debug);
				mutex->lock();					  //2012.03.19: boost mutex
				send(socketSend, buf_hello, size,0);
				mutex->unlock();				  //2012.03.19: boost mutex

				____debug->packet(buf_hello,size,OUT);
				break;

			case OFPT_ERROR:               /* Symmetric message */
				____debug->gossip("process OFPT_ERROR");
				break;

			case OFPT_ECHO_REQUEST:        /* Symmetric message */
				____debug->gossip("process OFPT_ECHO_REQUEST");

		    	size = sizeof(ofp_header);
		    	char buf_echo[size];
		    	packet_create(buf_echo,&size,OFPT_ECHO_REPLY,paConfig,____debug);
		    	mutex->lock();
		    	send(socketSend, buf_echo, size, 0);
		    	mutex->unlock();

		    	____debug->packet(buf_echo,size,OUT);
		    	break;


			case OFPT_ECHO_REPLY:          /* Symmetric message */
				____debug->gossip("process OFPT_ECHO_REPLY");
				break;

			case OFPT_VENDOR:              /* Symmetric message */
				____debug->gossip("received OFPT_VENDOR, sending OFPT_ERROR");

				size = sizeof(ofp_error_msg);
				char buf_error[size];
				packet_create(buf_error, &size, OFPT_ERROR, paConfig,____debug);
		    	mutex->lock();
		    	send(socketSend, buf_error, size, 0);
		    	mutex->unlock();

		    	____debug->packet(buf_error, size, OUT);
		    	break;


			/* Switch configuration messages. */
			case OFPT_FEATURES_REQUEST:    /* Controller/switch message */
				____debug->gossip("received OFPT_FEATURES_REQUEST, sending  OFPT_FEATURES_REPLY");

				size = sizeof(make_features_reply_fake);
				char buf_features[size];
				packet_create(buf_features, &size, OFPT_FEATURES_REPLY, paConfig, ____debug);
				mutex->lock();
				send(socketSend, buf_features,size, 0);
				mutex->unlock();

				____debug->packet(buf_features,size,OUT);
				break;

			case OFPT_FEATURES_REPLY:      /* Controller/switch message */
				____debug->gossip("process OFPT_FEATURES_REPLY");

				break;

			case OFPT_GET_CONFIG_REQUEST:  /* Controller/switch message */
				____debug->gossip("process OFPT_GET_CONFIG_REQUEST");

				break;

			case OFPT_GET_CONFIG_REPLY:    /* Controller/switch message */
				____debug->gossip("process OFPT_GET_CONFIG_REPLY");
				break;

			case OFPT_SET_CONFIG:          /* Controller/switch message */   //2012.03.19
				____debug->gossip("process OFPT_SET_CONFIG");
				ofp_switch_config *set_switch_config;
				set_switch_config = (ofp_switch_config*) paBuf;
				paConfig->of_switch_config_to_send.flags = set_switch_config->flags;
				paConfig->of_switch_config_to_send.miss_send_len = set_switch_config->miss_send_len;
				break;

			/* Asynchronous messages. */
			case OFPT_PACKET_IN:           /* Async message */
				____debug->gossip("process OFPT_PACKET_IN");
				break;

			case OFPT_FLOW_REMOVED:        /* Async message */
				____debug->gossip("process OFPT_FLOW_REMOVED");
				break;

			case OFPT_PORT_STATUS:         /* Async message */
				____debug->gossip("process OFPT_PORT_STATUS");
				break;


			/* Controller command messages. */
			case OFPT_PACKET_OUT:          /* Controller/switch message */
				____debug->gossip("process OFPT_PACKET_OUT");
				char strTMP[50];
				ofp_packet_out *packetOut;
				packetOut = (ofp_packet_out *) paBuf;
				if (packet_out_is_lldp(packetOut))
				{
					printf("P A C K E T   O U T   I S   L L D P !\n");
					if (packet_out_is_ofdp(packetOut))
					{
						printf("P A C K E T   O U T   I S   O F D P !\n");

						int send_port = packet_out_extract_port(packetOut);
						printf("sendport : %d\n", send_port);
						int send_switch = paConfig->ID;
						int receive_switch;
						int receive_port;
						topo.GetConnection(send_switch, send_port,receive_switch, receive_port);
						cout << "Config: " << " ss:" << send_switch << " sp:" << send_port << " rs:"  << receive_switch << " rp:" << receive_port <<  endl;

						if (receive_switch && receive_port){
							//-------------------
							//Prepare packet_in
							//-------------------
							____debug->whisper("Creating OFPT_PACKET_IN (OFDP) Packet.");


							char buf[255];
							struct ofp_packet_in * pi = (ofp_packet_in*) buf;

							char * ptr = (char*) packetOut;
							ptr += sizeof(struct ofp_packet_out) + ntohs(packetOut->actions_len);

							int ofdp_len = paIntSize- (ptr-paBuf);
							memset(buf,0,sizeof(ofp_packet_in)+ ofdp_len);
							memcpy( pi->data,ptr,ofdp_len); //ofdp

							pi->header.version = OFP_VERSION;
							pi->header.type    = OFPT_PACKET_IN;
							pi->header.length  = htons(sizeof(ofp_packet_in)+ ofdp_len - 2);
							pi->header.xid     = htonl(0);
							pi->buffer_id      = htonl(0);
							pi->total_len      = htons(ofdp_len);
							pi->in_port        = htons(receive_port);
							pi->reason         = OFPR_NO_MATCH;


							char strTMP[50];
							sprintf(strTMP,"PACKET_IN (OFDP): %d",ntohl(pi->buffer_id));
							____debug->whisper(strTMP);
							//____debug->all("OFPT_PACKET_IN packet is created.");
							____debug->packet((char *)&packet_in_fake,sizeof(packet_in_fake),CREATE);

							packet_sender(buf, sizeof(ofp_packet_in)+ ofdp_len - 2, 1);
						}

					}
				}else{

					//i = ntohl(packetOut->header.xid);
					i = ntohl(packetOut->buffer_id);     //2012.03.20 umstieg auf xid
					if (i > TIMESTAMP_COUNT+1){
						____debug->error_message("out of timestamp range, broken packet");
					}else if (i>0) {  //2012.04.09 requested flow_mod > 0
						paConfig->ReceivePacketID(i); //2012.03.12 Neue statistik
					}else { //<1
						____debug->message("non requested packet_out");
					}
					sprintf(strTMP,"PACKET_OUT: %d",i);
					____debug->whisper(strTMP);
				}

				//cout << "xid: " << ntohl(packetOut->header.xid) << " bufferID: "<<ntohl(packetOut->buffer_id) << " SwirtchID: "<<  paConfig->ID << endl;

				//paConfig->intStatPacketArrived++;   //2012.03.20 kill old statistic
//				paConfig->intStatSum += timestamp(NULL) - paConfig->sent_packets_timestamp[i];      //2012.03.19 ALte statistik
//				paConfig->intStatSumSquare += (timestamp(NULL) - paConfig->sent_packets_timestamp[i])*(timestamp(NULL) - paConfig->sent_packets_timestamp[i]);
//				if (timestamp(NULL) - paConfig->sent_packets_timestamp[i] < paConfig->intStatMin) {
//					paConfig->intStatMin = timestamp(NULL) - paConfig->sent_packets_timestamp[i];
//				}
//				if (timestamp(NULL) - paConfig->sent_packets_timestamp[i] > paConfig->intStatMax) {
//					paConfig->intStatMax = timestamp(NULL) - paConfig->sent_packets_timestamp[i];
//				}

//				sprintf(tempMessage, "Packet out buffer_id: %d\n",i);   //2012.03.19 ALte statistik
//				____debug->all(tempMessage);
//				sprintf(tempMessage, "Elapsed: %Lf\n",timestamp(NULL) - paConfig->sent_packets_timestamp[i]);
//				____debug->all(tempMessage);
//				paConfig->sent_packets_timestamp[i] = 0;  // packet timestamp clear for round robin
				break;

			case OFPT_FLOW_MOD:            /* Controller/switch message */
				____debug->gossip("process OFPT_FLOW_MOD");
				//2012.04.08 flow_mod gets into statistic
				ofp_flow_mod *flowMod;
				flowMod = (ofp_flow_mod *) paBuf;
				i = ntohl(flowMod->buffer_id);

				if (i > TIMESTAMP_COUNT+1){
					____debug->error_message("out of timestamp range, broken packet");
				}else if (i>0) {  //2012.04.09 requested flow_mod > 0
					paConfig->ReceivePacketID(i);
				}else {   //<1
					____debug->message("non requested flow_mod");
				}
				sprintf(strTMP,"FLOW_MOD: %d",i);
				____debug->whisper(strTMP);

				break;

			case OFPT_PORT_MOD:            /* Controller/switch message */
				____debug->gossip("process OFPT_PORT_MOD");
				break;

			/* Statistics messages. */
			case OFPT_STATS_REQUEST:       /* Controller/switch message */
				____debug->gossip("process OFPT_STATS_REQUEST");
				break;

			case OFPT_STATS_REPLY:         /* Controller/switch message */
				____debug->gossip("process OFPT_STATS_REPLY");
				break;

			/* Barrier messages. */
			case OFPT_BARRIER_REQUEST:     /* Controller/switch message */
				____debug->gossip("process OFPT_BARRIER_REQUEST");
				break;

			case OFPT_BARRIER_REPLY:       /* Controller/switch message */
				____debug->gossip("process OFPT_BARRIER_REPLY");
				break;

			/* Queue Configuration messages. */
			case OFPT_QUEUE_GET_CONFIG_REQUEST:  /* Controller/switch message */
				____debug->gossip("process OFPT_QUEUE_GET_CONFIG_REQUEST");
				break;

			case OFPT_QUEUE_GET_CONFIG_REPLY:     /* Controller/switch message */
				____debug->gossip("process OFPT_QUEUE_GET_CONFIG_REPLY");
				break;

			default:
				fprintf(stderr, "Unrecognized Packet");
				break;

	}

}

/**
 * There are more Packets in the buffer -> send to process
 * 2012.02.04: boost mutex
 * 2012.03.19: socketSend
 * 2012.05.19: packet fraction error
 */
void process_buffer_in(char *&paBuf, int paIntSize, debug *____debug, config* paConfig,  boost::mutex* mutex, int socketSend, void (*packet_sender)(char *po, int size, int switch_ID)) //&(VSwitch->config_switch)  //2012.02.04 (pthread_mutex_t* mutex)
{
	struct ofp_header *ofpHeader;
	int ofpSize = 1;
	char strTmp[100];
	sprintf(strTmp,"Buffer readed size: %d\n",paIntSize);
	____debug->whisper(strTmp);
	____debug->whisper("Buffer to process:");
	____debug->packet(paBuf, paIntSize,IN);
	while ((paIntSize > 0) && (ofpSize > 0) && (paIntSize >= ofpSize))
	{
		ofpHeader = (ofp_header*) paBuf;
		ofpSize = ntohs(ofpHeader->length);

//		printf("--------------------\n");
//		printf("paIntSize: %d\n", paIntSize);
//		printf("ofpSize: %d\n", ofpSize);
//		printf("--------------------\n");

		if (ofpSize > paIntSize){ //2012.05.19
			____debug->message("Ofp packet fragment sliced by buffer, or desync");
			____debug->message("--------------------\n");
			sprintf(strTmp,"paIntSize: %d\n ofpSize: %d\n", paIntSize, ofpSize);
			____debug->message(strTmp);
			____debug->message("--------------------\n");
			break;

		}
		if (ofpSize > 100) {
			sprintf(strTmp,"ofpSize too big: %d\n",ofpSize);
			____debug->message(strTmp);


		}

		if (ofpSize == 0){
			____debug->message("Packets out of Sync!(ofpSize == 0) ");


		}
		//____debug->packet(paBuf,ofpSize,INFO);
		if (paIntSize >= ofpSize)
		{
			process_packet(paBuf, ofpSize, ____debug, paConfig, mutex, socketSend, packet_sender);
			paBuf += ofpSize;
			paIntSize -= ofpSize;
		} else {
			____debug->error_message("Wrong place, shouldn't enter here!!!");
		}

	}

}

void packet_pcap(char *paBuf, int *paIntSize, config* paConfig, debug *____debug)
{

}


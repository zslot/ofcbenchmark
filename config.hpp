/*
 * setup.c
 *
 *  Created on: Oct 26, 2011
 *      Author: Zsolt Magyari
 */

#include "minIni.h"
#include <time.h>
#include <math.h>
#include <vector>
#include "openflow.h"
#include <strings.h>
//#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include<fstream>


const char inifile[] = "config.ini";
const int  usPs = 1000000;  // microseconds per second

#define TIMESTAMP_COUNT 200000   /*how many timestamps are saved, (round-robin)*/
#define N_TRY_TO_CONNECT 10      // how many times tries to reconnect the controller
#define SLEEP_BTW_TRIES 2        // sleep between two tries
#define SESSION_END_TIMEOUT 5000000 // 5 sec


struct delta_stat {
	long double rtt_mean;
	long double rtt_ca;
	long int packet_sent;
	long int packet_received;
};


/*        F L O W        */
struct sw_flow_key {
	uint32_t nw_src;	/* IP source address. */
	uint32_t nw_dst;	/* IP destination address. */
	uint16_t in_port;	/* Input switch port */
	uint16_t dl_vlan;	/* Input VLAN id. */
	uint16_t dl_type;	/* Ethernet frame type. */
	uint16_t tp_src;	/* TCP/UDP source port. */
	uint16_t tp_dst;	/* TCP/UDP destination port. */
	uint8_t dl_src[ETH_ALEN]; /* Ethernet source address. */
	uint8_t dl_dst[ETH_ALEN]; /* Ethernet destination address. */
	uint8_t dl_vlan_pcp;	/* Input VLAN priority. */
	uint8_t nw_tos;		/* IPv4 DSCP */
	uint8_t nw_proto;	/* IP protocol. */
	uint8_t pad[3];
	uint32_t wildcards;	/* Wildcard fields (host byte order). */
	uint32_t nw_src_mask;	/* 1-bit in each significant nw_src bit. */
	uint32_t nw_dst_mask;	/* 1-bit in each significant nw_dst bit. */
};

struct sw_flow_actions {
	size_t actions_len;
	struct ofp_action_header actions[0];
};

struct sw_flow {
	struct sw_flow_key key;

	uint16_t priority;      /* Only used on entries with wildcards. */
	uint16_t idle_timeout;	/* Idle time before discarding (seconds). */
	uint16_t hard_timeout;  /* Hard expiration time (seconds) */
	uint64_t used;          /* Last used time (in jiffies). */
	uint8_t send_flow_rem;  /* Send a flow removed to the controller */
	uint8_t emerg_flow;     /* Emergency flow indicator */

	struct sw_flow_actions *sf_acts;

	uint64_t created;        /* When the flow was created (in jiffies_64). */
	uint64_t packet_count;   /* Number of packets associated with this entry */
	uint64_t byte_count;     /* Number of bytes associated with this entry */
};
/* ----F-L-O-W------- */

class config{
private:

public:
	int ID;  //Identification
	int SessionID;

	//char packets_from_pcap_file;

	//c o n f i g
	char MISSION_CONTROL_IP[20];
	int  MISSION_CONTROL_PORT;
	char CONTROLLER_IP[20];
	int  CONTROLLER_PORT;
	int  MacAdressPool;   //not used yet

	char PCAP_FILE_OR_DIR[200];
	bool BATCH_FROM_FILE;
	int  PCAP_ROUND_ROBIN;
	char DELAY_CONFIG_FILE[200];

	//b a t c h
	int intUSleepBatchprocess;   //sleep time between packets

	long double sent_packets_timestamp2[TIMESTAMP_COUNT];   // the round robin timestamps

	void init();
	void reset_statistic();


public:
	long double timestamp(long double *ldblPaTimestamp);
	bool file_exists(const char* filename);

	//overall
	long double tsSTART;               // time stamp start
	long double tsSTOP;
	long double tsLAST_PACKET_SENT;
	long double tsLAST_PACKET_RECEIVED;
	long double PACKET_sent_count;
	long double PACKET_received_count;  //RTT count too

	long double RTT_sum;               //packet received
	long double RTT_sum_square;
	long double RTT_min;
	long double RTT_max;
	long double RTT_mean();                     //overall
	long double RTT_VAR();
	long double RTT_Ca();

	long double PPS_sent();                   //packet per seconds overall
	long double PPS_received();

	//Deltas
	long double Delta_t;             //Dt in microseconds
	long double tsCur_delta_START;   // delta start time stamp
	long double tsCur_delta_STOP;

	long double RTT_in_delta_count;  //how many delta RTT-s are measured
	long double RTT_in_delta_sum;        //the sum of Rtt deltas     mean = RTT_in_delta_sum

	//long double RTT_cur_delta_sum;
	//long double RTT_cur_delta_sum_square;
	//long double RTT_cur_delta_mean();
	//long double RTT_cur_delta_VAR();
	//long double RTT_cur_delta_Ca();

	vector <delta_stat> vec_deltas;     //type of rtt delta : mean + Ca   --> vector   //consolidation of RTT-s
	vector <sw_flow> vec_sw_flow;


	long double PPS_cur_delta_PACKET_received; //count
	long double PPS_cur_delta_received();      //per second (Dt independent)
	long double PPS_cur_delta_PACKET_sent;     //count of sent packets
	long double PPS_cur_delta_sent();          //sent packet per seconds in current delta

	long double PPS_delta_counter;             //how many deltas

	long double PPS_delta_sum_received;        //sum of deltas pps  (5000pps + 6000pps)  Delta_T doesn't matter
	long double PPS_delta_sum_square_received; //sum of squares
	long double PPS_delta_min_received;        //the minimum of the delte pps-es
	long double PPS_delta_max_received;
	long double PPS_delta_mean_received();     //mean of pps-es measured in deltas
	long double PPS_delta_VAR_received();      //variance of pps-es measured in deltas
	long double PPS_delta_Ca_received();       //coefficient of variance of deltas

	long double PPS_delta_sum_sent;            //sum of deltas sent pps
	long double PPS_delta_sum_square_sent;
	long double PPS_delta_min_sent;
	long double PPS_delta_max_sent;
	long double PPS_delta_mean_sent();
	long double PPS_delta_VAR_sent();         //variance of the deltas of sent packets
	long double PPS_delta_Ca_sent();

	//Counter functions
	int SendPacketID();                       //stamping the packets
	void ReceivePacketID(int packetID);       //tracking the packets

	void ChangeDelta();                          //delta statistics will be updated and a new delta will be created
	void SaveDelta();

	//Switch info
	bool SwitchOutOfOrder();         // Switch is not able for measurements
	bool SessionTimeouted();         // Ready for the next session


	//------------  OF protocoll config  --------------------------------------------

	struct ofp_switch_config of_switch_config_to_send;  //to set and send switch configuration

	//------------   end OF config     ----------------------------------------------
};

//int config::load_from_file(char *file)
//{
//	return 0;
//}

void config::reset_statistic(){

	//new statistic
	tsSTART = timestamp(NULL);
	tsLAST_PACKET_SENT     = 0;
	tsLAST_PACKET_RECEIVED = 0;
	tsCur_delta_START = timestamp(NULL); //start of the first delta
	tsCur_delta_STOP  = tsCur_delta_START + Delta_t;

	PACKET_sent_count = 0;
	PACKET_received_count = 0;

	RTT_sum = 0;               //packet received
	RTT_sum_square = 0;
	RTT_min = 9999999;
	RTT_max = 0;

	PPS_delta_counter = 1;             //how many deltas
	PPS_cur_delta_PACKET_received = 0;
	PPS_cur_delta_PACKET_sent = 0;

	PPS_delta_sum_received = 0;        //sum of delta pps  (5000pps + 6000pps)  Delta_T doesn' matter
	PPS_delta_sum_square_received = 0; //sum of squares
	PPS_delta_min_received = 9999999;        //the minimum of the delte pps-es
	PPS_delta_max_received = 0;

	PPS_delta_sum_sent = 0;
	PPS_delta_sum_square_sent = 0;
	PPS_delta_min_sent = 9999999;
	PPS_delta_max_sent = 0;


	for (int i=0;i < TIMESTAMP_COUNT; i++){     //Maestro sends old packets back  - Restart Maestro
		sent_packets_timestamp2[i] = 0;
	}

	RTT_in_delta_sum    = 0;
	RTT_in_delta_count  = 0;

	vec_deltas.clear();
	//--new statistic

}

//---------------------------------------------------new statistics-----------------------------------------------------------------
/**
 * Mean of Roud Trip Time
 */
long double config::RTT_mean()
{
	return RTT_sum / PACKET_received_count;
}

/**
 * Variation of Round Trip Time
 */
long double config::RTT_VAR()
{
	return (RTT_sum_square / PACKET_received_count) - ((RTT_sum / PACKET_received_count)*(RTT_sum / PACKET_received_count));
}

/**
 * Coefficient of Variation of RTT
 */
long double config::RTT_Ca()
{
	return sqrt(RTT_VAR()) / RTT_mean();
}

/**
 * Sent Packets Per Second
 */
long double config::PPS_sent()
{
																			//       		     1 sec [usec]
	return PACKET_sent_count * usPs / (timestamp(NULL) - tsSTART);          //   sentcount   *  -------------
	//return (PACKET_sent_count / (timestamp(NULL) - tsSTART)) * usPs;      //   	    	     Dt    [usec]

}

/**
 * Received Packets per Second
 */
long double config::PPS_received()
{
	return PACKET_received_count * usPs / (timestamp(NULL) - tsSTART);
	//return (PACKET_received_count / (timestamp(NULL) - tsSTART)) * usPs;
}

/**
 * Sent Packets per Seconds in Current Delta
 */
long double config::PPS_cur_delta_sent()
{
	return PPS_cur_delta_PACKET_sent * usPs / (tsCur_delta_STOP - tsCur_delta_START);
}

/**
 * Received Packets per Seconds in Current Delta
 */
long double config::PPS_cur_delta_received()
{
	return PPS_cur_delta_PACKET_received * usPs / (tsCur_delta_STOP - tsCur_delta_START);
}

/**
 * 2012.03.28 on no deltas returns 0
 */
long double config::PPS_delta_mean_sent()
{
	return PPS_delta_counter == 0 ? 0 : PPS_delta_sum_sent / PPS_delta_counter;
}

/**
 * 2012.03.28 on no deltas returns 0
 */
long double config::PPS_delta_mean_received()
{
	return PPS_delta_counter == 0 ? 0 : PPS_delta_sum_received / PPS_delta_counter;
}

long double config::PPS_delta_VAR_sent()
{
	return PPS_delta_sum_square_sent / PPS_delta_counter - PPS_delta_mean_sent() * PPS_delta_mean_sent();
}

long double config::PPS_delta_VAR_received()
{
	return PPS_delta_sum_square_received / PPS_delta_counter - PPS_delta_mean_received() * PPS_delta_mean_received();
}

/**
 * on delta_mean = 0 returns 0
 */
long double config::PPS_delta_Ca_sent()
{
	return PPS_delta_mean_sent() == 0 ? 0 : sqrt(PPS_delta_VAR_sent()) / PPS_delta_mean_sent();
}

/**
 * on delta_mean = 0 returns 0
 */
long double config::PPS_delta_Ca_received()
{
	return PPS_delta_mean_received() == 0 ? 0 : sqrt(PPS_delta_VAR_received()) /PPS_delta_mean_received();
}

/**
 * Stamping the Packets in a Vector for Tracking
 * 2012.04.09 the buffer id between 1 and N+1 because of non requested flow_mod (0)
 */
int config::SendPacketID()
{
	tsLAST_PACKET_SENT = timestamp(NULL);
	PACKET_sent_count++;
	int packet_id;
	packet_id = (int)(PACKET_sent_count) % TIMESTAMP_COUNT;

	if (sent_packets_timestamp2[packet_id] != 0){
		cerr << "Error! Buffer under run in round robin!! ID: " << packet_id << " Switch: " << ID << endl;
	}

	sent_packets_timestamp2[packet_id] = timestamp(NULL);

	//delta
	ChangeDelta();
	PPS_cur_delta_PACKET_sent++;

	return packet_id + 1 ; //2012.04.09

}

/**
 * Tracking the Packets: Showing their ID in Vector and Updating Statistics
 * 2012.04.09 the buffer id between 1 and N+1 for non requested flow_mod (0)
 */
void config::ReceivePacketID(int packetID)
{
	packetID -= 1; ////2012.04.09
	tsLAST_PACKET_RECEIVED = timestamp(NULL);
	PACKET_received_count++;
	if (sent_packets_timestamp2[packetID] == 0){
		cerr << "Error! Received packet to the wrong time stamp (0)!!! ID:" << packetID << " Switch: " << ID << endl;
	}else{
		long double rtt = timestamp(NULL) - sent_packets_timestamp2[packetID];
		sent_packets_timestamp2[packetID] = 0;
		RTT_sum += rtt;
		RTT_sum_square += rtt * rtt;
		RTT_min = rtt < RTT_min ? rtt : RTT_min;
		RTT_max = rtt > RTT_max ? rtt : RTT_max;

		//delta
		ChangeDelta();
		PPS_cur_delta_PACKET_received++;
		//RTT in Delta
		RTT_in_delta_sum    += rtt;//timestamp(NULL) - sent_packets_timestamp2[packetID];
		RTT_in_delta_count  += 1;

	}
}

/**
 * Creating a New Delta if Enough Time Elapsed, the Current Delta will be Saved
 */
void config::ChangeDelta()
{
	while (tsCur_delta_STOP < timestamp(NULL)) {  //saves the deltas until reaches the current delta
		SaveDelta();
		tsCur_delta_START = tsCur_delta_STOP;
		tsCur_delta_STOP = tsCur_delta_START + Delta_t;
	}
}

/**
 * Saving the current Delta with all the Gathered Statistic
 */
void config::SaveDelta()
{
	delta_stat rd;

	PPS_delta_counter++;
	//received
	PPS_delta_sum_received += PPS_cur_delta_received();
	cout << "PPS_cur_delta_received():" << PPS_cur_delta_received() << endl;
	rd.packet_received = PPS_cur_delta_received();

	PPS_delta_sum_square_received += PPS_cur_delta_received() * PPS_cur_delta_received();
	PPS_delta_min_received = PPS_cur_delta_received() < PPS_delta_min_received  ? PPS_cur_delta_received() : PPS_delta_min_received;
	PPS_delta_max_received = PPS_cur_delta_received() > PPS_delta_max_received  ? PPS_cur_delta_received() : PPS_delta_max_received;
	PPS_cur_delta_PACKET_received = 0;
	//sent
	PPS_delta_sum_sent += PPS_cur_delta_sent();
	cout << "PPS_cur_delta_sent():" << PPS_cur_delta_sent() << endl;
	rd.packet_sent= PPS_cur_delta_sent();

	PPS_delta_sum_square_sent += PPS_cur_delta_sent() * PPS_cur_delta_sent();
	PPS_delta_min_sent = PPS_cur_delta_sent() < PPS_delta_min_sent ? PPS_cur_delta_sent() : PPS_delta_min_sent;
	PPS_delta_max_sent = PPS_cur_delta_sent() > PPS_delta_max_sent ? PPS_cur_delta_sent() : PPS_delta_max_sent;
	PPS_cur_delta_PACKET_sent = 0;
	//RTT Deltas

	rd.rtt_mean = RTT_in_delta_sum / RTT_in_delta_count;

	vec_deltas.push_back(rd);
	//cout << "(sum:" << RTT_in_delta_sum << " count:"<< RTT_in_delta_count <<")"<< endl;
	RTT_in_delta_sum    = 0;
	RTT_in_delta_count  = 0;
}




//--END-----------------------------------------------new statistics------------------------------------------------------------------

bool config::SwitchOutOfOrder()         // Switch is not able for measurements
{
	return tsSTART > tsLAST_PACKET_RECEIVED;
}

/**
 * Checks if there are Packet Transmissions in the last SESSION_END_TIMEOUT Interval
 */
bool config::SessionTimeouted()         // Ready for the next session
{
	long double tsOld = timestamp(NULL) - SESSION_END_TIMEOUT;
	return (tsLAST_PACKET_RECEIVED < tsOld) && (tsLAST_PACKET_SENT < tsOld);
}

/**
 * Gets a Timestamp in Microseconds
 */
long double config::timestamp(long double *ldblPaTimestamp)
{
	struct timeval tvTimestamp;
	gettimeofday(&tvTimestamp, NULL);
	long double ldblTimestampreturn;
	ldblTimestampreturn = (long double) tvTimestamp.tv_sec * 1000000;
	ldblTimestampreturn += tvTimestamp.tv_usec;

	if (ldblPaTimestamp != NULL) {
		*ldblPaTimestamp = ldblTimestampreturn;
	}

	return ldblTimestampreturn;
}

/**
 * Set up Configuration
 */
void config::init()
{

	//Configuration Init
	//char str[100];
	long n;
	//int s, k;
	//char section[50];

	/* read from config.ini */
	n = ini_gets("MissionControl", "ip", "127.0.0.1", MISSION_CONTROL_IP, sizeof(MISSION_CONTROL_IP), inifile);
	n = ini_gets("Controller",     "ip", "127.0.0.1", CONTROLLER_IP, sizeof(CONTROLLER_IP), inifile);
	MISSION_CONTROL_PORT = ini_getl("MissionControl", "port", 10, inifile);
	CONTROLLER_PORT = ini_getl("Controller", "port", 10, inifile);
	n = ini_gets("Switch","pcap_file_dir","",PCAP_FILE_OR_DIR, sizeof(PCAP_FILE_OR_DIR),inifile);
	n = ini_gets("Switch", "delay_config_file", "", DELAY_CONFIG_FILE, sizeof(DELAY_CONFIG_FILE), inifile);
	PCAP_ROUND_ROBIN = ini_getl("Switch", "pcap_round_robin", 0, inifile);

	printf("Mission control: %s \n", MISSION_CONTROL_IP);
	printf("Controller: %s \n", CONTROLLER_IP);
	printf("port: %d \n", MISSION_CONTROL_PORT);
	printf("port: %d \n", CONTROLLER_PORT);
	printf("pcap file dir: %s \n", PCAP_FILE_OR_DIR);
	printf("delay config file: %s \n", DELAY_CONFIG_FILE);

	//Packet Delay
	intUSleepBatchprocess = 0;
	if (file_exists(DELAY_CONFIG_FILE)){
		//printf("Fiel %s exist!\n",DELAY_CONFIG_FILE);
		char strSwitchID[20];
		sprintf(strSwitchID,"%d",ID);
		intUSleepBatchprocess = ini_getl("SwitchPacketDelay", strSwitchID, 0, DELAY_CONFIG_FILE);
		printf("packetSleep: %d\n",intUSleepBatchprocess);
	}else{
		printf("File %s doesn't exist\n",DELAY_CONFIG_FILE);
	}

	//Pcap_file
	char pcapFile[200];
	sprintf(pcapFile, "%s/%d", PCAP_FILE_OR_DIR, ID);
	strcpy(PCAP_FILE_OR_DIR, pcapFile);
	BATCH_FROM_FILE = file_exists(PCAP_FILE_OR_DIR);
	if (BATCH_FROM_FILE){
		printf("Fiel %s exist!\n",PCAP_FILE_OR_DIR);
	}else{
		printf("File %s doesn't exist\n",PCAP_FILE_OR_DIR);
	}
	printf("pcap roubd riobin: %d\n", PCAP_ROUND_ROBIN);


	reset_statistic();

	MacAdressPool = 1;

	int i;
	for (i=0;i < TIMESTAMP_COUNT; i++){
		//sent_packets_timestamp[i] = 0;  //2012.03.19 alte statistik
		sent_packets_timestamp2[i] = 0;
	}

}

/**
 * Filecheck
 */
bool config::file_exists(const char* filename){
	bool result;
	std::ifstream fin(filename);
	result = fin;
	fin.close();
	return result;
}













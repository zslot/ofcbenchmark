/*
 * switch.h
 *
 *  Created on: Oct 27, 2011
 *      Author: Zsolt Magyari
 */
#ifndef switch_h
#define switch_h


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>   //2012.05.17
#include <stdio.h>
#include <stdlib.h>
//#include <pthread.h>
#include <sys/socket.h>
#include <net/ethernet.h>
//#include <exception>
#include <iostream>
#include <cstdlib>
//#include "openflow.h"
#include "packets.hpp"
//#include "config.h"
#include "unistd.h"

#include "communication.hpp"

#include <boost/thread.hpp>
#include "mypcap.hpp"


#define MAXBUFSIZE 1024*1024   //1 MB
#define MAXREADBUFSIZE 1024*100
#define MAXWRITEBUFSIZE 1024*5

using namespace std;


class vswitch {

private:

//	pthread_t thrStart;
//	pthread_t thrControllerCommunication;
//	pthread_t thrControllerBatch;
//	pthread_t thrMissionControl;

	boost::thread thrStart;
	boost::thread thrControllerCommunication;
	boost::thread thrControllerBatch;
	boost::thread thrMissionControl;

	debug debug_switch;


	//vswitch(const vswitch&);              //not implemented (for thread *this)  //2012.02.04 no static cast
	//vswitch& operator=(const vswitch&);   //not implemented (for thread *this)  //2012.02.04 no static cast

	static void* start(void*);
	static void* controller_communication(void*);   //thread proc
	static void* controller_batch(void*);	        //thread proc
	static void* mission_control(void*);			//thread proc

	//mission control
	bool login_to_mission_control_();
	int get_command_();
	void send_command           (message_header *mhOutgoing, char *Payload, int intPayloadSize);
	void command_picker_       	(message_header *mhIncomming);
	void get_					(message_header *CommandHeader);
	void set_					(message_header *CommandHeader);
	void get_file_				(message_header *CommandHeader);
	void put_file_				(message_header *CommandHeader);

	void quit_                  (message_header *CommandHeader);
	void restart_               (message_header *CommandHeader);
	void command_not_recognized_(message_header *CommandHeader);
	void hello_					(message_header *CommandHeader);

	void (*send_to_switch)(char *po, int size, int switch_ID);
public:



	int sockControllerCommunication;
	int sockControllerBatch;
	int sockMissionControl;

	void stop_					();
	void start_					();

	config config_switch;

	vswitch(int ID, void (*packet_sender)(char *po, int size, int switch_ID));
	virtual ~vswitch();

	int get_id();
	void set_id(int id);

	int setup_controller_com_connection();
	int setup_controller_batch_connection();
	int setup_mission_control_connection();

	int close_controller_com_connection();
	int close_controller_batch_connection();
	int close_mission_control_connection();

	int start_start_thread();
	int start_controller_com_thread();
	int start_controller_batch_thread();
	int start_mission_control_thread();

	int join_start_thread();
	int join_controller_com_thread();
	int join_controller_batch_thread();
	int join_mission_control_thread();

	boost::mutex mutxSemaphoreControllerSocket;
	boost::mutex mutxSemaphoreLoadConfigFromMissionControl;
	boost::mutex mutxBatchRun;

	int send_packet_to_controller(char *po, int size);
//	pthread_mutex_t mutxSemaphoreControllerSocket;
//	pthread_mutex_t mutxSemaphoreLoadConfigFromMissionControl;
//	pthread_mutex_t mutxBatchRun;

//	const pthread_t& get_start_TID() const {return thrStart;}
//	const pthread_t& get_controller_communication_TID() const {return thrControllerCommunication;}
//	const pthread_t& get_controller_batch_TID() const {return thrControllerBatch;}
//	const pthread_t& get_mission_control_TID() const {return thrMissionControl; }

	debug debug_start;
	debug debug_communication;
	debug debug_batch;
	debug debug_mission;

	bool RunBatch;
	bool RunControllerCommunication;
	bool RunMissionControl;

};

/**
 * Virtual switch constructor with callback send function for OFDP and LLDP redirection
 */
vswitch::vswitch(int id, void (*packet_sender)(char *po, int size, int switch_ID))
{
	set_id(id);
	//all_switches = vs_array;  //to know the others
	if (start_start_thread() != 0){
		debug_switch.crytical_error_message("-could not start the 'START' thread-");
		exit(EXIT_FAILURE);
	}else{
		debug_switch.message("-The 'start' thread started-");
	}

	send_to_switch = packet_sender;  //callback function to send packets from other switches to controller
	//Test callback
	//char a = 'H';
	//send_to_switch(&a, 1, 1);
}

/**
 * 2012.02.04: Boost mutex
 */
vswitch::~vswitch()
{
	int close_controller_batch_connection();
	int close_controller_com_connection();
	int close_mission_control_connection();
//	pthread_mutex_destroy(&mutxSemaphoreControllerSocket);				//2012.02.04: Boost Mutex
//	pthread_mutex_destroy(&mutxSemaphoreLoadConfigFromMissionControl);
//	pthread_mutex_destroy(&mutxBatchRun);
}

void vswitch::set_id(int id)
{
	config_switch.ID = id;
}

int vswitch::get_id()
{
	return config_switch.ID;
}

/**
 * connection setup the communication to the controller
 */
int vswitch::setup_controller_com_connection()
{
	//create socket
	if ((sockControllerCommunication = socket(AF_INET, SOCK_STREAM, 0)) > 0){
		debug_switch.gossip("controller socket created");
	}else{
		debug_switch.error_message("controller socket could not be created");
		return -1;
	}

	// disable Nagle's algorithm //2012.05.17
	int zero = 0;
	if (setsockopt(sockControllerCommunication, IPPROTO_TCP, TCP_NODELAY, &zero, sizeof(zero) ) < 0)
	{
		debug_switch.error_message("couldn't disable Nagle's algorithm");
		return -1;
	}
	else
	{
		debug_switch.gossip("Nagle's algorithm disabled");
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(config_switch.CONTROLLER_PORT);
	inet_aton(config_switch.CONTROLLER_IP, &address.sin_addr);

	//connect controller
	if (connect(sockControllerCommunication, (struct sockaddr *) &address, sizeof(address))==0){
		//ok, gossip
		char strGossip[50];
		sprintf(strGossip, "Controller: %s:%d connected", config_switch.CONTROLLER_IP, config_switch.CONTROLLER_PORT);
		debug_switch.gossip(strGossip);
	}else{
		//error, msg
		char strError[50];
		sprintf(strError, "Error connect controller: %s %d", config_switch.CONTROLLER_IP, config_switch.CONTROLLER_PORT);
		debug_switch.error_message(strError);
		return -1;
	}

	return 0;
}

/**
 * packet generator connection (same as the communication socket)
 */
int vswitch::setup_controller_batch_connection()
{
	//replicate controller socket for batch thread
	if ((sockControllerBatch = dup(sockControllerCommunication)) > -1){
		debug_switch.gossip("Socket for batch duplicated successfully");
		return 0;
	}else{
		debug_switch.error_message("Socket for batch couldn't be duplicated");
		return -1;
	}

}

/**
 * Connection with the control center
 */
int vswitch::setup_mission_control_connection()
{
	char strTmp[100];
	//create socket
	if ((sockMissionControl = socket(AF_INET, SOCK_STREAM, 0)) > 0){
		debug_switch.gossip("mission control socket created");
	}else{
		debug_switch.error_message("mission control socket couldn't be created");
		return -1;
	}

	sprintf(strTmp, "MissionControl Port: %d", config_switch.MISSION_CONTROL_PORT);
	debug_switch.gossip(strTmp);

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(config_switch.MISSION_CONTROL_PORT);
	inet_aton(config_switch.MISSION_CONTROL_IP, &address.sin_addr);

	//connect controller
	if (connect(sockMissionControl, (struct sockaddr *) &address, sizeof(address))==0){
		char strGossip[50];
		sprintf(strGossip, "MissionControl: %s:%d connected", config_switch.MISSION_CONTROL_IP, config_switch.MISSION_CONTROL_PORT);
		debug_switch.gossip(strGossip);
	}else{
		//error, msg
		char strError[50];
		sprintf(strError, "Error connect controller: %s %d", config_switch.MISSION_CONTROL_IP, config_switch.MISSION_CONTROL_PORT);
		debug_switch.error_message(strError);
		return -1;
	}
	return 0;
}

/**
 * Start the switch
 * 2012.02.04: boost mutex
 */
void* vswitch::start(void* paVswitch)
{

	char strTmp[150];

	//setup debug
	vswitch *VSwitch = (vswitch*) paVswitch;  ////2012.02.04 no static cast  (static_cast<vswitch*>(paVswitch))
	debug *____debug_thread = (debug *)&VSwitch->debug_start;
	____debug_thread->set_debuglevel(WHISPER);
	____debug_thread->set_debug_to_stdout();
	____debug_thread->set_debug_to_stderr();
	sprintf(strTmp, "debug_switch_%d.log", VSwitch->config_switch.ID);
	____debug_thread->set_debug_to_logfile(strTmp);
	____debug_thread->message("S T A R T   T H R E A D   S T A R T E D");


	//green for the two threads
	//VSwitch->RunBatch = true;
	VSwitch->RunControllerCommunication  = true;
	VSwitch->RunMissionControl			 = true;
	VSwitch->RunBatch 					 = false;

	/*switch initialize from config.ini, mission control */
	VSwitch->config_switch.init();


	//init controller socket MUTEX     2012.02.04: boost mutex
//	if (pthread_mutex_init(&(VSwitch->mutxSemaphoreControllerSocket), NULL) != 0){
//		____debug_thread->crytical_error_message("mutxSemaphoreControllerSocket could not be initialized");
//		exit(EXIT_FAILURE);
//	}else{
//		____debug_thread->gossip("mutxSemaphoreControllerSocket initialized");
//	}

	//init mission control setup load MUTEX   2012.02.04: boost mutex
//	if (pthread_mutex_init(&(VSwitch->mutxBatchRun), NULL) != 0){
//		____debug_thread->crytical_error_message("mutxBatchRun could not be initialized");
//		exit(EXIT_FAILURE);
//	}else{
//		____debug_thread->gossip("mutxBatchRun initialized");
//	}


	//init mission control setup load MUTEX    2012.02.04: boost mutex
//	if (pthread_mutex_init(&(VSwitch->mutxSemaphoreLoadConfigFromMissionControl), NULL) != 0){
//		____debug_thread->crytical_error_message("mitxSemaphoreLoadConfigFromMissionControl could not be initialized");
//		exit(EXIT_FAILURE);
//	}else{
//		____debug_thread->gossip("mitxSemaphoreLoadConfigFromMissionControl initialized");
//	}

	//CONNECT to mission control
//	if (VSwitch->setup_mission_control_connection() !=0){
//		____debug_thread->crytical_error_message("could not connect to mission control");
//		exit(EXIT_FAILURE);
//	}else{
//		____debug_thread->message("mission control connected");
//	}

	//CONNECT to mission control, more tries  //--// ettol lefele
	bool boConnectionSuccess = false;
	bool boTriesOut = false;  //intToTry = N_TRY_TO_CONNECT
	int  intTry = 0;
//	while ((!boConnectionSuccess) && (!boTriesOut)){  //(intToTry-->0)
//		boTriesOut = (intTry++ >= N_TRY_TO_CONNECT);
//		sprintf(strTmp, "Switch %d tries to connect the MissionControl: %s:%d", VSwitch->config_switch.ID, VSwitch->config_switch.MISSION_CONTROL_IP,VSwitch->config_switch.MISSION_CONTROL_PORT );
//		____debug_thread->whisper(strTmp);
//		boConnectionSuccess = (VSwitch->setup_mission_control_connection() ==0);
//		sleep(SLEEP_BTW_TRIES);
//	}
//	if (!boConnectionSuccess){
//		____debug_thread->crytical_error_message("could not connect to MissionControl");
//		exit(EXIT_FAILURE);
//	}
//
//	//Start mission control thread
//	if (VSwitch->start_mission_control_thread() != 0){
//		____debug_thread->crytical_error_message("-could not start the mission control thread-");
//		exit(EXIT_FAILURE);
//	}else{
//		____debug_thread->message("-mission control thread started-");
//	}

	//WAIT UNTIL MISSION CONTROL LOADS CONFIGURATION (EXTRA MUTEX), or Delay
	sleep(2);

	//CONNECT to controller, single try
//	if (VSwitch->setup_controller_com_connection() != 0){
//		____debug_thread->crytical_error_message("could not connect to controller");
//		exit(EXIT_FAILURE);
//	}else{
//		____debug_thread->message("controller connected");
//	}

	//CONNECT to controller, more tries
	boConnectionSuccess = false;
	boTriesOut = false;
	intTry = 0;
	while ((!boConnectionSuccess) && (!boTriesOut)){
		boTriesOut = (intTry++ >= N_TRY_TO_CONNECT);
		sprintf(strTmp, "Switch %d tries to connect the Controller: %s:%d", VSwitch->config_switch.ID, VSwitch->config_switch.CONTROLLER_IP,VSwitch->config_switch.CONTROLLER_PORT );
		____debug_thread->whisper(strTmp);
		boConnectionSuccess = (VSwitch->setup_controller_com_connection() == 0);
		sleep(SLEEP_BTW_TRIES);
	}
	if (!boConnectionSuccess){
		____debug_thread->crytical_error_message("could not connect to controller");
		exit(EXIT_FAILURE);
	}

	//start communication thread
	if (VSwitch->start_controller_com_thread() != 0){
		____debug_thread->crytical_error_message("could not start the controller communication thread");
		exit(EXIT_FAILURE);
	}else{
		____debug_thread->message("controller communication thread started");
	}


	//WAIT UNTIL HANDSHAKE IS DONE (EXTRA MUTEX?) - no! just duplicate
	if (VSwitch->setup_controller_batch_connection() != 0){
		____debug_thread->crytical_error_message("could not duplicate the controller communication socket");
	}else{
		____debug_thread->message("controller communication socket duplicated");
	}


	//START batch thread
	//start form Mission Control -,but just unlock
	if (VSwitch->start_controller_batch_thread() != 0){
		____debug_thread->crytical_error_message("could not start controller batch thread");
		exit(EXIT_FAILURE);
	}else{
		____debug_thread->message("controller batch thread started");
	}


	//Simulation time

	//sleep(100);      //--------Time Controll----

	//Set variables to shutdown the threads
	//VSwitch->RunControllerCommunication = false;   //--------Time Controll----
	//VSwitch->RunBatch = false;					//--------Time Controll----
	//VSwitch->RunMissionControl = false;  //just from MC



	//WAIT for threads
	sprintf(strTmp,"comthread join: %d",VSwitch->join_controller_com_thread());
	____debug_thread->whisper(strTmp);
	printf("batchthread join: %d",VSwitch->join_controller_batch_thread());
	____debug_thread->whisper(strTmp);
//	printf("mcthread join: %d",VSwitch->join_mission_control_thread());      //--//misssion control
//	____debug_thread->whisper(strTmp);



	//Read the statistics
//	sprintf(strTmp,"Packet missed: %d", VSwitch->config_switch.intStatPacketMissed);    //2012.03.20 kill old statistic
//	____debug_thread->message(strTmp);
//	sprintf(strTmp,"Packet arrived: %d", VSwitch->config_switch.intStatPacketArrived);
//	____debug_thread->message(strTmp);
//	sprintf(strTmp,"Packet time sum: %Lf usec", VSwitch->config_switch.intStatSum);
//	____debug_thread->message(strTmp);
//	sprintf(strTmp,"Packet square-time sum: %Lf usec", VSwitch->config_switch.intStatSumSquare);
//	____debug_thread->message(strTmp);
//	sprintf(strTmp,"Packet Avg:: %Lf usec", VSwitch->config_switch.intStatSum / VSwitch->config_switch.intStatPacketArrived);
//	____debug_thread->message(strTmp);
//	sprintf(strTmp,"Packet VAR:: %Lf usec", (VSwitch->config_switch.intStatSumSquare / VSwitch->config_switch.intStatPacketArrived) - ((VSwitch->config_switch.intStatSum / VSwitch->config_switch.intStatPacketArrived)*(VSwitch->config_switch.intStatSum / VSwitch->config_switch.intStatPacketArrived)));
//		____debug_thread->message(strTmp);
//	sprintf(strTmp,"Total Time: %Lf usec", VSwitch->config_switch.tsEndTime - VSwitch->config_switch.tsStartTime);
//	____debug_thread->message(strTmp);
//	sprintf(strTmp,"P/S: %Lf", VSwitch->config_switch.intStatPacketArrived /(VSwitch->config_switch.tsEndTime - VSwitch->config_switch.tsStartTime) * 1000000);
//	____debug_thread->message(strTmp);
//	sprintf(strTmp,"Min: %d usec", VSwitch->config_switch.intStatMin);
//	____debug_thread->message(strTmp);
//	sprintf(strTmp,"Max: %d usec", VSwitch->config_switch.intStatMax);
//	____debug_thread->message(strTmp);

	return 0;
}


/**
 * communnication thread process code
 * //2012.02.04 no static cast
 */

void* vswitch::controller_communication(void * paVswitch)
{
	vswitch *VSwitch = (vswitch*) paVswitch;  //2012.02.04 no static cast  (static_cast<vswitch*>(paVswitch))

	char strTmp[150];
//	debug ____debug_thread;  //each thread has a file
	debug *____debug_thread = (debug *)&VSwitch->debug_communication;
	____debug_thread->set_debuglevel(MESSAGE);
	____debug_thread->set_debug_to_stdout();
	____debug_thread->set_debug_to_stderr();
	sprintf(strTmp, "debug_controller_communication_%d.log", VSwitch->config_switch.ID);
	____debug_thread->set_debug_to_logfile(strTmp);
	____debug_thread->message("C O N T R O L L E R   C O M M U N I C A T I O T   T H R E A D   S T A R T E D");

	/*H A N D S H A K E*/

	char buf[MAXBUFSIZE];
	//packet pack_hello(OFPT_HELLO);
	//packet pack_features_reply(OFPT_FEATURES_REPLY);
	int size;

	____debug_thread->message("Handshake start");
//  2012.04.08 let the controller initiate the handshake
//	____debug_thread->gossip("Create an initial hello packet for start the handshake");
//	packet_create(buf,&size,OFPT_HELLO, &(VSwitch->config_switch), &(VSwitch->debug_communication));
//	____debug_thread->packet(buf,size,INFO);
//
//	____debug_thread->gossip("send hello packet");
//	send(VSwitch->sockControllerCommunication, buf, size,0);
//	____debug_thread->packet(buf,size,OUT);

	____debug_thread->gossip("wait for incomming message");

	try{
		char *processingEnd = buf;;  //2012.05.19 until this point(adress) the whole packets were recognized
		while (VSwitch->RunControllerCommunication){

			size = recv(VSwitch->sockControllerCommunication,processingEnd,MAXBUFSIZE-(processingEnd - buf),0);  //2012.05.19
			//size = recv(VSwitch->sockControllerCommunication,buf,MAXBUFSIZE,0);  //2012.05.19 -
			____debug_thread->whisper("Incomming stream");
			____debug_thread->packet(processingEnd,size,IN);

			if (processingEnd != buf) {
				____debug_thread->whisper("After correction");
				____debug_thread->packet(buf,size + (processingEnd - buf),IN);
			}

			if (size == 0){
				sprintf(strTmp, "Peer Disconnected / %d"+ VSwitch->config_switch.ID);
				____debug_thread->message(strTmp);
				//VSwitch->mutxBatchRun.lock(); //stop the Batch Thread too!!   //2012.05.24
				//break;
			}

			size += (processingEnd - buf);  //if there are unprocessed packetfrags copied to the begining
			processingEnd = buf;

			//process...
			process_buffer_in(processingEnd,size,____debug_thread,&(VSwitch->config_switch), &(VSwitch->mutxSemaphoreControllerSocket), VSwitch->sockControllerCommunication, VSwitch->send_to_switch);
			//process_buffer_in(buf,size,____debug_thread,&(VSwitch->config_switch), &(VSwitch->mutxSemaphoreControllerSocket), VSwitch->sockControllerCommunication, VSwitch->send_to_switch); //2012.05.19

			//2012.05.19 if there is half packet, copy to begin, start receiving the rest
			if (buf+size != processingEnd)  //there is a broken packet at the end of buffer
			{
				____debug_thread->gossip("Broken packet recovery");
				if (buf+size > processingEnd){
					unsigned int left = buf + size- processingEnd; //bytes undprocessed left on the end of buffer
					sprintf(strTmp,"left moved to front: %d\n", left);
					____debug_thread->gossip(strTmp);
					memmove(buf, processingEnd, left); //copy the left bytes to the begining of the buffer
					processingEnd = buf + left;
				} else {
					____debug_thread->message("....................processingEnd > size ...................????????");
				}
			} else {
				processingEnd = buf;
			}

		}
	}catch (exception e){
		printf("Exception: %s", e.what());
	}
	//Save end time
	//VSwitch->config_switch.tsEndTime = VSwitch->debug_switch.timestamp(NULL); //2012.03.20 kill old statistic

	printf("Controller Communication ended\n");
	return 0;

}

/**
 * Packet generator thread code
 * 2012.02.04: boost mutex
 * 2012.02.04 no static cast
 */
void* vswitch::controller_batch(void * paVswitch)
{
	char strTmp[150];
	vswitch *VSwitch = (vswitch*)paVswitch; //2012.02.04 no static cast (static_cast<vswitch*>(paVswitch))
	debug *__debug = (debug *)&VSwitch->debug_batch;
	__debug->set_debuglevel(MESSAGE);
	__debug->set_debug_to_stdout();
	__debug->set_debug_to_stderr();
	sprintf(strTmp, "debug_controller_batch_%d.log", VSwitch->config_switch.ID);
	__debug->set_debug_to_logfile(strTmp);
	__debug->message("C O N T R O L L E R   B A T C H  T H R E A D   S T A R T");


	char buf[MAXBUFSIZE];
	int size;
	int send_size;
	struct ofp_packet_in * pi;
	struct ether_header * eth;
	int mac_address = 100;
	int switch_id = 1;

	//sleep (6);   //wait for handshake  //doesn't need to wait, handshake already established

	//Reset Statistics
	VSwitch->config_switch.reset_statistic();
	//VSwitch->config_switch.tsStartTime  = VSwitch->debug_switch.timestamp(NULL);  //2012.02.20  kill old statistics
	//VSwitch->config_switch.tsEndTime=0;

	if (VSwitch->config_switch.BATCH_FROM_FILE){  //send packets from pcap file
		mypcap mp(VSwitch->config_switch.PCAP_FILE_OR_DIR, VSwitch->config_switch.PCAP_ROUND_ROBIN);
	}

	//PCAP init
	mypcap *mp;
	if (VSwitch->config_switch.BATCH_FROM_FILE){
		mp = new mypcap(VSwitch->config_switch.PCAP_FILE_OR_DIR, VSwitch->config_switch.PCAP_ROUND_ROBIN);  //initialized by config on create
	}

	//pthread_mutex_lock(&(VSwitch->mutxBatchRun)); //start locked  //2012.02.04: boost mutex
	VSwitch->mutxBatchRun.lock();									//2012.02.04: boost mutex
	while (1){  //VSwitch->RunBatch
		//sleep(0);
		usleep(VSwitch->config_switch.intUSleepBatchprocess);

		//ask for lock
		VSwitch->mutxBatchRun.lock();
		VSwitch->mutxBatchRun.unlock();
		//pthread_mutex_lock(&(VSwitch->mutxBatchRun));
		//pthread_mutex_unlock(&(VSwitch->mutxBatchRun));

//		memcpy(buf, packet_in_fake, sizeof(packet_in_fake));
//		pi = (struct ofp_packet_in *) buf;
//		pi->header.version = OFP_VERSION;
//		pi->header.xid = htonl(1);//nix
//		pi->buffer_id = htonl(1);
//		eth = (struct ether_header * ) pi->data;
//		memcpy(&eth->ether_shost[1], &mac_address, sizeof(mac_address));
//		eth->ether_dhost[5] = switch_id;
//		eth->ether_shost[5] = switch_id;

		if (VSwitch->config_switch.BATCH_FROM_FILE) {
			//PCAP
			//printf("read from batch!!!!!  %d  \n", VSwitch->config_switch.ID);
			mp->get_next_packet(buf, &size);
			if (size > 0){
				printf("SW%d size: %d\n",VSwitch->config_switch.ID, size);

				int packet_id;
				packet_id = VSwitch->config_switch.SendPacketID();
				pi = (struct ofp_packet_in *) buf;
				pi->buffer_id = htonl(packet_id);
				pi->header.xid = htonl(packet_id);

				VSwitch->mutxSemaphoreControllerSocket.lock();
				send_size = send(VSwitch->sockControllerCommunication, buf, size,0);
				VSwitch->mutxSemaphoreControllerSocket.unlock();
				if (send_size == 0){
					__debug->message("!sendsize ==0!");
				}
			}

		}else{
			//FAKE PACKET_IN
			packet_create(buf, &size, OFPT_PACKET_IN, &(VSwitch->config_switch),&(VSwitch->debug_batch));
			__debug->gossip("Send OFPT_PACKET_IN");
			__debug->whisper("Mutex lock and send...");
			//pthread_mutex_lock(&(VSwitch->mutxSemaphoreControllerSocket));  //2012.02.04: boost mutex
			VSwitch->mutxSemaphoreControllerSocket.lock();					  //2012.02.04: boost mutex
			send_size = send(VSwitch->sockControllerCommunication, buf, sizeof(packet_in_fake),0);
			VSwitch->mutxSemaphoreControllerSocket.unlock();				  //2012.02.04: boost mutex
			//pthread_mutex_unlock(&(VSwitch->mutxSemaphoreControllerSocket));//2012.02.04: boost mutex
			__debug->packet(buf,sizeof(packet_in_fake),OUT);
			__debug->whisper("Mutex unlocked.");
			if (send_size == 0){
				__debug->message("!sendsize ==0!");
			}

		}
	}

	delete(mp);

//	pthread_mutex_lock(&(VSwitch->mutxSemaphoreControllerSocket));
//	pthread_mutex_unlock(&(VSwitch->mutxSemaphoreControllerSocket));
	__debug->message("C O N T R O L L E R   B A T C H  T H R E A D   E N D");

	__debug->~debug();
	pthread_exit(NULL);  //kill the thread

	return 0;

}

/**
 * Control Central thread code
 */
void* vswitch::mission_control(void * paVswitch)
{
	char strTmp[150];
	vswitch *VSwitch = (vswitch*) paVswitch;  //2012.02.04 no static cast (static_cast<vswitch*>(paVswitch))
	debug *____debug = (debug *)&VSwitch->debug_mission;
	____debug->set_debuglevel(WHISPER);
	____debug->set_debug_to_stdout();
	____debug->set_debug_to_stderr();
	//sleep(3);
	sprintf(strTmp, "debug_mission_control_%d.log", VSwitch->config_switch.ID);
	____debug->set_debug_to_logfile(strTmp);
	____debug->message("M I S S I O N   C O N T R O L   T H R E A D   S T A R T");

	//Login to mission control
	while (!VSwitch->login_to_mission_control_()) {   //!VSwitch->
		//probably must the connection establishment into the login_to_mission_control
		sleep(3);
	}

	//sleep(20);

	//Logged in, wait for commands
	//while (VSwitch->RunMissionControl){
	while(VSwitch->RunMissionControl){
	    char strTmp[100];
		sprintf(strTmp, "-MC- Getting the new command from the Mission Control...");
		____debug->gossip(strTmp);
		if (!VSwitch->get_command_()){
			____debug->gossip("-MC- received 0-size packets = connection lost,STOP MissionColtrol");
			VSwitch->RunMissionControl = false;
		}
	}
	____debug->message("M I S S I O N   C O N T R O L   T H R E A D   E N D");

	//end mission Control (flush files?)
	____debug->~debug();
	return 0;

}

/**
 * Start all threads
 */
int vswitch::start_start_thread()
{
//	int nerr = ::pthread_create(&thrStart, 0, start, static_cast<void*>(this));
//	if (nerr) {
//		debug_switch.error_message("Start thread not created");
//	}
//	return 0;

	try{
		thrStart = boost::thread(&start,this);
	}catch (exception e){
		printf("Exception(start_start_thread): %s", e.what());
	}
	return 0;


}

/**
 * controller thread start
 */
int vswitch::start_controller_com_thread()
{
//	RunControllerCommunication = true;
//	int nerr;
//	try{
//		nerr =::pthread_create(&thrControllerCommunication, 0, controller_communication, static_cast<void*>(this) );
//	}catch (exception e){
//		printf("Exception: %s", e.what());
//	}
//	if (nerr) {
//		debug_switch.error_message("Controller communication thread not created");
//	}
//	return nerr;

	RunControllerCommunication = true;
	try{
		thrControllerCommunication = boost::thread(&vswitch::controller_communication,this);
	}catch (exception e){
		//char strTmp[100];
		//sprintf(strTmp,"Exception(start_controller_com_thread): %s", e.what());
		//debug_switch.error_message(strTmp);
		printf("Exception(start_controller_com_thread): %s", e.what());
	}
	return 0;

}

/**
 * batch thread start
 */
int vswitch::start_controller_batch_thread()
{
//	RunBatch = true;
//	int nerr;
//
//	//Thread killen if exists
////	if (thrControllerBatch !=0){
////		pthread_(thrControllerBatch);
////	}
//    //lock bevor start
//	pthread_mutex_lock(&(this->mutxBatchRun));
//
//	nerr =::pthread_create(&thrControllerBatch, 0, controller_batch, static_cast<void*>(this) );
//
//	if (nerr) {
//		debug_switch.error_message("Controller batch thread not created");
//	}
//	return nerr;

	try{
		thrControllerBatch = boost::thread(&controller_batch,this);
	}catch (exception e){
		//char strTmp[100];
		//sprintf(strTmp, "Exception(start_controller_batch_thread): %s", e.what());
		//debug_switch.error_message(strTmp);
		printf("Exception(start_controller_batch_thread): %s", e.what());
	}
	return 0;

}

/**
 * control central thread start
 */
int vswitch::start_mission_control_thread()
{
//	RunMissionControl = true;
//	int nerr =::pthread_create(&thrMissionControl, 0, mission_control, static_cast<void*>(this) );
//	if (nerr) {
//		debug_switch.error_message("Mission thread not created");
//	}
//	return nerr;

	RunMissionControl = true;
	try{
		thrMissionControl = boost::thread(&mission_control,this);
	}catch (exception e){
		printf("Exception(start_controller_batch_thread): %s", e.what());
	}
	return 0;
}


int vswitch::join_start_thread()
{
//	int nerr = ::pthread_join(get_start_TID(),0);
//	return nerr;
	thrStart.join();
}

int vswitch::join_controller_com_thread()
{
//	int nerr = ::pthread_join(get_controller_communication_TID(),0);
//	return nerr;
	thrControllerCommunication.join();
}

int vswitch::join_controller_batch_thread()
{
//	int nerr = ::pthread_join(get_controller_batch_TID(),0);
//	return nerr;
}

int vswitch::join_mission_control_thread()
{
//	int nerr = ::pthread_join(get_mission_control_TID(),0);
//	return nerr;
	thrControllerBatch.join();
}




int vswitch::close_controller_com_connection()
{
	close(sockControllerCommunication);
	debug_switch.gossip("Controller communication socket closed");
	return 0;
}

int vswitch::close_controller_batch_connection()
{
	//close(sockControllerBatch);
	debug_switch.gossip("Controller Batch connection closed - ?? any error ??");
	return 0;
}

int vswitch::close_mission_control_connection()
{
	close(sockMissionControl);
	debug_switch.gossip("Misson control connection is closed");
	return 0;
}


/**
 * *****************************************************************************************
 * Communication with the mission control
 * *****************************************************************************************
 */

bool vswitch::login_to_mission_control_()
{
	char strTmp[100];
	char buf[MAXREADBUFSIZE];
	int size = 0;
	size = recv(sockMissionControl, buf, MAXREADBUFSIZE-1,0);
	buf[size] = 0;
	sprintf(strTmp, "Welcome message from the Server: %s", buf);
	this->debug_mission.gossip(strTmp);

	message_header *MessageHeader;
	char bufwrite[MAXWRITEBUFSIZE];
	MessageHeader = (message_header*) bufwrite;
	MessageHeader->version = htons(COM_VERSION);//htons(COM_VERSION);
	MessageHeader->command = htons(LOGIN_REQUEST);
	MessageHeader->param   = htons(ID);
	MessageHeader->error   = htons(NO_ERROR_OK);
	MessageHeader->length = htonl(sizeof(PASSWORD));

	char *password;
	password = bufwrite + sizeof(message_header);

	int intSendLength;
	strcpy(password, PASSWORD);

	sprintf(strTmp,"Sending login request...");
	this->debug_mission.gossip(strTmp);

	intSendLength = send(sockMissionControl, bufwrite, sizeof(message_header)+sizeof(PASSWORD),0);

	sprintf(strTmp,"Login request sent, size: %d", intSendLength);
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp,"Reading Answer...");
	this->debug_mission.whisper(strTmp);

	size = recv(sockMissionControl, buf, sizeof(message_header),0);

	sprintf(strTmp,"Answer red, size: %d", size);
	this->debug_mission.whisper(strTmp);

	MessageHeader = (message_header*) buf;

	sprintf(strTmp,"Version   :%d", MessageHeader->version);
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp,"Command:   %d", MessageHeader->command);
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp,"Parameter: %d", MessageHeader->param);
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp,"Error:     %d", MessageHeader->error);
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp,"Size:      %d", MessageHeader->length);
	this->debug_mission.whisper(strTmp);

	bool boCheckLogin = true;
	//packet size
	if (boCheckLogin){
		if (size <= 0) {
			sprintf(strTmp,"Packet Size Problem!");
			this->debug_mission.whisper(strTmp);

			boCheckLogin = false;
		}else{
			sprintf(strTmp,"Packet Size ok!");
			this->debug_mission.whisper(strTmp);
		}
	}
	//command
	if (boCheckLogin){
		if (MessageHeader->command != LOGIN_OK) {
			sprintf(strTmp,"command do not LOGIN_OK!");
			this->debug_mission.whisper(strTmp);
			boCheckLogin = false;
		}else{
			sprintf(strTmp,"command LOGIN_OK!");
			this->debug_mission.whisper(strTmp);
		}
	}
	//password
	char PasswordReceived[MessageHeader->length];
	size = recv(sockMissionControl, &PasswordReceived, MessageHeader->length,0);
	if (boCheckLogin){
		if (strcmp((char*)PasswordReceived, (char*) &PASSWORD)!=0){
			sprintf(strTmp,"password do not much: rcv:%s sent:%s!",&PasswordReceived, &PASSWORD);
			this->debug_mission.whisper(strTmp);

			boCheckLogin = false;
		}else{
			sprintf(strTmp,"password ok: %s", &PasswordReceived);
			this->debug_mission.whisper(strTmp);
		}
	}
	if (boCheckLogin){
		sprintf(strTmp,"Auth Successfull! ");
		this->debug_mission.gossip(strTmp);
	}
	return boCheckLogin;
//	if (size > 0) {
//		MessageHeader = (message_header*) buf;
//		printf("Ver:%d\n", MessageHeader->version);
//		printf("Command:%d\n", MessageHeader->command);
//		printf("Param: %d\n", MessageHeader->param);
//		printf("Error: %d\n", MessageHeader->error);
//		printf("Size: %d\n", MessageHeader->length);
//		if ((MessageHeader->command == LOGIN_OK) && (MessageHeader->length == sizeof(PASSWORD))){
//			printf("login ok, size password\n");
//			password = buf + sizeof(message_header);
//			if (strcmp((char*)password, (char*) &PASSWORD)==0){
//				printf("PasswordOK \n");
//				return true;
//			} else {
//				printf("Password not ok \n");
//				return false;
//			}
//		} else {
//
//		}
//	}else{
//		printf("password or logink not ok\n");
//		return false;
//	}
}

/**
 * getting command from mission control
 */
int vswitch::get_command_()
{
	//read from socket and propagate to command_picker
	char strTmp[100];
	char buf[sizeof(message_header)];
	int size = 0;
	size = recv(sockMissionControl, buf, sizeof(message_header),0);
	message_header *MessageHeader;
	MessageHeader = (message_header*) buf;
	if (!size) {
		sprintf(strTmp,"-MC- Received Header Size: %d, connection lost. Returning %d", size, size);
		this->debug_mission.error_message(strTmp);
		return size;
	}
	if (size != sizeof(message_header)){
		buf[size] = 0;
		sprintf(strTmp,"-MC- GetCommand: This is not a header! length: %d", size);
		this->debug_mission.error_message(strTmp);
		//connection close!!!
	} else {
		sprintf(strTmp,"-MC- header from the mission control, size: %d\n", size);
		this->debug_mission.whisper(strTmp);

		//Version
		if (MessageHeader->version != COM_VERSION){
			sprintf(strTmp, "Wrong version: %d (actual version:%d)",MessageHeader->version, COM_VERSION );
			this->debug_mission.error_message(strTmp);

			//connection close,
		}else {
			sprintf(strTmp, "Version OK: %d",MessageHeader->version);
			this->debug_mission.gossip(strTmp);
		}
		//

	}
	this->debug_mission.whisper("Command Picker ->");

	command_picker_(MessageHeader);
	return size;
}

/**
 * command parsing
 */
void vswitch::command_picker_(message_header *CommandHeader)
{
	char strTmp[100];
	sprintf(strTmp,"Ver:%d", CommandHeader->version);
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp,"Command:%d", CommandHeader->command);
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp,"Param: %d", CommandHeader->param);
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp,"Error: %d", CommandHeader->error);
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp,"Size: %d", CommandHeader->length);
	this->debug_mission.whisper(strTmp);

	switch (CommandHeader->command) {
		case HELLO		: hello_    (CommandHeader); break;
		case GET 		: get_      (CommandHeader); break;
		case SET	    : set_      (CommandHeader); break;
		case START		: start_    (); 			 break;
		case STOP		: stop_     (); 			 break;
		case QUIT_      : quit_     (CommandHeader); break;
		case RESTART	: restart_  (CommandHeader); break;
		case GET_FILE   : get_file_ (CommandHeader); break;
		case SEND_FILE  : put_file_ (CommandHeader); break;
		default         : command_not_recognized_(CommandHeader); break;
	}

}

/**
 * command answers to control central
 */
void vswitch::send_command(message_header *mhOutgoing, char *paPayload, int paIntPayloadSize)
{
	char strTmp[100];
	sprintf(strTmp, "Sending Message: %d", htons(mhOutgoing->command));
    this->debug_mission.message(strTmp);

	sprintf(strTmp, "Version: %d",htons(mhOutgoing->version));
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp, "command: %d",htons(mhOutgoing->command));
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp, "param: %d",htons(mhOutgoing->param));
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp, "error: %d",htons(mhOutgoing->error ));
	this->debug_mission.whisper(strTmp);
	sprintf(strTmp, "length: %d",htonl(mhOutgoing->length));
	this->debug_mission.whisper(strTmp);

    int intHeaderLength, intPayloadLength = 0;
    intHeaderLength = send(sockMissionControl, mhOutgoing, sizeof(message_header),0);
    if (paIntPayloadSize > 0){
    	intPayloadLength = send(sockMissionControl, paPayload, paIntPayloadSize,0);
    }

	sprintf(strTmp,"Message with header size: %d and payload size %d sent.", intHeaderLength, intPayloadLength);
	this->debug_mission.gossip(strTmp);

}

/**
 * on receiving hello command
 */
void vswitch::hello_(message_header *CommandHeader){
	this->debug_mission.message("got HELLO");
	this->debug_mission.message("Sending HELLO back");

	char buf[12];
	char buf2[1]; //empty
	message_header *CommandResponse;
	CommandResponse = (message_header*) buf;
	CommandResponse->version = htons(COM_VERSION);
	CommandResponse->command = htons(HELLO);
	CommandResponse->param   = htons(ID);
	CommandResponse->error   = htons(NO_ERROR_OK);
	CommandResponse->length  = htons(0);

	send_command(CommandResponse, buf2, 0);

}




void vswitch::command_not_recognized_(message_header *CommandHeader)
{

}

/**
 * on receiving set command
 */
void vswitch::set_(message_header *CommandHeader)
{
	char buf[MAXREADBUFSIZE];
	char strTmp[100];
	char PayloadReceived[CommandHeader->length];
	int size;
	size = recv(sockMissionControl, &PayloadReceived, CommandHeader->length,0);

	sprintf(strTmp, "Command SET with PayloadLength: %d", CommandHeader->length);
	this->debug_mission.gossip(strTmp);

	char *rest;  // for convert
	switch (CommandHeader->param){
	case ID :

		sprintf(strTmp, "Received ID: %s", &PayloadReceived);
		this->debug_mission.gossip(strTmp);

		//convert
		long int id;
		id = strtol(PayloadReceived, &rest, 10);
		config_switch.ID = id;

		sprintf(strTmp, " config_switch.ID: %d", config_switch.ID);
		this->debug_mission.gossip(strTmp);

		break;
	case CONTROLLER_IP :
		sprintf(strTmp, "Received CONTROLLER_IP : %s", &PayloadReceived);
		this->debug_mission.gossip(strTmp);
		strcpy(config_switch.CONTROLLER_IP, PayloadReceived);
		sprintf(strTmp, "New Controller IP: %s", config_switch.CONTROLLER_IP);
		this->debug_mission.message(strTmp);
		break;
	case CONTROLLER_PORT :
		sprintf(strTmp, "Received CONTROLLER_PORT : %s", &PayloadReceived);
		this->debug_mission.gossip(strTmp);

		//convert
		long int port;
		port = strtol(PayloadReceived, &rest, 10);
		config_switch.CONTROLLER_PORT = port;
		sprintf(strTmp, "New Controller Port: %d", config_switch.CONTROLLER_PORT);
		this->debug_mission.message(strTmp);
		break;
	case USLEEP_BATCHPROCESS:
		sprintf(strTmp, "Received USLEEP_BATCHPROCESS : %s", &PayloadReceived);
		this->debug_mission.gossip(strTmp);

		//convert
		long int usleeptime;
		usleeptime = strtol(PayloadReceived, &rest, 10);
		config_switch.intUSleepBatchprocess = usleeptime;
		sprintf(strTmp, "New Batch Process Sleep: %d usec", config_switch.intUSleepBatchprocess);
		this->debug_mission.message(strTmp);

		break;
	case SESSION_ID:
		sprintf(strTmp, "Received SESSION_ID : %s", &PayloadReceived);
		this->debug_mission.gossip(strTmp);
		long int sessionid;
		sessionid = strtol(PayloadReceived, &rest, 10);
		config_switch.SessionID = sessionid;
		sprintf(strTmp, "New SessionID: %d usec", config_switch.SessionID);
		this->debug_mission.message(strTmp);
		break;
	default:
		this->debug_mission.error_message("Unrecognized SET command");
		break;

	}
}


/**
 * on receiving get command
 * Rewrite!!!!
 */
void vswitch::get_(message_header *CommandHeader)
{
	this->debug_mission.gossip("Command GET");

	char strTmp[100];
	char bufwrite[MAXWRITEBUFSIZE];
	message_header *ResponseHeader;
	ResponseHeader = (message_header*) bufwrite;
	ResponseHeader->version = htons(COM_VERSION);
	ResponseHeader->command = htons(GET);
	ResponseHeader->error   = htons(NO_ERROR_OK);
	char *strPayload;
	strPayload = bufwrite + sizeof(message_header);
	switch (CommandHeader->param){
	case ID:
		ResponseHeader->param   = htons(ID);
		sprintf(strPayload, "%d", config_switch.ID);
		ResponseHeader->length  = htonl(strlen(strPayload)+1);
		break;
	case PACKET_RECEIVED:
		ResponseHeader->param   = htons(PACKET_RECEIVED);
		//sprintf(strPayload, "%d", config_switch.intStatPacketArrived);
		ResponseHeader->length  = htonl(strlen(strPayload)+1);
		break;
	case PACKET_SENT:
		ResponseHeader->param   = htons(PACKET_SENT);
		//sprintf(strPayload, "%d", config_switch.intStatPacketMissed + config_switch.intStatPacketArrived);
		ResponseHeader->length  = htonl(strlen(strPayload)+1);
		break;
	case PACKET_MEANTIME:
		ResponseHeader->param   = htons(PACKET_MEANTIME);
		//sprintf(strPayload, "%Lf", config_switch.intStatSum / config_switch.intStatPacketArrived);
		ResponseHeader->length  = htonl(strlen(strPayload)+1);
		break;
	case PACKET_VAR:
		ResponseHeader->param   = htons(PACKET_VAR);
		//sprintf(strPayload, "%Lf", (config_switch.intStatSumSquare / config_switch.intStatPacketArrived) - ((config_switch.intStatSum / config_switch.intStatPacketArrived)*(config_switch.intStatSum / config_switch.intStatPacketArrived)));
		ResponseHeader->length  = htonl(strlen(strPayload)+1);
	    break;
	case PACKET_P_SECOND:
		ResponseHeader->param   = htons(PACKET_P_SECOND);
		//sprintf(strPayload, "%Lf", config_switch.intStatPacketArrived /(debug_mission.timestamp(NULL) - config_switch.tsStartTime) * 1000000);
		ResponseHeader->length  = htonl(strlen(strPayload)+1);
		break;
	case USLEEP_BATCHPROCESS:
		ResponseHeader->param   = htons(USLEEP_BATCHPROCESS);
		sprintf(strPayload, "%d", config_switch.intUSleepBatchprocess);
		ResponseHeader->length  = htonl(strlen(strPayload)+1);
		break;
	case SESSION_ID:
		ResponseHeader->param   = htons(SESSION_ID);
		sprintf(strPayload, "%d", config_switch.SessionID);
		ResponseHeader->length  = htonl(strlen(strPayload)+1);
		break;
	default:
		this->debug_mission.error_message("Unrecognized GET command");
		break;
	}

	sprintf(strTmp, "Sending answer to GET, payload size: %d", strlen(strPayload)+1);
	this->debug_mission.whisper(strTmp);

	int intSendLength = send(sockMissionControl, bufwrite, sizeof(message_header) + strlen(strPayload)+1,0);

	this->debug_mission.whisper("GET Answer sent");

}

void vswitch::get_file_(message_header *CommandHeader)
{

}
void vswitch::put_file_(message_header *CommandHeader)
{

}

/**
 * on receiving stop command
 * //2012.02.04: boost mutex
 */
void vswitch::stop_()
{
	this->debug_mission.message("Command STOP received, stops Batch, ControllerCom");
	//stop batch

	//do not stop, just lock
	//RunBatch = false;
	//pthread_mutex_lock(&(this->mutxBatchRun));  //2012.02.04: boost mutex
	this->mutxBatchRun.lock();					  //2012.02.04: boost mutex

	//stop controller communication
	//RunControllerCommunication = false;  //do not shut down
}
/**
 * on receiving start command
 */
void vswitch::start_()
{
	this->debug_mission.message("Command START received, start Batch, ControllerCom");

	RunControllerCommunication = true;

	//this->start_controller_com_thread();    //do not shut down

	//no reconstruct, just unlock
	//if (!RunBatch) {
	//    this->start_controller_batch_thread();
	//}

	//pthread_mutex_unlock(&(this->mutxBatchRun));   //2012.02.04: boost mutex
	this->mutxBatchRun.unlock();  					 //2012.02.04: boost mutex



}

/**
 * on receiving quit command
 */
void vswitch::quit_(message_header *CommandHeader)
{
	this->debug_mission.message("Command QUIT received, stops MC");
	RunBatch = false;
	RunControllerCommunication = false;
	RunMissionControl = false;
}

/**
 * on receiving restarrt command
 */
void vswitch::restart_(message_header *CommandHeader)
{

}

/**
 * for pcap and lldp
 */

int vswitch::send_packet_to_controller(char *po, int size)
{
	char strTmp[50];
	sprintf(strTmp, "Send packet out to controller (lldp or pcap)");
	debug_communication.message(strTmp);
	debug_communication.packet(po,size,OUT);

	mutxSemaphoreControllerSocket.lock();
	send(sockControllerCommunication, po, size,0);
	mutxSemaphoreControllerSocket.unlock();
	return 0;
}



#endif /*switch_h*/






#include <stdio.h>
#include "debug.hpp"
#include <sstream>
#include "switch.hpp"
#include "communication.hpp"
#include "minIni.h"
#include <fstream>
#include <math.h>
#include "topology.hpp"
#include "mypcap.hpp"

#define MAXBUFLENGTH 1024*100
#define INIFILE "config.ini"

int ClientMCSocket;
char MC_IP[20];
int  MC_Port;
const char client_inifile[] = "config.ini";

char CLIENT_MISSION_CONTROL_IP[20];
int  CLIENT_MISSION_CONTROL_PORT;

vswitch *switches[401];

/**
 * Callbach function to send out packets on other switches. (for LLDP and OFDP packets)
 */
void send_packet_through_switch(char *po, int size, int switch_ID)
{
	//printf("callback function start!!!: %d %d \n", size, switch_ID );
	switches[switch_ID]->send_packet_to_controller(po,size);
}



int main()
{
	int intCountSwitches = 0;
	char strTmp2[100];
	debug debug_client;
	debug_client.set_debuglevel(WHISPER);
	debug_client.set_debug_to_stdout();
	debug_client.set_debug_to_stderr();
	debug_client.set_debug_to_logfile("debug_client.log");
	debug_client.message("S T A R T   C L I E N T");

//   char ts[50];

//   the_debug.timestamp_str(ts);
//   printf("meghiv: %s\n",ts);


//   int i;
//   for (i=0;i<10;i++){
//
//		long double dblTs, dblTs2;
//		dblTs = the_debug.timestamp(NULL);
//		printf("dbl Timestamp: %Lf\n", dblTs);
//
//		the_debug.timestamp(&dblTs2);
//		printf("dbl Timestamp: %Lf\n", dblTs);
//		printf("minus Timestamp: %Lf\n", dblTs2 - dblTs);
//
//   }

//   the_debug.set_debug_to_stdout();
//   the_debug.set_debug_to_stderr();
//   the_debug.set_debug_to_logfile("test.log");
//   the_debug.set_debug_to_htmlfile("test.html");
//
//   the_debug.crytical_error_message("CryticalERR");
//   the_debug.error_message("One error must be");
//   the_debug.exception("Except one");
//   the_debug.message("Hello");
//   the_debug.gossip("No info");
//
//   the_debug.packet(fake,sizeof(fake), IN);
//   the_debug.packet(fake,sizeof(fake), OUT);
//
//   the_debug.set_debug_to_streamfile_in("ccc.txt");
//   the_debug.set_debug_to_streamfile_out("ddd.txt");
//
//   the_debug.stream(fake,sizeof(fake), IN);
//   the_debug.stream(fake,sizeof(fake), OUT);

   //lesen aus der file ========================MISSION CONTROL CONFIG FROM FILE=============================================
	long n;
	n = ini_gets("MissionControl", "ip", "127.0.0.1", CLIENT_MISSION_CONTROL_IP, sizeof(CLIENT_MISSION_CONTROL_IP), inifile);
	CLIENT_MISSION_CONTROL_PORT = ini_getl("MissionControl", "port", 10, inifile);
   //socket aufbauen
     if ((ClientMCSocket = socket(AF_INET, SOCK_STREAM, 0)) > 0){
    	 debug_client.gossip("mission control socket created");
   	}else{
   		debug_client.error_message("mission control socket could not be created");
   		return -1;
   	}

//===========================CONNECT TO MISSION CONTROL===================================================================

   //verbindung   ==================================
//     sprintf(strTmp2, "Connenct to MissionControl %s:%d", CLIENT_MISSION_CONTROL_IP,CLIENT_MISSION_CONTROL_PORT);
//     debug_client.gossip(strTmp2);
//
//    struct sockaddr_in address;
//	address.sin_family = AF_INET;
//	address.sin_port = htons(CLIENT_MISSION_CONTROL_PORT);
//	inet_aton(CLIENT_MISSION_CONTROL_IP, &address.sin_addr);
//
//	if (connect(ClientMCSocket, (struct sockaddr *) &address, sizeof(address))==0){
//		char strGossip[50];
//		sprintf(strGossip, "MissionControl: %s:%d connected", CLIENT_MISSION_CONTROL_IP, CLIENT_MISSION_CONTROL_PORT);
//		debug_client.message(strGossip);
//	}else{
//		//error, msg
//		char strError[50];
//		sprintf(strError, "Error connect controller: %s %d", CLIENT_MISSION_CONTROL_IP, CLIENT_MISSION_CONTROL_PORT);
//		debug_client.error_message(strError);
//		return -1;
//	}
//
//	//login   ======================================
//	char buf[MAXBUFLENGTH];
//	int size = 0;
//	size = recv(ClientMCSocket, buf, MAXBUFLENGTH-1,0);
//	buf[size] = 0;
//	sprintf(strTmp2, "Welcome message from the Server: %s", buf);
//	debug_client.gossip(strTmp2);
//
//	message_header *MessageHeader;
//	char bufwrite[MAXWRITEBUFSIZE];
//	MessageHeader = (message_header*) bufwrite;                          //build OFCP message
//	MessageHeader->version = htons(COM_VERSION);//htons(COM_VERSION);
//	MessageHeader->command = htons(LOGIN_CLIENT_REQUEST);
//	MessageHeader->param   = htons(ID);
//	MessageHeader->error   = htons(NO_ERROR_OK);
//	MessageHeader->length = htonl(sizeof(PASSWORD));
//
//	char *password;
//	password = bufwrite + sizeof(message_header);
//
//	int intSendLength;
//	strcpy(password, PASSWORD);
//
//	sprintf(strTmp2,"Sending login request...");
//	debug_client.gossip(strTmp2);
//
//	intSendLength = send(ClientMCSocket, bufwrite, sizeof(message_header)+sizeof(PASSWORD),0);   //send password
//
//	sprintf(strTmp2,"Login request sent, size: %d", intSendLength);
//	debug_client.whisper(strTmp2);
//	sprintf(strTmp2,"Reading Answer...");
//	debug_client.whisper(strTmp2);
//
//	size = recv(ClientMCSocket, buf, sizeof(message_header),0);
//
//	sprintf(strTmp2,"Answer red, size: %d", size);
//	debug_client.whisper(strTmp2);
//
//	MessageHeader = (message_header*) buf;
//
//	sprintf(strTmp2,"Version   :%d", htons(MessageHeader->version));
//	debug_client.whisper(strTmp2);
//	sprintf(strTmp2,"Command:   %d", htons(MessageHeader->command));
//	debug_client.whisper(strTmp2);
//	printf(strTmp2,"Parameter: %d", htons(MessageHeader->param));
//	debug_client.whisper(strTmp2);
//	sprintf(strTmp2,"Error:     %d", htons(MessageHeader->error));
//	debug_client.whisper(strTmp2);
//	sprintf(strTmp2,"Size:      %d", htonl(MessageHeader->length));
//	debug_client.whisper(strTmp2);
//
//	bool boCheckLogin = true;
//	//packet size
//	if (boCheckLogin){
//		if (size <= 0) {
//			sprintf(strTmp2,"Packet Size Problem!");
//			debug_client.whisper(strTmp2);
//
//			boCheckLogin = false;
//		}else{
//			sprintf(strTmp2,"Packet Size ok!");
//			debug_client.whisper(strTmp2);
//		}
//	}
//	//command
//	if (boCheckLogin){
//		if (MessageHeader->command != LOGIN_OK) {
//			sprintf(strTmp2,"command do not LOGIN_OK!");
//			debug_client.whisper(strTmp2);
//			boCheckLogin = false;
//		}else{
//			sprintf(strTmp2,"command LOGIN_OK!");
//			debug_client.whisper(strTmp2);
//		}
//	}
//	//password
//	char PasswordReceived[MessageHeader->length];
//	size = recv(ClientMCSocket, &PasswordReceived, MessageHeader->length,0);
//	if (boCheckLogin){
//		if (strcmp((char*)PasswordReceived, (char*) &PASSWORD)!=0){
//			sprintf(strTmp2,"password do not match: rcv:%s sent:%s!",&PasswordReceived, &PASSWORD);
//			debug_client.whisper(strTmp2);
//#include <fstream>
//			boCheckLogin = false;
//		}else{
//			sprintf(strTmp2,"password ok: %s", &PasswordReceived);
//			debug_client.whisper(strTmp2);
//		}
//	}
//	if (boCheckLogin){
//		sprintf(strTmp2,"Auth for client Successfull! ");
//		debug_client.gossip(strTmp2);
//	}

	int i,j;

	//Test one switch===========================================================================================
//	intCountSwitches++;
//	switches[intCountSwitches] = new vswitch(intCountSwitches);
//	//switches[intCountSwitches]->start_controller_batch_thread();
//	strcpy(switches[intCountSwitches]->config_switch.CONTROLLER_IP, "192.168.42.128");
//	switches[intCountSwitches]->config_switch.CONTROLLER_PORT = 6633;
//	sleep(10);
//	char strTmpp[50];
//	for (i=1;i<=400;i++){
//		sprintf(strTmpp,"lauf: %d start\n",i );
//		//debug_client.whisper(strTmpp);
//		switches[intCountSwitches]->RunBatch = true;
//		switches[intCountSwitches]->start_();
//		sleep(1);
//		switches[intCountSwitches]->stop_();
//		//switches[intCountSwitches]->RunBatch = false;
//		sleep(1);
//		sprintf(strTmpp,"lauf: %d stop, received packets: %d\n",i, switches[intCountSwitches]->config_switch.intPacketCounter );
//		debug_client.whisper(strTmpp);
//		switches[intCountSwitches]->config_switch.reset_statistic();
//	}
	//=================================================================================================

	//Test more switches===========================================================================================
//	for (i=1;i<=100;i++){
//		intCountSwitches++;
//		switches[intCountSwitches] = new vswitch(intCountSwitches);
//		sleep(0);
//		switches[intCountSwitches]->config_switch.ID = i;   //no mission control
//
//		//switches[intCountSwitches]->start_controller_batch_thread();
//		strcpy(switches[intCountSwitches]->config_switch.CONTROLLER_IP, "192.168.42.128");
//		//switches[intCountSwitches]->config_switch.CONTROLLER_PORT = 6633;
//	}
//	sleep(10);
//	char strTmpp[50];
//	for (i=1;i<=400;i++){
//		sprintf(strTmpp,"lauf: %d start\n",i );
//		//debug_client.whisper(strTmpp);
//		for (j=1;j<=intCountSwitches;j++){
//			switches[j]->RunBatch = true;
//			switches[j]->start_();
//		}
//			sleep(1);
//		for (j=1;j<=intCountSwitches;j++){
//			switches[j]->stop_();
//			//switches[intCountSwitches]->RunBatch = false;
//		}
//			sleep(20);
//		for (j=1;j<=intCountSwitches;j++){
//			sprintf(strTmpp,"lauf: %d stop, received packets: %d\n",i, switches[j]->config_switch.intPacketCounter );
//			debug_client.whisper(strTmpp);
//			switches[j]->config_switch.reset_statistic();
//		}
//	}
	//================================0
	//         TOPOLOGY
	//================================0
	topo.all_switches = (void **) switches;
	topo.LoadFromFile("topology.ini");
	topo.Print(20);
	topo.PrintConnection(1,3);
	topo.PrintConnection(1,1);
	topo.PrintConnection(3,2);
	topo.PrintConnection(4,2);

//	//================================0
//	//         PCAP
//	//================================0
//	mypcap mp("test.pcap", true);
//	int sz =0;
//	char bf[100000];
//	while (sz>-1){
//		mp.get_next_packet(bf, &sz);
//		printf("PCAP size: %d\n", sz);
//	}





	//=================================================================================================
	//     S I M U L A T I O N      ====================================
	//=================================================================================================
	//n = ini_gets("MissionControl", "ip", "127.0.0.1", MISSION_CONTROL_IP, sizeof(MISSION_CONTROL_IP), inifile);
	//n = ini_gets("Controller",     "ip", "127.0.0.1", CONTROLLER_IP, sizeof(CONTROLLER_IP), inifile);
	//MISSION_CONTROL_PORT = ini_getl("MissionControl", "port", 10, inifile);
	//CONTROLLER_PORT = ini_getl("Controller", "port", 10, inifile);
	//INIFILE;

	//Config  -- load from file
	int MAX_SWITCHES     = ini_getl("Simulation", "max_switches", 100, INIFILE);
	int MIN_SWITCHES     = ini_getl("Simulation", "min_switches", 1, INIFILE);
	int RUNS_PER_CONFIG  = ini_getl("Simulation", "runs_per_config", 1, INIFILE); //not actually used
	int DELAY_CREATE     = ini_getl("Simulation", "delay_create", 6, INIFILE);
	int RUN_TIME         = ini_getl("Simulation", "run_time", 1, INIFILE);
	int STOP_DELAY_MULTIPLIER = ini_getl("Simulation", "stop_delay_multiplier", 1000, INIFILE);
	int DELTA_T          = ini_getl("Simulation", "delta_t", 1000000, INIFILE);
	int BETWEEN_PACKET_DELAY = ini_getl("Simulation", "between_packet_delay", 0, INIFILE);

	char OUTPUT_CSV[50];
	char OUTPUT_CSV_ALL[50];
	char OUTPUT_DELTA_SENT_CSV[50];
	char OUTPUT_DELTA_RECEIVED_CSV[50];
	char OUTPUT_DELTA_RTT_MEAN_CSV[50];
	n = ini_gets("Simulation", "output_csv", "test.csv", OUTPUT_CSV, sizeof(OUTPUT_CSV), INIFILE);
	//n = ini_gets("Simulation", "output_csv_all", "test_all.csv", OUTPUT_CSV_ALL, sizeof(OUTPUT_CSV_ALL), INIFILE);
	strcpy(OUTPUT_CSV_ALL, OUTPUT_CSV);
	strcpy(OUTPUT_DELTA_SENT_CSV, OUTPUT_CSV);
	strcpy(OUTPUT_DELTA_RECEIVED_CSV, OUTPUT_CSV);
	strcpy(OUTPUT_DELTA_RTT_MEAN_CSV, OUTPUT_CSV);

	strcat(OUTPUT_CSV,".csv");
	strcat(OUTPUT_CSV_ALL, "_all.csv");
	strcat(OUTPUT_DELTA_SENT_CSV, "_delta_sent.csv");
	strcat(OUTPUT_DELTA_RECEIVED_CSV, "_delta_received.csv");
	strcat(OUTPUT_DELTA_RTT_MEAN_CSV, "_delta_rtt_mean.csv");

	cout << "max_switches:          "<<MAX_SWITCHES <<endl;
	cout << "min_switches:          "<<MIN_SWITCHES <<endl;
	cout << "runs_per_config:       "<<RUNS_PER_CONFIG <<endl;
	cout << "delay_crate:           "<<DELAY_CREATE <<endl;
	cout << "run_time:              "<<RUN_TIME <<endl;
	cout << "stop_delay_multiplier: "<<STOP_DELAY_MULTIPLIER <<endl;
	cout << "output_csv:            "<<OUTPUT_CSV <<endl;
	cout << "output_csv_all:        "<<OUTPUT_CSV_ALL <<endl;

	//Output files
	fstream csv_stream(OUTPUT_CSV, ios::out | ios::trunc);
	csv_stream.close();
	csv_stream.open(OUTPUT_CSV, ios::in | ios::out);
	//csv_stream << "Count_SW;P_rcv;RTTmean;P/S;Ca;Delay"<< endl;  //2012.03.13 neue statistik
	csv_stream << "Count Switches; Packet Sent; Packet Received; RTT Mean; PPS Sent; PPS Received; RTT Ca; PPS Delta Sent Ca; PPS Delta Received Ca; Delay" << endl;

	fstream csv_stream_all(OUTPUT_CSV_ALL, ios::out | ios::trunc);
	csv_stream_all.close();
	csv_stream_all.open(OUTPUT_CSV_ALL, ios::in | ios::out);
	//csv_stream_all << "ID;P.rcv;RTTmean;P/s;VAR;Delay,Session"<< endl;
	csv_stream_all
			<<	"ID; Count Sent; Count Received; RTT Mean; PPS Sent; PPS Received; RTT VAR; RTT Ca; "
			<<	"PPS Delta Mean Sent; PPS Delta Mean Received; PPS Delta VAR Sent; PPS Delta VAR Received;PPS Delta Ca Sent; PPS Delta Ca Received;"
			<<	"Sleep; Session "
			<< endl;
	//deltas
	fstream csv_delta_sent(OUTPUT_DELTA_SENT_CSV, ios::out | ios::trunc);
	fstream csv_delta_received(OUTPUT_DELTA_RECEIVED_CSV, ios::out | ios::trunc);
	fstream csv_delta_rtt_mean(OUTPUT_DELTA_RTT_MEAN_CSV, ios::out | ios::trunc);
	csv_delta_sent.close();
	csv_delta_received.close();
	csv_delta_rtt_mean.close();

	csv_delta_sent.open(OUTPUT_DELTA_SENT_CSV, ios::in | ios::out);
	csv_delta_received.open(OUTPUT_DELTA_RECEIVED_CSV, ios::in | ios::out);
	csv_delta_rtt_mean.open(OUTPUT_DELTA_RTT_MEAN_CSV, ios::in | ios::out);

	csv_delta_sent
			<< "Session; Switch; Sent per Dt"
			<< endl;

	csv_delta_received
			<< "Session; Switch; Received per Dt"
			<< endl;

	csv_delta_rtt_mean
			<< "Session; Switch; Rtt mean per Dt"
			<< endl;





	//Init min switches
	for (i = 1;i < MIN_SWITCHES;i++){
		intCountSwitches++;
		switches[intCountSwitches] = new vswitch(intCountSwitches, send_packet_through_switch);
		switches[intCountSwitches]->config_switch.Delta_t = DELTA_T;
		//switches[intCountSwitches]->config_switch.intUSleepBatchprocess =  BETWEEN_PACKET_DELAY;
	}
	sleep(MIN_SWITCHES + 10);  //2012.06.02 (DELAY_CREATE * MIN_SWITCHES )


//	//================================0
//	//         PCAP
//	//================================0
//	mypcap mp("test.pcap", true);
//	int sz =0;
//	char bf[100000];
//	while (sz>-1){
//		mp.get_next_packet(bf, &sz);  //as packet in
//		if (sz>0){
//			switches[1]->send_packet_to_controller(bf,sz);
//			printf("PCAP size: %d\n", sz);
//			debug_client.packet(bf,sz,INFO);
//		}
//	}

	//Simulation, starting switches one after other
	for (i = MIN_SWITCHES;i <= MAX_SWITCHES;i++){   //switches
		//////////////////////Create new switch
		intCountSwitches++;
		switches[intCountSwitches] = new vswitch(intCountSwitches, send_packet_through_switch);
		switches[intCountSwitches]->config_switch.Delta_t = DELTA_T;
		//switches[intCountSwitches]->config_switch.intUSleepBatchprocess =  BETWEEN_PACKET_DELAY;
		sleep(DELAY_CREATE);
		/////////////
		//test switch 1
		//if (i>1)
		//   cout << "inter switch communication: " << switches[intCountSwitches]->all_switches[intCountSwitches-1]->config_switch.ID << endl;
		/////////////////////Start all switches
		for (j=1;j<=intCountSwitches;j++){
			switches[j]->config_switch.reset_statistic();
			switches[j]->debug_communication.message("N E W   S E S S I O N  !");
			switches[j]->debug_batch.message("N E W   S E S S I O N  !");
			switches[j]->start_();
		}
		sleep(RUN_TIME);
		/////////////////////Read Statistics

		long double packet_sent_sum_switches = 0;
		long double packet_rcv_sum_switches  = 0;
		long double rtt_mean_sum             = 0;
		long double pps_sent_sum             = 0;
		long double pps_received_sum         = 0;
		long double rtt_ca_sum               = 0;

		long double pps_delta_ca_sent_sum    = 0;
		long double pps_delta_ca_received_sum = 0;

		long double delay_sum                = 0;

		int intAvailableSwitches = 0;
		for (j=1;j<=intCountSwitches;j++){
			if (!switches[j]->config_switch.SwitchOutOfOrder()){
				intAvailableSwitches++;
				csv_stream_all
						<< switches[j]->config_switch.ID << ";"						//ID
						<< switches[j]->config_switch.PACKET_sent_count << ";"      //P.sent
						<< switches[j]->config_switch.PACKET_received_count << ";"  //P.rcv
						<< switches[j]->config_switch.RTT_mean()          << ";"	//RTT mean
						<< switches[j]->config_switch.PPS_sent() << ";"				//PPS
						<< switches[j]->config_switch.PPS_received() << ";"				//PPS
						<< switches[j]->config_switch.RTT_VAR() << ";"				//VAR
						<< switches[j]->config_switch.RTT_Ca() << ";"				//VAR

						<< switches[j]->config_switch.PPS_delta_mean_sent() << ";"
						<< switches[j]->config_switch.PPS_delta_mean_received() << ";"
						<< switches[j]->config_switch.PPS_delta_VAR_sent() << ";"
						<< switches[j]->config_switch.PPS_delta_VAR_received() << ";"
						<< switches[j]->config_switch.PPS_delta_Ca_sent() << ";"
						<< switches[j]->config_switch.PPS_delta_Ca_received() << ";"

						<< switches[j]->config_switch.intUSleepBatchprocess << ";"	//DELAY
						<< intCountSwitches						     				//Session
						<< endl;


				//--neue statistik
				packet_sent_sum_switches += switches[j]->config_switch.PACKET_sent_count;
				packet_rcv_sum_switches  += switches[j]->config_switch.PACKET_received_count;
				rtt_mean_sum             += switches[j]->config_switch.RTT_mean();
				pps_sent_sum             += switches[j]->config_switch.PPS_sent();
				pps_received_sum         += switches[j]->config_switch.PPS_received();
				rtt_ca_sum               += switches[j]->config_switch.RTT_Ca();

				pps_delta_ca_sent_sum     += switches[j]->config_switch.PPS_delta_Ca_sent();
				pps_delta_ca_received_sum += switches[j]->config_switch.PPS_delta_Ca_received();

				delay_sum                += switches[j]->config_switch.intUSleepBatchprocess;

			}

		}

		//write statistics
		csv_stream
				<< intAvailableSwitches   		   		<< ";"  //   intCountSwitches
				<< packet_sent_sum_switches        		<< ";"
				<< packet_rcv_sum_switches         		<< ";"
				<< rtt_mean_sum / intAvailableSwitches 	<< ";"  //   intCountSwitches
				<< pps_sent_sum					   		<< ";"
				<< pps_received_sum                		<< ";"
				<< rtt_ca_sum / intAvailableSwitches   	<< ";"  //   intCountSwitches
				<< pps_delta_ca_sent_sum  / intAvailableSwitches          << ";"  //   intCountSwitches
				<< pps_delta_ca_received_sum  / intAvailableSwitches      << ";"  //   intCountSwitches
				<< delay_sum /  intAvailableSwitches   //   intCountSwitches
				<< endl;

		//Rtt IN deltas
		printf("Delta size: %d \n",switches[1]->config_switch.vec_deltas.size());
		try{
			for (int k=1; k<=intCountSwitches; k++) {
				for (int l=0; l < switches[k]->config_switch.vec_deltas.size(); l++) {
					cout << "Vector_delta sent:" << switches[k]->config_switch.vec_deltas[l].packet_sent  << endl;
					cout << "Vector_delta received: " << switches[k]->config_switch.vec_deltas[l].packet_received << endl;
					cout << "Vector_delta received: " << switches[k]->config_switch.vec_deltas[l].rtt_mean << endl;
					//cout << switches[k]->config_switch.vec_RTT_deltas[l].mean;
				}
				cout << endl;
			}
		}catch(exception e){
			cout << "Exception!!!!" << endl;
		}

		//Count_SW;P_rcv;RTTmean;P/S;Ca;Delay
//		csv_stream															//2012.03.13 neuse statistik
//		        <<  intCountSwitches				<< ";"
//				<<	pck_rcv_sum / intCountSwitches  << ";"
//				<<	rtt_mean_sum / intCountSwitches << ";"
//				<<	pps_mean_sum / intCountSwitches << ";"
//				<<	ca_sum / intCountSwitches       << ";"
//				<<	delay_sum / intCountSwitches    << endl;
		/////////////////////Stop all switches
		for (j=1;j<=intCountSwitches;j++){
			switches[j]->stop_();
		}

		//wait until session over (timeout of no packet transmission)

		bool wait = true;
		while (wait){
			// check the switches for session over
			cout << "Wait for all switches to end session... " << endl;
			sleep(1); //sleep one second
			wait = false;
			for (j=1;j<=intCountSwitches;j++){   //all switches timeouted from the session
				wait = wait || (!switches[j]->config_switch.SessionTimeouted());
			}
		}

		//sleep(RUN_TIME * intCountSwitches * STOP_DELAY_MULTIPLIER / 1000);   // the old wait

		//deltas after session  end

		//deltas
		for (j=1;j<=intCountSwitches;j++){
			csv_delta_sent  	<< intCountSwitches		<< ";"				    //Session
								<< switches[j]->config_switch.ID << ";";		//ID
			csv_delta_received	<< intCountSwitches		<< ";"				    //Session
								<< switches[j]->config_switch.ID << ";";		//ID
			csv_delta_rtt_mean	<< intCountSwitches		<< ";"				    //Session
								<< switches[j]->config_switch.ID << ";";		//ID

			for (int l=0; l < switches[j]->config_switch.vec_deltas.size(); l++) {
				csv_delta_sent  <<   switches[j]->config_switch.vec_deltas[l].packet_sent 			<< ";";
				csv_delta_received 	<<  switches[j]->config_switch.vec_deltas[l].packet_received	<< ";";
				csv_delta_rtt_mean 	<<   switches[j]->config_switch.vec_deltas[l].rtt_mean			<< ";";

			}
			csv_delta_sent << endl;
			csv_delta_received << endl;
			csv_delta_rtt_mean << endl;
		}


	}

	csv_stream.close();
	csv_stream.clear();
	csv_stream_all.close();
	csv_stream_all.clear();

	csv_delta_sent.close();
	csv_delta_sent.clear();
	csv_delta_received.close();
	csv_delta_received.clear();
	csv_delta_rtt_mean.close();
	csv_delta_rtt_mean.clear();


//===================================MISSION CONTROL BEGIN===========================================
    //=================================================================================================
   //waiting for commands  from the mission control=======================================

//	while(1){
//	    //get CreateNewSwitch command
//		int size = 0;
//		size = recv(ClientMCSocket, buf, sizeof(message_header),0);
//		debug_client.whisper("-CL- Command received");
//		message_header *MessageHeader;
//		MessageHeader = (message_header*) buf;
//		if (size == 0) {
//			//Connection lost
//			debug_client.error_message("Connection_Lost, Exiting..");
//			break;
//		}
//		if (size != sizeof(message_header)){
//			buf[size] = 0;
//			sprintf(strTmp2,"-CL- GetCommand: This is not a header! length: %d", size);
//			debug_client.error_message(strTmp2);
//			//connection close!!!
//
//		} else {
//			sprintf(strTmp2,"-CL-  header from the mission control, size: %d\n", size);
//			debug_client.whisper(strTmp2);
//
//			//Version
//			if (MessageHeader->version != COM_VERSION){
//				sprintf(strTmp2, "-CL-  Wrong version: %d (actual version:%d)",MessageHeader->version, COM_VERSION );
//				debug_client.error_message(strTmp2);
//
//				//connection close,
//			}else {
//				sprintf(strTmp2, "-CL- Version OK: %d",MessageHeader->version);
//				debug_client.gossip(strTmp2);
//			}
//		}
//
//      //Received commands from the Mission control
//		if (MessageHeader->command == CREATE_NEW_SWITCH){
//			debug_client.message("Creating neue VSwitch");
//			//vswitch tmpSwitch(intCountSwitches);
//			//switches[intCountSwitches++] = &tmpSwitch;
//			intCountSwitches++;
//			switches[intCountSwitches] = new vswitch(intCountSwitches);   //new switch
//			sprintf(strTmp2, "Switch %d created", intCountSwitches);
//			debug_client.gossip(strTmp2);
//		} else if (MessageHeader->command == QUIT_){
//			debug_client.message("Client received QUIT");
//			//for (i=1;i<=intCountSwitches; i++ ){
//				//switches[i]->RunBatch = false;
//				//switches[i]->RunControllerCommunication = false;
//				//switches[i]->RunMissionControl   = false;
//				//debug_client.whisper("Batch, Com and MC stopped");
//				//break;
//			//}
//			break;
//		} else if (MessageHeader->command == STOP){
//			int intSwitchNr = MessageHeader->param;
//			sprintf(strTmp2, "Client received STOP for %d", intSwitchNr );
//			debug_client.message(strTmp2);
//			switches[intSwitchNr]->RunBatch = false;
//			switches[intSwitchNr]->RunControllerCommunication = false;
//			switches[intSwitchNr]->RunMissionControl = false;
//		} else {
//			debug_client.message("Command not recognized!, client S T O P S ");
//			break;
//		}
//
//	}


//=====================================MISSION CONTROL END====================================

	//Join VSwitch-Threads
	for (i=1; i<=intCountSwitches; i++){
		sprintf(strTmp2, "Joining Switch: %d", i);
		debug_client.gossip(strTmp2);
		switches[i]->join_start_thread();
	}

	//Free VSwitchees
//	for (i=0; i<intCountSwitches; i++){
//		sprintf(strTmp2, "Freeing Switch: %d", i+1);
//		debug_client.gossip(strTmp2);
//		delete switches[i];
//	}

//   vswitch my_first_switch(1);
//   vswitch my_second_switch(2);
//   vswitch my_third_switch(3);
//   vswitch my_fourth_switch(4);
//   vswitch my_fifth_switch(5);
//   vswitch sixth(6);
//   vswitch seventh(7);
//   vswitch eight(8);
//   vswitch nine(9);
//   vswitch ten(10);
//   vswitch eleven(11);
//   vswitch twelve(12);
//   vswitch thirteen(13);
//   vswitch foutreen(14);
//   vswitch fifteen(15);
//   vswitch sixteen(16);



//   my_first_switch.join_start_thread();
//   my_second_switch.join_start_thread();
//   my_third_switch.join_start_thread();
//   my_fourth_switch.join_start_thread();
//   my_fifth_switch.join_start_thread();
//   sixth.join_start_thread();
//   seventh.join_start_thread();
//   eight.join_start_thread();
//   nine.join_start_thread();
//   ten.join_start_thread();
//   eleven.join_start_thread();
//   twelve.join_start_thread();
//   thirteen.join_start_thread();
//   foutreen.join_start_thread();
//   fifteen.join_start_thread();
//   sixteen.join_start_thread();

   debug_client.message("E N D   C L I E N T");
   return 0;

}





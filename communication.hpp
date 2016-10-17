/*
 * communication.h
 *
 *  Created on: Nov 23, 2011
 *      Author: Zsolt Magyari
 */
//
#ifndef communication_h
#define communication_h

//#define MC_MAXBUFSIZE 1024*1024   //1 MB
//#define MC_MAXREADBUFSIZE 1024*100
//#define MC_MAXWRITEBUFSIZE 1024*100


#define PASSWORD "PASS"
#define COM_VERSION 11


enum command{
	//Handshake

	LOGIN_REQUEST					=    1,
	LOGIN_OK						=    2,
	LOGIN_CLIENT_REQUEST            =    3,
	QUIT_                           =    4,

	HELLO							=    5,

	GET 							=   11,
	SET	     						=   12,

	CREATE_NEW_SWITCH               =   21,

	START							=   60,
	STOP							=   61,
	RESTART							=   62,

	GET_FILE						=   80,
	SEND_FILE						=   81
};

enum param{
	ID 				      =   1,
	SESSION_ID            =   2,

	CONTROLLER_IP	      =  11,
	CONTROLLER_PORT		  =  12,

	PACKET_RECEIVED       =  51,
	PACKET_SENT			  =  52,
	PACKET_MEANTIME       =  53,
	PACKET_P_SECOND       =  54,
	PACKET_VAR            =  55,

	USLEEP_BATCHPROCESS   =  61
};

enum error{
	NO_ERROR_OK           =   100,

	COMMAND_NOT_RECOGNIZED =  104,
	COMMAND_NOT_READY      =  105,

	PARAMETER_NOT_RECOGNIZED = 114,
	PARAMETER_WRONG          = 115

};


struct message_header{
	short version;
	short command;
	short param;
	short error;
	int length;
};

//bool login_to_mission_control_(vswitch *VS);
//void get_command(int socket);
//void command_picker(int socket, int command, int length, char *buf);
//void login(int socket, int command, int length, char *buf);
//void set_id(int socket, int command, int length, char *buf);
//void get_id(int socket, int command, int length, char *buf);
//void set_config(int socket, int command, int length, char *buf);
//void get_config(int socket, int command, int length, char *buf);
//void get_file(int socket, int command, int length, char *buf);
//void put_file(int socket, int command, int length, char *buf);
//void get_ip(int socket, int command, int length, char *buf);
//void get_mac(int socket, int command, int length, char *buf);
//void stop(int socket, int command, int length, char *buf);
//void start(int socket, int command, int length, char *buf);
//void restart(int socket, int command, int length, char *buf);
//void hello(int socket, int command, int length, char *buf);

//bool login_to_mission_control_(vswitch *VS)
//{
//	char strTmp[100];
//	char buf[MC_MAXREADBUFSIZE];
//	int size = 0;
//	size = recv(VS->sockMissionControl, buf, MC_MAXREADBUFSIZE-1,0);
//	buf[size] = 0;
//	sprintf(strTmp, "Welcome message from the Server: %s", buf);
//	VS->debug_mission.gossip(strTmp);
//
//	message_header *MessageHeader;
//	char bufwrite[MC_MAXWRITEBUFSIZE];
//	MessageHeader = (message_header*) bufwrite;
//	MessageHeader->version = htons(COM_VERSION);//htons(COM_VERSION);
//	MessageHeader->command = htons(LOGIN_REQUEST);
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
//	sprintf(strTmp,"Sending login request...");
//	VS->debug_mission.gossip(strTmp);
//
//	intSendLength = send(VS->sockMissionControl, bufwrite, sizeof(message_header)+sizeof(PASSWORD),0);
//
//	sprintf(strTmp,"Login request sent, size: %d", intSendLength);
//	VS->debug_mission.whisper(strTmp);
//	sprintf(strTmp,"Reading Answer...");
//	VS->debug_mission.whisper(strTmp);
//
//	size = recv(VS->sockMissionControl, buf, sizeof(message_header),0);
//
//	sprintf(strTmp,"Answer red, size: %d", size);
//	VS->debug_mission.whisper(strTmp);
//
//	MessageHeader = (message_header*) buf;
//
//	sprintf(strTmp,"Version   :%d", MessageHeader->version);
//	VS->debug_mission.whisper(strTmp);
//	sprintf(strTmp,"Command:   %d", MessageHeader->command);
//	VS->debug_mission.whisper(strTmp);
//	sprintf(strTmp,"Parameter: %d", MessageHeader->param);
//	VS->debug_mission.whisper(strTmp);
//	sprintf(strTmp,"Error:     %d", MessageHeader->error);
//	VS->debug_mission.whisper(strTmp);
//	sprintf(strTmp,"Size:      %d", MessageHeader->length);
//	VS->debug_mission.whisper(strTmp);
//
//	bool boCheckLogin = true;
//	//packet size
//	if (boCheckLogin){
//		if (size <= 0) {
//			sprintf(strTmp,"Packet Size Problem!");
//			VS->debug_mission.whisper(strTmp);
//
//			boCheckLogin = false;
//		}else{
//			sprintf(strTmp,"Packet Size ok!");
//			VS->debug_mission.whisper(strTmp);
//		}
//	}
//	//command
//	if (boCheckLogin){
//		if (MessageHeader->command != LOGIN_OK) {
//			sprintf(strTmp,"command do not LOGIN_OK!");
//			VS->debug_mission.whisper(strTmp);
//			boCheckLogin = false;
//		}else{
//			sprintf(strTmp,"command LOGIN_OK!");
//			VS->debug_mission.whisper(strTmp);
//		}
//	}
//	//password
//	char PasswordReceived[MessageHeader->length];
//	size = recv(VS->sockMissionControl, &PasswordReceived, MessageHeader->length,0);
//	if (boCheckLogin){
//		if (strcmp((char*)PasswordReceived, (char*) &PASSWORD)!=0){
//			sprintf(strTmp,"password do not much: rcv:%s sent:%s!",&PasswordReceived, &PASSWORD);
//			VS->debug_mission.whisper(strTmp);
//
//			boCheckLogin = false;
//		}else{
//			sprintf(strTmp,"password ok: %s", &PasswordReceived);
//			VS->debug_mission.whisper(strTmp);
//		}
//	}
//	if (boCheckLogin){
//		sprintf(strTmp,"Auth Successfull! ");
//		VS->debug_mission.gossip(strTmp);
//	}
//	return boCheckLogin;
////	if (size > 0) {
////		MessageHeader = (message_header*) buf;
////		printf("Ver:%d\n", MessageHeader->version);
////		printf("Command:%d\n", MessageHeader->command);
////		printf("Param: %d\n", MessageHeader->param);
////		printf("Error: %d\n", MessageHeader->error);
////		printf("Size: %d\n", MessageHeader->length);
////		if ((MessageHeader->command == LOGIN_OK) && (MessageHeader->length == sizeof(PASSWORD))){
////			printf("login ok, size password\n");
////			password = buf + sizeof(message_header);
////			if (strcmp((char*)password, (char*) &PASSWORD)==0){
////				printf("PasswordOK \n");
////				return true;
////			} else {
////				printf("Password not ok \n");
////				return false;
////			}
////		} else {
////
////		}
////	}else{
////		printf("password or logink not ok\n");
////		return false;
////	}
//}


#endif







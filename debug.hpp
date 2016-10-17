/*
 * debug.h
 *
 *  Created on: Oct 22, 2011
 *      Author: Zsolt Magyari
 */

#ifndef debug_hpp
#define debug_hpp

#include <strings.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

enum io { IN , OUT , CREATE, INFO};  //Packet Way
enum levels {                        //Debug Levels
	CRYTICAL_ERROOR_MESSAGE,
	ERROR_MESSAGE,
	EXCEPTION,
	MESSAGE,
	GOSSIP,
	WHISPER,
	ALL
};

using namespace std;

class debug {
private:
	int debuglevel;
	bool DEBUG_TO_STDOUT;
	bool DEBUG_TO_STDERR;
	bool DEBUG_TO_LOGFILE;
	bool DEBUG_TO_HTMLFILE;
	bool DEBUG_TO_INPUT_BINARY;
	bool DEBUG_TO_OUTPUT_BINARY;
	FILE* filLog;
	FILE* filHTML;        //not used yet
	int  fhStreamIn;
	int  fhStreamOut;
	char strLogfile[255];
	char strHTMLfile[255];

private:
	struct timeval tvTimestamp;

public:
    debug();
    ~debug();

	void set_debuglevel(int level);   //debug info redirection
	void set_debug_to_stdout();
	void set_debug_to_stderr();
	void set_debug_to_logfile(char *filename);
	void set_debug_to_streamfile_in(char *filename);
	void set_debug_to_streamfile_out(char* filename);
	void set_debug_to_htmlfile(char *filename);

	void timestamp_str(char *strTimestamp);
	long double timestamp(long double *ldblPaTimestamp);

	void crytical_error_message(char *message);   //debug levels
	void error_message(char *message);
	void exception(char *exception); //exception object!
	void message(char *message);
	void gossip(char *message);
	void whisper(char *message);
	void all(char *message);

	void packet(char* pack , int size, io xput);
	void stream(char* stri , int size, io xput);
};

debug::debug()
{
	DEBUG_TO_STDOUT        = false;
	DEBUG_TO_STDERR        = false;
	DEBUG_TO_LOGFILE       = false;
	DEBUG_TO_HTMLFILE      = false;
	DEBUG_TO_INPUT_BINARY  = false;
	DEBUG_TO_OUTPUT_BINARY = false;

	filLog = NULL;
}

debug::~debug()
{

	if (DEBUG_TO_LOGFILE){  //(filLog != NULL)
		fflush(filLog);
		fclose(filLog);
	}

	if (DEBUG_TO_HTMLFILE){  //(filHTML != NULL)
		fflush(filHTML);
		fclose(filHTML);
	}

	if (fhStreamIn > 0){        //test of desrtuctor - muss noch überptüft werden
		close(fhStreamIn);
	}


	if (fhStreamOut > 0){
		close(fhStreamOut);
	}



}


/**
 * Creates timestamp string
 */
void debug::timestamp_str(char *strTimestamp)
{
	gettimeofday(&tvTimestamp, NULL);

	struct tm *tmTime;
	tmTime = localtime(&tvTimestamp.tv_sec);

	//char strTimestamp[100];
	strftime(strTimestamp, 100, "[%Y-%m-%d %H:%M:%S.",tmTime);

	//milliseconds (zero fill)
	char strTimestamp_m[4];
	sprintf(strTimestamp_m, "%03d", tvTimestamp.tv_usec / 1000);

	//microseconds (zero fill)
	char strTimestamp_u[4];
    sprintf(strTimestamp_u, "%03d", tvTimestamp.tv_usec % 1000);

    strcat(strTimestamp, strTimestamp_m);
    strcat(strTimestamp, ",");
    strcat(strTimestamp, strTimestamp_u);
    strcat(strTimestamp, "]");

    //printf("%s\n", strTimestamp);
}

/**
 * Returns time stamp (10 Bytes), both as parameter and value  (usec)
 */
long double debug::timestamp(long double *ldblPaTimestamp)
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

/**
 * Sets debuglevel
 */
void debug::set_debuglevel(int level)
{
	debuglevel = level;
}

/**
 * Debug redirection to standard output
 */
void debug::set_debug_to_stdout()
{
	DEBUG_TO_STDOUT = true;
}

/**
 * Debug redirection to standard error
 */
void debug::set_debug_to_stderr()
{
	DEBUG_TO_STDERR = true;
}


/**
 * Debug redirection to log file
 */
void debug::set_debug_to_logfile(char *strPaFilename)
{
	DEBUG_TO_LOGFILE = true;
	filLog = fopen(strPaFilename, "w");   //"a+" "w"
	//printf("file descriptor of %s: %d\n",strPaFilename, &filLog );
	if (filLog == NULL){
		fprintf(stderr, "Could not open the log file: %s", strPaFilename);
		DEBUG_TO_LOGFILE = false;
	}
}

/**
 * Debug redirection to HTML file  - HTML tockens are missing
 */
void debug::set_debug_to_htmlfile(char *strPaFilename)
{
	DEBUG_TO_HTMLFILE = true;
	filHTML = fopen(strPaFilename, "w");  //"a+" "w"

	if (filHTML == NULL){
		fprintf(stderr, "Could not open the HTML file: %s", strPaFilename);
		DEBUG_TO_HTMLFILE = false;
	}

}

/**
 * Set Up Stream File
 */
void debug::set_debug_to_streamfile_in(char *filename)
{
	if ((fhStreamIn =open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0777)) == -1){
		fprintf(stderr,"Could not open %s for stream_in log", filename);
		DEBUG_TO_INPUT_BINARY = false;
	}else{
		DEBUG_TO_INPUT_BINARY = true;
	}
}

/**
 * Set Up Stream File
 */
void debug::set_debug_to_streamfile_out(char* filename)
{
	if ((fhStreamOut =open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0777)) == -1){
		fprintf(stderr,"Could not open %s for stream_in log", filename);
		DEBUG_TO_OUTPUT_BINARY = false;
	}else{
		DEBUG_TO_OUTPUT_BINARY = true;
	}

}


void debug::crytical_error_message(char *message)
{
	char strTimestamp[50];
	timestamp_str(strTimestamp);

	if (DEBUG_TO_STDOUT){
		fprintf(stdout,"%s <CRYTICAL ERROR>: %s\n",strTimestamp,message);
	}
	if (DEBUG_TO_STDERR){
		fprintf(stderr,"%s <CRYTICAL ERROR>: %s\n",strTimestamp,message);
	}
	if (DEBUG_TO_LOGFILE){
		fprintf(filLog,"%s <CRYTICAL ERROR>: %s\n",strTimestamp,message);
		fflush(filLog);
	}
	if (DEBUG_TO_HTMLFILE){
		fprintf(filHTML,"%s <CRYTICAL ERROR>: %s\n",strTimestamp,message);
		fflush(filHTML);
	}
}


void debug::error_message(char *message)
{
	if (debuglevel >= ERROR_MESSAGE){
		char strTimestamp[50];
		timestamp_str(strTimestamp);

		if (DEBUG_TO_STDOUT){
			fprintf(stdout,"%s   <ERROR>: %s\n",strTimestamp,message);
		}
		if (DEBUG_TO_STDERR){
			fprintf(stderr,"%s   <ERROR>: %s\n",strTimestamp,message);
		}
		if (DEBUG_TO_LOGFILE){
			fprintf(filLog,"%s   <ERROR>: %s\n",strTimestamp,message);
			fflush(filLog);
		}
		if (DEBUG_TO_HTMLFILE){
			fprintf(filHTML,"%s   <ERROR>: %s\n",strTimestamp,message);
		}
	}

}


void debug::exception(char *message)
{
	if (debuglevel >= EXCEPTION){
		char strTimestamp[50];
		timestamp_str(strTimestamp);

		if (DEBUG_TO_STDOUT){
			fprintf(stdout,"%s     <EXCEPTION>: %s\n",strTimestamp,message);
		}
		if (DEBUG_TO_STDERR){
			fprintf(stderr,"%s     <EXCEPTION>: %s\n",strTimestamp,message);
		}
		if (DEBUG_TO_LOGFILE){
			fprintf(filLog,"%s     <EXCEPTION>: %s\n",strTimestamp,message);
			fflush(filLog);
		}
		if (DEBUG_TO_HTMLFILE){
			fprintf(filHTML,"%s     <EXCEPTION>: %s\n",strTimestamp,message);
		}
	}
}

void debug::message(char *message)
{
	if (debuglevel >= MESSAGE){
		char strTimestamp[50];
		timestamp_str(strTimestamp);

		if (DEBUG_TO_STDOUT){
			fprintf(stdout,"%s       <MESSAGE>: %s\n",strTimestamp,message);
		}
		if (DEBUG_TO_LOGFILE){
			fprintf(filLog,"%s       <MESSAGE>: %s\n",strTimestamp,message);
			fflush(filLog);
		}
		if (DEBUG_TO_HTMLFILE){
			fprintf(filHTML,"%s       <MESSAGE>: %s\n",strTimestamp,message);
		}
	}
}

void debug::gossip(char *message)
{
	if (debuglevel >= GOSSIP){
		char strTimestamp[50];
		timestamp_str(strTimestamp);

		if (DEBUG_TO_STDOUT){
			fprintf(stdout,"%s         <GOSSIP>: %s\n",strTimestamp,message);
		}
		if (DEBUG_TO_LOGFILE){
			fprintf(filLog,"%s         <GOSSIP>: %s\n",strTimestamp,message);
			fflush(filLog);
		}
		if (DEBUG_TO_HTMLFILE){
			fprintf(filHTML,"%s         <GOSSIP>: %s\n",strTimestamp,message);
		}
	}

}


void debug::whisper(char *message)
{
	if (debuglevel >= WHISPER){
		char strTimestamp[50];
		timestamp_str(strTimestamp);

		if (DEBUG_TO_STDOUT){
			fprintf(stdout,"%s           <WHISPER>: %s\n",strTimestamp,message);
		}
		if (DEBUG_TO_LOGFILE){
			fprintf(filLog,"%s           <WHISPER>: %s\n",strTimestamp,message);
			fflush(filLog);
		}
		if (DEBUG_TO_HTMLFILE){
			fprintf(filHTML,"%s           <WHISPER>: %s\n",strTimestamp,message);
		}
	}
}


void debug::all(char *message)
{
	if (debuglevel >= ALL){
		char strTimestamp[50];
		timestamp_str(strTimestamp);

		if (DEBUG_TO_STDOUT){
			fprintf(stdout,"%s             <ALL>: %s\n",strTimestamp,message);
		}
		if (DEBUG_TO_LOGFILE){
			fprintf(filLog,"%s             <ALL>: %s\n",strTimestamp,message);
			fflush(filLog);
		}
		if (DEBUG_TO_HTMLFILE){
			fprintf(filHTML,"%s             <ALL>: %s\n",strTimestamp,message);
		}

	}


}

/**
 * Packet to String
 */
void debug::packet(char *pack, int size, io xput)
{
	if (debuglevel >= ALL){
		char strTimestamp[50];
		timestamp_str(strTimestamp);

		char chrPacketWay[20];
		if (xput == IN){
			strcpy(chrPacketWay, "(IN)<--<PACKET>");
		}else if (xput == OUT){
			strcpy(chrPacketWay, "<PACKET>-->(OUT)");
		}else if (xput == CREATE){
			strcpy(chrPacketWay, "<<PACKET><CREATE>>");
		}else if (xput == INFO){
			strcpy(chrPacketWay, "[<PACKET><info>]");
		}

		if (DEBUG_TO_STDOUT){
			fprintf(stdout,"%s %s: ",strTimestamp, chrPacketWay);

			char *c = pack;
			int i = size;
			for(;i>0;i--){
				fprintf(stdout, "0x%02X, ",(unsigned char)*c++);
			}

			fprintf(stdout,"|\n");
		}
		if (DEBUG_TO_LOGFILE){
			fprintf(filLog,"%s %s: ",strTimestamp, chrPacketWay);

			char *c = pack;
			int i = size;
			for(;i>0;i--){
				fprintf(filLog, "0x%02X, ",(unsigned char)*c++);
			}

			fprintf(filLog,"|\n");
			fflush(filLog);
		}
		if (DEBUG_TO_HTMLFILE){
			fprintf(filHTML,"%s %s: ",strTimestamp, chrPacketWay);

			char *c = pack;
			int i = size;
			for(;i>0;i--){
				fprintf(filHTML, "0x%02X, ",(unsigned char)*c++);
			}

			fprintf(filHTML,"|\n");
		}
	}
}


/**
 * Write Packet into Binary File
 */
void debug::stream(char *stri, int size, io xput)
{

	if (DEBUG_TO_INPUT_BINARY && (xput == IN)){
		if ((write(fhStreamIn,stri,size))==-1){
			fprintf(stderr, "could not write to input_binary_log");
		}
	}

	if (DEBUG_TO_OUTPUT_BINARY && (xput == OUT)){
		if ((write(fhStreamOut,stri,size))==-1){
			fprintf(stderr, "could not write to output_binary_log");
		}
	}

//	char strTimestamp[50];
//	timestamp_str(strTimestamp);
//
//	if (DEBUG_TO_STDOUT){
//		fprintf(stdout,"%s <PACKET>: ",strTimestamp);
//
//		char *c = stri;
//		int i = size;
//		for(;i>0;i--){
//			fprintf(stdout, "0x%02X, ",(unsigned char)*c++);
//		}
//
//		fprintf(stdout,"\n");
//	}
//	if (DEBUG_TO_LOGFILE){
//		fprintf(filLog,"%s <PACKET>: ",strTimestamp);
//
//		char *c = stri;
//		int i = size;
//		for(;i>0;i--){
//			fprintf(filLog, "0x%02X, ",(unsigned char)*c++);
//		}
//
//		fprintf(filLog,"\n");
//	}
//	if (DEBUG_TO_HTMLFILE){
//		fprintf(filHTML,"%s <PACKET>: ",strTimestamp);
//
//		char *c = stri;
//		int i = size;
//		for(;i>0;i--){
//			fprintf(filHTML, "0x%02X, ",(unsigned char)*c++);
//		}
//
//		fprintf(filHTML,"\n");
//	}
}

#endif
















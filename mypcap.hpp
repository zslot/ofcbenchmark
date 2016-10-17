/*
 * mypcap.hpp
 *
 *  Created on: Apr 24, 2012
 *      Author: Zsolt Magyari
 */

#ifndef MYPCAP_HPP_
#define MYPCAP_HPP_

//header of pcap file
struct pcap_hdr {
    uint32_t magic_number;   /* magic number */
    uint16_t version_major;  /* major version number */
    uint16_t version_minor;  /* minor version number */
    int32_t thiszone;        /* GMT to local correction */
    uint32_t sigfigs;        /* accuracy of timestamps */
    uint32_t snaplen;        /* max length of captured packets */
    uint32_t network;        /* data link type */
};
BUILD_ASSERT_DECL(sizeof(struct pcap_hdr) == 24);

//pcap packet struct
struct pcaprec_hdr {
    uint32_t ts_sec;         /* timestamp seconds */
    uint32_t ts_usec;        /* timestamp microseconds */
    uint32_t incl_len;       /* number of octets of packet saved in file */
    uint32_t orig_len;       /* actual length of packet */
};
BUILD_ASSERT_DECL(sizeof(struct pcaprec_hdr) == 16);



class mypcap{
private:

	FILE *_file_;
	char _filename_[1000];
	bool _loop_file_;       //loop the content of the file, jumps to the first packet on eof if set

	FILE *pcap_open(const char *file_name);
	int pcap_read_header(FILE *file);
	int pcap_read(FILE *file, char *buf);

public:

	mypcap(const char *filename, bool loopfile);  //constructor
	~mypcap();
	int get_next_packet(char *ret_pack, int *ret_size);
	//bool have_next_packet();
};

/**
 * constructor
 */
mypcap::mypcap(const char *filename, bool loopfile)
{


	strcpy(_filename_,filename);
	_loop_file_ = loopfile;

	cout << "Filename: " << filename << endl;
	_file_ = pcap_open(filename);
	if (!_file_){
		cerr << "file couldn't open" << endl;
	}

}

/**
 * destructor
 */
mypcap::~mypcap()
{
//	pcap_close(handle);  //close the pcap file

}

/**
 * get the next packet (reset by eof if _loop_file_ is true)
 */
int mypcap::get_next_packet(char *ret_pack, int *ret_size)
{
	*ret_size = pcap_read(_file_, ret_pack);

	if ((*ret_size == EOF) && _loop_file_){
		//reopen file for loop
		fclose(_file_);
		_file_ = pcap_open(_filename_);
		if (!_file_){
			cerr << "file couldn't open for loop" << endl;
		}
		*ret_size = pcap_read(_file_, ret_pack);
	}

	//insert paket_in header -- encapsulation in OpenFlow PacketIn Packet

	if (*ret_size > -1){
		char packIn[9001]; //jumboready
		memset(packIn,0,sizeof(packIn));
		struct ofp_packet_in * pi = (ofp_packet_in*) packIn;

		memcpy( pi->data,ret_pack,*ret_size); //ofdp

		pi->header.version = OFP_VERSION;
		pi->header.type    = OFPT_PACKET_IN;
		pi->header.length  = htons(sizeof(ofp_packet_in)+ *ret_size);
		pi->header.xid     = htonl(0);
		pi->buffer_id      = htonl(0);
		pi->total_len      = htons(*ret_size);
		pi->in_port        = htons(0);
		pi->reason         = OFPR_NO_MATCH;

		*ret_size += sizeof(ofp_packet_in);
		memcpy(ret_pack, pi, *ret_size);


	}

}

FILE *mypcap::pcap_open(const char *file_name)
{
    FILE *file;

    file = fopen(file_name, "r");
    if (file == NULL) {
        cerr << file_name << ": failed to open pcap file for " << "reading" << endl;
        return NULL;
    }else{
    	cout << "file opened" << endl;
    }


    if (pcap_read_header(file)) {
    	//returns not 0
		cout << "close file" << endl;
		fclose(file);
		return NULL;
	}

    return file;
}

int mypcap::pcap_read_header(FILE *file)
{

	struct pcap_hdr ph;
    if (fread(&ph, sizeof ph, 1, file) != 1) {
        int error = ferror(file) ? errno : EOF;
        cout << "failed to read pcap header: " <<  error > 0 ? strerror(error) :  "end of file";
        return error;
    }

    	int *i = (int*) &ph;
    	cout << "*";
    	cout << hex << *i << dec;
    	cout << endl;


    if (ph.magic_number != 0xa1b2c3d4 && ph.magic_number != 0xd4c3b2a1) {
        cerr << "bad magic " << hex <<ph.magic_number <<dec;
        cerr << " reading pcap file " << "(expected 0xa1b2c3d4 or 0xd4c3b2a1)" << ph.magic_number << endl;
        return EPROTO;
    }
    return 0;
}

/**
 * reading from the file
 */
int mypcap::pcap_read(FILE *file, char *buf)
{
    int error = 0;
	struct pcaprec_hdr prh;
    //struct ofpbuf *buf;
    //void *data;
    size_t len;

    //*bufp = NULL;


    /* Read header. */
    if (fread(&prh, sizeof prh, 1, file) != 1) {
        error = ferror(file) ? errno : EOF;
        //cout << "failed to read pcap record header: " <<  error > 0 ? strerror(error) : "end of file";
        //return error;
    }

    /* Calculate length. */
    if (!error){
		len = prh.incl_len;
		if (len > 0xffff) {
			uint32_t swapped_len = (((len & 0xff000000) >> 24) |
									((len & 0x00ff0000) >>  8) |
									((len & 0x0000ff00) <<  8) |
									((len & 0x000000ff) << 24));
			if (swapped_len > 0xffff) {
				cerr << "bad packet length "<< len << " or "<< swapped_len << " " <<  "reading pcap file" << endl;
				error = EPROTO;
				//return EPROTO;
			}
			len = swapped_len;
		}
    }

    /* Read packet. */

    if (!error){
		if (fread(buf, len, 1, file) != 1) {
			error = ferror(file) ? errno : EOF;
			cerr << "failed to read pcap packet: " <<  error > 0 ? strerror(error) : "end of file" ;
			//ofpbuf_delete(buf);
			//return error;
		}
    }

    //*bufp = buf;
    if (!error) {
    	return len;
    }else{
    	return error;
    }
}





#endif /* MYPCAP_HPP_ */




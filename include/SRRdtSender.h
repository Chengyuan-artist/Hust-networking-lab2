#ifndef SR_RDT_SENDER_H
#define SR_RDT_SENDER_H


#include "Global.h"
#define MAXN 20


class SRRdtSender : public RdtSender {
private:
    int send_base;
    int next_seq;
    const int N; // winSize
    const int ByteLen; // seq byte len
    const int MOD;

    struct packet_buf {
        bool is_acked = false;
        bool is_occupied = false;
        Packet packet;
    };

    packet_buf send_buf[MAXN];

    void make_pkg(Packet &packet, int seq, const Message &msg);
    bool is_corrupt(const Packet &packet);

public:
    bool send(const Message &message);
    void receive(const Packet &ackPkt);						//接受确认Ack，将被NetworkService调用	
    void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkService调用
	bool getWaitingState();								//返回RdtSender是否处于等待状态，如果发送方正等待确认或者发送窗口已满，返回true
    SRRdtSender();
	~SRRdtSender();
};



#endif 
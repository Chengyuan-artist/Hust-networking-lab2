#ifndef SIMPLE_TCP_RECEIVER
#define SIMPLE_TCP_RECEIVER

#include "Global.h"
#include "RdtReceiver.h"

#define MAXN 20

class SimpleTcpReceiver : public RdtReceiver {
private:
    int expNum; // expected sequence num of packet

    const int N; // winSize
    const int ByteLen; // sequence byte length
    const int MOD;

    // the receive packet buffer
    struct packet_buf {
        bool is_occupied = false;
        Packet packet;
    }rev_buf[MAXN];


    bool is_corrupt(const Packet &packet);

    void extrat_msg(Message &msg, const Packet &pac);
    
    void send_ack(int ackNum);

public:

    void receive(const Packet &packet);
    SimpleTcpReceiver(/* args */);
    ~SimpleTcpReceiver();
};



#endif
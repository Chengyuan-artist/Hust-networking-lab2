#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H

#include "Global.h"
#include "RdtReceiver.h"

#define MAXN 20



class SRRdtReceiver : public RdtReceiver {
private:
    int expNum; // expected sequence num of packet

    const int N; // winSize
    const int ByteLen; // sequence byte length
    const int MOD;

    // the receive packet buffer
    struct packet_buf {
        bool is_occupied = false;
        Packet packet;
    };

    packet_buf rev_buf[MAXN];

    bool is_corrupt(const Packet &packet);

    void extrat_msg(Message &msg, const Packet &pac);
    
    void send_ack(int ackNum);
    
public:
    void receive(const Packet &packet) override;
    SRRdtReceiver();
    ~SRRdtReceiver() override;
};


#endif
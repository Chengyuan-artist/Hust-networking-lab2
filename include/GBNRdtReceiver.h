//
// Created by 张承元 on 2021/10/9.
//

#ifndef GBNRDTRECEIVER_H
#define GBNRDTRECEIVER_H

#include "RdtReceiver.h"
#include "GBNRdtSender.h"

class GBNRdtReceiver : public RdtReceiver{
private:
    int expectedSeqNum;
    int ByteLen;
    unique_ptr<Packet> sndPck; // maintain a up-to-date ack packet
    unique_ptr<Packet> make_ack(int ackNum);
public:
    void receive(const Packet &packet) override;

    GBNRdtReceiver();

    ~GBNRdtReceiver() override;
};


#endif //GBNRDTRECEIVER_H

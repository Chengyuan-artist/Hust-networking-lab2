//
// Created by 张承元 on 2021/10/9.
//

#ifndef STOPWAIT_GBNRDTSENDER_H
#define STOPWAIT_GBNRDTSENDER_H
#include "RdtSender.h"

struct GBNConfig{
    static const int WinSize = 4;
    static const int ByteLen = 3;
    static const int MAXN = 20;
};

class GBNRdtSender : public RdtSender{
private:
    int base;
    int nextSeqNum;
    const int WinSize;
    const int ByteLen; // Sequence encoding's byte length
    int onTimeSeq; // the sequence that is on timer

    Packet sndPck[GBNConfig::MAXN];

    Packet make_pkg(int seq, const Message &message);
public:
    bool send(const Message &message) override;

    void receive(const Packet &ackPkt) override;

    void timeoutHandler(int seqNum) override;

    bool getWaitingState() override;

    GBNRdtSender();

    virtual ~GBNRdtSender();
};


#endif //STOPWAIT_GBNRDTSENDER_H

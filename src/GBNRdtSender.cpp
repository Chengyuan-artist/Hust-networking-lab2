//
// Created by 张承元 on 2021/10/9.
//
#include "Global.h"
#include "GBNRdtSender.h"
#include <memory>

bool GBNRdtSender::send(const Message &message) {
    if (!getWaitingState()) {
        sndPck[nextSeqNum] = make_pkg(nextSeqNum, message);
        pns->sendToNetworkLayer(RECEIVER, sndPck[nextSeqNum]);

        if (base == nextSeqNum) {
            onTimeSeq = nextSeqNum; // store it for convenience of stopping timer
            pns->startTimer(SENDER, Configuration::TIME_OUT, onTimeSeq);
        }
        nextSeqNum = (nextSeqNum + 1) % (1 << ByteLen);
        cout << "GBNRdtSender::send: nextSeqNum = " << nextSeqNum << endl;

        return true;
    } else {
        return false;
    }
}

void GBNRdtSender::receive(const Packet &ackPkt) {
    int checkSum = pUtils->calculateCheckSum(ackPkt);

    if (checkSum == ackPkt.checksum) {
        base = (ackPkt.acknum + 1) % (1 << ByteLen);

        cout << "GBNRdtSender::receive: the ackPkt.acknum = " << ackPkt.acknum << endl;
        cout << NAME << "the window is sliding to ";
        cout << "(";
        int i;
        for (i = base; (i - base + MOD) % MOD < WinSize - 1; i = (i + 1) % MOD) {
            cout << i << " ";
        }
        cout << i << ")" << endl;

        if (base == nextSeqNum) {
            pns->stopTimer(SENDER, onTimeSeq);
        } else {
            // stop the old timer then start the new timer
            pns->stopTimer(SENDER, onTimeSeq);
            onTimeSeq = nextSeqNum;
            pns->startTimer(SENDER, Configuration::TIME_OUT, onTimeSeq);
        }
    }
    // else do nothing to ignore the corrupt packet

}

void GBNRdtSender::timeoutHandler(int seqNum) {
    // restart the timer
    pns->stopTimer(SENDER, onTimeSeq);
    pns->startTimer(SENDER, Configuration::TIME_OUT, onTimeSeq);
    // Go back N
    for (int i = base; i != nextSeqNum; ++i) {
        pns->sendToNetworkLayer(RECEIVER, sndPck[i]);
        i = i % (1 << ByteLen);
    }
}

bool GBNRdtSender::getWaitingState() {
    bool flag = nextSeqNum != (base + WinSize) % (1 << ByteLen);
    return !flag;
}

GBNRdtSender::GBNRdtSender() : base(1), nextSeqNum(1), WinSize(GBNConfig::WinSize), ByteLen(GBNConfig::ByteLen), onTimeSeq(1), MOD(1 << Configuration::BYTE_LEN) {

}

GBNRdtSender::~GBNRdtSender() {

}

Packet GBNRdtSender::make_pkg(int seq, const Message &message) {
    Packet packet = Packet(); 
    packet.acknum = -1; // ignore
    packet.seqnum = seq;
    packet.checksum = 0;
    memcpy(packet.payload, message.data, sizeof(message.data));
    packet.checksum = pUtils->calculateCheckSum(packet);

    return packet;
}

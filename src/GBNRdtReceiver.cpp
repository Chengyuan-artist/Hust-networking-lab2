//
// Created by zcy on 2021/10/9.
//
#include "Global.h"
#include "GBNRdtReceiver.h"


void GBNRdtReceiver::receive(const Packet &packet) {
    int checkSum = pUtils->calculateCheckSum(packet);
    if (checkSum == packet.checksum && packet.seqnum == expectedSeqNum) {
        // extract message
        unique_ptr<Message> msg(new Message);
        memcpy(msg->data, packet.payload, sizeof(packet.payload));
        pns->delivertoAppLayer(SENDER, *msg);

        cout << "GBNRdtReceiver::receive: rev from sender: packet.seqnum = " <<  packet.seqnum << endl;

        // make and send the ack
        sndPck = make_ack(expectedSeqNum);
        pns->sendToNetworkLayer(SENDER, *sndPck);

        expectedSeqNum = (expectedSeqNum + 1) % (1 << ByteLen);
    } else {
        // resend the last ack
        pns->sendToNetworkLayer(SENDER, *sndPck);
    }
}

GBNRdtReceiver::~GBNRdtReceiver() {

}

GBNRdtReceiver::GBNRdtReceiver() : expectedSeqNum(0), ByteLen(GBNConfig::ByteLen) {
    sndPck = make_ack(0);
    expectedSeqNum = 1;
}

unique_ptr<Packet> GBNRdtReceiver::make_ack(int ackNum) {
    unique_ptr<Packet> packet(new Packet);
    packet->acknum = ackNum;
    packet->seqnum = -1; // ignore
    packet->checksum = 0;
    for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
        packet->payload[i] = '.';
    }
    packet->checksum = pUtils->calculateCheckSum(*packet);
    return packet;
}

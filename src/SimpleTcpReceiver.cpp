#include "SimpleTcpReceiver.h"

SimpleTcpReceiver::SimpleTcpReceiver(): expNum(0), N(Configuration::WIN_SIZE), ByteLen(Configuration::BYTE_LEN), MOD(1 << Configuration::BYTE_LEN) {
    
}

SimpleTcpReceiver::~SimpleTcpReceiver() {

}


bool SimpleTcpReceiver::is_corrupt(const Packet &packet) {
    int check_sum = pUtils->calculateCheckSum(packet);
    return check_sum != packet.checksum;
}

void SimpleTcpReceiver::send_ack(int ackNum) {
    unique_ptr<Packet> packet(new Packet);
    packet->acknum = ackNum;
    packet->seqnum = -1; // ignore
    packet->checksum = 0;
    for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
        packet->payload[i] = '.';
    }
    packet->checksum = pUtils->calculateCheckSum(*packet);
    
    pns->sendToNetworkLayer(SENDER, *packet);
}

void SimpleTcpReceiver::extrat_msg(Message &msg, const Packet &pac) {
    memcpy(msg.data, pac.payload, sizeof(pac.payload));
}

void SimpleTcpReceiver::receive(const Packet &packet) {
    if (is_corrupt(packet)) {
        return;
    }

    if (packet.seqnum == expNum) {

        // 做延迟 取决于后面是否还有buffer的数据
        rev_buf[expNum].is_occupied = true;
        rev_buf[expNum].packet = packet;

        while (rev_buf[expNum].is_occupied) {
            // deliver the message
            Message msg;
            extrat_msg(msg, rev_buf[expNum].packet);
            pns->delivertoAppLayer(RECEIVER, msg);

            rev_buf[expNum].is_occupied = false;

            expNum = (expNum + 1) % MOD;
        }

        send_ack(expNum);
               
        return;
    }

    // packet.seqnum != expNum gap detected
    if ((packet.seqnum - expNum + MOD) % MOD < N) {
        // packet fall in the rev window
        send_ack(expNum);

        // if it has not been bufferd, buffer it
        if (!rev_buf[packet.seqnum].is_occupied) {
            rev_buf[packet.seqnum].is_occupied = true;
            rev_buf[packet.seqnum].packet = packet;
        }
    } else {
        // packet don't fall in the window, which means it's the old packet in front of the window
        // resend the ack
        send_ack(expNum);
    }
    
}


#include "SimpleTcpSender.h"

void SimpleTcpSender::make_pkg(Packet &packet, int seq, const Message &msg) {
    packet.acknum = -1; // ignore
    packet.seqnum = seq;
    packet.checksum = 0;
    memcpy(packet.payload, msg.data, sizeof(msg.data));
    packet.checksum = pUtils->calculateCheckSum(packet);
}

bool SimpleTcpSender::is_corrupt(const Packet &packet) {
    int check_sum = pUtils->calculateCheckSum(packet);
    return check_sum != packet.checksum;
}



bool SimpleTcpSender::send(const Message &message) {
    if (getWaitingState() == false) {
        
        make_pkg(send_buf[next_seq].packet, next_seq, message);

        pns->sendToNetworkLayer(RECEIVER, send_buf[next_seq].packet);

        if (send_base == next_seq) { //start the timer
            // it means that the whole pipe line is starting a newly running
            pns->startTimer(SENDER, Configuration::TIME_OUT, send_base);
        }

        next_seq = (next_seq + 1) % MOD;
        return true;
    } else {
        return false;
    }
}

bool SimpleTcpSender::getWaitingState() {
    if ((next_seq - send_base + MOD) % MOD < N) {
        // available
        return false;
    } else {
        return true;
    }
}

void SimpleTcpSender::receive(const Packet &ackPkt) {
    if (!is_corrupt(ackPkt)) {
        // ackPkt.acknum means the seq num that the receiver wants
        int diff = (ackPkt.acknum - send_base + MOD) % MOD;
        if (diff <= N && diff > 0) { // ackPkt.acknum > send_base while falling in the window
            // stop the older timer
            pns->stopTimer(SENDER, send_base);

            send_base = ackPkt.acknum;

            cout << NAME << "the window is sliding to ";
            cout << "(";
            int i;
            for (i = send_base; (i - send_base + MOD) % MOD < N - 1; i = (i + 1) % MOD) {
                cout << i << " ";
            }
            cout << i << ")" << endl;
            

            if (next_seq != send_base) { // there are currently any not-yet-acked segments
                pns->startTimer(SENDER, Configuration::TIME_OUT, send_base);
            }
        } else {
            
            if (ack_buf.ack_num == ackPkt.acknum) {
                ack_buf.count ++;
            } else {
                ack_buf.ack_num = ackPkt.acknum;
                ack_buf.count = 1;
            }

            if (ack_buf.count == 3) {
                // Fast retransmit
                pns->sendToNetworkLayer(RECEIVER, send_buf[ack_buf.ack_num].packet);
                
                cout << NAME << "Fast restransmit " << ack_buf.ack_num << endl;

                ack_buf.count = 0;
                ack_buf.ack_num = -1;
            }
        
            // cout << "SimpleTcpSender::receive:" <<
            //         "Strange thing happens" << endl;
            
        }
    } // else do nothing
}

void SimpleTcpSender::timeoutHandler(int seqNum) {
    pns->stopTimer(RECEIVER, seqNum);

    pns->sendToNetworkLayer(RECEIVER, send_buf[seqNum].packet);

    pns->startTimer(SENDER, Configuration::TIME_OUT ,seqNum);
}


SimpleTcpSender::SimpleTcpSender(): send_base(0), next_seq(0), N(Configuration::WIN_SIZE), ByteLen(Configuration::BYTE_LEN), MOD(1 << Configuration::BYTE_LEN) {}


SimpleTcpSender::~SimpleTcpSender() {}
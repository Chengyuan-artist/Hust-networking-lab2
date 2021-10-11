#include "SRRdtSender.h"


SRRdtSender::~SRRdtSender() {}

void SRRdtSender::make_pkg(Packet &packet, int seq, const Message &msg) {
    packet.acknum = -1; // ignore
    packet.seqnum = seq;
    packet.checksum = 0;
    memcpy(packet.payload, msg.data, sizeof(msg.data));
    packet.checksum = pUtils->calculateCheckSum(packet);
}


bool SRRdtSender::is_corrupt(const Packet &packet) {
    int check_sum = pUtils->calculateCheckSum(packet);
    return check_sum != packet.checksum;
}

bool SRRdtSender::send(const Message &message) {
    if (getWaitingState() == false) { // has available space in window
        send_buf[next_seq].is_occupied = true;
        send_buf[next_seq].is_acked = false;
        make_pkg(send_buf[next_seq].packet, next_seq, message);

        pns->sendToNetworkLayer(RECEIVER, send_buf[next_seq].packet);
        pns->startTimer(SENDER, Configuration::TIME_OUT, next_seq);

        next_seq = (next_seq + 1) % MOD;
        return true;
    } else { // has not available space in window
        return false;
    }
}


void SRRdtSender::receive(const Packet &ackPkt) {
    if (!is_corrupt(ackPkt)) {
        if ((ackPkt.acknum - send_base + MOD) % MOD < N) { // the seq of ack fall into the send window
            if (send_buf[ackPkt.acknum].is_occupied) {
                send_buf[ackPkt.acknum].is_acked = true;
                pns->stopTimer(SENDER, ackPkt.acknum);

                if (ackPkt.acknum == send_base) { // need to move the window
                    while (send_buf[send_base].is_acked) {
                        send_buf[send_base].is_occupied = false;
                        send_buf[send_base].is_acked = false;
                        
                        send_base = (send_base + 1) % MOD;
                    }

                    cout << NAME << "the window is sliding to ";
                    cout << "(";
                    int i;
                    for (i = send_base; (i - send_base + MOD) % MOD < N - 1; i = (i + 1) % MOD) {
                        cout << i << " ";
                    }
                    cout << i << ")" << endl;
                    

                    return; //just to notify; you mustn't do anything after
                }
            } else {
                cout << "Strange thing happens:maybe received a very long ago packet" << endl;
            }
        } // else do nothing
    } // else do nothing
}


void SRRdtSender::timeoutHandler(int seqNum) {
    // restart the timer, resend the packet
    pns->stopTimer(SENDER, seqNum);

    pns->sendToNetworkLayer(RECEIVER, send_buf[seqNum].packet);

    pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
}

bool SRRdtSender::getWaitingState() {
    if ((next_seq - send_base + MOD) % MOD < N) {
        // available
        return false;
    } else {
        return true;
    }
}

SRRdtSender::SRRdtSender(): send_base(1), next_seq(1), N(Configuration::WIN_SIZE), ByteLen(Configuration::BYTE_LEN), MOD(1 << Configuration::BYTE_LEN) {}
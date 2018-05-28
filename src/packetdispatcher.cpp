#include "packetdispatcher.h"

void PacketDispatcher::register_testcase(BaseTestCase* testcase) {
	this->testcase = testcase;
}

void PacketDispatcher::send_loop() {
	if (this->send_loop_running) {
		return;
	}
	else {
		this->send_loop_running = true;
	}
    

	// Start send loop
	// Run while loop
	double t = utils::timestamp() * 1000.0;	
	
	//struct timespec tspec = { 0 };
	while (!testcase->finished) {			            
        //printf("t: %.4f, last-send: %.4f, diff: %.4f, next-btime: %.4f\n", t, testcase->last_send, t - testcase->last_send, testcase->next_burst_time);
            
        // #######
		//if ((t - testcase->last_send) >= (testcase->next_burst_time)) {
		testcase->next_burst_time = testcase->scheduler->next_inter_burst_time();
                
		// Check if counter's limit is reached (time limit, msg_num limit)
		if (testcase->cntr->limit_reached()) {	                    
            testcase->finish_requesting();				
			break;
		}

		// Missing packet calc	
		testcase->_missing_packets = testcase->_missing_packets + testcase->_missing_packets_per_sendinterval;
		int missed_packets_rounded = floor(testcase->_missing_packets);
		if (missed_packets_rounded > 0)
			testcase->_missing_packets = testcase->_missing_packets -  (float) missed_packets_rounded;
		// Missing packets are added to the burst rate
		int burst_rate = testcase->scheduler->next_burst_size() + missed_packets_rounded;	

		//std::cout << "send-intvl: " << testcase->next_burst_time << " T: " << testcase->tenant->id << " Burst size:" << burst_rate << " with missing_pkts: " << missed_packets_rounded << "\n";
				
		// #######
		testcase->last_send = utils::timestamp() * 1000;
                
		
        //std::cout << burst_rate << ",";
		for (int a = 0; a < burst_rate; a++)
		{						
			testcase->pre_packet_send_dataplane();

			// OF message
			if (!testcase->dataplane) {	
				
				// Ctrl as sender
				if(!testcase->switch_sender){
					// Send to all connections (Multi Switch)
					for (uint8_t c = 0; c < testcase->ctrl->conns.size(); c++) {
					//TESTING for onos: for (uint8_t c = 0; c < 1; c++) {

						OFConnection* conn = testcase->ctrl->conns.at(c);
							    
						// Get pre-created packet
						fluid_msg::OFMsg* msg = testcase->packet();		

						// Set new XID: request-nr + 10M * conn-id								
						msg->xid(testcase->cntr->requests.at(conn->list_index) + 10000000 * conn->list_index + 1);						

						// Call pre packet processing
						// e.g. Packetout: reassemble packet with new data
						msg = testcase->pre_packet_send(msg, 0, 0);																					

						// Create buffer data		
						uint8_t* buffer;
						buffer = msg->pack();

						testcase->cntr->count_request(msg->xid(), utils::timestamp(), msg->length(), msg->type(), conn->list_index);

						conn->send(buffer, msg->length());
								

						// Free the buffer memory
						fluid_msg::OFMsg::free_buffer(buffer);
					}
				}
				// Switch as sender
				else {
					for (uint8_t c = 0; c < testcase->switches.size(); c++) {								
						Switch* sw = testcase->switches.at(c);
						OFConnection* conn = testcase->switches.at(c)->conn;
							    
						// Get pre-created packet
						fluid_msg::OFMsg* msg = testcase->packet();		

						// Set new XID: request-nr + 10M * conn-id								
						msg->xid(testcase->cntr->requests.at(conn->list_index) + 10000000 * conn->list_index + 1);			

						//std::cout << "conn-list-index: " << conn->list_index << "\n";			

						// Call pre packet processing
						// e.g. Packetout: reassemble packet with new data
						msg = testcase->pre_packet_send(msg, sw->datapath_id, conn->list_index);																					

						// Create buffer data		
						uint8_t* buffer;
						buffer = msg->pack();

						testcase->cntr->count_request(msg->xid(), utils::timestamp(), msg->length(), msg->type(), conn->list_index);

						conn->send(buffer, msg->length());							

						// Free the buffer memory
						fluid_msg::OFMsg::free_buffer(buffer);
					}
				}						
						
						
			}					
			// Dataplane packet (PacketIn testcase)
			// FIXME: Multi-Connection handling
			else {												
				testcase->cntr->count_request(std::accumulate(testcase->cntr->requests.begin(), testcase->cntr->requests.end(), 0), utils::timestamp(),
					8, 255, 0);
                raw_socket::send_data(testcase->_dataplane_packet, testcase->_dataplane_packet_size);
			}

			// Intra Burst time
			if(burst_rate > 1 && testcase->intra_burst_time() > 0){
				usleep(testcase->intra_burst_time());
			}
		}			
		double elapsed_time = (utils::timestamp() * 1000.0 - t);		

		// boost sleep_for
		double sleeptime = testcase->next_burst_time - elapsed_time;			
		boost::this_thread::sleep_for(boost::chrono::microseconds((int)((sleeptime * (1 - 0.080 * (1.0/testcase->next_burst_time)) * 1000.0))));		

		t = utils::timestamp() * 1000.0;
    }
}



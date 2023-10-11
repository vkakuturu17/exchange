#pragma once
#include "ankerl/unordered_dense.h"
#include "doublylinkedlist/dll.h"
#include "struct/structs.h"
#include "orderbook/orderbook.h"
#include <vector>
#include <string>
#include <utility>
#include <thread>

#define SERVER_PORT 3000
#define QUEUE_MAX_SIZE 1024
#define BUFFER_SIZE 128

// #define STD_OME_QUEUE_BOOL false
// #ifdef STD_OME_QUEUE_BOOL // Use std queue library instead of spsc
// // Default spsc queue // TODO: Rigtorp's spsc queue's front is diff than std::queue front
//   #include <rigtorp/SPSCQueue.h>
//   template <class T>
//   using SPSC_queue = rigtorp::SPSCQueue<T>;
// #endif

#include <rigtorp/SPSCQueue.h>
template <class T>
using SPSC_queue = rigtorp::SPSCQueue<T>;

using ankerl::unordered_dense::map;
using std::vector;
using std::string;
using std::pair;
using Exchange::OrderBookPtr;
using Exchange::Timer;

typedef map<std::string, OrderBookPtr> OrderBooks; // Map Symbol to OrderBook

namespace Exchange {
class OrderMatchingEngine {
  public:
    OrderMatchingEngine(Timer* timer);
    ~OrderMatchingEngine();

    void load_orderbook_symbols(std::string filepath);
    void load_orderbook_symbols(std::vector<std::string> symbols);
    vector<std::string> get_symbols() const;

    // Accessors
    // Order* getOrder(std::string key);

    // Parsers I/O
    // Input
    // void parse_order(char* arr);

    // FSM Logic and Event Loop //
    void run();

    /**
     * @brief Add an inbound packet to the inbound queue
     * @details Adds packet from TCP socket to the OUCH inbound queue buffer
     * 
     * @param inbound_buffer_queue The same as the class's OUCH_inbound_buffer_queue_. 
     * Used to store char buffer from reading TCP packet
    */
    void add_inbound_packet_to_queue(SPSC_queue<unsigned char*>& inbound_buffer_queue);

    /**
     * @brief Convert inbound packet to order
     * @details Converts the inbound packet from the OUCH inbound queue buffer to an order
     * and places it in the OUCH inbound order queue. Uses the Parser.h library to convert.
     * 
     * @param inbound_buffer_queue The same as the class's OUCH_inbound_buffer_queue_ which is
     * a queue of char buffers from raw TCP packet.
     * @param inbound_order_queue The same as the class's OUCH_inbound_order_queue_. Used for storing
     * the inbound orders in the internal struct representation.
    */
    void convert_OUCH_inbound_packet_to_order(SPSC_queue<unsigned char*>& inbound_buffer_queue, SPSC_queue<InboundOrderPtr>& inbound_order_queue);
    void convert_OUCH_inbound_packet_to_order_IMPL(SPSC_queue<unsigned char*>& inbound_buffer_queue, SPSC_queue<InboundOrderPtr>& inbound_order_queue);

    /**
     * @brief Add inbound order to the orderbook
     * @details Adds the Inbound Order from the OUCH inbound order queue to the orderbook and
     * places the symbol in the orderbook symbol queue to be executed.
     * 
     * @param inbound_order_queue The same as the class's OUCH_inbound_order_queue_. Takes orders from this
     * queue in order to add to the orderbook.
     * @param orderbook_symbol_to_execute_queue The same as the class's orderbook_symbol_to_execute_queue_. Records
     * which orderbook was changed for later execution.
    */
    void add_inbound_order_to_orderbook(SPSC_queue<InboundOrderPtr>& inbound_order_queue, SPSC_queue<std::string>& orderbook_symbol_to_execute_queue);
    void add_inbound_order_to_orderbook_IMPL(SPSC_queue<InboundOrderPtr>& inbound_order_queue, SPSC_queue<std::string>& orderbook_symbol_to_execute_queue);

    /**
     * @brief Execute the next order in the orderbook
     * @details Executes the next orderbook in the orderbook symbol queue by calling
     * the orderbook's respective process_next_order()
     * 
     * @param orderbook_symbol_to_execute_queue The same as the class's orderbook_symbol_to_execute_queue_. Executes
     * the orderbooks from the symbols on the queue.
    */
    void execute_orderbook_next(SPSC_queue<string>& orderbook_symbol_to_execute_queue, SPSC_queue<string>& orderbook_symbol_OUCH_to_output_queue, SPSC_queue<string>& orderbook_symbol_ITCH_to_output_queue);
    void execute_orderbook_next_IMPL(SPSC_queue<string>& orderbook_symbol_to_execute_queue, SPSC_queue<string>& orderbook_symbol_OUCH_to_output_queue, SPSC_queue<string>& orderbook_symbol_ITCH_to_output_queue);

    /**
     * @brief Grab output from the orderbook
     * @details Grabs the OUCH outbound from the orderbook and places it in the OUCH outbound queue
    */
    void grab_OUCH_outbound_from_orderbook(SPSC_queue<string>& orderbook_symbol_OUCH_to_output_queue, SPSC_queue<OutboundOrderPtr>& OUCH_outbound_order_queue);
    void grab_OUCH_outbound_from_orderbook_IMPL(SPSC_queue<string>& orderbook_symbol_OUCH_to_output_queue, SPSC_queue<OutboundOrderPtr>& OUCH_outbound_order_queue);

    /**
     * @brief Grab output from the orderbook
     * @details Grabs the ITCH outbound from the orderbook and places it in the ITCH outbound queue
    */
    void grab_ITCH_outbound_from_orderbook(SPSC_queue<string>& orderbook_symbol_ITCH_to_output_queue, SPSC_queue<ITCH_MessagePtr>& ITCH_outbound_order_queue);
    void grab_ITCH_outbound_from_orderbook_IMPL(SPSC_queue<string>& orderbook_symbol_ITCH_to_output_queue, SPSC_queue<ITCH_MessagePtr>& ITCH_outbound_order_queue);

    /**
     * @brief Convert OUCH outbound order to packet
     * Converts the outbound order from the OUCH outbound order queue to a packet
    */
    void convert_OUCH_outbound_order_to_packet(SPSC_queue<OutboundOrderPtr>& OUCH_outbound_order_queue, SPSC_queue<unsigned char*>& OUCH_outbound_buffer_queue);
    void convert_OUCH_outbound_order_to_packet_IMPL(SPSC_queue<OutboundOrderPtr>& OUCH_outbound_order_queue, SPSC_queue<unsigned char*>& OUCH_outbound_buffer_queue);

    /**
     * @brief Send outbound packet
     * Sends the outbound packet from the OUCH outbound packet queue to the TCP socket
    */
    void send_outbound_OUCH_packet(SPSC_queue<unsigned char*>& OUCH_outbound_buffer_queue);
    void send_outbound_OUCH_packet_IMPL(SPSC_queue<unsigned char*>& OUCH_outbound_buffer_queue);

    /**
     * @brief Convert ITCH outbound order to packet
    */
    void convert_ITCH_outbound_order_to_packet(SPSC_queue<ITCH_MessagePtr>& ITCH_outbound_order_queue, SPSC_queue<unsigned char*>& ITCH_outbound_buffer_queue);
    void convert_ITCH_outbound_order_to_packet_IMPL(SPSC_queue<ITCH_MessagePtr>& ITCH_outbound_order_queue, SPSC_queue<unsigned char*>& ITCH_outbound_buffer_queue);

    /**
     * @brief Send outbound packet
    */
    void send_outbound_ITCH_packet(SPSC_queue<unsigned char*>& ITCH_outbound_buffer_queue);
    void send_outbound_ITCH_packet_IMPL(SPSC_queue<unsigned char*>& ITCH_outbound_buffer_queue);


    // Accessors
    OrderBooks get_orderbooks() const { return orderbooks_; }
    OrderBookPtr get_orderbook(std::string symbol) const {
      auto it = orderbooks_.find(symbol);
      if (it != orderbooks_.end()) {
        return it->second;
      }
      return nullptr;
    }

  private:
    Timer* timer_;
    OrderMap orderID_to_order_;
    OrderBooks orderbooks_;

    void __destroy();
  public:
    // TODO: Convert to SPSC queue
    SPSC_queue<unsigned char*> OUCH_inbound_buffer_queue_;
    SPSC_queue<unsigned char*> OUCH_outbound_buffer_queue_;
    SPSC_queue<unsigned char*> ITCH_outbound_buffer_queue_;

    SPSC_queue<InboundOrderPtr> OUCH_inbound_order_queue_;
    SPSC_queue<OutboundOrderPtr> OUCH_outbound_order_queue_;
    SPSC_queue<ITCH_MessagePtr> ITCH_outbound_order_queue_;

    SPSC_queue<std::string> orderbook_symbol_to_execute_queue_;

    // int is for how many pops to do off queue front. Different types of orders create different number of pops
    SPSC_queue<string> orderbook_symbol_OUCH_to_output_queue_;
    SPSC_queue<string> orderbook_symbol_ITCH_to_output_queue_;

    // Parsers I/O private
    // char* parse_order_entry(char* arr);
    
};
} // namespace Exchange
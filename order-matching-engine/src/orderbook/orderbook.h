#pragma once
#include <memory>
#include <sstream>
#include <mutex>
#include "ankerl/unordered_dense.h"
#include "struct/structs.h"
#include "struct/nasdaq_ouch.h"
#include "struct/nasdaq_itch.h"
#include "order_matching_engine/machine_time.h"
#include "order_matching_engine/util.h"

// #define STD_ORDERBOOK_QUEUE_BOOL true
// #ifdef STD_ORDERBOOK_QUEUE_BOOL // Use std queue library instead of spsc
//   #include <queue>
//   template <class T>
//   using SPSC_queue = std::queue<T>;
// #else // Default spsc queue
//   #include <rigtorp/SPSCQueue.h>
//   template <class T>
//   using SPSC_queue = rigtorp::SPSCQueue<T>;
// #endif

#include <queue>
// template <class T>
// using SPSC_queue = std::queue<T>;
using std::queue;

using ankerl::unordered_dense::map;
using Exchange::Timer;
using Exchange::StringHash;

typedef map<std::string, OrderPtr> OrderMap; // Map OrderID to Order
typedef map<uint32_t, PriceLevelPtr> PriceLevelMap; // Map Price to PriceLevel

namespace Exchange {
class OrderBook {
  public:
    OrderBook(const std::string& stock_symbol, Timer* machine_timer);

    OrderBook(const OrderBook& other) = delete;
    OrderBook& operator=(const OrderBook& other) = delete;
    OrderBook(OrderBook&& other) = delete;
    OrderBook& operator=(OrderBook&& other) = delete;

    // Event Loop
    bool process_next_order(); // Process next order in inbound queue, true = correct parse, false = queue empty

    // TODO: Pass in std::unique or shared_ptr for OrderEntry

    // Modifiers
    // TODO: If queue is full process_next_order() then add_order();
    // TODO: This is only OUCH, what about ITCH?
    void add_order(InboundOrderPtr order); // TODO: If you just have this function and remove the other two it breaks
    void add_order(OrderEntryPtr new_order);
    void add_order(OrderCancelPtr new_order);

    PriceLevelPtr operator[](uint32_t price) { return price_to_priceLevel_[price]; }
    // const PriceLevelPtr operator[](uint32_t price) const { return price_to_priceLevel_[price]; }

    // Accessors
    std::string get_stock_symbol() const { return stock_symbol_; }
    OutboundOrderUniquePtr get_OUCH_outbound_order() { OutboundOrderUniquePtr order = std::move(outbound_orders_.front()); outbound_orders_.pop(); return order; }
    ITCH_MessageUniquePtr get_ITCH_outbound_order() { ITCH_MessageUniquePtr order = std::move(outbound_ITCH_messages_.front()); outbound_ITCH_messages_.pop(); return order; }
    uint32_t get_max_bid_price() const { uint32_t price = (best_bid_ != nullptr) ? (best_bid_->price_level->price) : -1; return price;}
    uint32_t get_min_ask_price() const { uint32_t price = (best_offer_ != nullptr) ? (best_offer_->price_level->price) : -1; return price;}

    // Print
    std::ostringstream ostringstream_top_of_book();
    std::ostringstream ostringstream_order_ids();

    // Statistics of orderbook
    uint32_t get_num_OUCH_inbound_queue() const { return inbound_orders_.size(); }
    uint32_t get_num_OUCH_outbound_queue() const { return outbound_orders_.size(); }
    uint32_t get_num_ITCH_outbound_queue() const { return outbound_ITCH_messages_.size(); }
    std::vector<std::string> get_order_ids() const;

    // Match number (thread lock)
    uint32_t increment_match_number() { return ++match_number_; }
  
    std::mutex mutex_lock_; // TODO: Make sure this is ok being public
  private:
    std::string stock_symbol_;
    Order last_order_;
    uint32_t match_number_ = 0; // TODO: Maybe make this static?
    int tick_size = -2; // Power of 10
    Timer* machine_timer_; // TODO: Make this a shared_ptr
    // TODO: Enforce tick_size
    

    struct Node;
    typedef std::shared_ptr<Node> NodePtr;
    struct Node { // Singly LinkedList
      NodePtr next;
      PriceLevelPtr price_level;
    };
    
    NodePtr best_bid_ = nullptr;
    NodePtr best_offer_ = nullptr;

    OrderMap orderID_to_order_;
    PriceLevelMap price_to_priceLevel_; // TODO: Check if PriceLevel needs to be shared or unique
    // TODO: Implement queues with circular buffer
    queue<InboundOrderUniquePtr> inbound_orders_;
    queue<OutboundOrderUniquePtr> outbound_orders_;
    queue<ITCH_MessageUniquePtr> outbound_ITCH_messages_;
    
    /**
     * Adds Order to orderbook or executes it. Will deallocate new_order upon completion.
     * @brief Order Entry
     * 
     * @param new_order 
     * @param timestamp 
     */
    void _order_entry(OrderEntry& new_order, uint64_t timestamp); // TODO: OUCH specialized... maybe add ITCH support?
    void _order_cancel(const OrderCancel& order_cancel, uint64_t timestamp);
    // order_modify(const std::string& orderID);
    // order_replace(const std::string& orderID);
    
    // OUCH
    OrderAcceptedOutboundUniquePtr _create_order_accepted_outbound(const OrderEntry& new_order, uint64_t timestamp, char order_state, char bbo_weight_indicator);
    OrderCanceledOutboundUniquePtr _create_order_canceled_outbound(const OrderCancel& order_cancel, uint64_t timestamp);
    CancelRejectOutboundUniquePtr _create_order_cancel_rejected_outbound(const OrderCancel& order_cancel, uint64_t timestamp);
    OrderExecutedOutboundUniquePtr _create_order_executed_outbound(str_cref mpid, str_cref order_token, uint64_t timestamp, uint32_t price, uint32_t shares, uint32_t match_number);

    // ITCH
    ITCH_AddOrderMPIDUniquePtr _create_itch_add_order_mpid(const OrderEntry& new_order, uint64_t timestamp, uint64_t order_ref_num);
    ITCH_OrderExecutedPriceUniquePtr _create_itch_order_executed_price(uint64_t timestamp, uint64_t order_ref_num, uint32_t executed_shares, uint64_t match_num, char printable, uint32_t execution_price);
    ITCH_OrderCancelUniquePtr _create_itch_order_cancel(uint64_t timestamp, uint64_t order_ref_num, uint32_t canceled_shares);
    

    OrderPtr _create_internal_order(const OrderEntry& new_order, uint64_t timestamp);
    // Internal add to queue for multithreading
    void _add_to_inbound_queue(InboundOrderUniquePtr msg) { inbound_orders_.push(std::move(msg)); }
    void _add_to_outbound_queue(OutboundOrderUniquePtr msg) { outbound_orders_.push(std::move(msg)); }
    void _add_to_itch_queue(ITCH_MessageUniquePtr msg) { outbound_ITCH_messages_.push(std::move(msg)); }

    void _insert_order_into_orderbook(OrderPtr& order, char buy_sell_indicator);

    void _insert_best_bid(NodePtr& node); // Insert into linked list for bid
    void _insert_best_offer(NodePtr& node); // Insert into linked list for ask
};

typedef std::shared_ptr<OrderBook> OrderBookPtr;
}
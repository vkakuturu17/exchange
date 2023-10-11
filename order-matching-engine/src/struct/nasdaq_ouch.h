// NASDAQ O*U*C*H v4.2
// http://www.nasdaqtrader.com/content/technicalsupport/specifications/tradingproducts/ouch4.2.pdf

#pragma once
#include <string>
#include <string.h>
#include <memory>
#include "order_matching_engine/util.h"

#define ORDER_TOKEN_LEN 14
#define MPID_LEN 4
#define STOCK_SYM_LEN 8
#define ORDER_ID_LEN 8

using Exchange::CopyStringToCharArray;

enum sizes
{
  SystemEventMessageSize = 12,
  StockDirectoryMessageSize = 39,
  StockTradingActionMessageSize = 25,
  RegShoRestrictionMessageSize = 20,
  MarketParticipantPositionMessageSize = 26,
  MarketWideCircuitBreakerDeclineLevelMessageSize = 35,
  MarketWideCircuitBreakerStatusMessageSize= 12,
  IPOQuotingPeriodUpdateMessageSize = 28,
  LULDAuctionCollarMessageSize = 35,
  OperationalHaltMessageSize = 21,
  AddOrderMessageSize = 36,
  AddOrderMessageWithMPIDSize = 40,
  OrderExecutedMessageSize = 31,
  OrderExecutedWithPriceMessageSize = 36,
  OrderCancelMessageSize = 23,
  OrderDeleteMessageSize = 19,
  OrderReplaceMessageSize = 35,
  TradeMessageSize = 44,
  CrossTradeMessageSize = 40,
  BrokenTradeMessageSize = 19,
  NetOrderImbalanceIndicatorMessageSize = 50,
  RetailPriceImprovementIndicatorMessageSize = 20,
};

// TODO: Actually use these enums meaningfully instead of hardcoding chars
enum class OUCH_OrderType_Inbound {
  ORDER_ENTRY = 'O',
  ORDER_CANCEL = 'X',
  ORDER_REPLACE = 'U',
  ORDER_MODIFY = 'M',
};

enum class OUCH_OrderType_Outbound {
  SYSTEM_EVENT_MESSAGE = 'S',
  ORDER_ACCEPTED = 'A',
  ORDER_REPLACED = 'U',
  ORDER_CANCELLED = 'C',
  ORDER_EXECUTED = 'E',
  REJECTED_MESSAGE = 'J',
  CANCEL_REJECT = 'I',
  ORDER_PRIORITY_UPDATE = 'T',
};

typedef const std::string& str_cref;

typedef struct InboundOrder {
  char otype; // Order Type, exactly same as Order types. This is for switch in Orderbook queue.
  std::string symbol;

  InboundOrder() {}
  virtual ~InboundOrder() = default;
  // InboundOrder(char type) : otype(type) {}
} InboundOrder;
typedef std::unique_ptr<InboundOrder> InboundOrderUniquePtr;
typedef std::shared_ptr<InboundOrder> InboundOrderPtr;

typedef struct OutboundOrder {
  char otype;

  OutboundOrder() {}
  virtual ~OutboundOrder() = default;
} OutboundOrder;
typedef std::unique_ptr<OutboundOrder> OutboundOrderUniquePtr;
typedef std::shared_ptr<OutboundOrder> OutboundOrderPtr;

// 2.1
typedef struct OrderEntry : InboundOrder {
  const char type = 'O';
  char order_token[14];
  char buy_sell_indicator; // TODO: Enforce B, S, T, or E only
  uint32_t shares;
  char stock_sym[8];
  uint32_t price;
  uint32_t time_in_force; // NOT USED
  char mpid[4]; // aka firm
  char display; // NOT USED
  // TODO: Everything below needs to be added to the constructor
  char capacity; // NOT USED
  char intermarket_sweep_eligibility; // NOT USED
  uint32_t minimum_quantity; // NOT USED
  char cross_type; // NOT USED
  char customer_type; // NOT USED

  OrderEntry() {
    InboundOrder();
    InboundOrder::otype = type;
  }
  OrderEntry(str_cref ort, char bs, uint32_t sh, str_cref ss, uint32_t pr, uint32_t tif, str_cref mp, char d) :
    buy_sell_indicator(bs), shares(sh), price(pr), time_in_force(tif), display(d) {
    InboundOrder();
    InboundOrder::otype = type;
    InboundOrder::symbol = ss;

    memset(order_token, 0, ORDER_TOKEN_LEN);
    memset(stock_sym, 0, STOCK_SYM_LEN);
    memset(mpid, 0, MPID_LEN);
    CopyStringToCharArray(order_token, ort, ORDER_TOKEN_LEN);
    CopyStringToCharArray(stock_sym, ss, STOCK_SYM_LEN);
    CopyStringToCharArray(mpid, mp, MPID_LEN);
    // strncpy(order_token, ort.c_str(), 14);
    // strncpy(stock_sym, ss.c_str(), 8);
    // strncpy(mpid, mp.c_str(), 4);
  }
} OrderEntry;
typedef std::unique_ptr<OrderEntry> OrderEntryUniquePtr;
typedef std::shared_ptr<OrderEntry> OrderEntryPtr;

// 2.2
typedef struct OrderReplace : InboundOrder {
  const char type = 'U';
  char order_token[14];
  char replacement_order_token[14];
  uint32_t shares;
  uint32_t price;
  uint32_t time_in_force; // NOT USED
  char display; // NOT USED
  char intermarket_sweep_eligibility;
  uint32_t minimum_quantity; // NOT USED
} OrderReplace;

// 2.3
typedef struct OrderCancel : InboundOrder {
  const char type = 'X';
  char mpid[4];
  char order_token[14];
  uint32_t shares; // New intended order size, 0 = total cancel

  OrderCancel() {
    InboundOrder();
    InboundOrder::otype = type;
  }
  OrderCancel(str_cref mp, str_cref ot, uint32_t sh) : shares(sh) {
    InboundOrder();
    InboundOrder::otype = type;

    memset(order_token, 0, ORDER_TOKEN_LEN);
    memset(mpid, 0, MPID_LEN);
    CopyStringToCharArray(order_token, ot, ORDER_TOKEN_LEN);
    CopyStringToCharArray(mpid, mp, MPID_LEN);
    // strncpy(order_token, ot.c_str(), 14);
    // strncpy(mpid, mp.c_str(), 4);
  }
} OrderCancel;
typedef std::unique_ptr<OrderCancel> OrderCancelUniquePtr;
typedef std::shared_ptr<OrderCancel> OrderCancelPtr;

typedef struct OrderModify : InboundOrder {
  const char type = 'M';
  std::string order_token;
  char buy_sell_indicator; // TODO: Check for S->T, S->E, E->T, E->S, T->E, T->S
  uint32_t shares;
} OrderModify;

// 3.1
typedef struct SystemEventMessage : OutboundOrder {
  const char type = 'S';
  uint64_t timestamp;
  char event_code; // TODO: Enforce S (start of day), E (end of day)

  SystemEventMessage() {
    OutboundOrder();
    OutboundOrder::otype = type;
  }
  SystemEventMessage(uint64_t ts, char ec) : timestamp(ts), event_code(ec) {
    OutboundOrder();
    OutboundOrder::otype = type;
  }
} SystemEventMessage;
typedef std::unique_ptr<SystemEventMessage> SystemEventMessageUniquePtr;
typedef std::shared_ptr<SystemEventMessage> SystemEventMessagePtr;

// 3.3
typedef struct OrderAcceptedOutbound : OutboundOrder {
  const char type = 'A';
  uint64_t timestamp;
  char order_token[14];
  char buy_sell_indicator;
  uint32_t shares;
  char stock_sym[8];
  uint32_t price;
  uint32_t time_in_force; // NOT USED
  char mpid[4]; // aka firm
  char display; // NOT USED
  char orderID[8]; // Exchange orderID, reference number, 8 byte Int in Nasdaq
  char capacity; // NOT USED
  char intermarket_sweep_eligibility; // NOT USED
  uint32_t minimum_quantity; // NOT USED
  char cross_type; // NOT USED
  char order_state; // TODO: Enforce L or D only
  char bbo_weight_indicator; // NOT USED, 0, 1, 2, 3, S - Nasdaq QBBO, or N - NBBO

  OrderAcceptedOutbound() {
    OutboundOrder();
    OutboundOrder::otype = type;
  }
  OrderAcceptedOutbound(uint64_t time, str_cref ord, char bs, uint32_t sh, str_cref st, uint32_t pr, uint32_t tif, str_cref mp, char di, str_cref orID, char orst, char bbo)
    : timestamp(time), buy_sell_indicator(bs), shares(sh), price(pr), time_in_force(tif), display(di), order_state(orst), bbo_weight_indicator(bbo) {
      OutboundOrder();
      OutboundOrder::otype = type;

      memset(order_token, 0, ORDER_TOKEN_LEN);
      memset(stock_sym, 0, STOCK_SYM_LEN);
      memset(mpid, 0, MPID_LEN);
      memset(orderID, 0, ORDER_ID_LEN);

      CopyStringToCharArray(order_token, ord, ORDER_TOKEN_LEN);
      CopyStringToCharArray(stock_sym, st, STOCK_SYM_LEN);
      CopyStringToCharArray(mpid, mp, MPID_LEN);
      CopyStringToCharArray(orderID, orID, ORDER_ID_LEN);
      // memcpy(order_token, ord.c_str(), 14);
      // memcpy(stock_sym, st.c_str(), 8);
      // memcpy(mpid, mp.c_str(), 4);
      // memcpy(orderID, orID.c_str(), 8);
      // strncpy(order_token, ord.c_str(), 14);
      // strncpy(stock_sym, st.c_str(), 8);
      // strncpy(mpid, mp.c_str(), 4);
      // strncpy(orderID, orID.c_str(), 8);
      capacity = 0x0;
      intermarket_sweep_eligibility = 0x0;
    }
} OrderAcceptedOutbound;
typedef std::unique_ptr<OrderAcceptedOutbound> OrderAcceptedOutboundUniquePtr;
typedef std::shared_ptr<OrderAcceptedOutbound> OrderAcceptedOutboundPtr;

// 3.4
typedef struct OrderReplacedOutbound : OutboundOrder {
  const char type = 'U';
  uint64_t timestamp;
  char order_token[14];
  char buy_sell_indicator_old; // TODO: Review what this should be
  uint32_t shares; // Number of shares outstanding
  char stock_sym[8];
  uint32_t price;
  uint32_t time_in_force; // NOT USED
  char mpid[4]; // aka firm
  char display; // NOT USED
  uint64_t orderID; // Exchange orderID, 8 byte Int in Nasdaq
  char capacity; // NOT USED
  char intermarket_sweep_eligibility;
  uint32_t minimum_quantity; // NOT USED
  char cross_type; // NOT USED
  char order_state; // L or D
  char order_token_old[14]; // Order token of the order being replaced
  char bbo_weight_indicator; // NOT USED, 0, 1, 2, 3, S - Nasdaq QBBO, or N - NBBO
} OrderReplacedOutbound;

// 3.5
typedef struct OrderCancelledOutbound : OutboundOrder {
  const char type = 'C';
  uint64_t timestamp;
  char mpid[4];
  char order_token[14];
  uint32_t decrement_shares; // This number is incremental, not cumulative
  char reason; // TODO: Pick which one(s) I want to use (User request cancel is 'U')

  OrderCancelledOutbound() {
    OutboundOrder();
    OutboundOrder::otype = type;
  }
  OrderCancelledOutbound (uint64_t time, str_cref mp, str_cref ord, uint32_t dec, char r) : timestamp(time), decrement_shares(dec), reason(r) {
    OutboundOrder();
    OutboundOrder::otype = type;
    
    memset(mpid, 0, MPID_LEN);
    memset(order_token, 0, ORDER_TOKEN_LEN);
    CopyStringToCharArray(mpid, mp, MPID_LEN);
    CopyStringToCharArray(order_token, ord, ORDER_TOKEN_LEN);
  }
} OrderCancelledOutbound;
typedef std::unique_ptr<OrderCancelledOutbound> OrderCanceledOutboundUniquePtr;
typedef std::shared_ptr<OrderCancelledOutbound> OrderCanceledOutboundPtr;

// 3.7
typedef struct OrderExecutedOutbound : OutboundOrder {
  const char type = 'E';
  uint64_t timestamp;
  char mpid[4];
  char order_token[14];
  uint32_t executed_shares;
  uint32_t executed_price;
  char liquidity_flag; // NOT USED
  uint64_t match_number; // Unique for every match, same on buy/sell

  OrderExecutedOutbound() {
    OutboundOrder();
    OutboundOrder::otype = type;
  }
  OrderExecutedOutbound(uint64_t time, str_cref mp, str_cref ord, uint32_t ex_sh, uint32_t ex_pr, char li, uint32_t ma) 
  : timestamp(time), executed_shares(ex_sh), executed_price(ex_pr), liquidity_flag(li), match_number(ma) {
    OutboundOrder();
    OutboundOrder::otype = type;
    
    memset(mpid, 0, MPID_LEN);
    memset(order_token, 0, ORDER_TOKEN_LEN);
    CopyStringToCharArray(mpid, mp, MPID_LEN);
    CopyStringToCharArray(order_token, ord, ORDER_TOKEN_LEN);
    // strncpy(order_token, ord.c_str(), 14);
    // strncpy(mpid, mp.c_str(), 4);
  }
} OrderExecutedOutbound;
typedef std::unique_ptr<OrderExecutedOutbound> OrderExecutedOutboundUniquePtr;
typedef std::shared_ptr<OrderExecutedOutbound> OrderExecutedOutboundPtr;

// 3.11
typedef struct RejectedMessageOutbound : OutboundOrder {
  const char type = 'J';
  uint64_t timestamp;
  std::string order_token;
  char reason; // TODO: Pick which one(s) I want to use
} RejectedMessageOutbound;

// 3.13
typedef struct CancelRejectOutbound : OutboundOrder {
  const char type = 'I';
  uint64_t timestamp;
  char mpid[4];
  char order_token[14];

  CancelRejectOutbound(uint64_t time, str_cref mp, str_cref ord) : timestamp(time) {
    OutboundOrder();
    OutboundOrder::otype = type;

    strncpy(mpid, mp.c_str(), 4);
    strncpy(order_token, ord.c_str(), 14);
  }
} CancelRejectOutbound;
typedef std::unique_ptr<CancelRejectOutbound> CancelRejectOutboundUniquePtr;

// 3.14
typedef struct OrderPriorityUpdateOutbound : OutboundOrder {
  const char type = 'T';
  uint64_t timestamp;
  char order_token[14];
  uint32_t price;
  char display;
  uint64_t order_reference_number;
} OrderPriorityUpdateOutbound;

// 3.15
// typedef 
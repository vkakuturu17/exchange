// NASDAQ ITCHv5.0
// https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf

#pragma once
#include <string>
#include <memory>
#include "order_matching_engine/util.h"

#define ITCH_MPID_LEN 4
#define ITCH_STOCK_LEN 8

using Exchange::CopyStringToCharArray;

enum class ITCH_OrderType {
  ADD_ORDER = 'F',
  ORDER_EXECUTED = 'C',
  ORDER_CANCEL = 'X'
};

typedef const std::string& str_cref;

typedef struct ITCH_Message {
  char otype;

  ITCH_Message() {}
  virtual ~ITCH_Message() = default;
} ITCH_Message;
typedef std::unique_ptr<ITCH_Message> ITCH_MessageUniquePtr;
typedef std::shared_ptr<ITCH_Message> ITCH_MessagePtr;

// 1.3.2 Add Order with MPID Attribution
typedef struct ITCH_AddOrderMPID : ITCH_Message {
  const char type = 'F';
  uint16_t stock_locate; // not used currently
  uint16_t tracking_number; // not used currently
  uint64_t timestamp; // TODO: timestamp is 48 bits in reality
  uint64_t order_ref_num; // not used currently
  char buy_sell_indicator;
  uint32_t shares;
  char stock[8];
  uint32_t price;
  char mpid[4]; // Also called attribution

  ITCH_AddOrderMPID() {
    ITCH_Message();
    ITCH_Message::otype = type;
  }
  ITCH_AddOrderMPID(uint16_t sl, uint16_t tn, uint64_t ts, uint64_t orn, char bs, uint32_t sh, str_cref st, uint32_t pr, str_cref attr) :
    stock_locate(sl), tracking_number(tn), timestamp(ts), order_ref_num(orn), buy_sell_indicator(bs), shares(sh), price(pr) {
      ITCH_Message();
      ITCH_Message::otype = type;

      memset(stock, 0, ITCH_STOCK_LEN);
      memset(mpid, 0, ITCH_MPID_LEN);

      CopyStringToCharArray(stock, st, ITCH_STOCK_LEN);
      CopyStringToCharArray(mpid, attr, ITCH_MPID_LEN);
      // strncpy(stock, st.c_str(), 8);
      // strncpy(mpid, attr.c_str(), 4);
    }
} ITCH_AddOrderMPID;
typedef std::unique_ptr<ITCH_AddOrderMPID> ITCH_AddOrderMPIDUniquePtr;
typedef std::shared_ptr<ITCH_AddOrderMPID> ITCH_AddOrderMPIDPtr;

// 1.4.2 Order Executed with Price Message
// This is always sent regardless of if order is executed at same price or different price than the order.
typedef struct ITCH_OrderExecutedPrice : ITCH_Message {
  const char type = 'C';
  uint16_t stock_locate; // not used currently
  uint16_t tracking_number; // not used currently
  uint64_t timestamp;     // TODO: timestamp is 48 bits in reality
  uint64_t order_ref_num; // not used currently
  uint32_t executed_shares;
  uint64_t match_num;
  char printable;
  uint32_t execution_price;

  ITCH_OrderExecutedPrice() {
    ITCH_Message();
    ITCH_Message::otype = type;
  }
  ITCH_OrderExecutedPrice(uint16_t sl, uint16_t tn, uint64_t ts, uint64_t orn, uint32_t es, uint64_t mn, char p, uint32_t ep) :
    stock_locate(sl), tracking_number(tn), timestamp(ts), order_ref_num(orn), executed_shares(es), match_num(mn), printable(p), execution_price(ep) {
      ITCH_Message();
      ITCH_Message::otype = type;
    }
} ITCH_OrderExecutedPrice;
typedef std::unique_ptr<ITCH_OrderExecutedPrice> ITCH_OrderExecutedPriceUniquePtr;

// 1.4.3 Order Cancel Message
typedef struct ITCH_OrderCancel : ITCH_Message {
  const char type = 'X';
  uint16_t stock_locate;
  uint16_t tracking_number;
  uint64_t timestamp;     // TODO: timestamp is 48 bits in reality
  uint64_t order_ref_num;
  uint32_t canceled_shares;

  ITCH_OrderCancel() {
    ITCH_Message();
    ITCH_Message::otype = type;
  }
  ITCH_OrderCancel(uint16_t sl, uint16_t tn, uint64_t ts, uint64_t orn, uint32_t cs) :
    stock_locate(sl), tracking_number(tn), timestamp(ts), order_ref_num(orn), canceled_shares(cs) {
      ITCH_Message();
      ITCH_Message::otype = type;
    }
} ITCH_OrderCancel;
typedef std::unique_ptr<ITCH_OrderCancel> ITCH_OrderCancelUniquePtr;
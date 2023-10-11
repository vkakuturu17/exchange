#include <catch2/catch.hpp>
#include <iostream>
#include <string.h>
#include "struct/nasdaq_ouch.h"
#include "struct/nasdaq_itch.h"
#include "parser/parser.h"

using Exchange::OUCH_Parser;
using Exchange::ITCH_Parser;

// TODO: Don't use memcmp because it is not descriptive for wrong test output
TEST_CASE("Test parser OUCH Inbound", "[parser][ouch]") {
  SECTION("Order Entry Inbound") {
    unsigned char buffer[128];
    char type = 'O';
    char order_token[14] = {'1', '2', '3', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N'};
    char buy_sell_indicator = 'B';
    uint32_t shares = 100;
    char stock_sym[8] = {'A', 'P', 'P', 'L', ' ', ' ', ' ', ' '};
    uint32_t price = 1200;
    char mpid[4] = {'A', 'B', 'C', 'D'};

    memcpy(buffer, &type, 1);
    memcpy(buffer + 1, order_token, 14);
    memcpy(buffer + 15, &buy_sell_indicator, 1);
    memcpy(buffer + 16, &shares, 4);
    memcpy(buffer + 20, stock_sym, 8);
    memcpy(buffer + 28, &price, 4);
    memcpy(buffer + 36, &mpid, 4);

    InboundOrderPtr order = OUCH_Parser::parse_inbound(buffer);
    
    OrderEntry* order_entry = dynamic_cast<OrderEntry*>(order.get());
    REQUIRE(order_entry != nullptr);
    REQUIRE(order_entry->type == 'O');
    REQUIRE(memcmp(order_entry->order_token, order_token, 14) == 0);
    REQUIRE(order_entry->buy_sell_indicator == 'B');
    REQUIRE(order_entry->shares == 100);
    REQUIRE(memcmp(order_entry->stock_sym, stock_sym, 8) == 0);
    REQUIRE(order_entry->price == 1200);
    REQUIRE(memcmp(order_entry->mpid, mpid, 4) == 0);
  }

  SECTION("Order Cancel Inbound") {
    unsigned char buffer[128];
    char type = 'X';
    char mpid[4] = {'A', 'B', 'C', 'D'};
    char order_token[14] = {'1', '2', '3', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N'};
    uint32_t shares = 100;

    memcpy(buffer, &type, 1);
    memcpy(buffer + 1, order_token, 14);
    memcpy(buffer + 15, &shares, 4);
    memcpy(buffer + 19, mpid, 4);

    InboundOrderPtr order = OUCH_Parser::parse_inbound(buffer);

    OrderCancel* order_cancel = dynamic_cast<OrderCancel*>(order.get());
    REQUIRE(order_cancel != nullptr);
    REQUIRE(order_cancel->type == type);
    REQUIRE(memcmp(order_cancel->order_token, order_token, 14) == 0);
    REQUIRE(order_cancel->shares == shares);
    REQUIRE(memcmp(order_cancel->mpid, mpid, 4) == 0);
  }

  SECTION("Order Replace Inbound") {
    // TODO: Add test cases after implementing order replace
    REQUIRE(true);
  }

  SECTION("Order Modify Inbound") {
    // TODO: Add test cases after implementing order modify
    REQUIRE(true);
  }
}

TEST_CASE("Test parser OUCH Outbound", "[parser][ouch]") {
  SECTION("System Event Message Outbound") {
    SystemEventMessage message;
    message.timestamp = 123456789;
    message.event_code = 'S';

    unsigned char buffer[128];
    memset(buffer, 0, 128);
    SystemEventMessagePtr order_ptr = std::make_shared<SystemEventMessage>(message);
    OUCH_Parser::parse_outbound(order_ptr, buffer);

    REQUIRE(memcmp(buffer, "S", 1) == 0);
    REQUIRE(memcmp(buffer + 1, &message.timestamp, 8) == 0);
    REQUIRE(memcmp(buffer + 9, &message.event_code, 1) == 0);
  }

  SECTION("Order Entry Accepted Outbound") {
    OrderAcceptedOutbound message;
    message.timestamp = 123456789;
    unsigned char ort[14] = {'1', '2', '3', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N'};
    memcpy(message.order_token, ort, 14);
    message.buy_sell_indicator = 'B';
    message.shares = 100;
    unsigned char ssym[8] = {'A', 'P', 'P', 'L', ' ', ' ', ' ', ' '};
    memcpy(message.stock_sym, ssym, 8);
    message.price = 1200;
    unsigned char mpid[4] = {'A', 'B', 'C', 'D'};
    memcpy(message.mpid, mpid, 4);
    unsigned char orid[8] = {'1', '2', '3', '4', '5', '6', '7', '8'};
    memcpy(message.orderID, orid, 8);
    message.order_state = 'L';
    message.bbo_weight_indicator = 'N';

    unsigned char buffer[128];
    memset(buffer, 0, 128);
    OUCH_Parser::parse_outbound(std::make_shared<OrderAcceptedOutbound>(message), buffer);
    
    REQUIRE(memcmp(buffer, "A", 1) == 0);
    REQUIRE(memcmp(buffer + 1, &message.timestamp, 8) == 0);
    REQUIRE(memcmp(buffer + 9, &message.order_token, 14) == 0);
    REQUIRE(memcmp(buffer + 23, &message.buy_sell_indicator, 1) == 0);
    REQUIRE(memcmp(buffer + 24, &message.shares, 4) == 0);
    REQUIRE(memcmp(buffer + 28, &message.stock_sym, 4) == 0);
    REQUIRE(memcmp(buffer + 36, &message.price, 4) == 0);
    // time_in_force
    REQUIRE(memcmp(buffer + 44, &message.mpid, 4) == 0);
    // display
    REQUIRE(memcmp(buffer + 49, &message.orderID, 8) == 0);
    // capacitry, intermarket_sweep_eligibility, minimum_quantity, cross_type
    REQUIRE(memcmp(buffer + 64, &message.order_state, 1) == 0);
    REQUIRE(memcmp(buffer + 65, &message.bbo_weight_indicator, 1) == 0);
  }

  SECTION("Order Replaced Outbound") {
    REQUIRE(true);
  }

  SECTION("Order Cancel Outbound") {
    OrderCancelledOutbound message;
    message.timestamp = 123456789;
    unsigned char mpid[4] = {'A', 'B', 'C', 'D'};
    memcpy(message.mpid, mpid, 4);
    unsigned char ort[14] = {'1', '2', '3', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N'};
    memcpy(message.order_token, ort, 14);
    message.decrement_shares = 100;
    char reason = 'U';

    unsigned char buffer[128];
    memset(buffer, 0, 128);
    OUCH_Parser::parse_outbound(std::make_shared<OrderCancelledOutbound>(message), buffer);

    REQUIRE(memcmp(buffer, "C", 1) == 0);
    REQUIRE(memcmp(buffer + 1, &message.timestamp, 8) == 0);
    REQUIRE(memcmp(buffer + 9, &message.order_token, 14) == 0);
    REQUIRE(memcmp(buffer + 23, &message.decrement_shares, 4) == 0);
    REQUIRE(memcmp(buffer + 27, &message.reason, 1) == 0);
    REQUIRE(memcmp(buffer + 28, &message.mpid, 4) == 0);
  }

  SECTION("Order Executed Outbound") {
    OrderExecutedOutbound message;
    message.timestamp = 123456789;
    unsigned char mpid[4] = {'A', 'B', 'C', 'D'};
    memcpy(message.mpid, mpid, 4);
    unsigned char ort[14] = {'1', '2', '3', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N'};
    memcpy(message.order_token, ort, 14);
    message.executed_shares = 100;
    message.executed_price = 1200;
    message.liquidity_flag = 'A';
    message.match_number = 999;

    unsigned char buffer[128];
    memset(buffer, 0, 128);
    OUCH_Parser::parse_outbound(std::make_shared<OrderExecutedOutbound>(message), buffer);

    REQUIRE(memcmp(buffer, "E", 1) == 0);
    REQUIRE(memcmp(buffer + 1, &message.timestamp, 8) == 0);
    REQUIRE(memcmp(buffer + 9, &message.order_token, 14) == 0);
    REQUIRE(memcmp(buffer + 23, &message.executed_shares, 4) == 0);
    REQUIRE(memcmp(buffer + 27, &message.executed_price, 4) == 0);
    REQUIRE(memcmp(buffer + 31, &message.liquidity_flag, 1) == 0);
    REQUIRE(memcmp(buffer + 32, &message.match_number, 8) == 0);
    REQUIRE(memcmp(buffer + 40, &message.mpid, 4) == 0);
  }
}

TEST_CASE("Test parser ITCH outbound", "[parser][itch]") {
  SECTION("Add Order MPID") {
    ITCH_AddOrderMPID message;
    message.timestamp = 123456789;
    message.order_ref_num = 1010101010;
    message.buy_sell_indicator = 'B';
    message.shares = 100;
    unsigned char ssym[8] = {'A', 'P', 'P', 'L', ' ', ' ', ' ', ' '};
    memcpy(message.stock, ssym, 8);
    message.price = 1200;
    unsigned char mpid[4] = {'A', 'B', 'C', 'D'};
    memcpy(message.mpid, mpid, 4);

    unsigned char buffer[128];
    memset(buffer, 0, 128);
    ITCH_Parser::parse_ITCH_outbound(std::make_shared<ITCH_AddOrderMPID>(message), buffer);

    REQUIRE(memcmp(buffer, "F", 1) == 0);
    // stock locate, tracking number
    REQUIRE(memcmp(buffer + 5, &message.timestamp, 6+2) == 0); // +2 is for 8 byte timestamp instead of 6 byte in itch
    REQUIRE(memcmp(buffer + 11+2, &message.order_ref_num, 8) == 0);
    REQUIRE(memcmp(buffer + 19+2, &message.buy_sell_indicator, 1) == 0);
    REQUIRE(memcmp(buffer + 20+2, &message.shares, 4) == 0);
    REQUIRE(memcmp(buffer + 24+2, &message.stock, 8) == 0);
    REQUIRE(memcmp(buffer + 32+2, &message.price, 4) == 0);
    REQUIRE(memcmp(buffer + 36+2, &message.mpid, 4) == 0);
  }

  SECTION("Order Executed Price") {
    ITCH_OrderExecutedPrice message;
    message.timestamp = 123456789;
    message.order_ref_num = 1010101010;
    message.executed_shares = 100;
    message.match_num = 123456789;
    message.printable = 'Y';
    message.execution_price = 1200;

    unsigned char buffer[128];
    memset(buffer, 0, 128);
    ITCH_Parser::parse_ITCH_outbound(std::make_shared<ITCH_OrderExecutedPrice>(message), buffer);

    REQUIRE(memcmp(buffer, "C", 1) == 0);
    // stock locate, tracking number
    REQUIRE(memcmp(buffer + 5, &message.timestamp, 6+2) == 0); // +2 is for 8 byte timestamp instead of 6 byte in itch
    REQUIRE(memcmp(buffer + 11+2, &message.order_ref_num, 8) == 0);
    REQUIRE(memcmp(buffer + 19+2, &message.executed_shares, 4) == 0);
    REQUIRE(memcmp(buffer + 23+2, &message.match_num, 8) == 0);
    REQUIRE(memcmp(buffer + 31+2, &message.printable, 1) == 0);
    REQUIRE(memcmp(buffer + 32+2, &message.execution_price, 4) == 0);
  }

  SECTION("Order Cancel") {
    ITCH_OrderCancel message;
    message.timestamp = 123456789;
    message.order_ref_num = 1010101010;
    message.canceled_shares = 100;

    unsigned char buffer[128];
    memset(buffer, 0, 128);
    ITCH_Parser::parse_ITCH_outbound(std::make_shared<ITCH_OrderCancel>(message), buffer);

    REQUIRE(memcmp(buffer, "X", 1) == 0);
    // stock locate, tracking number
    REQUIRE(memcmp(buffer + 5, &message.timestamp, 6+2) == 0); // +2 is for 8 byte timestamp instead of 6 byte in itch
    REQUIRE(memcmp(buffer + 11+2, &message.order_ref_num, 8) == 0);
    REQUIRE(memcmp(buffer + 19+2, &message.canceled_shares, 4) == 0);
  }
}
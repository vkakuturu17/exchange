#pragma once
#include "struct/nasdaq_ouch.h"
#include "struct/nasdaq_itch.h"

// #define BUFFER_SIZE 128

namespace Exchange {

class Parser {
  public:
    Parser();

};

class OUCH_Parser : Parser {
  public:
    static InboundOrderPtr parse_inbound(unsigned char* msg); // Inbound
    static void parse_outbound(OutboundOrderPtr order, unsigned char buffer[]); // Outbound

    static void parse_OUCH_inbound_into_buffer(InboundOrderPtr order, unsigned char buffer[]); // Inbound

  private:
    // Inbound orders to Exchange
    static OrderEntry _parse_order_entry(unsigned char* msg);
    static OrderCancel _parse_order_cancel(unsigned char* msg);
    // OrderReplace _parse_order_replace(char* arr);
    // OrderModify _parse_order_modify(char* arr);

    static void _parse_order_entry_into_buffer(OrderEntry& order, unsigned char buffer[]);

    // Outbound from Exchange to Market Participant
    static void _parse_system_event_message(SystemEventMessage& msg, unsigned char buffer[]);
    static void _parse_order_accepted_outbound(OrderAcceptedOutbound& msg, unsigned char buffer[]);
    static void _parse_order_replaced_outbound(OrderReplacedOutbound& msg, unsigned char buffer[]);
    static void _parse_order_cancelled_outbound(OrderCancelledOutbound& msg, unsigned char buffer[]);
    static void _parse_order_executed_outbound(OrderExecutedOutbound& msg, unsigned char buffer[]);
    static void _parse_rejected_message_outbound(RejectedMessageOutbound& msg, unsigned char buffer[]);
    static void _parse_cancel_reject_outbound(CancelRejectOutbound& msg, unsigned char buffer[]);
    static void _parse_order_priority_update_outbound(OrderPriorityUpdateOutbound& msg, unsigned char buffer[]);
    // TODO: Add more message types (see nasdaq_ouch.h)
};

class ITCH_Parser : Parser {
  public:
    static void parse_ITCH_outbound(ITCH_MessagePtr msg, unsigned char buffer[]); // Outbound

  private:
    static void _parse_ITCH_add_order(ITCH_AddOrderMPID& msg, unsigned char buffer[]);
    static void _parse_ITCH_order_executed_price(ITCH_OrderExecutedPrice& msg, unsigned char buffer[]);
    static void _parse_ITCH_order_cancel(ITCH_OrderCancel& msg, unsigned char buffer[]);
    // TODO: Add more message types (see nasdaq_itch.h)
};

}
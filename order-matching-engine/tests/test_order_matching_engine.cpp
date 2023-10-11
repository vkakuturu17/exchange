#include <catch2/catch.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "order_matching_engine/order_matching_engine.h"
#include "parser/parser.h"

using std::cout;
using std::endl;

using Exchange::SimpleMachineTimer;
using Exchange::OrderMatchingEngine;
using Exchange::OUCH_Parser;
using Exchange::ITCH_Parser;

using std::vector;
using std::string;

vector<string> AAPL_VEC() {
  vector<string> vec;
  vec.push_back("AAPL");
  return vec;
}

/**
 * @brief Test the OrderMatchingEngine::add_inbound_packet_to_queue function
 * @details Note that for a lot of these tests we have to use the IMPL (implementation) functions
 * because the regular functions are run behind a while(true) loop. 
*/
SCENARIO("OrderMatchingEngine::add_inbound_packet_to_queue", "[ome][test]") {
  //  TODO: Finish test case
  GIVEN("A new OrderMatchingEngine") {
    SimpleMachineTimer timer;
    OrderMatchingEngine ome(timer.get_instance());
    ome.load_orderbook_symbols(AAPL_VEC());

    // WHEN("A new packet is added to the inbound queue") {
    //   THEN("The packet is added to OUCH_inbound_buffer_queue_") {
    //     REQUIRE(ome.OUCH_inbound_buffer_queue_.size() == 0);
    //     ome.add_inbound_packet_to_queue(ome.OUCH_inbound_buffer_queue_);
    //     REQUIRE(ome.OUCH_inbound_buffer_queue_.size() == 1);
    //   }
    // }
    SimpleMachineTimer::destroy_instance();
  }
}

SCENARIO("OrderMatchingEngine::convert_OUCH_inbound_packet_to_order", "[ome]") {
  GIVEN("A new OrderMatchingEngine") {
    SimpleMachineTimer timer;
    OrderMatchingEngine ome(timer.get_instance());
    ome.load_orderbook_symbols(AAPL_VEC());

    WHEN("An Order Entry packet is received") {
      // Make a order entry packet
      unsigned char* packet = new unsigned char[100];
      OrderEntry order_entry("0001", 'B', 1000, "AAPL", 100, -1, "Behman", 'Y');
      OUCH_Parser::parse_OUCH_inbound_into_buffer(std::make_shared<OrderEntry>(order_entry), packet);

      ome.OUCH_inbound_buffer_queue_.push(packet);

      WHEN("Function is called to process next packet and process to correct order") {
        THEN("Require queue sizes beforehand are correct") {
          REQUIRE(ome.OUCH_inbound_order_queue_.size() == 0);
          REQUIRE(ome.OUCH_inbound_buffer_queue_.size() == 1);
        }
        
        // Function call here
        ome.convert_OUCH_inbound_packet_to_order_IMPL(ome.OUCH_inbound_buffer_queue_, ome.OUCH_inbound_order_queue_);
        
        THEN("Require queue sizes after parsing are correct") {
          REQUIRE(ome.OUCH_inbound_order_queue_.size() == 1);
          REQUIRE(ome.OUCH_inbound_buffer_queue_.size() == 0);
        }

        THEN("Assert that order was parsed correctly") {
          InboundOrderPtr order = *ome.OUCH_inbound_order_queue_.front();
          REQUIRE(order->otype == 'O');
          OrderEntry* order_entry_ptr = dynamic_cast<OrderEntry*>(order.get());

          REQUIRE(memcmp(order_entry_ptr->stock_sym, "AAPL", 4) == 0);
          REQUIRE(order_entry_ptr->buy_sell_indicator == 'B');
          REQUIRE(order_entry_ptr->shares == 1000);
          REQUIRE(order_entry_ptr->price == 100);
          REQUIRE(memcmp(order_entry_ptr->order_token, "0001", 4) == 0);
          REQUIRE(memcmp(order_entry_ptr->mpid, "Behman", 4) == 0);
        }
        
      }
    }
    SimpleMachineTimer::destroy_instance();
  }
}

SCENARIO("OrderMatchingEngine::add_inbound_order_to_orderbook", "[ome]") {
  GIVEN("A new OrderMatchingEngine") {
    SimpleMachineTimer timer;
    OrderMatchingEngine ome(timer.get_instance());
    ome.load_orderbook_symbols(AAPL_VEC());

    WHEN("A new Order Entry InboundOrder is received") {
      OrderEntry order_entry("0001", 'B', 1000, "AAPL", 100, -1, "Behman", 'Y'); 
      InboundOrderPtr order = std::make_shared<OrderEntry>(order_entry);

      ome.OUCH_inbound_order_queue_.push(order);
      REQUIRE(ome.OUCH_inbound_order_queue_.size() == 1);

      using Exchange::OrderBook;
      OrderBookPtr orderbook = ome.get_orderbook("AAPL");
      assert(orderbook != nullptr);
      REQUIRE(orderbook->get_stock_symbol() == "AAPL");

      WHEN("The order is added to the orderbook [AAPL]") {
        THEN("Check to make sure orderbook queues are empty") {
          REQUIRE(orderbook->get_num_OUCH_inbound_queue() == 0);
          REQUIRE(orderbook->get_num_OUCH_outbound_queue() == 0);
          REQUIRE(orderbook->get_num_ITCH_outbound_queue() == 0);
        }

        THEN("Check to make sure OrderMatchingEngine queue sizes are correct") {
          REQUIRE(ome.OUCH_inbound_order_queue_.size() == 1);
          REQUIRE(ome.orderbook_symbol_to_execute_queue_.size() == 0);
        }

        // Function call here
        ome.add_inbound_order_to_orderbook_IMPL(ome.OUCH_inbound_order_queue_, ome.orderbook_symbol_to_execute_queue_);
        
        THEN("Check to make sure order is in Orderbook OUCH inbound queue"){
          REQUIRE(orderbook->get_num_OUCH_inbound_queue() == 1);
          REQUIRE(orderbook->get_num_OUCH_outbound_queue() == 0);
          REQUIRE(orderbook->get_num_ITCH_outbound_queue() == 0);
        }

        THEN("Check to make sure OrderMatchingEngine queues were updated") {
          REQUIRE(ome.OUCH_inbound_order_queue_.size() == 0);
          REQUIRE(ome.orderbook_symbol_to_execute_queue_.size() == 1);
        }
      }
    }

    SimpleMachineTimer::destroy_instance();
  }
}

SCENARIO("OrderMatchingEngine::execute_orderbook_next", "[ome]") {
  GIVEN("A new OrderMatchingEngine") {
    SimpleMachineTimer timer;
    OrderMatchingEngine ome(timer.get_instance());
    ome.load_orderbook_symbols(AAPL_VEC());

    GIVEN("An OrderEntry in Orderbook [AAPL] OUCH Inbound queue") {
      using Exchange::OrderBook;
      OrderBookPtr orderbook = ome.get_orderbook("AAPL");
      REQUIRE(orderbook->get_stock_symbol() == "AAPL");
      
      // Add order to orderbook and queue it inside OME
      OrderEntry order_entry("0001", 'B', 1000, "AAPL", 100, -1, "Behman", 'Y');
      orderbook->add_order(std::make_shared<OrderEntry>(order_entry));
      REQUIRE(orderbook->get_num_OUCH_inbound_queue() == 1);
      ome.orderbook_symbol_to_execute_queue_.push("AAPL");
      REQUIRE(ome.orderbook_symbol_to_execute_queue_.size() == 1);

      WHEN("Execute the order in the orderbook_symbol_to_execute_queue") {
        // Function call here
        ome.execute_orderbook_next_IMPL(ome.orderbook_symbol_to_execute_queue_, \
                                        ome.orderbook_symbol_OUCH_to_output_queue_, \
                                        ome.orderbook_symbol_ITCH_to_output_queue_);

        THEN("Check to make sure orderbook queue sizes are correct") {
          REQUIRE(orderbook->get_num_OUCH_inbound_queue() == 0);
          REQUIRE(orderbook->get_num_OUCH_outbound_queue() > 0);
          REQUIRE(orderbook->get_num_ITCH_outbound_queue() > 0);
        }

        THEN("Check to make sure OrderMatchingEngine queues were updated") {
          REQUIRE(ome.orderbook_symbol_to_execute_queue_.size() == 0);
          REQUIRE(ome.orderbook_symbol_OUCH_to_output_queue_.size() == 1);
          REQUIRE(ome.orderbook_symbol_ITCH_to_output_queue_.size() == 1);
        }
      }
    }

    SimpleMachineTimer::destroy_instance();
  }
}

SCENARIO("OrderMatchingEngine::grab_OUCH_outbound_from_orderbook", "[ome]") {
  GIVEN("A new OrderMatchingEngine") {
    SimpleMachineTimer timer;
    OrderMatchingEngine ome(timer.get_instance());
    ome.load_orderbook_symbols(AAPL_VEC());

    GIVEN("An OrderEntry in Orderbook [AAPL] OUCH Inbound queue") {
      using Exchange::OrderBook;
      OrderBookPtr orderbook = ome.get_orderbook("AAPL");
      REQUIRE(orderbook->get_stock_symbol() == "AAPL");
      
      // Add order to orderbook and queue it inside OME
      OrderEntry order_entry("0001", 'B', 1000, "AAPL", 100, -1, "Behman", 'Y');
      orderbook->add_order(std::make_shared<OrderEntry>(order_entry));
      orderbook->process_next_order();

      // Setup OrderMatchingEngine to be state right before function call
      ome.orderbook_symbol_OUCH_to_output_queue_.push("AAPL");
      ome.orderbook_symbol_ITCH_to_output_queue_.push("AAPL");

      WHEN("Grab OUCH outbound(s) from Orderbook") {
        // Function call here
        ome.grab_OUCH_outbound_from_orderbook_IMPL(ome.orderbook_symbol_OUCH_to_output_queue_, \
                                                    ome.OUCH_outbound_order_queue_);

        THEN("Check to make sure orderbook queue sizes are correct") {
          REQUIRE(orderbook->get_num_OUCH_inbound_queue() == 0);
          REQUIRE(orderbook->get_num_OUCH_outbound_queue() == 0);
          REQUIRE(orderbook->get_num_ITCH_outbound_queue() > 0);
        }

        THEN("Check to make sure OrderMatchingEngine queues were updated") {
          REQUIRE(ome.orderbook_symbol_to_execute_queue_.size() == 0);
          REQUIRE(ome.orderbook_symbol_OUCH_to_output_queue_.size() == 0);
          REQUIRE(ome.orderbook_symbol_ITCH_to_output_queue_.size() == 1);
          REQUIRE(ome.OUCH_outbound_order_queue_.size() == 1);
        }
        
        THEN("Check to make sure OUCH outbound queue was populated with non-faulty order (only check order type)") {
          OutboundOrderPtr order_ptr = *ome.OUCH_outbound_order_queue_.front();
          REQUIRE(order_ptr->otype == 'A');
        }
      }
    }

    SimpleMachineTimer::destroy_instance();
  }
}

SCENARIO("OrderMatchingEngine::grab_ITCH_outbound_from_orderbook", "[ome]") {
  GIVEN("A new OrderMatchingEngine") {
    SimpleMachineTimer timer;
    OrderMatchingEngine ome(timer.get_instance());
    ome.load_orderbook_symbols(AAPL_VEC());

    GIVEN("An OrderEntry in Orderbook [AAPL] OUCH Inbound queue") {
      using Exchange::OrderBook;
      OrderBookPtr orderbook = ome.get_orderbook("AAPL");
      REQUIRE(orderbook->get_stock_symbol() == "AAPL");
      
      // Add order to orderbook and queue it inside OME
      OrderEntry order_entry("0001", 'B', 1000, "AAPL", 100, -1, "Behman", 'Y');
      orderbook->add_order(std::make_shared<OrderEntry>(order_entry));
      orderbook->process_next_order();

      // Setup OrderMatchingEngine to be state right before function call
      ome.orderbook_symbol_OUCH_to_output_queue_.push("AAPL");
      ome.orderbook_symbol_ITCH_to_output_queue_.push("AAPL");

      WHEN("Grab ITCH outbound(s) from Orderbook") {
        // Function call here
        ome.grab_ITCH_outbound_from_orderbook_IMPL(ome.orderbook_symbol_ITCH_to_output_queue_, \
                                                    ome.ITCH_outbound_order_queue_);

        THEN("Check to make sure orderbook queue sizes are correct") {
          REQUIRE(orderbook->get_num_OUCH_inbound_queue() == 0);
          REQUIRE(orderbook->get_num_OUCH_outbound_queue() > 0);
          REQUIRE(orderbook->get_num_ITCH_outbound_queue() == 0);
        }

        THEN("Check to make sure OrderMatchingEngine queues were updated") {
          REQUIRE(ome.orderbook_symbol_to_execute_queue_.size() == 0);
          REQUIRE(ome.orderbook_symbol_OUCH_to_output_queue_.size() == 1);
          REQUIRE(ome.orderbook_symbol_ITCH_to_output_queue_.size() == 0);
          REQUIRE(ome.ITCH_outbound_order_queue_.size() == 1);
        }
        
        THEN("Check to make sure OUCH outbound queue was populated with non-faulty order (only check order type)") {
          ITCH_MessagePtr order_ptr = *ome.ITCH_outbound_order_queue_.front();
          REQUIRE(order_ptr->otype == 'F'); // ITCH_AddOrderMPID
        }
      }
    }

    SimpleMachineTimer::destroy_instance();
  }
}

SCENARIO("OrderMatchingEngine::convert_OUCH_outbound_order_to_packet", "[ome]") {
  GIVEN("A new OrderMatchingEngine") {
    SimpleMachineTimer timer;
    OrderMatchingEngine ome(timer.get_instance());
    ome.load_orderbook_symbols(AAPL_VEC());

    GIVEN("An OrderAcceptedOutbound in OUCH_outbound_order_queue_") {
      OrderAcceptedOutbound order_accepted(1234567890, "0001", 'B', 1000, "AAPL", 100, -1, "Behman", '\0', "orderID", 'L', 'N');
      ome.OUCH_outbound_order_queue_.push(std::make_shared<OrderAcceptedOutbound>(order_accepted));

      WHEN("Convert OUCH outbound order to packet") {
        // Function call here
        ome.convert_OUCH_outbound_order_to_packet_IMPL(ome.OUCH_outbound_order_queue_, \
                                                        ome.OUCH_outbound_buffer_queue_);

        THEN("Check to make sure OUCH outbound order queue was updated") {
          REQUIRE(ome.OUCH_outbound_order_queue_.size() == 0);
        }

        THEN("Check to make sure OUCH outbound packet queue was updated") {
          REQUIRE(ome.OUCH_outbound_buffer_queue_.size() == 1);
        }

        THEN("Check to make sure OUCH outbound packet queue was populated with non-faulty packet") {
          unsigned char* packet = *ome.OUCH_outbound_buffer_queue_.front();
          REQUIRE(packet[0] == 'A');
          
          unsigned char* expected_packet = new unsigned char[66];
          memset(expected_packet, 0, 66);
          OUCH_Parser::parse_outbound(std::make_shared<OrderAcceptedOutbound>(order_accepted), expected_packet);
          REQUIRE(memcmp(packet, expected_packet, 66) == 0);
          delete[] expected_packet;
        }
      }
    }

    SimpleMachineTimer::destroy_instance();
  }
}

SCENARIO("OrderMatchingEngine::send_outbound_OUCH_packet", "[ome]") {
  //  TODO: Finish test case
  GIVEN("A new OrderMatchingEngine") {
    SimpleMachineTimer timer;
    OrderMatchingEngine ome(timer.get_instance());
    ome.load_orderbook_symbols(AAPL_VEC());

    // TODO: Add test cases here
    SimpleMachineTimer::destroy_instance();
  }
}

SCENARIO("OrderMatchingEngine::convert_ITCH_outbound_order_to_packet", "[ome]") {
  GIVEN("A new OrderMatchingEngine") {
    SimpleMachineTimer timer;
    OrderMatchingEngine ome(timer.get_instance());
    ome.load_orderbook_symbols(AAPL_VEC());

    GIVEN("An ITCH_Message in ITCH_outbound_order_queue_") {
      ITCH_AddOrderMPID ITCH_add_order(0000000000, 10001, 1234567890, 10001, 'B', 1000, "AAPL", 100, "Behman");
      ome.ITCH_outbound_order_queue_.push(std::make_shared<ITCH_AddOrderMPID>(ITCH_add_order));

      WHEN("Convert ITCH outbound order to packet") {
        // Function call here
        ome.convert_ITCH_outbound_order_to_packet_IMPL(ome.ITCH_outbound_order_queue_, \
                                                        ome.ITCH_outbound_buffer_queue_);

        THEN("Check to make sure OUCH outbound order queue was updated") {
          REQUIRE(ome.ITCH_outbound_order_queue_.size() == 0);
        }

        THEN("Check to make sure OUCH outbound packet queue was updated") {
          REQUIRE(ome.ITCH_outbound_buffer_queue_.size() == 1);
        }

        THEN("Check to make sure OUCH outbound packet queue was populated with non-faulty packet") {
          unsigned char* packet = *ome.ITCH_outbound_buffer_queue_.front();
          REQUIRE(packet[0] == 'F');
          
          unsigned char* expected_packet = new unsigned char[40+2];
          memset(expected_packet, 0, 40+2);
          ITCH_Parser::parse_ITCH_outbound(std::make_shared<ITCH_AddOrderMPID>(ITCH_add_order), expected_packet);
          REQUIRE(memcmp(packet, expected_packet, 40+2) == 0);
          delete[] expected_packet;
        }
      }
    }

    SimpleMachineTimer::destroy_instance();
  }
}

SCENARIO("OrderMatchingEngine::send_outbound_ITCH_packet", "[ome]") {
  //  TODO: Finish test case
  GIVEN("A new OrderMatchingEngine") {
    SimpleMachineTimer timer;
    OrderMatchingEngine ome(timer.get_instance());
    ome.load_orderbook_symbols(AAPL_VEC());

    // TODO: Add test cases here
    SimpleMachineTimer::destroy_instance();
  }
}
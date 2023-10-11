#include <catch2/catch.hpp>
#include "orderbook/orderbook.h"


using Exchange::OrderBook;
using Exchange::SimpleMachineTimer;

TEST_CASE("Orderbook::get_stock_symbol", "[orderbook]") {
  SimpleMachineTimer timer;
  OrderBook orderbook("AAPL", timer.get_instance());
  REQUIRE(orderbook.get_stock_symbol() == "AAPL");
} // TODO: Add more unit test for each public function in OrderBook

// Behavior driven development
// https://github.com/catchorg/Catch2/blob/devel/docs/test-cases-and-sections.md
SCENARIO("Orderbook Buy", "[orderbook][buy]") {
  SimpleMachineTimer timer;
  GIVEN("An empty orderbook") {
    OrderBook orderbook("AAPL", SimpleMachineTimer::get_instance());
    WHEN("A B100 @ $99 order is added") {
      OrderEntry order1("AAPL0001", 'B', 100, "AAPL", 99, -1, "Goldman", 'Y');
      orderbook.add_order(std::make_shared<OrderEntry>(order1));
      THEN("The orderbook should have one order in the inbound queue") {
        REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 1);
      }

      WHEN("The order is processed") {
        orderbook.process_next_order();
        THEN("The orderbook should have no orders in the inbound queue") {
          REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 0);
        }
        THEN("The orderbook should have a new order in the outbound queue") {
          REQUIRE(orderbook.get_num_OUCH_outbound_queue() == 1);
        }
        THEN("The orderbook should have a max bid price of 99") {
          REQUIRE(orderbook.get_max_bid_price() == 99);
        }
        THEN("The orderbook should add an order to ITCH outbound queue") {
          REQUIRE(orderbook.get_num_ITCH_outbound_queue() == 1);
        }

        WHEN("Another B100 @ $99 order at same price level comes in") {
          OrderEntry order2("AAPL0002", 'B', 100, "AAPL", 99, -1, "Goldman", 'Y');
          orderbook.add_order(std::make_shared<OrderEntry>(order2));
          THEN("The orderbook should have one order in the inbound queue") {
            REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 1);
          }

          WHEN("The 2nd order is processed") {
            orderbook.process_next_order();
            THEN("The orderbook should have a max bid price of 99") {
              REQUIRE(orderbook.get_max_bid_price() == 99);
            }
            THEN("The orderbook should have a new order in the outbound queue") {
              REQUIRE(orderbook.get_num_OUCH_outbound_queue() == 2);
            }
            THEN("The orderbook should add an order to ITCH outbound queue") {
              REQUIRE(orderbook.get_num_ITCH_outbound_queue() == 2);
            }
          }
        }

        WHEN("Another B100 @ $98 order at lower price level comes in") {
          OrderEntry order2("AAPL0002", 'B', 100, "AAPL", 98, -1, "Goldman", 'Y');
          orderbook.add_order(std::make_shared<OrderEntry>(order2));
          THEN("The orderbook should have one order in the inbound queue") {
            REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 1);
          }

          WHEN("The 2nd order is processed") {
            orderbook.process_next_order();
            THEN("The orderbook should have a max bid price of 99") {
              REQUIRE(orderbook.get_max_bid_price() == 99);
            }

            THEN("The orderbook should add an order to ITCH outbound queue") {
              REQUIRE(orderbook.get_num_ITCH_outbound_queue() == 2);
            }
          }
        }

        WHEN("Another B100 @ $100 order at higher price level comes in") {
          OrderEntry order2("AAPL0002", 'B', 100, "AAPL", 100, -1, "Goldman", 'Y');
          orderbook.add_order(std::make_shared<OrderEntry>(order2));
          THEN("The orderbook should have one order in the inbound queue") {
            REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 1);
          }

          WHEN("The 2nd order is processed") {
            orderbook.process_next_order();
            THEN("The orderbook should have a max bid price of 99") {
              REQUIRE(orderbook.get_max_bid_price() == 100);
            }
              
            THEN("ITCH outbound: 2 Order Accepted Messages") {
              REQUIRE(orderbook.get_num_ITCH_outbound_queue() == 2);
            } 
          }
        }
      }
    }

    WHEN("A S100 @ $100 order is added") {
      OrderEntry order1("AAPL0001", 'S', 100, "AAPL", 100, -1, "Goldman", 'Y');
      orderbook.add_order(std::make_shared<OrderEntry>(order1));
      THEN("The orderbook should have one order in the inbound queue") {
        REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 1);
      }

      WHEN("The order is processed") {
        orderbook.process_next_order();
        THEN("The orderbook should have no orders in the inbound queue") {
          REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 0);
        }
        THEN("The orderbook should have a new order in the outbound queue") {
          REQUIRE(orderbook.get_num_OUCH_outbound_queue() == 1);
        }
        THEN("The orderbook should have a min ask price of 99") {
          REQUIRE(orderbook.get_min_ask_price() == 100);
        }
        THEN("The orderbook should add an order to ITCH outbound queue") {
          REQUIRE(orderbook.get_num_ITCH_outbound_queue() == 1);
        }

        WHEN("A B100 @ $100 order at same sell order price level comes in") {
          OrderEntry order2("AAPL0002", 'B', 100, "AAPL", 100, -1, "Goldman", 'Y');
          orderbook.add_order(std::make_shared<OrderEntry>(order2));
          THEN("The orderbook should have one order in the inbound queue") {
            REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 1);
          }

          WHEN("The 2nd order is processed") {
            orderbook.process_next_order();
            THEN("Nothing else should be in the inbound queue") {
              REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 0);
            }
            THEN("The orderbook should have a min ask price of -1") {
              REQUIRE(orderbook.get_min_ask_price() == (uint32_t)(-1));
            }
            THEN("ITCH outbound: 2 Order Executed Messages") {
              REQUIRE(orderbook.get_num_OUCH_outbound_queue() == 4);
            }
          }
        }
      }
    }
  }

  SimpleMachineTimer::destroy_instance();
}

SCENARIO("Orderbook Sell", "[orderbook][sell]") {
  SimpleMachineTimer timer;
  GIVEN("An empty orderbook") {
    OrderBook orderbook("AAPL", SimpleMachineTimer::get_instance());

    THEN("The orderbook should have no orders in ANY queue") {
      REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 0);
      REQUIRE(orderbook.get_num_OUCH_outbound_queue() == 0);
      REQUIRE(orderbook.get_num_ITCH_outbound_queue() == 0);
    }

    WHEN("A sell order is added") {
      OrderEntry order1("AAPL0001", 'S', 100, "AAPL", 101, -1, "Goldman", 'Y');
      orderbook.add_order(std::make_shared<OrderEntry>(order1));
      THEN("The orderbook should have one order in the inbound queue") {
        REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 1);
      }

      WHEN("The order is processed") {
        orderbook.process_next_order();
        THEN("The orderbook should have no orders in the inbound queue") {
          REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 0);
        }
        THEN("The orderbook should have a new order in the outbound queue") {
          REQUIRE(orderbook.get_num_OUCH_outbound_queue() == 1);
        }
        THEN("The orderbook should have a min ask price of 99") {
          REQUIRE(orderbook.get_min_ask_price() == 101);
        }
        THEN("ITCH outbound: Send order accepted") {
          REQUIRE(orderbook.get_num_ITCH_outbound_queue() == 1);
        }

        WHEN("Another sell order at same price level comes in") {
          OrderEntry order2("AAPL0002", 'S', 100, "AAPL", 101, -1, "Goldman", 'Y');
          orderbook.add_order(std::make_shared<OrderEntry>(order2));
          THEN("The orderbook should have one order in the inbound queue") {
            REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 1);
          }

          WHEN("The 2nd order is processed") {
            orderbook.process_next_order();
            THEN("The orderbook should have a min ask price of 101") {
              REQUIRE(orderbook.get_min_ask_price() == 101);
            }

            THEN("ITCH outbound: Send order accepted") {
              REQUIRE(orderbook.get_num_ITCH_outbound_queue() == 2);
            }
          }
        }

        WHEN("Another sell order at lower price level comes in") {
          OrderEntry order2("AAPL0002", 'S', 100, "AAPL", 100, -1, "Goldman", 'Y');
          orderbook.add_order(std::make_shared<OrderEntry>(order2));
          THEN("The orderbook should have one order in the inbound queue") {
            REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 1);
          }

          WHEN("The 2nd order is processed") {
            orderbook.process_next_order();
            THEN("The orderbook should have a min ask price of 99") {
              REQUIRE(orderbook.get_min_ask_price() == 100);
            }

            THEN("ITCH outbound: Send order accepted") {
              REQUIRE(orderbook.get_num_ITCH_outbound_queue() == 2);
            }
          }
        }

        WHEN("Another sell order at higher price level comes in") {
          OrderEntry order2("AAPL0002", 'S', 100, "AAPL", 102, -1, "Goldman", 'Y');
          orderbook.add_order(std::make_shared<OrderEntry>(order2));
          THEN("The orderbook should have one order in the inbound queue") {
            REQUIRE(orderbook.get_num_OUCH_inbound_queue() == 1);
          }

          WHEN("The 2nd order is processed") {
            orderbook.process_next_order();
            THEN("The orderbook should have a min ask price of 99") {
              REQUIRE(orderbook.get_min_ask_price() == 101);
            }

            THEN("ITCH outbound: Send order accepted") {
              REQUIRE(orderbook.get_num_ITCH_outbound_queue() == 2);
            }
          }
        }
      }
    }
  }

  SimpleMachineTimer::destroy_instance();
}
# Order Book

This project implements an **Order Book** system for managing buy and sell orders in a financial market. The system supports different types of orders and provides functionalities to add, cancel, and match orders.

## Key Components

### Enums

- **OrderType**: Defines the type of order.
  - `GoodTillCancelled`
  - `FillAndKill`

- **Side**: Defines the side of the order.
  - `Buy`
  - `Sell`

### Type Aliases

- **Price**: Represents the price of an order.
- **Quantity**: Represents the quantity of an order.
- **OrderId**: Represents the unique identifier for an order.

### Structs

- **LevelInfo**: Contains information about a price level.
  - `price_`: The price at this level.
  - `quantity_`: The total quantity at this level.

- **TradeInfo**: Contains information about a trade.
  - `orderId_`: The ID of the order involved in the trade.
  - `price_`: The price at which the trade occurred.
  - `quantity_`: The quantity traded.

### Classes

- **OrderbookLevelInfos**: Holds bid and ask level information.
  - `GetBids()`: Returns bid levels.
  - `GetAsks()`: Returns ask levels.

- **Order**: Represents an individual order.
  - `GetOrderId()`: Returns the order ID.
  - `GetSide()`: Returns the side of the order.
  - `GetPrice()`: Returns the price of the order.
  - `GetOrderType()`: Returns the type of the order.
  - `GetInitialQuantity()`: Returns the initial quantity of the order.
  - `GetRemainingQuantity()`: Returns the remaining quantity of the order.
  - `GetFilledQuantity()`: Returns the filled quantity of the order.
  - `IsFilled()`: Checks if the order is completely filled.
  - `Fill(quantity)`: Fills the order with the specified quantity.

- **OrderModify**: Represents a modification to an order.
  - `GetOrderId()`: Returns the order ID.
  - `GetSide()`: Returns the side of the order.
  - `GetPrice()`: Returns the price of the order.
  - `GetQuantity()`: Returns the quantity of the order.
  - `ToOrderPointer(type)`: Converts to an `OrderPointer`.

- **Trade**: Represents a trade between a bid and an ask order.
  - `GetBidTrade()`: Returns the bid trade information.
  - `GetAskTrade()`: Returns the ask trade information.

- **Orderbook**: Manages the collection of orders and handles matching.
  - `AddOrder(order)`: Adds a new order to the order book.
  - `CancelOrder(orderId)`: Cancels an existing order.
  - `MatchOrder(order)`: Matches a modified order.
  - `Size()`: Returns the number of orders in the order book.
  - `GetOrderInfos()`: Returns the bid and ask level information.

## Example Usage

```cpp
int main(){
    Orderbook orderbook;
    const OrderId orderId = 1;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancelled, orderId, Side::Buy, 100, 10));
    orderbook.CancelOrder(orderId);
    std::cout << orderbook.Size() << std::endl;
    return 0;
}
```

This example demonstrates how to create an order book, add an order, cancel it, and print the size of the order book.

## Dependencies

- **C++ Standard Library**: Includes headers such as `<iostream>`, `<map>`, `<vector>`, etc.

## Compilation

To compile the project, use a C++ compiler that supports C++17 or later. For example, using `g++`:

```sh
g++ -std=c++17 -o orderbook orderbook1.cpp
```

## License

This project is licensed under the MIT License.

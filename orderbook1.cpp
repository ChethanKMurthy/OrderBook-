#include<iostream>
#include<map>
#include<set>
#include<list>
#include<queue>
#include<deque>
#include<stack>
#include<vector>
#include<cmath>
#include<ctime>
#include<limits>
#include<string>
#include<numeric>
#include<algorithm>
#include<unordered_map>
#include<memory>
#include<variant>
#include<optional>
#include<tuple>
#include<format>
#include <cstdint>

enum class OrderType{
    GoodTillCancelled,
    FillAndKill
};

enum class Side{
    Buy,
    Sell
};

using Price = std::int32_t;
using Quantity = std::int32_t;
using OrderId = std::uint64_t;

struct LevelInfo{
    Price price_;
    Quantity quantity_;
};

using LevelInfos=std::vector<LevelInfo>;

class OrderbookLevelInfos{
    public:
        OrderbookLevelInfos(const LevelInfos& bids , const LevelInfos& asks)
            :bids_{bids},
            asks_{asks}
        {}
        const LevelInfos& GetBids() const{return bids_;}
        const LevelInfos& GetAsks() const{return asks_;}

    private:
        LevelInfos bids_;
        LevelInfos asks_;
};

class Order{
    public:
        Order(OrderType orderType, OrderId orderid, Side side, Price price , Quantity quantity )
            :orderType_{orderType},
            orderid_{orderid},
            side_{side},
            price_{price},
            initialQuantity_{quantity},
            remainingQuantity_{quantity}

        {}
        OrderId GetOrderId() const {return orderid_;}
        Side GetSide() const {return side_;}
        Price GetPrice() const {return price_;}
        OrderType GetOrderType() const {return orderType_;}
        Quantity GetInitialQuantity() const {return initialQuantity_;}
        Quantity GetRemainingQuantity() const {return remainingQuantity_;}
        Quantity GetFilledQuantity() const {return initialQuantity_ - remainingQuantity_;}
        bool IsFilled() const {return remainingQuantity_ == 0;}
        void Fill(Quantity quantity){
            if(quantity > GetRemainingQuantity()){
                throw std::logic_error("order(" + std::to_string(GetOrderId()) + ") cannot be filled for more than its remaining quantity");
            }
            remainingQuantity_ -= quantity;
        }
    private:
        OrderType orderType_;
        OrderId orderid_;
        Side side_;
        Price price_;
        Quantity initialQuantity_;
        Quantity remainingQuantity_;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;

class OrderModify
{
public:
       OrderModify(OrderId OrderId, Side side, Price price, Quantity quantity)
        :OrderId_{OrderId},
        side_{side},
        price_{price},
        quantity_{quantity}
    {} 

    OrderId GetOrderId() const {return OrderId_;}
    Side GetSide() const {return side_;}        
    Price GetPrice() const {return price_;}
    Quantity GetQuantity() const {return quantity_;}

    OrderPointer ToOrderPointer(OrderType type) const {
        return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
    }
private:
    OrderId OrderId_;
    Side side_;
    Price price_;
    Quantity quantity_;
};

struct TradeInfo{
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
};

class Trade
{
public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
        :bidTrade_{bidTrade},
        askTrade_{askTrade}
        
    {}

    const TradeInfo& GetBidTrade() const{return bidTrade_;}
    const TradeInfo& GetAskTrade() const{return askTrade_;}


private:
    TradeInfo bidTrade_;
    TradeInfo askTrade_;
};

using Trades = std::vector<Trade>;

class Orderbook
{
private:
    
    struct OrderEntry{
        OrderPointer order_{ nullptr};
        OrderPointers::iterator location_;
    };

    std::map<Price, OrderPointers, std::greater<Price>> bids_;
    std::map<Price, OrderPointers, std::less<Price>> asks_;
    std::unordered_map<OrderId, OrderEntry> orders_;

    bool CanMatch(Side side, Price price) const 
    {
        if(side == Side::Buy)
        {
            if(asks_.empty())
                return false;

            const auto& [bestAsk, _] = *asks_.begin();
            return price >= bestAsk;
        }
        else{
            if (bids_.empty())
                return false;

            const auto& [bestBid, _] = *bids_.begin();
            return price <= bestBid;
        }
    }

    Trades MatchOrders()
    {
        Trades trades;
        trades.reserve(orders_.size());
        while(true)
        {
            if (bids_.empty() || asks_.empty())
                break;

            auto& [bidPrice, bids] = *bids_.begin();
            auto& [askPrice, asks] = *asks_.begin();

            if(bidPrice < askPrice)
                break;

            while(bids.size() && asks.size())
            {
                auto& bidOrder = bids.front();
                auto& askOrder = asks.front();

                Quantity quantity = std::min(bidOrder->GetRemainingQuantity(), askOrder->GetRemainingQuantity());
                bidOrder->Fill(quantity);
                askOrder->Fill(quantity);

                if(bidOrder->IsFilled())
                {
                    bids.pop_front();
                    orders_.erase(bidOrder->GetOrderId());
                }
                if (askOrder->IsFilled())
                {
                    asks.pop_front();
                    orders_.erase(askOrder->GetOrderId());
                }
                if(bids.empty())
                    bids_.erase(bidPrice);
                
                if(asks.empty())
                    asks_.erase(askPrice);

                trades.push_back(Trade{
                    TradeInfo{bidOrder->GetOrderId(), bidOrder->GetPrice(), quantity},
                    TradeInfo{askOrder->GetOrderId(), askOrder->GetPrice(), quantity}});
            }
        }
        if (!bids_.empty())
        {
            auto& [_, bids] = *bids_.begin();
            auto& order = bids.front();
            if (order->GetOrderType() == OrderType::FillAndKill)
                CancelOrder(order->GetOrderId());
        }

        if(!asks_.empty())
        {
            auto& [_, asks] = *asks_.begin();
            auto& order = asks.front();
            if(order->GetOrderType() == OrderType::FillAndKill)
                CancelOrder(order->GetOrderId());
        }

        return trades;
    }

public:
     Trades AddOrder(OrderPointer order)
     {
        if(orders_.find(order->GetOrderId()) != orders_.end())
            return {};

        if(order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice()))
            return {};

        OrderPointers::iterator iterator;
        
        if(order->GetSide() == Side::Buy)
        {
            auto& orders = bids_[order->GetPrice()];
            orders.push_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        }
        else
        {
            auto& orders = asks_[order->GetPrice()];
            orders.push_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        }

        orders_.insert({order->GetOrderId(), OrderEntry{order, iterator}});
        return MatchOrders();
    }

    void CancelOrder(OrderId orderId)
    {
        if(orders_.find(orderId) == orders_.end())
            return;

        const auto& [order, iterator] = orders_.at(orderId);
        orders_.erase(orderId);

        if(order->GetSide() == Side::Sell)
        {
            auto price = order->GetPrice();
            auto& orders = asks_.at(price);
            orders.erase(iterator);
            if(orders.empty())
                asks_.erase(price);
        }
        else
        {
            auto price = order->GetPrice();
            auto& orders = bids_.at(price);
            orders.erase(iterator);
            if(orders.empty())
                bids_.erase(price);
        }
    }

    Trades MatchOrder(OrderModify order)
    {
        if(orders_.find(order.GetOrderId()) == orders_.end())
            return {};

        const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
        CancelOrder(order.GetOrderId());
        return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
    }
    
    std::size_t Size() const {return orders_.size();}

    OrderbookLevelInfos GetOrderInfos() const 
    {
        LevelInfos bidInfos, askInfos;
        bidInfos.reserve(orders_.size());
        askInfos.reserve(orders_.size());

        auto CreateLevelInfos = [](Price price, const OrderPointers& orders)
        {
            return LevelInfo{price, std::accumulate(orders.begin(), orders.end(), (Quantity)0,
                [](std::size_t runningSum, const OrderPointer& order)
                {
                    return runningSum + order->GetRemainingQuantity();
                })};
        };

        for (const auto& [price, orders] : bids_)
            bidInfos.push_back(CreateLevelInfos(price, orders));

        for(const auto& [price, orders] : asks_)
            askInfos.push_back(CreateLevelInfos(price, orders)); 

        return OrderbookLevelInfos{bidInfos, askInfos};
    }
};

int main(){
    Orderbook orderbook;
    const OrderId orderId = 1;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancelled, orderId, Side::Buy, 100, 10));
    orderbook.CancelOrder(orderId);
    std::cout << orderbook.Size() << std::endl;
    return 0;
}
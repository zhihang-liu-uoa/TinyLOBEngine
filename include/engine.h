#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <functional>

enum class OrderType { BUY, SELL, INVALID };

struct Order {
    int order_id;
    std::string trader_name;
    double price;
    int shares;
    int timestamp;
    OrderType type;
};

struct Stats {
    std::vector<int> history;
    int total_volume = 0;
    double cash = 0;
};

struct TradeRecord {
    std::string buyer;
    std::string seller;
    double price;
    int shares;
};

struct BuyComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) return a.price < b.price;
        if (a.timestamp != b.timestamp) return a.timestamp > b.timestamp;
        return a.shares < b.shares;
    }
};

struct SellComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) return a.price > b.price;
        if (a.timestamp != b.timestamp) return a.timestamp > b.timestamp;
        return a.shares < b.shares;
    }
};

struct AuctionBuyComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) return a.price < b.price;
        if (a.shares != b.shares) return a.shares < b.shares;
        return a.timestamp > b.timestamp;
    }
};

struct AuctionSellComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) return a.price > b.price;
        if (a.shares != b.shares) return a.shares < b.shares;
        return a.timestamp > b.timestamp;
    }
};

using order_queue = std::priority_queue<Order, std::vector<Order>, std::function<bool(Order, Order)>>;
using net_position = std::unordered_map<std::string, int>;

bool parse_order(const std::string& line, Order& order);
void match_order(Order& order, order_queue& buy_queue, order_queue& sell_queue, net_position& net_pos, std::vector<TradeRecord>& trades);
net_position simulate_auction(const std::vector<Order>& orders, double& clearing_price);
void analyze_stats(const std::vector<TradeRecord>& trades);

#endif // ENGINE_H

#include "engine.h"
#include "config.h"

static OrderType parse_order_type_from_string(const std::string& s) {
    if (s == "BUY") return OrderType::BUY;
    if (s == "SELL") return OrderType::SELL;
    return OrderType::INVALID;
}

bool parse_order(const std::string& line, Order& order) {
    std::istringstream ss(line);
    std::string order_id_str, price_str, shares_str, timestamp_str, type_str;
    try {
        std::getline(ss, order_id_str, ',');
        std::getline(ss, order.trader_name, ',');
        std::getline(ss, price_str, ',');
        std::getline(ss, shares_str, ',');
        std::getline(ss, timestamp_str, ',');
        std::getline(ss, type_str, ',');

        if (order_id_str.empty() || price_str.empty() || shares_str.empty() || timestamp_str.empty()) return false;

        order.order_id = std::stoi(order_id_str);
        order.price = std::stod(price_str);
        order.shares = std::stoi(shares_str);
        order.timestamp = std::stoi(timestamp_str);
        order.type = parse_order_type_from_string(type_str);
    } catch (...) {
        return false;
    }
    return order.type != OrderType::INVALID;
}

void match_order(Order& order,
                 order_queue& buy_queue,
                 order_queue& sell_queue,
                 net_position& net_pos,
                 std::vector<TradeRecord>& trades) {
    static std::ofstream trade_log(trade_log_file);
    if (order.type == OrderType::BUY) {
        while (!sell_queue.empty() && order.price >= sell_queue.top().price && order.shares > 0) {
            Order sell_order = sell_queue.top();
            sell_queue.pop();
            int traded_shares = std::min(order.shares, sell_order.shares);
            double trade_price = order.timestamp < sell_order.timestamp ? order.price : sell_order.price;
            trade_log << order.trader_name << "," << sell_order.trader_name << "," << trade_price << "," << traded_shares << "\n";
            trades.push_back({order.trader_name, sell_order.trader_name, trade_price, traded_shares});
            order.shares -= traded_shares;
            sell_order.shares -= traded_shares;
            net_pos[order.trader_name] += traded_shares;
            net_pos[sell_order.trader_name] -= traded_shares;
            if (sell_order.shares > 0) sell_queue.push(sell_order);
        }
        if (order.shares > 0) buy_queue.push(order);
    } else {
        while (!buy_queue.empty() && order.price <= buy_queue.top().price && order.shares > 0) {
            Order buy_order = buy_queue.top();
            buy_queue.pop();
            int traded_shares = std::min(order.shares, buy_order.shares);
            double trade_price = order.timestamp < buy_order.timestamp ? order.price : buy_order.price;
            trade_log << buy_order.trader_name << "," << order.trader_name << "," << trade_price << "," << traded_shares << "\n";
            trades.push_back({buy_order.trader_name, order.trader_name, trade_price, traded_shares});
            order.shares -= traded_shares;
            buy_order.shares -= traded_shares;
            net_pos[buy_order.trader_name] += traded_shares;
            net_pos[order.trader_name] -= traded_shares;
            if (buy_order.shares > 0) buy_queue.push(buy_order);
        }
        if (order.shares > 0) sell_queue.push(order);
    }
}

net_position simulate_auction(const std::vector<Order>& orders, double& clearing_price) {
    net_position result;
    double max_amount = -1.0;
    double best_price = 0.0;

    std::vector<double> all_prices;
    for (const auto& order : orders) {
        all_prices.push_back(order.price);
    }
    std::sort(all_prices.begin(), all_prices.end());

    for (double p : all_prices) {
        order_queue buy_queue{AuctionBuyComparator()};
        order_queue sell_queue{AuctionSellComparator()};
        for (const auto& order : orders) {
            if (order.type == OrderType::BUY && order.price >= p)
                buy_queue.push(order);
            else if (order.type == OrderType::SELL && order.price <= p)
                sell_queue.push(order);
        }

        int total_volume = 0;
        net_position temp_position;

        while (!buy_queue.empty() && !sell_queue.empty()) {
            Order buy = buy_queue.top(); buy_queue.pop();
            Order sell = sell_queue.top(); sell_queue.pop();

            int traded = std::min(buy.shares, sell.shares);
            total_volume += traded;

            temp_position[buy.trader_name] += traded;
            temp_position[sell.trader_name] -= traded;

            buy.shares -= traded;
            sell.shares -= traded;

            if (buy.shares > 0) buy_queue.push(buy);
            if (sell.shares > 0) sell_queue.push(sell);
        }

        double amount = total_volume * p;
        if (amount > max_amount || (amount == max_amount && p > best_price)) {
            max_amount = amount;
            best_price = p;
            result = temp_position;
        }
    }

    clearing_price = best_price;
    return result;
}

void analyze_stats(const std::vector<TradeRecord>& trades) {
    static std::ofstream stats_out(historical_data_file);
    std::unordered_map<std::string, Stats> stats;
    std::unordered_map<std::string, int> current_position;

    for (const auto& trade : trades) {
        double amount = trade.price * trade.shares;

        current_position[trade.buyer] += trade.shares;
        stats[trade.buyer].cash -= amount;
        stats[trade.buyer].total_volume += trade.shares;
        stats[trade.buyer].history.push_back(current_position[trade.buyer]);

        current_position[trade.seller] -= trade.shares;
        stats[trade.seller].cash += amount;
        stats[trade.seller].total_volume += trade.shares;
        stats[trade.seller].history.push_back(current_position[trade.seller]);
    }

    stats_out << "\n--- Position History & PnL ---\n";
    for (const auto& [name, s] : stats) {
        const auto& hist = s.history;
        int min_pos = *std::min_element(hist.begin(), hist.end());
        int max_pos = *std::max_element(hist.begin(), hist.end());
        double avg = hist.empty() ? 0.0 : std::accumulate(hist.begin(), hist.end(), 0.0) / hist.size();

        stats_out << name << ": total volume = " << s.total_volume
                  << ", cash = " << s.cash
                  << ", final pos = " << hist.back()
                  << ", min = " << min_pos
                  << ", max = " << max_pos
                  << ", avg = " << avg
                  << ", PnL = " << s.cash << "\n";
    }

    double total_cash = 0.0;
    for (const auto& [name, s] : stats) {
        total_cash += s.cash;
    }
    stats_out << "\n[CHECK] Total Cash across all traders = " << total_cash << "\n";

}
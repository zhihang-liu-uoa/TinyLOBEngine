#include "engine.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <orders.csv>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Cannot open file " << argv[1] << "\n";
        return 1;
    }

    std::string line;
    order_queue buy_queue{BuyComparator()};
    order_queue sell_queue{SellComparator()};
    net_position net_pos;
    std::vector<Order> all_orders;
    std::vector<TradeRecord> trades;

    while (std::getline(file, line)) {
        Order order;
        if (parse_order(line, order)) {
            all_orders.push_back(order);
            match_order(order, buy_queue, sell_queue, net_pos, trades);
        }
    }

    std::cout << "\n--- Net Positions ---\n";
    for (const auto& [name, net] : net_pos) {
        if (net == 0) continue;
        std::cout << name << " " << (net > 0 ? "L" : "S") << " " << std::abs(net) << "\n";
    }

    double clearing_price;
    net_position auction_result = simulate_auction(all_orders, clearing_price);

    std::cout << "\n--- Auction Result ---\n";
    std::cout << "Clearing Price: " << clearing_price << "\n";
    for (const auto& [name, net] : auction_result) {
        if (net == 0) continue;
        std::cout << name << " " << (net > 0 ? "L" : "S") << " " << std::abs(net) << "\n";
    }

    analyze_stats(trades);

    return 0;
}

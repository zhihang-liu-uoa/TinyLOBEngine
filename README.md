# Orderbook Matching Engine

This project implements a simplified orderbook matching engine supporting three features:

- Real-time matching of buy/sell orders
- Determining the auction clearing price maximizing total transaction amount
- Analyzing net positions, trading volume, profit and loss(PnL) and position history

---

## 📁 Project Structure

```
orderbook_engine/
├── Makefile
├── include/
│   └── engine.h
├── src/
│   ├── main.cpp        # Main program
│   └── engine.cpp      # Matching logic
├── bin/                # Compiled executable files
├── build/              # Intermediate object
└── data/
    └── orders.csv      # Sample input data
```

---

## ⚙️ Build and Run

cd to orderbook_engine, run:

```bash
make
```

Make sure you have a CSV file in `data/orders.csv` with the following format:

```
ID,party,price,quantity,timestamp,side
```

run:

```bash
./bin/orderbook ./data/orders.csv
```

---

## 📌 Notes
- All orders are assumed to be for a **single stock**.
- Matching is based on **price > time > size** priority rules.
- Auction logic computes the clearing price for maximum dollar turnover.

---


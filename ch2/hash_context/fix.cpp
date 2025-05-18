// HashRing.hpp --------------------------------------------------------------
#pragma once

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

struct Node {
    int hashValue;                                     // ノード座標
    std::unordered_map<int, std::string> resources;    // ぶら下がるリソース

    explicit Node(int hv = 0) : hashValue(hv) {}
};

class HashRing {
public:
    explicit HashRing(unsigned int kBits)
        : k_(kBits),
          min_(0),
          max_((1u << kBits) - 1u) {}

    /* ------------ 公開 API ------------------------------------------------ */

    void addNode(int hashValue) {
        if (!legalRange(hashValue)) return;

        auto [it, inserted] = nodes_.try_emplace(hashValue, hashValue);
        if (!inserted) return;                                     // 既に存在

        std::cout << "Adding a node " << hashValue << "...\n";

        if (nodes_.size() > 1) {                                   // 後続ノードから資源を奪う
            auto succ = nextIt(it);
            moveResources(it->second, succ->second, /*deleteAll=*/false);
        }
    }

    void addResource(int hashValRes) {
        if (!legalRange(hashValRes)) return;

        std::cout << "Adding a resource " << hashValRes << "...\n";

        if (nodes_.empty()) {
            std::cout << "Can't add a resource to an empty hashring\n";
            return;
        }
        auto it = lookupNode(hashValRes);
        it->second.resources.emplace(
            hashValRes,
            "Dummy resource value of " + std::to_string(hashValRes));
    }

    void removeNode(int hashValue) {
        auto it = nodes_.find(hashValue);
        if (it == nodes_.end()) {
            std::cout << "Nothing to remove.\n";
            return;
        }

        std::cout << "Removing the node " << hashValue << ":\n";

        if (nodes_.size() > 1) {                                   // 後続へ引き継ぎ
            auto succ = nextIt(it);
            moveResources(succ->second, it->second, /*deleteAll=*/true);
        }
        nodes_.erase(it);
    }

    void printHashRing() const {
        std::cout << "*****\nPrinting the hashring in clockwise order:\n";
        if (nodes_.empty()) {
            std::cout << "Empty hashring\n*****\n";
            return;
        }
        for (const auto& [hv, node] : nodes_) {
            std::cout << "Node: " << hv << ", Resources: ";
            if (node.resources.empty()) {
                std::cout << "Empty";
            } else {
                for (const auto& [key, _] : node.resources) std::cout << key << " ";
            }
            std::cout << '\n';
        }
        std::cout << "*****\n";
    }

    int distance(int a, int b) const {                 // Python と同じ距離関数
        if (a == b) return 0;
        if (a < b)  return b - a;
        return (1u << k_) + (b - a);
    }

private:
    /* ------------ 内部状態 ---------------------------------------------- */
    const unsigned int k_;
    const int min_, max_;
    std::map<int, Node> nodes_;                        // <hashValue, Node>

    /* ------------ ヘルパ -------------------------------------------------- */
    bool legalRange(int v) const { return min_ <= v && v <= max_; }

    //  clockwise に 1 歩進めるイテレータ（終端なら先頭へラップ）
    template <class It>
    It nextIt(It it) const {
        ++it;
        return (it == nodes_.end()) ? nodes_.begin() : it;
    }

    // hashValue を超えた（またはピッタリの）最初のノードを返す
    auto lookupNode(int hashValue) {
        auto it = nodes_.lower_bound(hashValue);
        if (it == nodes_.end()) it = nodes_.begin();   // wrap-around
        return it;
    }

    // リソース移動ロジック
    void moveResources(Node& dest, Node& orig, bool deleteAll) {
        std::vector<int> eraseKeys;
        for (const auto& [key, val] : orig.resources) {
            if (deleteAll ||
                distance(key, dest.hashValue) < distance(key, orig.hashValue)) {
                dest.resources[key] = val;
                eraseKeys.push_back(key);
                std::cout << "\tMoving a resource " << key << " from "
                          << orig.hashValue << " to " << dest.hashValue << '\n';
            }
        }
        for (int k : eraseKeys) orig.resources.erase(k);
    }
};

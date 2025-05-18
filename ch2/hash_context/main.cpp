#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <limits>

struct Node {
    int hashValue;                                            // ノードの位置
    std::unordered_map<int, std::string> resources;           // リソース
    Node* next  = nullptr;                                    // 時計回り
    Node* prev  = nullptr;                                    // 反時計回り

    explicit Node(int hv) : hashValue(hv) {}
};

class HashRing {
public:
    explicit HashRing(unsigned int kBits)
        : head_(nullptr),
          k_(kBits),
          min_(0),
          max_((1u << kBits) - 1u) {}

    ~HashRing() { clear(); }

    /* ---------- 操作 API ---------- */

    void addNode(int hashValue) {
        if (!legalRange(hashValue)) return;

        Node* newNode = new Node(hashValue);

        if (!head_) {                                         // 初めてのノード
            newNode->next = newNode->prev = newNode;
            head_ = newNode;
            std::cout << "Adding a head node " << hashValue << "...\n";
            return;
        }

        Node* target = lookupNode(hashValue);                 // 右隣になるノード
        newNode->next = target;
        newNode->prev = target->prev;
        newNode->prev->next = newNode;
        newNode->next->prev = newNode;

        std::cout << "Adding a node " << hashValue
                  << ". Its prev is " << newNode->prev->hashValue
                  << ", and its next is " << newNode->next->hashValue << ".\n";

        moveResources(newNode, newNode->next, /*deleteTrue=*/false);

        if (hashValue < head_->hashValue) head_ = newNode;
    }

    void addResource(int hashValRes) {
        if (!legalRange(hashValRes)) return;

        std::cout << "Adding a resource " << hashValRes << "...\n";
        Node* target = lookupNode(hashValRes);
        if (target) {
            target->resources[hashValRes] =
                "Dummy resource value of " + std::to_string(hashValRes);
        } else {
            std::cout << "Can't add a resource to an empty hashring\n";
        }
    }

    void removeNode(int hashValue) {
        Node* victim = lookupNode(hashValue);
        if (!victim || victim->hashValue != hashValue) {
            std::cout << "Nothing to remove.\n";
            return;
        }

        std::cout << "Removing the node " << hashValue << ":\n";
        moveResources(victim->next, victim, /*deleteTrue=*/true);

        victim->prev->next = victim->next;
        victim->next->prev = victim->prev;

        if (head_ == victim) {
            head_ = (victim->next == victim) ? nullptr : victim->next;
        }
        delete victim;
    }

    void printHashRing() const {
        std::cout << "*****\nPrinting the hashring in clockwise order:\n";
        if (!head_) {
            std::cout << "Empty hashring\n*****\n";
            return;
        }
        const Node* cur = head_;
        do {
            std::cout << "Node: " << cur->hashValue << ", Resources: ";
            if (cur->resources.empty()) {
                std::cout << "Empty";
            } else {
                for (const auto& [key, _] : cur->resources)
                    std::cout << key << " ";
            }
            std::cout << '\n';
            cur = cur->next;
        } while (cur != head_);
        std::cout << "*****\n";
    }

    /* ---------- ユーティリティ ---------- */

    // Python 版と同じ距離関数
    int distance(int a, int b) const {
        if (a == b) return 0;
        if (a < b) return b - a;
        return (1u << k_) + (b - a);
    }

private:
    Node* head_;
    const unsigned int k_;
    const int min_;
    const int max_;

    bool legalRange(int v) const { return min_ <= v && v <= max_; }

    // hashValue を超えた（またはピッタリ一致する）最初のノードを返す
    Node* lookupNode(int hashValue) const {
        if (!legalRange(hashValue) || !head_) return nullptr;

        Node* cur = head_;
        while (distance(cur->hashValue, hashValue) >
               distance(cur->next->hashValue, hashValue)) {
            cur = cur->next;
        }
        return (cur->hashValue == hashValue) ? cur : cur->next;
    }

    // Python moveResources 相当
    static void moveResources(Node* dest, Node* orig, bool deleteTrue) {
        std::vector<int> toDelete;
        for (const auto& [key, val] : orig->resources) {
            if (deleteTrue ||
                dest->hashValue == key ||               // 距離 0 は必ず移動
                isCloser(key, dest->hashValue, orig->hashValue, dest->k_)) {
                dest->resources[key] = val;
                toDelete.push_back(key);
                std::cout << "\tMoving a resource " << key << " from "
                          << orig->hashValue << " to " << dest->hashValue << "\n";
            }
        }
        for (int key : toDelete) orig->resources.erase(key);
    }

    // 距離比較（距離を求めるには k_ が必要なので static にはできない）
    static bool isCloser(int res,
                         int destHash,
                         int origHash,
                         unsigned int kBits) {
        auto dist = [&](int a, int b) {
            if (a == b) return 0;
            if (a < b) return b - a;
            return (1u << kBits) + (b - a);
        };
        return dist(res, destHash) < dist(res, origHash);
    }

    // デストラクタ用
    void clear() {
        if (!head_) return;
        Node* cur = head_->next;
        while (cur != head_) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
        delete head_;
        head_ = nullptr;
    }
};

/* ---------------- demo ---------------- */
int main() {
    HashRing hr(5);

    std::cout << hr.distance(29, 5)  << '\n';
    std::cout << hr.distance(29, 12) << '\n';
    std::cout << hr.distance(5, 29)  << '\n';

    // Python サンプルの一部
    hr.addNode(12);
    hr.addNode(18);
    hr.addResource(24);
    hr.addResource(2);
    hr.addResource(29);
    hr.printHashRing();

    hr.addNode(5);
    hr.addNode(27);
    hr.addNode(30);
    hr.printHashRing();

    hr.removeNode(12);
    hr.printHashRing();
}

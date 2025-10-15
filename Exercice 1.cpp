#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory> // pour unique_ptr

std::string simpleHash(const std::string& input) {
    size_t h = std::hash<std::string>{}(input);
    std::stringstream ss;
    ss << "h" << std::hex << std::setw(16) << std::setfill('0') << h;
    return ss.str();
}

struct MerkleNode {
    std::string hashValue;
    std::unique_ptr<MerkleNode> left;
    std::unique_ptr<MerkleNode> right;

    MerkleNode(const std::string& data) : hashValue(data) {}

    MerkleNode(std::unique_ptr<MerkleNode> l, std::unique_ptr<MerkleNode> r)
        : left(std::move(l)), right(std::move(r)) {
        if (left && right) {
            hashValue = simpleHash(left->hashValue + right->hashValue);
        }
        else if (left) {
            hashValue = simpleHash(left->hashValue + left->hashValue);
        }
    }
};

class MerkleTree {
private:
    std::unique_ptr<MerkleNode> root;
    std::vector<std::string> leafHashes;

    std::unique_ptr<MerkleNode> buildTree(std::vector<std::unique_ptr<MerkleNode>>& level) {
        if (level.empty()) return nullptr;
        if (level.size() == 1) return std::move(level[0]);

        std::vector<std::unique_ptr<MerkleNode>> nextLevel;
        for (size_t i = 0; i < level.size(); i += 2) {
            auto left = std::move(level[i]);
            auto right = (i + 1 < level.size()) ? std::move(level[i + 1]) : std::move(left);
            nextLevel.push_back(std::make_unique<MerkleNode>(std::move(left), std::move(right)));
        }
        return buildTree(nextLevel);
    }

public:
    MerkleTree(const std::vector<std::string>& data) {
        if (data.empty()) return;

        std::vector<std::unique_ptr<MerkleNode>> currentLevel;
        for (const auto& d : data) {
            std::string h = simpleHash(d);
            leafHashes.push_back(h);
            currentLevel.push_back(std::make_unique<MerkleNode>(h));
        }

        root = buildTree(currentLevel);
    }

    std::string getRootHash() const {
        return root ? root->hashValue : "";
    }

    void printLeaves() const {
        std::cout << "Feuilles (hashes des données):\n";
        for (size_t i = 0; i < leafHashes.size(); ++i) {
            std::cout << "  [" << i << "] " << leafHashes[i] << "\n";
        }
    }
};

void runExample(const std::string& title, const std::vector<std::string>& data) {
    std::cout << "=== " << title << " ===\n";
    MerkleTree tree(data);
    tree.printLeaves();
    std::cout << "Hash racine: " << tree.getRootHash() << "\n\n";
}

int main() {
    std::cout << "=== Vérification du bon fonctionnement de l'arbre de Merkle ===\n\n";

    // Exemple 1 : 4 transactions (cas pair)
    runExample("Exemple 1 : 4 transactions (cas pair)",
        { "Alice → Bob: 1 BTC", "Charlie → Dave: 2 BTC", "Eve → Frank: 0.5 BTC", "Grace → Henry: 3 BTC" });

    // Exemple 2 : 3 transactions (cas impair → duplication du dernier)
    runExample("Exemple 2 : 3 transactions (cas impair)",
        { "Tx A", "Tx B", "Tx C" });

    // Exemple 3 : 1 transaction (racine = feuille)
    runExample("Exemple 3 : 1 transaction (cas minimal)",
        { "Seule transaction dans le bloc" });

    return 0;
}
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <random>
#include <algorithm>
#include <memory> // unique_ptr

// ==================================================
// UTILITAIRES DE HACHAGE
// ==================================================
std::string sha256_sim(const std::string& input) {
    static std::hash<std::string> hasher;
    size_t h1 = hasher(input);
    size_t h2 = hasher(input + "salt");
    size_t h3 = hasher(std::to_string(h1) + input);
    size_t h4 = hasher(std::to_string(h2) + "pepper");

    std::stringstream ss;
    ss << std::hex << std::setfill('0')
        << std::setw(16) << h1
        << std::setw(16) << h2
        << std::setw(16) << h3
        << std::setw(16) << h4;
    std::string hash = ss.str();
    if (hash.size() > 64) hash = hash.substr(0, 64);
    while (hash.size() < 64) hash += '0';
    return hash;
}

bool startsWithZeros(const std::string& hash, int difficulty) {
    if (difficulty <= 0) return true;
    if (static_cast<size_t>(difficulty) > hash.size()) return false;
    return std::all_of(hash.begin(), hash.begin() + difficulty, [](char c) { return c == '0'; });
}

// ==================================================
// TRANSACTION
// ==================================================
class Transaction {
public:
    std::string id;
    std::string sender;
    std::string receiver;
    double amount;

    Transaction(const std::string& _sender, const std::string& _receiver, double _amount)
        : sender(_sender), receiver(_receiver), amount(_amount) {
        id = sha256_sim(sender + receiver + std::to_string(amount)).substr(0, 16);
    }

    std::string toString() const {
        return sender + "->" + receiver + ":" + std::to_string(amount);
    }
};

// ==================================================
// MERKLE ROOT
// ==================================================
std::string computeMerkleRoot(const std::vector<Transaction>& transactions) {
    if (transactions.empty()) {
        return sha256_sim("empty");
    }

    std::vector<std::string> hashes;
    for (const auto& tx : transactions) {
        hashes.push_back(sha256_sim(tx.toString()));
    }

    while (hashes.size() > 1) {
        std::vector<std::string> newLevel;
        for (size_t i = 0; i < hashes.size(); i += 2) {
            std::string left = hashes[i];
            std::string right = (i + 1 < hashes.size()) ? hashes[i + 1] : left;
            newLevel.push_back(sha256_sim(left + right));
        }
        hashes = newLevel;
    }
    return hashes[0];
}

// ==================================================
// VALIDATEUR
// ==================================================
struct Validator {
    std::string id;
    double stake;
    Validator(const std::string& _id, double _stake) : id(_id), stake(_stake) {}
};

// ==================================================
// BLOC DE BASE
// ==================================================
class Block {
public:
    size_t index;
    std::string previousHash;
    std::string merkleRoot;
    std::string timestamp;
    std::vector<Transaction> transactions;
    std::string hash;

    Block(size_t idx, const std::string& prevHash, const std::vector<Transaction>& txs)
        : index(idx), previousHash(prevHash), transactions(txs) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        timestamp = oss.str();
        merkleRoot = computeMerkleRoot(transactions);
        hash = calculateHash();
    }

    virtual ~Block() {}

    
    virtual std::string calculateHash() const {
        std::string data = std::to_string(index) + previousHash + merkleRoot + timestamp;
        return sha256_sim(data);
    }

    virtual void finalize() {}
    virtual std::string getConsensusInfo() const { return "Base"; }
};

// ==================================================
// BLOC PoW
// ==================================================
class PoWBlock : public Block {
public:
    long long nonce;
    int difficulty;

    PoWBlock(size_t idx, const std::string& prevHash, const std::vector<Transaction>& txs, int diff)
        : Block(idx, prevHash, txs), nonce(0), difficulty(diff) {
        hash = calculateHash();
    }


    virtual std::string calculateHash() const {
        std::string data = std::to_string(index) + previousHash + merkleRoot + timestamp + std::to_string(nonce);
        return sha256_sim(data);
    }

    void finalize() {
        while (!startsWithZeros(hash, difficulty)) {
            nonce++;
            hash = calculateHash();
        }
    }

    std::string getConsensusInfo() const {
        return "PoW (nonce=" + std::to_string(nonce) + ", diff=" + std::to_string(difficulty) + ")";
    }
};

// ==================================================
// BLOC PoS
// ==================================================
class PoSBlock : public Block {
public:
    std::string validatorId;

    PoSBlock(size_t idx, const std::string& prevHash, const std::vector<Transaction>& txs)
        : Block(idx, prevHash, txs), validatorId("none") {}

    void selectValidator(const std::vector<Validator>& validators) {
        double totalStake = 0.0;
        for (const auto& v : validators) totalStake += v.stake;
        if (totalStake == 0) {
            validatorId = "default";
            return;
        }

        size_t seed = std::hash<std::string>{}(hash);
        std::mt19937 gen(static_cast<unsigned int>(seed));
        std::uniform_real_distribution<double> dis(0.0, totalStake);
        double randVal = dis(gen);

        double cumulative = 0.0;
        for (const auto& v : validators) {
            cumulative += v.stake;
            if (cumulative >= randVal) {
                validatorId = v.id;
                break;
            }
        }
    }

    void finalize() {}

    std::string getConsensusInfo() const {
        return "PoS (validator=" + validatorId + ")";
    }
};

// ==================================================
// BLOCKCHAIN
// ==================================================
class Blockchain {
private:
    std::vector<std::unique_ptr<Block> > chain;
    std::vector<Validator> validators;
    int powDifficulty;

public:
    Blockchain(int difficulty = 2)
        : powDifficulty(difficulty) {

        chain.push_back(std::unique_ptr<Block>(new Block(0, "0", std::vector<Transaction>())));

        std::cout << " Blockchain créée (bloc génèse)\n";
    }

    void setValidators(const std::vector<Validator>& _validators) {
        validators = _validators;
    }

    void addBlockPoW(const std::vector<Transaction>& transactions) {
        std::string lastHash = chain.back()->hash;
        //  unique_ptr
        std::unique_ptr<PoWBlock> block(new PoWBlock(chain.size(), lastHash, transactions, powDifficulty));
        auto start = std::chrono::high_resolution_clock::now();
        block->finalize();
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << " Bloc PoW ajouté [" << block->index << "] en " << ms << " ms\n";
        std::cout << "   Hash : " << block->hash << "\n";
        std::cout << "   " << block->getConsensusInfo() << "\n\n";

        chain.push_back(std::move(block));
    }

    void addBlockPoS(const std::vector<Transaction>& transactions) {
        if (validators.empty()) {
            std::cerr << "  Aucun validateur configuré pour PoS !\n";
            return;
        }
        std::string lastHash = chain.back()->hash;
        std::unique_ptr<PoSBlock> block(new PoSBlock(chain.size(), lastHash, transactions));
        block->selectValidator(validators);
        auto start = std::chrono::high_resolution_clock::now();
        block->finalize();
        auto end = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        std::cout << " Bloc PoS ajouté [" << block->index << "] en " << us << " µs\n";
        std::cout << "   Hash : " << block->hash << "\n";
        std::cout << "   " << block->getConsensusInfo() << "\n\n";

        chain.push_back(std::move(block));
    }

    bool isValid() const {
        if (chain.empty()) return true;
        for (size_t i = 1; i < chain.size(); ++i) {
            const auto& current = chain[i];
            const auto& previous = chain[i - 1];

            if (current->previousHash != previous->hash) {
                std::cerr << " Erreur : previousHash invalide au bloc " << i << "\n";
                return false;
            }
        }
        return true;
    }

    void printChain() const {
        std::cout << "\n=== BLOCKCHAIN ===\n";
        for (const auto& block : chain) {
            std::cout << "Bloc #" << block->index
                << " | Hash: " << block->hash.substr(0, 10) << "..."
                << " | Merkle: " << block->merkleRoot.substr(0, 10) << "..."
                << " | " << block->getConsensusInfo() << "\n";
        }
        std::cout << "==================\n\n";
    }
};

// ==================================================
// UTILITAIRE TRANSACTIONS
// ==================================================
std::vector<Transaction> createSampleTransactions(int count) {
    std::vector<Transaction> txs;
    std::vector<std::string> users = { "Alice", "Bob", "Charlie", "Dave", "Eve" };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> userDist(0, static_cast<int>(users.size()) - 1);
    std::uniform_real_distribution<> amountDist(0.1, 10.0);

    for (int i = 0; i < count; ++i) {
        std::string sender = users[userDist(gen)];
        std::string receiver;
        do {
            receiver = users[userDist(gen)];
        } while (receiver == sender);
        double amount = amountDist(gen);
        txs.push_back(Transaction(sender, receiver, amount));
    }
    return txs;
}

// ==================================================
// MAIN
// ==================================================
int main() {
    std::cout << "=== Exercice 4 : Mini-blockchain  ===\n\n";

    int powDiff = 3;
    Blockchain chain(powDiff);

    std::vector<Validator> validators = {
        Validator("Node_A", 40.0),
        Validator("Node_B", 30.0),
        Validator("Node_C", 20.0),
        Validator("Node_D", 10.0)
    };
    chain.setValidators(validators);

    std::cout << " Ajout de 2 blocs avec PoW (difficulté = " << powDiff << ")\n";
    chain.addBlockPoW(createSampleTransactions(3));
    chain.addBlockPoW(createSampleTransactions(2));

    std::cout << " Ajout de 2 blocs avec PoS\n";
    chain.addBlockPoS(createSampleTransactions(4));
    chain.addBlockPoS(createSampleTransactions(3));

    std::cout << " Vérification...\n";
    if (chain.isValid()) {
        std::cout << " Chaîne valide !\n\n";
    }
    else {
        std::cout << " Chaîne corrompue !\n\n";
    }

    chain.printChain();

    std::cout << " PoW = lent mais sécurisé par calcul.\n";
    std::cout << "   PoS = rapide, sécurisé par enjeu.\n";

    return 0;
}
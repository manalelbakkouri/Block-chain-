#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <random>
#include <map>
#include <algorithm>
#include <numeric>

// =============== UTILITAIRES DE HACHAGE (réutilisés de l'Exercice 2) ===============
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

bool isValidHash(const std::string& hash, int difficulty) {
    if (difficulty <= 0) return true;
    if (static_cast<size_t>(difficulty) > hash.size()) return false;
    return std::all_of(hash.begin(), hash.begin() + difficulty, [](char c) { return c == '0'; });
}

// =============== ARBRE DE MERKLE (version compacte) ===============
std::string computeMerkleRoot(const std::vector<std::string>& transactions) {
    if (transactions.empty()) return sha256_sim("empty");
    auto hashTx = [](const std::string& tx) { return sha256_sim(tx); };
    std::vector<std::string> hashes;
    for (const auto& tx : transactions) hashes.push_back(hashTx(tx));
    while (hashes.size() > 1) {
        std::vector<std::string> newHashes;
        for (size_t i = 0; i < hashes.size(); i += 2) {
            std::string left = hashes[i];
            std::string right = (i + 1 < hashes.size()) ? hashes[i + 1] : left;
            newHashes.push_back(sha256_sim(left + right));
        }
        hashes = newHashes;
    }
    return hashes[0];
}

// =============== STRUCTURE VALIDATEUR POUR PoS ===============
struct Validator {
    std::string id;
    double stake; // en unités de monnaie

    Validator(const std::string& _id, double _stake) : id(_id), stake(_stake) {}
};

// =============== CLASSE BLOC DE BASE ===============
class Block {
public:
    size_t index;
    std::string previousHash;
    std::string merkleRoot;
    std::string timestamp;
    std::vector<std::string> transactions;
    std::string hash;

    Block(size_t idx, const std::string& prevHash, const std::vector<std::string>& txs)
        : index(idx), previousHash(prevHash), transactions(txs) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        timestamp = oss.str();
        merkleRoot = computeMerkleRoot(transactions);
        hash = calculateHash();
    }

    std::string calculateHash() const {
        std::string data = std::to_string(index) + previousHash + merkleRoot + timestamp;
        return sha256_sim(data);
    }
};

// =============== PROOF OF WORK (PoW) ===============
class PoWBlock : public Block {
public:
    long long nonce;

    PoWBlock(size_t idx, const std::string& prevHash, const std::vector<std::string>& txs)
        : Block(idx, prevHash, txs), nonce(0) {
        hash = calculateHash();
    }

    std::string calculateHash() const {
        std::string data = std::to_string(index) + previousHash + merkleRoot + timestamp + std::to_string(nonce);
        return sha256_sim(data);
    }

    void mine(int difficulty) {
        while (!isValidHash(hash, difficulty)) {
            nonce++;
            hash = calculateHash();
        }
    }
};

// =============== PROOF OF STAKE (PoS) ===============
class PoSBlock : public Block {
public:
    std::string validatorId;

    PoSBlock(size_t idx, const std::string& prevHash, const std::vector<std::string>& txs)
        : Block(idx, prevHash, txs), validatorId("") {}

    // Sélection du validateur pondérée par le stake
    void selectValidator(const std::vector<Validator>& validators) {
        double totalStake = 0.0;
        for (const auto& v : validators) totalStake += v.stake;

        if (totalStake == 0) {
            validatorId = "default";
            return;
        }

        // Utiliser le hash du bloc comme source de "hasard" déterministe
        std::string seed = hash;
        size_t hashVal = std::hash<std::string>{}(seed);
        double randVal = static_cast<double>(hashVal % 10000) / 10000.0; // [0,1)
        double target = randVal * totalStake;

        double cumulative = 0.0;
        for (const auto& v : validators) {
            cumulative += v.stake;
            if (cumulative >= target) {
                validatorId = v.id;
                break;
            }
        }
    }
};

// =============== FONCTIONS DE TEST ET COMPARAISON ===============
void runPoWTest(int difficulty, const std::vector<std::string>& txs) {
    std::cout << " Proof of Work (difficulte = " << difficulty << ")\n";
    auto start = std::chrono::high_resolution_clock::now();

    PoWBlock block(1, "0000...", txs);
    block.mine(difficulty);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "    Hash : " << block.hash << "\n";
    std::cout << "    Temps : " << ms << " ms\n\n";
}

void runPoSTest(const std::vector<std::string>& txs, const std::vector<Validator>& validators) {
    std::cout << " Proof of Stake\n";
    auto start = std::chrono::high_resolution_clock::now();

    PoSBlock block(1, "0000...", txs);
    block.selectValidator(validators);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "    Validateur elu : " << block.validatorId << "\n";
    std::cout << "    Hash : " << block.hash << "\n";
    std::cout << "    Temps : " << ms << " µs (" << (ms / 1000.0) << " ms)\n\n";
}

// =============== PROGRAMME PRINCIPAL ===============
int main() {
    std::cout << "=== Exercice 3 : Proof of Stake + Comparaison PoW vs PoS ===\n\n";

    // Données communes
    std::vector<std::string> transactions = {
        "Alice → Bob: 5 coins",
        "Charlie → Dave: 3 coins",
        "Eve → Frank: 2 coins"
    };

    // Validateurs pour PoS
    std::vector<Validator> validators = {
        Validator("Validator_A", 40.0),
        Validator("Validator_B", 30.0),
        Validator("Validator_C", 20.0),
        Validator("Validator_D", 10.0)
    };

    // Comparaison
    int difficulty = 3; // difficulté modérée pour PoW

    runPoWTest(difficulty, transactions);
    runPoSTest(transactions, validators);

    std::cout << " Conclusion : PoS est beaucoup plus rapide car il n'y a pas de minage.\n";
    std::cout << "   PoW depend de la difficulte ; PoS est quasi instantane.\n";

    return 0;
}
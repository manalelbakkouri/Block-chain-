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

// =============== UTILITAIRES DE HACHAGE ===============
// Fonction de hachage déterministe qui retourne une chaîne hexadécimale de 64 caractères
// (simule SHA-256 pour les besoins du PoW)
std::string sha256_sim(const std::string& input) {
    // On utilise une graine fixe pour la reproductibilité pendant le PoW
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
    // Tronquer ou étendre à 64 caractères si nécessaire
    if (hash.size() > 64) hash = hash.substr(0, 64);
    while (hash.size() < 64) hash += '0';
    return hash;
}

// Vérifie si le hash commence par 'difficulty' zéros
bool isValidHash(const std::string& hash, int difficulty) {
    if (difficulty <= 0) return true;
    if (static_cast<size_t>(difficulty) > hash.size()) return false;
    return std::all_of(hash.begin(), hash.begin() + difficulty, [](char c) { return c == '0'; });
}

// =============== ARBRE DE MERKLE (version minimale intégrée) ===============

std::string computeMerkleRoot(const std::vector<std::string>& transactions) {
    if (transactions.empty()) return sha256_sim("empty");

    auto hashTx = [](const std::string& tx) { return sha256_sim(tx); };
    std::vector<std::string> hashes;
    for (const auto& tx : transactions) {
        hashes.push_back(hashTx(tx));
    }

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

// =============== CLASSE BLOC ===============
class Block {
public:
    size_t index;
    std::string previousHash;
    std::string merkleRoot;
    std::string timestamp;
    std::vector<std::string> transactions;
    long long nonce;
    std::string hash;

    Block(size_t idx, const std::string& prevHash, const std::vector<std::string>& txs)
        : index(idx), previousHash(prevHash), transactions(txs), nonce(0) {
        // Horodatage ISO 8601 simplifié
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        timestamp = oss.str();

        merkleRoot = computeMerkleRoot(transactions);
        hash = calculateHash();
    }

    std::string calculateHash() const {
        std::string data = std::to_string(index) + previousHash + merkleRoot + timestamp + std::to_string(nonce);
        return sha256_sim(data);
    }

    std::string mineBlock(int difficulty) {
        std::cout << "Minage du bloc " << index << " avec difficulté " << difficulty << "...\n";
        auto start = std::chrono::high_resolution_clock::now();

        while (!isValidHash(hash, difficulty)) {
            nonce++;
            hash = calculateHash();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << " Bloc miné ! Nonce = " << nonce << "\n";
        std::cout << " Temps : " << duration.count() << " ms\n";
        std::cout << " Hash : " << hash << "\n\n";

        return hash;
    }
};

// =============== FONCTION D'AIDE POUR EXEMPLES ===============
void testProofOfWork(int difficulty) {
    std::cout << "----- Test avec difficulté = " << difficulty << " -----\n";
    Block block(1, "0000000000000000000000000000000000000000000000000000000000000000",
        { "Alice paie Bob 1 BTC", "Charlie paie Dave 2 BTC" });
    block.mineBlock(difficulty);
}

// =============== PROGRAMME PRINCIPAL ===============
int main() {
    std::cout << "=== Exercice 2 : Proof of Work ===\n\n";

    // Tester plusieurs niveaux de difficulté
    testProofOfWork(2);  // Facile
    testProofOfWork(3);  // Moyen
    testProofOfWork(4);  // Plus difficile

    return 0;
}
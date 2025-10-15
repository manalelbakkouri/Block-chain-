#  Atelier 1 : Implémentation complète d’une Blockchain simplifiée en C++

##  Description

Ce dépôt contient la **solution complète de l’Atelier 1** sur la **construction d’une blockchain simplifiée en C++**, réalisée **from scratch**.  
L’objectif est de comprendre et d’implémenter les concepts fondamentaux des technologies blockchain — tels que l’arbre de Merkle, la preuve de travail (*Proof of Work*), la preuve d’enjeu (*Proof of Stake*), et la structure d’une mini-blockchain — sans recourir à des bibliothèques externes dédiées.

---

##  Contenu de l’atelier

### 🔹 Exercice 1 : Arbre de Merkle
- Implémentation d’un **Merkle Tree** basique en C++.
- Génération et vérification du **Merkle Root** à partir d’une liste de transactions.
- Tests d’exécution pour vérifier le bon fonctionnement de l’arbre.

### 🔹 Exercice 2 : Proof of Work (PoW)
- Implémentation du **Proof of Work** en C++.
- Création de blocs et d’une chaîne simple.
- Variation du **niveau de difficulté** (nombre de zéros initiaux dans le hash).
- Calcul et comparaison du **temps d’exécution** pour chaque niveau de difficulté.

### 🔹 Exercice 3 : Proof of Stake (PoS)
- Implémentation du **Proof of Stake** en C++.
- Sélection d’un validateur selon un poids proportionnel à son “stake”.
- Comparaison expérimentale entre **PoW** et **PoS** en termes de **rapidité**.

### 🔹 Exercice 4 : Mini-Blockchain complète
Intégration de tous les éléments précédents pour construire une blockchain fonctionnelle :

#### Partie 1 – Structure des blocs et de la chaîne
- Classe `Transaction` (id, sender, receiver, amount).  
- Calcul du **Merkle Root** des transactions.
- Classe `Block` (id, timestamp, previous hash, merkle root, nonce, hash).  
- Classe `Blockchain` avec :
  - ajout de bloc,
  - vérification d’intégrité de la chaîne.

#### Partie 2 – Proof of Work
- Fonction `mineBlock(difficulty)` dans la classe `Block`.
- Minage de plusieurs blocs à difficulté variable.
- Vérification de la validité de la chaîne.

#### Partie 3 – Proof of Stake
- Sélection aléatoire d’un validateur selon son **stake**.
- Validation des blocs via PoS au lieu du minage.
- Comparaison expérimentale entre PoW et PoS (temps de validation).

#### Partie 4 – Analyse comparative
- Ajout de plusieurs blocs via PoW et PoS.  
- Comparaison des résultats :
  -  Rapidité d’ajout des blocs  
  -  Consommation de ressources (temps CPU)  
  -  Facilité de mise en œuvre  

---

##  Technologies utilisées
- **Langage :** C++  
- **IDE recommandé :** Visual Studio Code / Code::Blocks  


---

##  Exécution
1. **Cloner le dépôt :**
   ```bash
   git clone https://github.com/manalelbakkouri/Block-chain-.git
   cd block-chain-

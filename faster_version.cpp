#include <iostream>
#include <algorithm>
#include <vector>
#include <array>
#include <chrono>
#include <random>
#include <unordered_map>

using std::string;
using std::array;
using std::vector;
using std::cout;
using std::endl;

// The bitboards will be used to perform more efficient calculations for the AI
using Bitboard = uint16_t;
#define SIGNIFICANT_BITS 0b111111111 // this will be used with AND operation to only use the last 9 bits
#define WIN 10
#define LOSS -10

constexpr array<uint16_t, 8> patterns = {
    0b100100100,  // COL1
    0b010010010,  // COL2
    0b001001001,  // COL3
    0b111000000,  // ROW1
    0b000111000,  // ROW2
    0b000000111,  // ROW3
    0b100010001,  // DIA1
    0b001010100   // DIA2
};

std::unordered_map<uint32_t, int> TranspositionTable; 

constexpr array<int, 512> PopcountTable = []() { // Table of popcounts calculated at compile time
    array<int, 512> table{};
    for (int i = 0; i < 512; ++i) {
        table[i] = __builtin_popcount(i);
    }
    return table;
}();

inline int FastPopcount(uint16_t x) {
    return PopcountTable[x & 0x1FF];
}

// bool ChooseWhoStarts() {
//     static std::mt19937 gen{std::random_device{}()};
// //     return std::uniform_int_distribution<>(0, 1)(gen) == 0;
// // }

// inline vector<Bitboard> PossibleMoves(Bitboard board) {
//     vector<Bitboard> moves;
//     Bitboard free = (~board) & SIGNIFICANT_BITS;
//     while (free) {
//         Bitboard move = free & (-free);
//         moves.push_back(move);
//         free &= (free - 1);
//     }
//     return moves;
// }

inline int Eval(Bitboard cpu, Bitboard player) {
    // Check for immediate wins or losses
    for (auto pattern : patterns) {
        if ((cpu & pattern) == pattern) return WIN;
        if ((player & pattern) == pattern) return LOSS;
    }

    int score = 0;
    for (auto pattern : patterns) {
        int cpuCount = FastPopcount(cpu & pattern);
        int playerCount = FastPopcount(player & pattern);
        
        if (cpuCount == 2 && playerCount == 0) score += 3;
        else if (playerCount == 2 && cpuCount == 0) score -= 3;
        else if (cpuCount == 1 && playerCount == 0) score += 1;
        else if (playerCount == 1 && cpuCount == 0) score -= 1;
    }

    return score;
}

void PrintBoard(Bitboard CPU, Bitboard Player) {
    for (int row = 2; row >= 0; --row) {
        for (int col = 0; col < 3; ++col) {
            int bitIndex = row * 3 + col;
            if ((CPU >> bitIndex) & 1) cout << " @ " ;
            else if ((Player >> bitIndex) & 1) cout << " X "; 
            else cout << " . ";
            if (col < 2) cout << "|";
        }
        cout << endl;
        if (row > 0) cout << "-----------" << endl;
    }
}

void PlayerTurn(Bitboard &player, Bitboard cpu, Bitboard &board) {
    Bitboard free_cells = (~board) & SIGNIFICANT_BITS; // This will let us consider only the free cells
    cout << "Current board:" << endl;
    
    for (int row = 2; row >= 0; --row) {
        for (int col = 0; col < 3; ++col) {
            int bitIndex = row * 3 + col;
            if ((cpu >> bitIndex) & 1) {
                cout << " @ ";
            } else if ((player >> bitIndex) & 1) {
                cout << " X "; 
            } else {
                cout << " " << (bitIndex + 1) << " "; 
            }
            if (col < 2) cout << "|";
        }   
        cout << endl;
        if (row > 0) cout << "-----------" << endl;
    }

    cout << "Choose a free cell: ";
    int choice;
    std::cin >> choice;
    int move = choice - 1;
    while (!(free_cells & (1 << move))) {
        cout << "Invalid choice. Choose a free cell: ";
        std::cin >> choice;
        move = choice - 1;
    }
    player |= (1 << move);
    board |= (1 << move);
}

inline int Minimax(Bitboard board, Bitboard cpu, Bitboard player, bool isMaximizing, int alpha, int beta) {
    uint32_t key = (board << 18) | (cpu << 9) | player | (isMaximizing ? (1 << 18) : 0);
    auto it = TranspositionTable.find(key);
    if (it != TranspositionTable.end()) {
        return it->second;
    }

    int score = Eval(cpu, player);
    if (score == WIN || score == LOSS || FastPopcount(board) == 9) {
        TranspositionTable[key] = score;
        return score;
    }

    int bestScore = isMaximizing ? -1000 : 1000;
    Bitboard free = (~board) & SIGNIFICANT_BITS;

    while (free) {
        Bitboard move = free & (-free);
        Bitboard newBoard = board | move;
        
        int moveScore;
        if (isMaximizing) {
            Bitboard newCpu = cpu | move;
            moveScore = Minimax(newBoard, newCpu, player, false, alpha, beta);
            bestScore = std::max(bestScore, moveScore);
            alpha = std::max(alpha, bestScore);
        } else {
            Bitboard newPlayer = player | move;
            moveScore = Minimax(newBoard, cpu, newPlayer, true, alpha, beta);
            bestScore = std::min(bestScore, moveScore);
            beta = std::min(beta, bestScore);
        }

        if (beta <= alpha) break;
        free &= (free - 1);
    }

    TranspositionTable[key] = bestScore;
    return bestScore;
}

void CPUTurn(Bitboard &cpu, Bitboard &board, Bitboard player) {
    Bitboard bestMove = 0;
    int bestScore = -1000;

    auto start = std::chrono::steady_clock::now();

    Bitboard free = (~board) & SIGNIFICANT_BITS;
    while (free) {
        Bitboard move = free & (-free);
        Bitboard newCpu = cpu | move;
        Bitboard newBoard = board | move;

        if (Eval(newCpu, player) == WIN) {
            bestMove = move;
            break;
        }
        
        int moveScore = Minimax(newBoard, newCpu, player, false, -1000, 1000);
        if (moveScore > bestScore) {
            bestScore = moveScore;
            bestMove = move;
        }
        free &= (free - 1);
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_time = end - start;

    std::cout << "Search time (nanoseconds): " << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_time).count() << std::endl;
    std::cout << "Search time (seconds): " << elapsed_time.count() << std::endl;
    cpu |= bestMove;
    board |= bestMove;
}

int main() {
    Bitboard board = 0b000000000; // Occupied cells
    Bitboard CPU = 0b000000000;   // Board of the cells occupied by the computer
    Bitboard Player = 0b000000000;// Board of the cells occupied by the player

    int c = 0; // Number of turns
    // bool turn = ChooseWhoStarts(); // true for player, false for CPU *** REMOVE COMMENTS FOR RANDOM TURN
    bool turn = 0; // In this program the cpu will play first so that it can be directly compared to the benchmark program

    while (c < 9) {
        if (turn) {
            PlayerTurn(Player, CPU, board);
            if (Eval(CPU, Player) == LOSS) { // Check if player wins
                PrintBoard(CPU, Player);
                cout << "The player has won!" << endl;
                return 0;
            }
        } else {
            CPUTurn(CPU, board, Player);
            if (Eval(CPU, Player) == WIN) { // Check if CPU wins
                PrintBoard(CPU, Player);
                cout << "The computer has won!" << endl;
                return 0;
            }
        }
        turn = !turn;
        c++;
    }
    // If no one has won and the board is full, it's a draw
    if (FastPopcount(board) == 9) {
        PrintBoard(CPU, Player);
        cout << "Draw!" << endl;
    }
    return 0;
}

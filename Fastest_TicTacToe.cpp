#include <iostream>
#include <algorithm>
#include <array>
#include <chrono>
#include <unordered_map>

using std::string;
using std::array;
using std::cout;
using std::endl;

using Bitboard = uint16_t;
#define SIGNIFICANT_BITS 0x1FF
#define WIN 10
#define LOSS -10

constexpr array<uint16_t, 8> patterns = {
    0b100100100, 0b010010010, 0b001001001, // COLS
    0b111000000, 0b000111000, 0b000000111, // ROWS
    0b100010001, 0b001010100               // DIAGONALS
};

std::unordered_map<uint64_t, int> TranspositionTable;

constexpr array<int, 512> PopcountTable = []() {
    array<int, 512> table{};
    for (int i = 0; i < 512; ++i) table[i] = __builtin_popcount(i);
    return table;
}();

inline __attribute__((always_inline)) int FastPopcount(uint16_t x) {
    return PopcountTable[x & SIGNIFICANT_BITS];
}

inline __attribute__((always_inline)) int Eval(Bitboard cpu, Bitboard player) {
    for (const auto& pattern : patterns) {
        if ((player & pattern) == pattern) return LOSS;
        if ((cpu & pattern) == pattern) return WIN;
    }

    int score = 0;
    for (const auto& pattern : patterns) {
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
            if ((CPU >> bitIndex) & 1) cout << " @ ";
            else if ((Player >> bitIndex) & 1) cout << " X ";
            else cout << " . ";
            if (col < 2) cout << "|";
        }
        cout << endl;
        if (row > 0) cout << "-----------" << endl;
    }
}

void PlayerTurn(Bitboard &player, Bitboard cpu, Bitboard &board) {
    Bitboard free_cells = (~board) & SIGNIFICANT_BITS;

    cout << "Current board:" << endl;
    for (int row = 2; row >= 0; --row) {
        for (int col = 0; col < 3; ++col) {
            int bitIndex = row * 3 + col;
            if ((cpu >> bitIndex) & 1) cout << " @ ";
            else if ((player >> bitIndex) & 1) cout << " X ";
            else cout << " " << (bitIndex + 1) << " ";
            if (col < 2) cout << "|";
        }
        cout << endl;
        if (row > 0) cout << "-----------" << endl;
    }

    cout << "Choose a free cell: ";
    int choice;
    std::cin >> choice;
    int move = choice - 1;
    while (!((~board & SIGNIFICANT_BITS) & (1 << move))) {
        cout << "Invalid choice. Choose a free cell: ";
        std::cin >> choice;
        move = choice - 1;
    }
    player |= (1 << move);
    board |= (1 << move);
}

inline int Minimax(Bitboard board, Bitboard cpu, Bitboard player, bool isMax, int alpha, int beta) {
    uint64_t key = (static_cast<uint64_t>(board) << 27)
                 | (static_cast<uint64_t>(cpu) << 18)
                 | (static_cast<uint64_t>(player) << 9)
                 | (isMax ? 1 : 0);
    auto it = TranspositionTable.find(key);
    if (it != TranspositionTable.end()) return it->second;

    int eval = Eval(cpu, player);
    if (eval == WIN || eval == LOSS || FastPopcount(board) == 9) {
        TranspositionTable[key] = eval;
        return eval;
    }

    int bestScore = isMax ? -1000 : 1000;
    Bitboard free = (~board) & SIGNIFICANT_BITS;

    while (free) {
        int moveIdx = __builtin_ctz(free);
        Bitboard move = 1 << moveIdx;
        Bitboard newBoard = board | move;

        int moveScore;
        if (isMax) {
            moveScore = Minimax(newBoard, cpu | move, player, false, alpha, beta);
            bestScore = std::max(bestScore, moveScore);
            alpha = std::max(alpha, bestScore);
        } else {
            moveScore = Minimax(newBoard, cpu, player | move, true, alpha, beta);
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
        int moveIdx = __builtin_ctz(free);
        Bitboard move = 1 << moveIdx;
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
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    auto sec = std::chrono::duration<double>(end - start).count();

    cout << "Search time (nanoseconds): " << ns << endl;
    cout << "Search time (seconds): " << sec << endl;

    cpu |= bestMove;
    board |= bestMove;
}

int main() {
    TranspositionTable.reserve(20000);

    Bitboard board = 0, CPU = 0, Player = 0;
    int c = 0;
    bool turn = 0;

    while (c < 9) {
        if (turn) {
            PlayerTurn(Player, CPU, board);
            if (Eval(CPU, Player) == LOSS) {
                PrintBoard(CPU, Player);
                cout << "The player has won!" << endl;
                return 0;
            }
        } else {
            CPUTurn(CPU, board, Player);
            if (Eval(CPU, Player) == WIN) {
                PrintBoard(CPU, Player);
                cout << "The computer has won!" << endl;
                return 0;
            }
        }
        turn = !turn;
        c++;
    }

    if (FastPopcount(board) == 9) {
        PrintBoard(CPU, Player);
        cout << "Draw!" << endl;
    }

    return 0;
}

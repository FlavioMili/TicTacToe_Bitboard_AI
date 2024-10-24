#include <iostream>
#include <algorithm>
#include <vector>
#include <array>
#include <chrono>
#include <random>

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

// std::unordered_map<Bitboard, int> NormalisedBoards;

// Winning positions
// uint16_t COL1 = 0b100100100;
// uint16_t COL2 = 0b010010010;
// uint16_t COL3 = 0b001001001;
// uint16_t ROW1 = 0b111000000;
// uint16_t ROW2 = 0b000111000;
// uint16_t ROW3 = 0b000000111;
// uint16_t DIA1 = 0b100010001;
// uint16_t DIA2 = 0b001010100;

array<uint16_t, 8> patterns = {
        0b100100100,  // COL1
        0b010010010,  // COL2
        0b001001001,  // COL3
        0b111000000,  // ROW1
        0b000111000,  // ROW2
        0b000000111,  // ROW3
        0b100010001,  // DIA1
        0b001010100   // DIA2
};

// REMOVE THE COMMENTS ALSO IN THE MAIN IN ORDER TO RANDOMISE THE FIRST PLAYER
// bool ChooseWhoStarts() {
//    static std::mt19937 gen{std::random_device{}()};
//    return std::uniform_int_distribution<>(0, 1)(gen) == 0;
//}

vector<Bitboard> PossibleMoves(Bitboard board) {
    vector<Bitboard> moves;
    Bitboard free = (~board) & SIGNIFICANT_BITS;
    while (free) {
        Bitboard move = free & (-free);
        moves.push_back(move);
        free &= (free - 1);
    }
    return moves;
}

static inline int Eval(Bitboard cpu, Bitboard player) {
    // Check for immediate wins or losses
    for (auto pattern : patterns) {
        if ((cpu & pattern) == pattern) return WIN;
        if ((player & pattern) == pattern) return LOSS;
    }

    int score = 0;
    for (auto pattern : patterns) {
        int cpuCount = __builtin_popcount(cpu & pattern);
        int playerCount = __builtin_popcount(player & pattern);
        
        if (cpuCount == 2 && playerCount == 0) score += 3;
        else if (playerCount == 2 && cpuCount == 0) score -= 3;
        else if (cpuCount == 1 && playerCount == 0) score += 1;
        else if (playerCount == 1 && cpuCount == 0) score -= 1;
    }

    return score;
}

// bool CheckLastMove(Bitboard &board, Bitboard &cpu, Bitboard player){ // NOTA BENE: cpu sarebbe chi muove in questo turno per fare il calcolo
//     for(auto pattern : patterns){
//         Bitboard temp = cpu & pattern;
//         if (__builtin_popcount(temp) == 2 && (board & pattern) != pattern){
//                 return true; // Can win
//             // This can be further changed to update the board in case of win
//                 // Bitboard move = pattern & ~board; 
//                 // cpu |= move;
//                 // board |= move;
//         }
//     }
//     return false; // Not yet won
// }

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

static inline int Minimax(Bitboard board, Bitboard cpu, Bitboard player, bool isMaximizing, int &nodes, int alpha, int beta) {
    nodes++; // This was used for debugging and improving performances
    int score = Eval(cpu, player);
    if (score == WIN || score == LOSS || __builtin_popcount(board) == 9) {
        return score;
    }
    vector<Bitboard> moves = PossibleMoves(board);

    if (isMaximizing) {
        int bestScore = -1000;
        for (Bitboard move : moves) {
            Bitboard newCpu = cpu | move;
            Bitboard newBoard = board | move;

            int moveScore = Minimax(newBoard, newCpu, player, false, nodes, alpha, beta);
            bestScore = std::max(bestScore, moveScore);
            alpha = std::max(alpha, bestScore);
            if (beta <= alpha) break;
        }
        return bestScore;
    } 
    else {
        int bestScore = 1000;
        for (Bitboard move : moves) {
            Bitboard newPlayer = player | move;
            Bitboard newBoard = board | move;
            
            int moveScore = Minimax(newBoard, cpu, newPlayer, true, nodes, alpha, beta);
            bestScore = std::min(bestScore, moveScore);
            beta = std::min(beta, bestScore);
            if (beta <= alpha) break;
        }
        return bestScore;
    }
}

void CPUTurn(Bitboard &cpu, Bitboard &board, Bitboard player) {
    Bitboard bestMove = 0;
    int bestScore = -1000;
    int nodes = 0;

    auto start = std::chrono::steady_clock::now();

    vector<Bitboard> moves = PossibleMoves(board);
    // Stop if you can win immediately
    for (Bitboard move : moves) {
        Bitboard newCpu = cpu | move;
        Bitboard newBoard = board | move;
        if (Eval(newCpu, player) == WIN) {
            bestMove = move;
            bestScore = WIN;
            break; 
        }
    }

    if (bestScore != WIN) {
        for (Bitboard move : moves) {
            Bitboard newCpu = cpu | move;
            Bitboard newBoard = board | move;
            int moveScore = Minimax(newBoard, newCpu, player, false, nodes, -1000, 1000);
            if (moveScore > bestScore) {
                bestScore = moveScore;
                bestMove = move;
            }
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_time = end - start;

    std::cout << "Search time (nanoseconds): " << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_time).count() << std::endl;
    std::cout << "Search time (seconds): " << elapsed_time.count() << std::endl;
    // std::cout << "Agent played move: " << __builtin_ctz(bestMove) + 1 << std::endl;
    // std::cout << "Number of nodes: " << nodes << std::endl;
    cpu |= bestMove;
    board |= bestMove;
}

int main() {
    Bitboard board = 0b000000000; // Occupied cells
    Bitboard CPU = 0b000000000;   // Board of the cells occupied by the computer
    Bitboard Player = 0b000000000;// Board of the cells occupied by the player

    int c = 0; // Number of turns
    // bool turn = ChooseWhoStarts(); // true for player, false for CPU *** REMOVE COMMENTS FOR RANDOM TURN
    bool turn = 0; // In this program the cpu will play first so that it can be directly compared to the benchmark program.

    while (c < 9) {
        if (turn) {
            PlayerTurn(Player, CPU, board);
            if (Eval(CPU, Player) == LOSS) { // Check if player wins
                PrintBoard(CPU, Player);
                cout << "The player has won!" << endl;
                return 0;
            }
            turn = !turn;
            c++;
        } else {
            CPUTurn(CPU, board, Player);
            if (Eval(CPU, Player) == WIN) { // Check if CPU wins
                PrintBoard(CPU, Player);
                cout << "The computer has won!" << endl;
                return 0;
            }
            turn = !turn;
            c++;
        }
    }
    // If no one has won and the board is full, it's a draw
    if (__builtin_popcount(board) == 9) {
        PrintBoard(CPU, Player);
        cout << "Draw!" << endl;
    }
    return 0;
}

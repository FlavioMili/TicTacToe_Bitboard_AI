# Efficient Tic-Tac-Toe AI implemented with Bitboards

This project is a Tic-Tac-Toe AI implemented using bitboards. I developed this AI to learn how to use bitboards and how to use minimax algorithm.

## Overview

The core of this project revolves around utilizing bitboards for efficient board state representation and move generation. By leveraging bitwise operations, the "AI" evaluates possible moves and plays optimally with no option to lose.

## Features

- **Bitboard Representation**: Efficient board state management using bitwise operations.
- **Minimax Algorithm**: Implementation with alpha-beta pruning for optimal move decisions.
- **Evaluation Function**: Custom evaluation to balance offensive and defensive strategies as managing double threats.

## Usage

The AI will automatically play against a human player. The player is prompted to select a cell for their move, and the AI responds with its move based on the minimax algorithm with bitboards.

## Performance

#### Benchmark: ([tjnorwat's implementation](https://github.com/tjnorwat/tic-tac-toe-ai-bitboard)) 

  For clarity, the results are tested using the same compilation flags, on the same machine and using the same first calculated move, supposedly the hardest one to calculate.

## tjnorwat implementation 
    1. Search time nanoseconds: 30000
    2. Search time nanoseconds: 11083
    3. Search time nanoseconds: 63916
    4. Search time nanoseconds: 43250
    5. Search time nanoseconds: 42458
    6. Search time nanoseconds: 41000
    7. Search time nanoseconds: 26417
    8. Search time nanoseconds: 33333
    9. Search time nanoseconds: 26291
    10. Search time nanoseconds: 39167
With a mean average of 35691.5 nanoseconds

## My implementation
    1. Search time (nanoseconds): 4500
    2. Search time (nanoseconds): 6834
    3. Search time (nanoseconds): 6667
    4. Search time (nanoseconds): 7750
    5. Search time (nanoseconds): 4167
    6. Search time (nanoseconds): 5666
    7. Search time (nanoseconds): 4750
    8. Search time (nanoseconds): 3208
    9. Search time (nanoseconds): 7416
    10. Search time (nanoseconds): 6917
With a mean average of 5787.7 nanoseconds

## Result 
    My implementation was eventually **6.16 times** faster than tjnorwat's implementation.


# Efficient Tic-Tac-Toe AI implemented with Bitboards

This project is a Tic-Tac-Toe AI implemented using bitboards. I developed this AI to learn how to use bitboards and how to use minimax algorithm.

## Overview

The core of this project revolves around utilizing bitboards for efficient board state representation and move generation. By leveraging bitwise operations, the "AI" evaluates possible moves and plays optimally with no option to lose.

## Performance

The implementation is approximately 60% faster than the one I used as benchmark ([tjnorwat's implementation](https://github.com/tjnorwat/tic-tac-toe-ai-bitboard)) and that was also aimed to be very efficient.

## Features

- **Bitboard Representation**: Efficient board state management using bitwise operations.
- **Minimax Algorithm**: Implementation with alpha-beta pruning for optimal move decisions.
- **Evaluation Function**: Custom evaluation to balance offensive and defensive strategies as managing double threats.

## Usage

The AI will automatically play against a human player. The player is prompted to select a cell for their move, and the AI responds with its move based on the minimax algorithm with bitboards.

# IPC Battle Arena - System V Multiplayer Game

![video](https://github.com/dantol29/lem-ipc/blob/main/lem_ipc.gif)

## Overview

IPC Battle Arena is a multiplayer strategy game where players, grouped into teams, battle to be the last team standing on a 2D board. The game is a demonstration of System V IPC mechanisms - shared memory, message queues, and semaphores.

Each player is a separate process, and all processes interact through shared memory and message queues, ensuring synchronized access using semaphores.

## Features

1. A\* algorithm for finding the shorthest path
2. Intuitive UI
3. Ability to place walls

## Building & Running

1. `make re`
2. `./ipc 1`
3. `./start.sh`

## Inter-Process Communication

1. Shared Memory (SHM)
   Stores the game board — a grid with player positions and team identifiers.

2. Semaphores
   Prevent concurrent access to shared memory, ensuring atomic updates and reads.

3. Message Queues (MSGQ)
   Used for communication between players (coordinated attacks).

## Commands

- `ipcs` - list shared memory, semaphores and queues
- `ipcrm -m id` - delete shared memory

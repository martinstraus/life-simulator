# life-simulator
Life simulator

## How to play

## Game Mechanics

### Creatures

- Each creature occupies a cell in the world grid and has its own **DNA (genome)**, energy, age, and generation number.
- Creatures can perform actions each tick: **move**, **eat**, **reproduce**, or **do nothing**. The action is determined by their DNA and current state.

### DNA (Genome)

- The DNA is a 32-bit integer that encodes behavioral traits:
    - **Probability to move**: Lower bits determine how likely a creature is to move.
    - **Probability to reproduce**: Some bits control the chance to attempt reproduction.
    - **Hunger threshold**: DNA encodes how much energy a creature needs before it tries to eat.
    - **Reproduction age**: DNA determines at what age a creature can reproduce.
- When a creature reproduces, its offspring inherit its DNA, with a chance of mutation (a random bit flip).

### Energy and Food

- Creatures consume energy each tick, with different actions costing different amounts.
- **Eating**: If a creature is hungry and there is food in its cell, it will eat to regain energy.
- **Moving**: Moving to a neighboring cell costs energy.
- **Reproduction**: Reproducing splits the parent's energy among offspring and costs additional energy.
- If a creature's energy drops to zero, it dies.

### Reproduction

- When conditions are met (sufficient age and energy, and a random chance based on DNA), a creature can reproduce.
- Reproduction creates two offspring in nearby free cells, each inheriting the parent's DNA (with possible mutation).
- The parent dies after reproduction.

### Mutation

- Each reproduction event has a configurable probability of mutation.
- Mutation randomly flips one bit in the offspring's DNA, introducing new traits into the population.

### Selection

- You can select a creature by clicking on it. Information about the selected creature (age, energy, generation, etc.) is displayed if information mode is enabled.

### Simulation End

- The simulation ends when no creatures remain alive.

## Controls

- Pause the game: press 'p'.
- Toggle information: press 'i'.
- Speed up: press 'f'.
- Slow down: press 's'.
- Zoom in or out: press '+' or '-', or scroll the mouse wheel.

## Run

### Parameters

* `-h` or `--help`: Get command line help.
* `-s` or `--seed`: Seed to use for randomization; the next parameter must be a positive integer.
* `-c` or `--creatures`: Initial creatures count; the next parameter must be a positive integer.
* `-g` or `--genepool`: Use a gene pool instead of random DNA generation for each creature.
* `-p` or `--poolsize`: The number of DNAs in the gene pool; the next parameter must be a positive integer.
* `-u` or `--update`: The update interval for animation; the next parameter must be a positive integer.
* `-w` or `--width`: Width of the world.
* `-h` or `--height`: Height of the world.
* `-m` or `--mutation`: The probability of mutation when reproducing.
* `-d` or `--density`: Initial creature density (as a float between 0 and 1); overrides `--creatures` if specified.


### Examples

Run with default settings:

    ./life

Get help:

    ./life -h

## Building on MacOS

    brew install glew glfw glm mesa
    sudo mkdir /usr/local/include
    sudo ln -s /opt/homebrew/Cellar/glew/2.2.0_1/include/GL GL
    sudo ln -s /opt/homebrew/Cellar/glfw/3.3.8/include/GLFW/ GLFW
    sudo ln -s /opt/homebrew/Cellar/glm/0.9.9.8/include/glm/ glm

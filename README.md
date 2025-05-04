# life-simulator
Life simulator

## How to play

### Actions

- Pause the game: press 'p'.
- Toggle information: press 'i'.
- Speed up: press 'f'.
- Slow down: press 's'.
- Zoom in or out: press '+' or '-', or scroll the mouse wheel.

### Run

#### Parameters

* '-h' or '--help': get command line help.
* '-s' or '--seed': seed to use for randomization; the next parameter must be a positive integer number.
* '-c' or '--creatures': initial creatures count; the next parameter must be a positive integer number.
* '-g' or '--genepool': use a gene pool instead of random DNA generation for each creature.
* '-p' or '--poolsize': the number of DNAs in the gene pool; the next parameter must be a positive integer number.
* '-u' or '--update':  the update interval for animation; the next parameter must be a positive integer number.
* '-w' or '--width': width of the world.
* '-h' or '--height': height of the world.
* '-m' or '--mutation': the probability of mutation when reproducing.

#### Examples

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
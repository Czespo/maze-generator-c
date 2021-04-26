
# maze-generator-c
Maze generator written in C using SDL2. If you have any trouble compiling or running this software, please [start an issue](https://github.com/Czespo/maze-generator-c/issues/new)!

## Compiling
You need the SDL2 headers and static libraries. You can probably get them from <http://libsdl.org/download-2.0.php>.

## Running
Download the SDL2 runtime binaries, or build them from source: <http://libsdl.org/download-2.0.php>.

The maze generator takes some options from a command line, creates a maze, and writes it to a .bmp file. By default, a window will pop up which shows you the construction of the maze in real time. If you don't want to see the maze generation, use the `-q` flag.

### All options
- `-h`, shows help message.
- `-w <width> <height>`, width and height of the viewer window. Default 800x640.
- `-f -fps <number>`, FPS of the viewer. Default 10.
- `-s -size <width> <height>`, width and height of the maze. Default 20x20.
- `-m -mode <name>`, method used to generate the maze. One of 'random', 'depth', 'breadth'. Default 'depth'.
- `-switch <chance>`, chance (0-100) that a head will switch to another branch. Only used in `random` mode. Default 10.
- `-step <size>`, number of steps the head takes in any direction. Default 2.
- `-h -heads <number>`, number of heads that create the maze. Default 1.
- `-q`, quiet mode. No window showing maze generation.
- `-seed <seed>`, seed used for random number generation. Default RANDOM.
- `-o <path>`, saves the final state of the maze to this file. Default 'maze.bmp'.

## Controls
- If not in quiet mode, use the `equals` and `minus` keys to make the viewer speed up or slow down, respectively.

## Algorithm
First of all, an array of cells is created with all of the cells initialized to 'walls'. Then, a number of 'heads' explore the array using various methods.

### Depth-first
In `depth` mode, each head picks a random adjacent wall and walks through it, setting the cells it walks over to 'floors'. If there are no adjacent walls, the head backtracks to the *last* cell where it saw a wall. If there is no last cell with walls, the maze generation is done.

### Breadth-first
`breadth` mode works exactly like `depth` mode, except that when there are no adjacent walls, the head returns to the *first* cell where it saw a wall.

### Random
With `-mode random` and `-switch 100`, the construction of the maze resembles [Prim's Algorithm](https://en.wikipedia.org/wiki/Prim%27s_algorithm). Otherwise, it resembles breadth- and depth-first except that the head backtracks to a random cell when there are no adjacent walls.

## Further reading
- https://en.wikipedia.org/wiki/Maze_generation_algorithm

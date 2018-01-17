![Snake Cube Puzzle](https://github.com/ThoAppelsin/snake-cube-puzzle-variations/raw/master/SnakeCubePuzzle.jpg)

Snake cube puzzle is a toy object of 27 small cubes (usually wooden) attached in pairs like a chain.
Under a certain configuration, it becomes a 3x3x3 cube.

This application, written in C, lists all the possible and solvable variations of the snake cube puzzle,
along with its solutions, in a very optimized way that it only takes about 15 seconds on a Windows 10 tablet PC.

## How it became interesting

Back in the March of 2015, I was first introduced to this puzzle by a graduate friend pursuing his PhD.
It took me about half an hour to solve it, but then a couple of weeks more to come up with this final version of this application.
Then this question came up to my mind:

### Is this the only solution?

We both were at the Computer Engineering department, so we thought that we could just:

1) Model the problem.
2) Let the computer solve it exhaustively with respect to the given constraints.

He did not have the time. I went ahead and have done it. Indeed, it was the only solution to the variation we had.

At this stage, I did not have to optimize my structures and algorithms in any way. The problem was small enough
that even my very lightweight tablet could handle within a snap. However, then I had this next question:

### Is this the only configuration with a unique solution?

Simple enough, I now only had to generate all the configurations, let them be solved by my readily available solver.
After that, filtering out the configurations without a solution, and then writing down all the configurations and their solutions
onto a file, should just about do it.

I did all that, and let my program run.
Seeing that it is taking a while, I added a progress indicator, then re-started it as we went to lunch.
After just about an hour, the program was seemingly stuck at some point, already consumed about 95% of the installed 4GB RAM.

Long story short, it took me weeks of re-thinking and re-writing to optimize my algorithms and approaches in modelling,
after which the program was finally able to do it under half an hour, then half a minute.

Those timings were with a Samsung Ativ Tab 7 on Windows 8.1.
The same code now runs under 15 seconds on a Surface Pro 4 with Windows 10.

## Results

The entire file of results can be found on the repository at [SnakeCubePuzzle/solutions.txt](https://raw.githubusercontent.com/ThoAppelsin/snake-cube-puzzle-variations/master/SnakeCubePuzzle/solutions.txt).
Here are some of the points that were important to me:

- **Number of configurations:** 11487
- **Number of configurations with unique solutions:** 3639
- **Maximum number of solutions to a configuration:** 142

These numbers might be an overstatement, for that various symmetries emerge during the process.
I had eliminated a couple of those, but may have missed a couple more.

Most notable symmetries that emerge are:

1) A configuration may be modelled in reverse, and it would actually be the same puzzle, in reverse.
2) A solution may be rotated in all three axes by 90, 180, and 270 degrees, and still be a solution.
3) Those rotations may be combined, and still be solutions.

These I must have eliminated, or at least tried to.

## Models

I had two aspects of the puzzle to model: the configurations, and the solutions.

### Configurations

My model of the configurations come from the types of cubes used on the puzzle itself.
There are two of them:

1) Straights &#8211; either the ones with a single hole, or the ones with two holes with coinciding axes, denoted with a `-`
2) Corners &#8211; the ones with two holes with axes perpendicular to each other, denoted with a `o`

The one we had had the following configuration:

```
--ooo-oo-ooo-o-oooo-o-o-o--
--o-o-o-oooo-o-ooo-oo-ooo--
```

Here, the first one is referred as the primary variant, and the secondary variant is the mirrored version of the primary.

### Solutions

Solutions are closely related to the configurations that they belong to.
While a configuration has 27 bits of information, a solution has 26, according to our model.
Starting from an assumed initial position that the first cube is located at, a solution dictates
towards which direction the next cube should be, in order to get the puzzle to its solution.

To apply a solution, solving individual first needs to assume and visualize 3 perpendicular directions to work upon.
It should not matter whether it is a right-handed or left-handed coordinate system.

As an example, the one we have the following configuration as its solution:

```
+x +x +y -x +z +z +y -z -z +x +z -y -y -x -x +y -z +y +z +z -y -y +x +x +y +y 
```

This solution is to be applied to the primary variant.
Just as I did, you can obtain this solution from the [solutions.txt](https://raw.githubusercontent.com/ThoAppelsin/snake-cube-puzzle-variations/master/SnakeCubePuzzle/solutions.txt), by searching for the configuration itself on the file.

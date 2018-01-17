#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <inttypes.h>

#define BIT(order) (1u << (order))
#define ISBITSET(x, bit) (!!((x) & BIT(bit)))

typedef struct coordinatetag
{
	int8_t x[3];
} coordinate;

typedef enum axistag
{
	xaxis,
	yaxis,
	zaxis
} axis;

typedef enum directiontag
{
	xplu,
	xmin,
	yplu,
	ymin,
	zplu,
	zmin
} direction;

typedef struct snakestatetag
{
	direction directions[27];
	enum dimensionalstatetag
	{
		linear,
		planar,
		spatial
	} dimensionalstate;
	coordinate occupiedcoords[27];
	int8_t limits[6];
} snakestate;

typedef struct snakestatestag
{
	// array of possible states
	snakestate * statesarray;
	size_t statecount;
	size_t arraysize;
} snakestates;

typedef struct configurationtreenodetag
{
	uint32_t configurationbits;
	snakestates possiblestates;
	struct configurationtreenodetag * streetorcorner[2];
} configurationtreenode;

typedef struct configurationstag
{
	configurationtreenode ** configurationsarray;
	size_t configurationcount;
	size_t arraysize;
} configurations;

int turn( direction current, int offset )
{
	switch ( current / 2 )
	{
		case 0:
			return offset + 2;
		case 1:
			return offset + (offset < 2 ? 0 : 2);
		case 2:
			return offset;
	}

	// invalid direction in the first place
	return -1;
}

/*
Directions:
0	x+
1	x-
2	y+
3	y-
4	z+
5	z-

Axes:
0	x
1	y
2	z

(direction / 2) is the corresponding axis
*/
coordinate coordinatestep( coordinate a, direction dir )
{
	if ( dir % 2 == 0 )
	{
		a.x[dir / 2]++;
	}
	else
	{
		a.x[dir / 2]--;
	}

	return a;
}

int samecoordinate( const coordinate * const c1, const coordinate * const c2 )
{
	for ( int i = 0; i < sizeof c1->x / sizeof * c1->x; i++ )
	{
		if ( c1->x[i] != c2->x[i] )
		{
			return 0;
		}
	}

	return 1;
}

// returns whether the move was valid or invalid
int move( snakestate * thestate, int current )
{
	direction currentdir = thestate->directions[current];
	axis diraxis = currentdir / 2;

	coordinate newcoordinate =
		thestate->occupiedcoords[current + 1] =
		coordinatestep( thestate->occupiedcoords[current], currentdir );

	if ( currentdir % 2 == 0 ) // positive direction
	{
		if ( thestate->limits[currentdir] < newcoordinate.x[diraxis] )
		{
			thestate->limits[currentdir] = newcoordinate.x[diraxis];
		}
	}
	else // negative direction
	{
		if ( thestate->limits[currentdir] > newcoordinate.x[diraxis] )
		{
			thestate->limits[currentdir] = newcoordinate.x[diraxis];
		}
	}

	if ( (thestate->limits[2 * diraxis] - thestate->limits[2 * diraxis + 1]) >= 3 )
	{
		return 0;
	}

	for ( int i = 0; i <= current; i++ )
	{
		if ( samecoordinate( &thestate->occupiedcoords[i], &newcoordinate ) )
		{
			return 0;
		}
	}

	return 1;
}

configurationtreenode * initializeconfigurationtree( void )
{
	configurationtreenode * firstbit = calloc( 1, sizeof * firstbit );

	firstbit->possiblestates.statecount = 1;
	firstbit->possiblestates.arraysize = 1;
	firstbit->possiblestates.statesarray = calloc( firstbit->possiblestates.arraysize, sizeof * firstbit->possiblestates.statesarray );
	move( &firstbit->possiblestates.statesarray[0], 0 );

	return firstbit;
}

void appendpossiblestate( configurationtreenode * anode, snakestate * astate )
{
	anode->possiblestates.statecount++;
	if ( anode->possiblestates.arraysize < anode->possiblestates.statecount )
	{
		anode->possiblestates.arraysize = anode->possiblestates.arraysize ? 2 * anode->possiblestates.arraysize : 1;
		anode->possiblestates.statesarray = realloc( anode->possiblestates.statesarray, anode->possiblestates.arraysize * sizeof * anode->possiblestates.statesarray );
	}
	anode->possiblestates.statesarray[anode->possiblestates.statecount - 1] = *astate;
}

int broisdeadbranch( uint32_t conf, int mainbroat )
{
	conf ^= BIT( mainbroat );

	if ( mainbroat > 13 )
	{
		uint32_t smallerhalf = conf;
		uint32_t greaterhalf = 0U;

		for ( int i = 0; i < 13; i++ )
		{
			greaterhalf = 2 * greaterhalf + smallerhalf % 2;
			smallerhalf /= 2;
		}

		smallerhalf /= 2;
		smallerhalf %= 1U << (mainbroat - 13);

		if ( greaterhalf < smallerhalf )
		{
			// dead
			return 1;
		}
	}

	for ( int i = 0; i < 2; i++ )
	{
		if ( ISBITSET( conf, mainbroat - i ) )
		{
			// fine
			return 0;
		}
	}

	// dead if not fine...
	return 1;
}

// returns the node which contains the information for the configuration in question
configurationtreenode * solve( uint32_t conf, configurationtreenode * firstbit )
{
	configurationtreenode * thisconfigurationpath[26] = { firstbit };
	int current = 0;

	uint32_t usedtobeconf = conf / 2;
	while ( thisconfigurationpath[current]->streetorcorner[usedtobeconf % 2] )
	{
		thisconfigurationpath[current + 1] = thisconfigurationpath[current]->streetorcorner[usedtobeconf % 2];
		current++;
		usedtobeconf /= 2;
	}

	if ( thisconfigurationpath[current]->possiblestates.statecount == 0 )
	{
		return NULL;
	}

	int branchingpoint = current;

	while ( ++current < 26 )
	{
		thisconfigurationpath[current] =
			thisconfigurationpath[current - 1]->streetorcorner[usedtobeconf % 2] =
			calloc( 1, sizeof * thisconfigurationpath[current] );

		if ( usedtobeconf % 2 )
		{
			for ( unsigned int i = 0; i < thisconfigurationpath[current - 1]->possiblestates.statecount; i++ )
			{
				snakestate basenextstate = thisconfigurationpath[current - 1]->possiblestates.statesarray[i];
				snakestate nextstate;

				switch ( thisconfigurationpath[current - 1]->possiblestates.statesarray[i].dimensionalstate )
				{
					case linear:
						nextstate = basenextstate;
						nextstate.directions[current] = yplu;
						if ( move( &nextstate, current ) )
						{
							nextstate.dimensionalstate = planar;
							appendpossiblestate( thisconfigurationpath[current], &nextstate );
						}
						break;
					case planar:
						for ( int j = 0; j < 2; j++ )
						{
							nextstate = basenextstate;
							nextstate.directions[current] = turn( nextstate.directions[current - 1], j );
							if ( move( &nextstate, current ) )
							{
								appendpossiblestate( thisconfigurationpath[current], &nextstate );
							}
						}
						nextstate = basenextstate;
						nextstate.directions[current] = zplu;
						if ( move( &nextstate, current ) )
						{
							nextstate.dimensionalstate = spatial;
							appendpossiblestate( thisconfigurationpath[current], &nextstate );
						}
						break;
					case spatial:
						for ( int j = 0; j < 4; j++ )
						{
							nextstate = basenextstate;
							nextstate.directions[current] = turn( nextstate.directions[current - 1], j );
							if ( move( &nextstate, current ) )
							{
								appendpossiblestate( thisconfigurationpath[current], &nextstate );
							}
						}
						break;
				}
			}
		}
		else
		{
			for ( unsigned int i = 0; i < thisconfigurationpath[current - 1]->possiblestates.statecount; i++ )
			{
				snakestate nextstate = thisconfigurationpath[current - 1]->possiblestates.statesarray[i];

				nextstate.directions[current] = nextstate.directions[current - 1];
				if ( move( &nextstate, current ) )
				{
					appendpossiblestate( thisconfigurationpath[current], &nextstate );
				}
			}
		}

		if ( thisconfigurationpath[current - 1]->streetorcorner[!(usedtobeconf % 2)] || broisdeadbranch( conf, current ) )
		{
			free( thisconfigurationpath[current - 1]->possiblestates.statesarray );
			thisconfigurationpath[current - 1]->possiblestates.statesarray = NULL;
			thisconfigurationpath[current - 1]->possiblestates.arraysize = 0;
			thisconfigurationpath[current - 1]->possiblestates.statecount = 0;
		}

		if ( thisconfigurationpath[current]->possiblestates.statecount == 0 )
		{
			/*while ( broisdeadbranch( conf, current ) ||
			current > 0 &&
			thisconfigurationpath[current - 1]->streetorcorner[!(ISBITSET( conf, current ))] &&
			thisconfigurationpath[current - 1]->streetorcorner[!(ISBITSET( conf, current ))]->possiblestates.statecount == 0 )
			{
			free( thisconfigurationpath[current - 1]->streetorcorner[0] );
			thisconfigurationpath[current - 1]->streetorcorner[0] = NULL;

			free( thisconfigurationpath[current - 1]->streetorcorner[1] );
			thisconfigurationpath[current - 1]->streetorcorner[1] = NULL;

			free( thisconfigurationpath[current - 1]->possiblestates.statesarray );
			thisconfigurationpath[current - 1]->possiblestates.statesarray = NULL;
			thisconfigurationpath[current - 1]->possiblestates.arraysize = 0;
			thisconfigurationpath[current - 1]->possiblestates.statecount = 0;

			current--;
			}*/

			return NULL;
		}

		usedtobeconf /= 2;
	}

	thisconfigurationpath[25]->configurationbits = conf;

	return thisconfigurationpath[25];
}

void fprintdirectivesolution( FILE * output, direction * solution )
{
	for ( int i = 0; i < 26; i++ )
	{
		char sign = (solution[i] % 2 == 0) ? '+' : '-';
		switch ( solution[i] / 2 )
		{
			case 0:		fprintf( output, "%cx ", sign );	break;
			case 1:		fprintf( output, "%cy ", sign );	break;
			case 2:		fprintf( output, "%cz ", sign );	break;
		}
	}
	fprintf( output, "\n\n" );
}

void fprintgraphicalsolution( FILE * output, int * solution )
{
	int layout[5][5][5] = { 0 };
	coordinate current =
	{
		.x = { 2, 2, 2 }
	};
	int counter = 1;

	layout[current.x[0]][current.x[1]][current.x[2]] = counter++;
	for ( int i = 0; i < 26; i++ )
	{
		current.x[solution[i] / 2] += (solution[i] % 2 == 0) ? 1 : -1;
		layout[current.x[0]][current.x[1]][current.x[2]] = counter++;
	}

	coordinate relativeorigin = current;
	for ( int i = 0; i < 3; )
	{
		if ( relativeorigin.x[i] == 0 )
		{
			i++;
			continue;
		}

		relativeorigin.x[i]--;

		if ( layout[relativeorigin.x[0]][relativeorigin.x[1]][relativeorigin.x[2]] == 0 )
		{
			relativeorigin.x[i]++;
			i++;
			continue;
		}
	}

	for ( int q = 0; q < 3 * 9 + 3 * 2; q++ )
	{
		fputc( '-', output );
	}
	fputc( '\n', output );

	for ( int k = 0; k < 3; k++ )
	{
		for ( int j = 0; j < 3; j++ )
		{
			for ( int i = 0; i < 3; i++ )
			{
				for ( int q = 0; q < 6 - 3 * j; q++ )
				{
					fputc( ' ', output );
				}

				fprintf( output, "%2d ", layout[relativeorigin.x[0] + i][relativeorigin.x[1] + j][relativeorigin.x[2] + k] );

				for ( int q = 0; q < 3 * j; q++ )
				{
					fputc( ' ', output );
				}

				if ( i != 2 )
				{
					fprintf( output, " | " );
				}
			}
			fputc( '\n', output );
		}

		for ( int q = 0; q < 3 * 9 + 3 * 2; q++ )
		{
			fputc( '-', output );
		}
		fputc( '\n', output );
	}

	fputc( '\n', output );
}

void realdealsolver( void )
{
	// --o-o-o-oooo-o-ooo-oo-ooo--
	uint32_t conf = BIT( 2 ) + BIT( 4 ) + BIT( 6 ) + BIT( 8 ) + BIT( 9 ) + BIT( 10 ) + BIT( 11 ) + BIT( 13 ) +
		BIT( 15 ) + BIT( 16 ) + BIT( 17 ) + BIT( 19 ) + BIT( 20 ) + BIT( 22 ) + BIT( 23 ) + BIT( 24 );
	configurationtreenode * firstbit = initializeconfigurationtree( );

	configurationtreenode * answerbit = solve( conf, firstbit );

	if ( answerbit == NULL )
	{
		puts( "No solution" );
		return;
	}

	printf( "There are %d solutions\n", answerbit->possiblestates.statecount );

	FILE * output;
	output = fopen( "solutions.txt", "w" );
	if ( output == NULL )
	{
		fprintf( stderr, "failed to generate output file\n" );
		return;
	}

	for ( unsigned int i = 0; i < answerbit->possiblestates.statecount; i++ )
	{
		fprintf( output, "Solution #%d:\n", i + 1 );
		fprintdirectivesolution( output, answerbit->possiblestates.statesarray[i].directions );
		// fprintgraphicalsolution( output, answerbit->possiblestates.statesarray[i].directions );
	}
}

int configurationvalidity( uint32_t configuration )
{
	int nonshoulderspree = 0;
	for ( int i = 1; i < 26; i++ )
	{
		if ( ISBITSET( configuration, i ) )
		{
			nonshoulderspree = 0;
		}
		else
		{
			nonshoulderspree++;
		}

		if ( nonshoulderspree == 2 )
		{
			return 0;
		}
	}

	return 1;
}

void solvableconfigurationsgatherer( configurationtreenode * currentbit, int currentcount, configurations * solvableconfigurations )
{
	if ( currentcount == 26 )
	{
		if ( currentbit->possiblestates.statecount )
		{
			solvableconfigurations->configurationcount++;
			if ( solvableconfigurations->arraysize < solvableconfigurations->configurationcount )
			{
				solvableconfigurations->arraysize *= 2;
				solvableconfigurations->configurationsarray = realloc( solvableconfigurations->configurationsarray, solvableconfigurations->arraysize * sizeof * solvableconfigurations->configurationsarray );
			}

			solvableconfigurations->configurationsarray[solvableconfigurations->configurationcount - 1] = currentbit;
		}

		return;
	}

	if ( currentbit->streetorcorner[0] )
	{
		solvableconfigurationsgatherer( currentbit->streetorcorner[0], currentcount + 1, solvableconfigurations );
	}

	if ( currentbit->streetorcorner[1] )
	{
		solvableconfigurationsgatherer( currentbit->streetorcorner[1], currentcount + 1, solvableconfigurations );
	}
}

void generalsolver( void )
{
	clock_t start = clock( );

	configurationtreenode * firstbit = initializeconfigurationtree( );

	// from 12 all 0's
	// to 12 all 1's
	// [0 ... 2^12)
	// 12 comes from (27 - 2) / 2

	// 0 i11 ... i01 i00 CENT j00 j01 ... j11 0
	// 0  1  ...  11  12  13   14  15 ...  25 26

	int progress = 0;
	int threshold = 0;
	int entireprogress = 1U << 23;
	for ( uint32_t i = 0; i < (1U << 12); i++ )
	{
		uint32_t configuration1 = 0U;

		int shiftsleft = 12;
		for ( uint32_t q = i; q; q /= 2 )
		{
			configuration1 = (configuration1 + q % 2) << 1;
			shiftsleft--;
		}
		configuration1 = configuration1 << shiftsleft;

		for ( uint32_t j = 0; j <= i; j++ )
		{
			uint32_t configuration2 = configuration1 + (j << 14);

			if ( configurationvalidity( configuration2 ) )
			{
				solve( configuration2, firstbit );
			}
			if ( configurationvalidity( configuration2 + BIT( 13 ) ) )
			{
				solve( configuration2 + BIT( 13 ), firstbit );
			}

			progress++;
			if ( progress > threshold * entireprogress / 50 )
			{
				printf( "%d%% done\n", threshold * 2 );
				threshold++;
			}
		}
	}

	configurations solvableconfigurations =
	{
		.configurationsarray = malloc( sizeof * solvableconfigurations.configurationsarray ),
		.configurationcount = 0,
		.arraysize = 1
	};

	solvableconfigurationsgatherer( firstbit, 1, &solvableconfigurations );

	printf( "There are %d snakes\n", solvableconfigurations.configurationcount );

	FILE * output;
	output = fopen( "solutions.txt", "w" );
	if ( output == NULL )
	{
		fprintf( stderr, "failed to generate output file\n" );
		return;
	}

	fprintf( output, "# %d ticks were required for calculation, CLOCKS_PER_SEC = %d\n\n", clock( ) - start, CLOCKS_PER_SEC );

	for ( size_t j = 0; j < solvableconfigurations.configurationcount; j++ )
	{
		fprintf( output, "\tStructure #%d: ", j + 1 );
		for ( int i = 0; i < 27; i++ )
		{
			fprintf( output, "%c", (ISBITSET( solvableconfigurations.configurationsarray[j]->configurationbits, i )) ? 'o' : '-' );
		}
		fputc( '\n', output );
		fputc( '\n', output );
		for ( unsigned int i = 0; i < solvableconfigurations.configurationsarray[j]->possiblestates.statecount; i++ )
		{
			fprintf( output, "Solution #%d:\n", i + 1 );
			fprintdirectivesolution( output, solvableconfigurations.configurationsarray[j]->possiblestates.statesarray[i].directions );
			// fprintgraphicalsolution( output, solvableconfigurations.configurationsarray[j]->possiblestates.statesarray[i].directions );
		}
		fputc( '\n', output );
	}

	fclose( output );
}

int main( void )
{
	clock_t cl = clock( );

	// realdealsolver( );
	generalsolver( );

	printf( "%u ticks\n", clock( ) - cl );

	return 0;
}
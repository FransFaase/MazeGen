#include <stdio.h>
#include <stdlib.h>
#include <time.h>

class Maze
{
public:
	Maze(int w, int h) : _w(w), _h(h), _depth(0)
	{
		_vert = new bool[(w-1)*h];
		for (int i = 0; i < (w-1)*h; i++)
			_vert[i] = true;
		_horz = new bool[w*(h-1)];
		for (int i = 0; i < w*(h-1); i++)
			_horz[i] = true;
	}
	~Maze()
	{
		delete[] _vert;
		delete[] _horz;
	}
	bool &right(int i, int j) { /*printf("right(%d,%d)\n", i, j);*/return _vert[_h*i + j]; }
	bool &left(int i, int j) { /*printf("left(%d,%d)\n", i, j);*/return _vert[_h*(i-1) + j]; }
	bool &bottom(int i, int j) { /*printf("bottom(%d,%d)\n", i, j);*/return _horz[i + _w*j]; }
	bool &top(int i, int j) { /*printf("top(%d,%d)\n", i, j);*/return _horz[i + _w*(j-1)]; }

	void generateRecursive()
	{
		bool *visited = new bool[_w*_h];
		for (int i = 0; i < _w*_h; i++)
			visited[i] = false;
		_recurse(0, 0, visited);
		delete[] visited;
	}
	void generateSplit()
	{
		_split(0, 0, _w, _h);
	}
	enum frac_type { frac_regular, frac_reverse, frac_random_orient_no_cross, frac_reverse_random_orient_no_cross, frac_random_orient, frac_all_random };
	void generateFractal(frac_type type)
	{
		generateFractal(_w/2, _h/2, type);
	}
	void generateFractal(int i, int j, frac_type type)
	{
		int size = 1;
		while (   i - size > 0 || i + size < _w-1
		       || j - size > 0 || j + size < _w-1)
			size *= 2;
		_fractal(i, j, size, type, -1);
	}
	void print()
	{
		for (int j = 0; j < _h; j++)
		{
			for (int i = 0; i < _w; i++)
				printf("+%c", _hasWall(i, j, 3) ? '-' : ' ');
			printf("+\n");
			for (int i = 0; i < _w; i++)
				printf("%c ", _hasWall(i, j, 2) ? '|' : ' ');
			printf("|\n");
		}
		for (int i = 0; i < _w; i++)
			printf("+-");
		printf("+\n");
	}
	void printStats()
	{
		int types[16];
		for (int i = 0; i < 16; i++)
			types[i] = 0;
		for (int i = 0; i < _w; i++)
			for (int j = 0; j < _h; j++)
				types[  (_hasWall(i, j, 0) ? 0 : 1)
				      | (_hasWall(i, j, 1) ? 0 : 2)
				      | (_hasWall(i, j, 2) ? 0 : 4)
				      | (_hasWall(i, j, 3) ? 0 : 8)]++;
		for (int i = 0; i < 16; i++)
			if (types[i] > 0)
			{
				printf(" ");
				for (int d = 0; d < 4; d++)
					if ((i & (1 << d)) != 0)
						printf("%c", "rblt"[d]);
				printf(":%d", types[i]);
			}
		printf("\n");
		printf("degree 1: %d\n", types[1] + types[2] + types[4] + types[8]);
		int two_straight = types[1+4] + types[2 + 8];
		int two_turn = types[1 + 2] + types[2 + 4] + types[4 + 8] + types[8 + 1];
		if (two_straight + two_turn > 0)
		{
			printf("degree 2: %d", two_straight + two_turn);
			if (two_straight > 0)
				printf(" straight:%d", two_straight);
			if (two_turn > 0)
				printf(" turn:%d", two_turn);
			printf("\n");
		}
		int three = types[1 + 2 + 4] + types[2 + 4 + 8] + types [4 + 8 + 1] + types[8 + 1 + 2];
		printf("degree 3: %d\n", three);
		if (types[1 + 2 + 4 + 8])
			printf("degree 4: %d\n", types[1 + 2 + 4 + 8]);
	}
	void dump()
	{
		for (int j = 0; j < _h; j++)
		{
			for (int i = 0; i < _w; i++)
				printf("%c%c%c%c ", 
						_hasWall(i, j, 0) ? ' ' : 'r',
						_hasWall(i, j, 1) ? ' ' : 'b',
						_hasWall(i, j, 2) ? ' ' : 'l',
						_hasWall(i, j, 3) ? ' ' : 't');
			printf("\n");
		}
	}

	class iterator
	{
	public:
		iterator(Maze &maze, int i, int j, int d) : _maze(maze), _s_i(i), _s_j(j), _s_d(d), _i(i), _j(j), _d(d), _turn(0), _state(0) { next(); }
		int i() { return _i; }
		int j() { return _j; }
		int d() { return _d; }
		int turn() { return _turn; }
		bool more() { return _state != 0; }
		void next()
		{
			switch(_state) {
				case 0: goto L0;
				case 1: goto L1;
				case 2: goto L2;
				case 3: goto L3;
			}
			
			L0:
			do
			{
				if (!_maze._hasWall(_i, _j, _d-1))
				{
					_turn = -1;
					_state = 1; return; L1:
					_d = (_d+3)%4;
					goto Lmove;
				}
				else if (!_maze._hasWall(_i, _j, _d))
				{
					Lmove:
					switch (_d)
					{
						case 0: _i++; break;
						case 1: _j++; break;
						case 2: _i--; break;
						case 3: _j--; break;
					}
					_turn = 0;
					_state = 2; return; L2:;
				}
				else
				{
					_turn = 1;
					_state = 3; return; L3:;
					_d = (_d+1)%4;
				}
			}
			while (_i != _s_i || _j != _s_j || _d != _s_d);
			_state = 0;
		}
	private:
		Maze &_maze;
		int _s_i, _s_j, _s_d;
		int _i, _j, _d;
		int _turn;
		int _state;
	};

	void removeCrosses()
	{
		for (int k = _w + _h - 2; k > 0; k--)
			for (int i = 0, j = k; i < _w; i++, j--)
				if (0 <= j && j < _h && _nrWalls(i, j) == 0)
				{
					printf("Cross at %d, %d\n", i, j);
					top(i, j) = true;
					printf("%d %d %d %d\n", right(i,j), bottom(i,j), left(i,j), top(i,j));
					int min_k = k;
					int best_i = i, best_j = j;
					for (iterator it(*this, i, j, 0); it.more(); it.next())
						if (it.turn() == 0 && it.i() + it.j() < min_k)
						{
							//printf("  %2d %2d %d\n", it.i(), it.j(), it.d());
							min_k = it.i() + it.j();
							best_i = it.i();
							best_j = it.j();
							if (min_k == 0)
								break;
						}
					printf(" Found min %d,%d %d\n", best_i, best_j, min_k);
					if (min_k == 0)
					{
						min_k = k-1;
						best_i = i;
						best_j = j-1;
						for (iterator it(*this, i, j-1, 2); it.more(); it.next())
							if (it.turn() == 0 && it.i() + it.j() < min_k)
							{
								min_k = it.i() + it.j();
								best_i = it.i();
								best_j = it.j();
							}
						printf(" Found min %d,%d\n", best_i, best_j);
					}
					if (best_j > 0)
						top(best_i, best_j) = false;
					else
						left(best_i, best_j) = false;
				}
	}
		
	bool check()
	{
		int count = 0;
		for (iterator it(*this, 0, 0, 0); it.more(); it.next())
			if (it.turn() == 0)
				count++;
		//printf("%d == %d\n", count, 2*(_w*_h - 1));
		return count == 2*(_w*_h - 1);
	}
	
	void svg(const char *filename, double wall_width, double hall_width, const char *color, double stroke_width)
	{
		FILE *f = fopen(filename, "wt");
		if (f == 0)
		{
			fprintf(stderr, "Cannot open file '%s' for writing\n", filename);
			return;
		}
		fprintf(f, "<svg width=\"%.0f\" height=\"%.0f\" xmlns=\"http://www.w3.org/2000/svg\">\n",
				(wall_width+hall_width)*(_w+1), (wall_width+hall_width)*(_h+1));
		fprintf(f, "<path d=\"M%.2lf %.2lf\n", hall_width/2, hall_width/2);
		fprintf(f, "L %.2lf %.2lf\n", hall_width/2 + (_w+1)*wall_width + _w*hall_width, hall_width/2);
		fprintf(f, "L %.2lf %.2lf\n", hall_width/2 + (_w+1)*wall_width + _w*hall_width, hall_width/2 + (_h+1)*wall_width + _h*hall_width);
		fprintf(f, "L %.2lf %.2lf\n", hall_width/2, hall_width/2 + (_h+1)*wall_width + _h*hall_width);
		fprintf(f, "Z\" stroke=\"%s\" stroke-width=\"%.2lf\" fill-opacity=\"0.0\"/>\n", color, stroke_width);
		fprintf(f, "<path d=\"M%.2lf %.2lf\n",
			(hall_width + wall_width) - hall_width/2,
			(hall_width + wall_width) - hall_width/2);
		for (iterator it(*this, 0, 0, 0); it.more(); it.next())
		{
			if (it.turn() == -1) 
				fprintf(f, "L %.2lf %.2lf\n",
					 (hall_width + wall_width)*(it.i()+1) + hall_width/2*((it.d() == 0 || it.d() == 3) ? -1 : 1),
					 (hall_width + wall_width)*(it.j()+1) + hall_width/2*((it.d() == 0 || it.d() == 1) ? -1 : 1));
			if (it.turn() == 1) 
				fprintf(f, "L %.2lf %.2lf\n",
					 (hall_width + wall_width)*(it.i()+1) + hall_width/2*((it.d() == 2 || it.d() == 3) ? -1 : 1),
					 (hall_width + wall_width)*(it.j()+1) + hall_width/2*((it.d() == 0 || it.d() == 3) ? -1 : 1));
		}
		fprintf(f, "\" stroke=\"%s\" stroke-width=\"%.2lf\" fill-opacity=\"0.0\"/></svg>\n", color, stroke_width);
		fclose(f);
	}
public:		
	/*
		fprintf(f, "<svg width=\"%.0f\" height=\"%.0f\" xmlns=\"http://www.w3.org/2000/svg\">\n",
				total_width, total_height);

			fprintf(f, "<path d=\"M%.2lf %.2lf\n", x, y);
			fprintf(f, "L %.2lf %.2lf\n", x, y);
		fprintf(f, "Z\" stroke=\"%s\" stroke-width=\"%.2lf\" fill-opacity=\"0.0\"/>\n", color, stroke_width);
	*/
private:
	int _depth;
	bool _hasWall(int i, int j, int d)
	{
		switch((d+4)%4)
		{
			case 0: return i >= _w-1 || right(i, j);
			case 1: return j >= _h-1 || bottom(i, j);
			case 2: return i <= 0    || left(i, j);
			case 3: return j <= 0    || top(i, j);
		}
		return true;
	}
	int _nrWalls(int i, int j)
	{
		int c = 0;
		for (int d = 0; d < 4; d++)
			if (_hasWall(i, j, d))
				c++;
		return c;
	}
	bool _notVisited(int i, int j, bool *visited)
	{
		return 0 <= i && i < _w && 0 <= j && j < _h && !visited[i + _w*j];
	}
	void _recurse(int i, int j, bool *visited)
	{
		//print();
		//printf("visit %d, %d\n", i, j, i + _w*j);
		visited[i + _w*j] = true;
		for (;;)
		{
			int c = 0;
			if (_notVisited(i-1, j, visited)) c++;
			if (_notVisited(i, j-1, visited)) c++;
			if (_notVisited(i+1, j, visited)) c++;
			if (_notVisited(i, j+1, visited)) c++;
			if (c == 0)
				break;
			int r = rand() % c;
			//printf("      %d,%d %d, %d\n", i, j, c, r);
			if (_notVisited(i-1, j, visited) && r-- == 0)
			{
				left(i, j) = false;
				_recurse(i-1, j, visited);
				
			}
			else if (_notVisited(i, j-1, visited) && r-- == 0)
			{
				top(i, j) = false;
				_recurse(i, j-1, visited);
			}
			else if (_notVisited(i+1, j, visited) && r-- == 0)
			{
				right(i, j) = false;
				_recurse(i+1, j, visited);
			}
			else if (_notVisited(i, j+1, visited) && r-- == 0)
			{
				bottom(i, j) = false;
				_recurse(i, j+1, visited);
			}
		}
	}
	
	void _split(int i, int j, int w, int h)
	{
		//printf("%*.*ssplit %d %d %d %d\n", _depth, _depth, "", i, j, w, h);
		_depth++;
		if (w == 1)
		{
			for (int k = 1; k < h; k++)
				top(i, j+k) = false;
		}
		else if (h == 1)
		{
			for (int k = 1; k < w; k++)
				left(i+k, j) = false;
		}
		else if (w < h)
		{
			int h_r = h == 2 ? 1 :
					  h <= 4 ? 1 + rand()%(h-1) :
					  h <= 6 ? 2 + rand()%(h-3) :
							   3 + rand()%(h-5);
			int o = rand()%w;
			//printf("h_r = %d, o = %d\n", h_r, o);
			top(i + o, j + h_r) = false;
			_split(i, j, w, h_r),
			_split(i, j+h_r, w, h-h_r);
		}
		else
		{
			int w_r = w == 2 ? 1 :
					  w <= 4 ? 1 + rand()%(w-1) :
					  w <= 6 ? 2 + rand()%(w-3) :
							   3 + rand()%(w-5);
			int o = rand()%h;
			//printf("w_r = %d, o = %d\n", w_r, o);
			left(i + w_r, j + o) = false;
			_split(i, j, w_r, h),
			_split(i+w_r, j, w-w_r , h);
		}
		_depth--;
	}

	void _fractal(int i, int j, int size, frac_type ft, int avoid_corner)
	{
		printf("frac %d %d %d\n", i, j, size);
		if (   i + size <= 0 || i - size >= _w
		    || j + size <= 0 || j - size >= _h)
		    return;
		
		int d = -1;
		if (i <= 0)
		{
			if (j > 0 && j < _h)
				top(_left_range(i, size, ft), j) = false;
		}
		else if (i >= _w)
		{
			if (j > 0 && j < _h)
				top(_right_range(i, size, ft), j) = false;
		}
		else if (j <= 0)
		{
			left(i, _bottom_range(j, size, ft)) = false;
		}
		else if (j >= _h)
		{
			left(i, _top_range(j, size, ft)) = false;
		}
		else
		{
			switch (ft)
			{
				case frac_regular:
				case frac_reverse:					d = 1; break;
				case frac_random_orient_no_cross:   d = (avoid_corner + 3 + rand() % 2) % 4; break;
				case frac_random_orient:
				case frac_reverse_random_orient_no_cross:
				case frac_all_random:				d = rand() % 4; break;
			}
			if (d != 0) top(_left_range(i, size, ft), j) = false;
			if (d != 1) left(i, _bottom_range(j, size, ft)) = false;
			if (d != 2) top(_right_range(i, size, ft), j) = false;
			if (d != 3) left(i, _top_range(j, size, ft)) = false;
		}
		
		if (size == 1)
			return;
		size /= 2;
		_fractal(i + size, j - size, size, ft, avoid_corner == 0 ? 0 : (d == 1 || d == 2) ? 2 : -1);
		_fractal(i + size, j + size, size, ft, avoid_corner == 1 ? 1 : (d == 2 || d == 3) ? 3 : -1);
		_fractal(i - size, j + size, size, ft, avoid_corner == 2 ? 2 : (d == 3 || d == 0) ? 0 : -1);
		_fractal(i - size, j - size, size, ft, avoid_corner == 3 ? 3 : (d == 0 || d == 1) ? 1 : -1);
	}
	int _min(int a, int b) { return a < b ? a : b; }
	int _left_range(int i, int size, frac_type ft)
	{
		int min = i < 0 ? 0 : i;
		int max = (i + size < _w ? i + size : _w) - 1;
		if (ft == frac_reverse || ft == frac_reverse_random_orient_no_cross) return max;
		if (ft != frac_all_random) return min;
		return min + (min < max ? rand()%(max+1 - min) : 0);
	}
	int _right_range(int i, int size, frac_type ft)
	{
		int max = (i > _w ? _w : i) - 1;
		int min = i - size > 0 ? i - size : 0;
		if (ft == frac_reverse || ft == frac_reverse_random_orient_no_cross) return min;
		if (ft != frac_all_random) return max;
		return min + (min < max ? rand()%(max+1 - min) : 0);
	}
	int _bottom_range(int j, int size, frac_type ft)
	{
		int min = j < 0 ? 0 : j;
		int max = (j + size < _h ? j + size : _h) - 1;
		if (ft == frac_reverse || ft == frac_reverse_random_orient_no_cross) return max;
		if (ft != frac_all_random) return min;
		return min + (min < max ? rand()%(max+1 - min) : 0);
	}
	int _top_range(int j, int size, frac_type ft)
	{
		int max = (j < _h ? j : _h) - 1;
		int min = j - size < 0 ? 0 : j - size;
		if (ft == frac_reverse || ft == frac_reverse_random_orient_no_cross) return min;
		if (ft != frac_all_random) return max;
		return min + (min < max ? rand()%(max+1 - min) : 0);
	}

	int _w, _h;
	bool *_vert, *_horz;
};

int main(int argc, char *argv[])
{
	//srand(time(0));
	Maze maze(40, 40);
	//maze.generateRecursive();
	//maze.removeCrosses();
	//maze.generateSplit();
	//maze.generateFractal(Maze::frac_regular);
	maze.generateFractal(Maze::frac_reverse_random_orient_no_cross);
	//maze.generateFractal(Maze::frac_random_orient_no_cross);
	//maze.generateFractal(Maze::frac_random_orient);
	//maze.generateFractal(Maze::frac_all_random);
	maze.print();
	maze.printStats();
	if (!maze.check())
		printf("Incorrect\n");
	maze.svg("Maze.svg", 2, 8, "red", 1);
	//maze.dump();
}

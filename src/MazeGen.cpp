#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

class Stat
{
public:
	Stat() : _sum(0.0), _sqrsum(0.0), _n(0) {}
	void add(double v)
	{
		_sum += v;
		_sqrsum += v*v;
		_n++;
	}
	double avg() { return _sum / _n; }
	double stddev() { return sqrt((_sqrsum - (_sum * _sum)/_n)/(_n - 1)); }
	static double dist(Stat &p, Stat &q)
	{
		// https://en.wikipedia.org/wiki/Bhattacharyya_distance
		double avg_p = p.avg();
		double std2_p = p.stddev();
		std2_p = std2_p * std2_p;
		double avg_q = q.avg();
		double std2_q = q.stddev();
		std2_q = std2_q * std2_q;
		
		return ( log((std2_p/std2_q + std2_q/std2_p + 2)/4)
		       + (avg_p - avg_q)*(avg_p - avg_q)/(std2_p + std2_q)
		       )/4;
	}
private:
	double _sum;
	double _sqrsum;
	int _n;
};



class Maze
{
private:
	enum state { s_wall, s_passage, s_undefined };
public:
	Maze(int w, int h) : _w(w), _h(h), _depth(0)
	{
		_vert = new state[(w-1)*h];
		for (int i = 0; i < (w-1)*h; i++)
			_vert[i] = s_undefined;
		_horz = new state[w*(h-1)];
		for (int i = 0; i < w*(h-1); i++)
			_horz[i] = s_undefined;
	}
	~Maze()
	{
		delete[] _vert;
		delete[] _horz;
	}
	state &right(int i, int j) { /*printf("right(%d,%d)\n", i, j);*/return _vert[_h*i + j]; }
	state &left(int i, int j) { /*printf("left(%d,%d)\n", i, j);*/return _vert[_h*(i-1) + j]; }
	state &bottom(int i, int j) { /*printf("bottom(%d,%d)\n", i, j);*/return _horz[i + _w*j]; }
	state &top(int i, int j) { /*printf("top(%d,%d)\n", i, j);*/return _horz[i + _w*(j-1)]; }

	void generateRecursive()
	{
		bool *visited = new bool[_w*_h];
		for (int i = 0; i < _w*_h; i++)
			visited[i] = false;
		_recurse(0, 0, visited);
		delete[] visited;
		_undefinedToWall();
	}
	void generateSplit()
	{
		_split(0, 0, _w, _h);
		_undefinedToWall();
	}
	enum frac_type { frac_regular, frac_reverse, frac_random_orient_no_cross, frac_reverse_random_orient_no_cross, frac_random_orient, frac_all_random };
	void generateFractal(frac_type type)
	{
		generateFractal(_w/2, _h/2, type);
	}
	void generateTrees()
	{
		fix();
	}
	void generateWilson()
	{
		int* state = new int[_w*_h];
		for (int i = 0; i < _w*_h; i++)
			state[i] = 0;
			
		state[(rand()%_w) + _w*(rand()%_h)] = 4;
		int to_go = _w*_h - 1;
		while (to_go > 0)
		{
			//printf("to go %d: ", to_go);
			int s = rand()%to_go;
			int s_i = -1;
			int s_j = 0;
			for (int i = 0; i < _w && s_i == -1; i++)
				for (int j = 0; j < _h && s_i == -1; j++)
					if (state[i + _w*j] != 4 && s-- == 0)
					{
						s_i = i;
						s_j = j;
					}
			int i = s_i;
			int j = s_j;
			//printf("start %d,%d: ", i, j);
			while (state[i + _w*j] != 4)
			{
				int d = rand()%4;
				if (!_hasWall(i, j, d))
				{
					state[i + _w*j] = d;
					switch(d)
					{
						case 0: i++; break;
						case 1: j++; break;
						case 2: i--; break;
						case 3: j--; break;
					}
				}
			}
			//printf("found\n");
			i = s_i;
			j = s_j;
			while (state[i + _w*j] != 4)
			{
				int d = state[i + _w*j];
				state[i + _w*j] = 4;
				switch(d)
				{
					case 0: right(i,j) = s_passage;  i++; break;
					case 1: bottom(i,j) = s_passage; j++; break;
					case 2: left(i,j) = s_passage;   i--; break;
					case 3: top(i,j) = s_passage;    j--; break;
				}
				to_go--;
			}
		}
		for (int i = 0; i < (_w-1)*_h; i++)
			if (_vert[i] == s_undefined)
				_vert[i] = s_wall;
		for (int i = 0; i < _w*(_h-1); i++)
			if (_horz[i] == s_undefined)
				_horz[i] = s_wall;
		delete state;
	}
	void generateRandom()
	{
		for (int i = 0; i < (_w-1)*_h; i++)
			_vert[i] = rand() % 2 == 0 ? s_wall : s_passage;
		for (int i = 0; i < _w*(_h-1); i++)
			_horz[i] = rand() % 2 == 0 ? s_wall : s_passage;
		fix();
	}
	void generateFractal(int i, int j, frac_type type)
	{
		int size = 1;
		while (   i - size > 0 || i + size < _w-1
		       || j - size > 0 || j + size < _w-1)
			size *= 2;
		_fractal(i, j, size, type, -1);
		_undefinedToWall();
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
		int one, two_straight, two_turn, three, four;
		_calcStats(types, &one, &two_straight, &two_turn, &three, &four);
		
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
		printf("degree 1: %d\n", one);
		if (two_straight + two_turn > 0)
		{
			printf("degree 2: %d", two_straight + two_turn);
			if (two_straight > 0)
				printf(" straight:%d", two_straight);
			if (two_turn > 0)
				printf(" turn:%d", two_turn);
			printf("\n");
		}
		printf("degree 3: %d\n", three);
		if (four > 0)
			printf("degree 4: %d\n", four);
	}
	void printAverageDist()
	{
		long int *dist = new long int[_w*_h];
		for (int i = 0; i < _w*_h; i++)
			dist[i] = 0;
		_calcDistances(dist);
		double sum = 0;
		double l = 1.0;
		for (int i = 1; i < _w*_h && dist[i] > 0; i++, l += 1.0)
		{
			printf("%ld ", dist[i]);
			sum += dist[i] * l;
		}
		printf(" %lf\n", sum / (_w*_h*(_w*_h-1)/2));
		delete[] dist;
	}
	void calcStats(Stat (&stats)[22])
	{
		int types[16];
		int one, two_straight, two_turn, three, four;
		_calcStats(types, &one, &two_straight, &two_turn, &three, &four);
		double tot = _w * _h;
		for (int i = 0; i < 16; i++)
		{
			stats[i].add(types[i]/tot);
			//printf("%d ", types[i]);
		}
		stats[16].add(one/tot);
		stats[17].add(two_straight/tot);
		stats[18].add(two_turn/tot);
		stats[19].add(three/tot);
		stats[20].add(four/tot);
		long int *dist = new long int[_w*_h];
		for (int i = 0; i < _w*_h; i++)
			dist[i] = 0;
		_calcDistances(dist);
		double sum = 0;
		double l = 1.0;
		for (int i = 1; i < _w*_h && dist[i] > 0; i++, l += 1.0)
			sum += dist[i] * l;
		stats[21].add(sum / (_w*_h*(_w*_h-1)/2));
		//printf("%lf\n", sum / (_w*_h*(_w*_h-1)/2));
	}
	long calcDist()
	{
		int types[16];
		int one, two_straight, two_turn, three, four;
		_calcStats(types, &one, &two_straight, &two_turn, &three, &four);
		long result = 0;
		int exp = _w * _h / 15;
		for (int i = 1; i < 15; i++)
		{
			int d = (exp - types[i]);
			if (d < 0)
				d = -d;
			printf("%4d", d);
			result += d*d*d;
		}
		return result;
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
				case 4: goto L4;
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
				else
				{
					_turn = 2;
					_state = 2; return; L2:;
					if (!_maze._hasWall(_i, _j, _d))
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
						_state = 3; return; L3:;
					}
					else
					{
						_turn = 1;
						_state = 4; return; L4:;
						_d = (_d+1)%4;
					}
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
					//printf("Cross at %d, %d\n", i, j);
					top(i, j) = s_wall;
					//printf("%d %d %d %d\n", right(i,j), bottom(i,j), left(i,j), top(i,j));
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
					//printf(" Found min %d,%d %d\n", best_i, best_j, min_k);
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
						//printf(" Found min %d,%d\n", best_i, best_j);
					}
					if (best_j > 0)
						top(best_i, best_j) = s_passage;
					else
						left(best_i, best_j) = s_passage;
				}
	}
	
	bool stamp(Maze &pattern)
	{
		if (pattern._w > _w || pattern._h > _h)
			return false;

		for (int i = 0; i < pattern._w-1; i++)
			for (int j = 0; j < pattern._h; j++)
				if (pattern.right(i, j) == s_wall)
				{
					int l = ((i+1) * _w)/pattern._w - 1;
					int t = (j * _h)/pattern._h;
					int b = ((j+1) * _h)/pattern._h;
					for (int k = t; k < b; k++)
						right(l, k) = s_wall;
				}
		for (int i = 0; i < pattern._w; i++)
			for (int j = 0; j < pattern._h-1; j++)
				if (pattern.bottom(i, j) == s_wall)
				{
					int l = (i * _w)/pattern._w;
					int r = ((i+1) * _w)/pattern._w;
					int b = ((j+1) * _h)/pattern._h - 1;
					for (int k = l; k < r; k++)
						bottom(k, b) = s_wall;
				}
		return true;
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
	
	void fix()
	{
		char *c_vert = new char[(_w-1)*_h];
		char *c_horz = new char[_w*(_h-1)];
		
		for (;;)
		{
			for (int i = 0; i < (_w-1)*_h; i++)
				c_vert[i] = 0;
			for (int i = 0; i < _w*(_h-1); i++)
				c_horz[i] = 0;
				
			int count = 0;
			int i = rand()%_w;
			int j = rand()%_h;
			int d = rand()%4;
			for (;;)
			{
				if (_nrWalls(i, j) > 0)
				{
					while (!_hasWall(i, j, d))
						d = (d+1)%4;
					d = (d+1)%4;
					break;
				}
				if (++i == _w)
				{
					i = 0;
					if (++j == _h)
						j = 0;
				}
			}
			for (iterator it(*this, i, j, d); it.more(); it.next())
				if (it.turn() == 0)
				{
					switch (it.d())
					{
						case 0: c_vert[_h*(it.i()-1) + it.j()]++; break;
						case 1: c_horz[it.i() + _w*(it.j()-1)]++; break;
						case 2: c_vert[_h*it.i() + it.j()]++; break;
						case 3: c_horz[it.i() + _w*it.j()]++; break;
					}
					count++;
				}
				else if (it.turn() == 2)
				{
					switch (it.d())
					{
						case 0: if (it.j() > 0) c_horz[it.i() + _w*(it.j()-1)]++; break;
						case 1: if (it.i() < _w-1) c_vert[_h*it.i() + it.j()]++; break;
						case 2: if (it.j() < _h-1) c_horz[it.i() + _w*it.j()]++; break;
						case 3: if (it.i() > 0) c_vert[_h*(it.i()-1) + it.j()]++; break;
					}
				}
			if (count == 2*(_w*_h - 1))
				break;
			
			count = 0;
			for (int i = 0; i < (_w-1)*_h; i++)
				if (c_vert[i] == 1)
					count++;
			for (int i = 0; i < _w*(_h-1); i++)
				if (c_horz[i] == 1)
					count++;
			//printf(" %d", count);
			int r = rand() % count;
			count = 0;
			for (int i = 0; i < (_w-1)*_h; i++)
				if (c_vert[i] == 1)
				{
					if (count == r)
						_vert[i] = (_vert[i] == s_wall) ? s_passage : s_wall;
					count++;
				}
			for (int i = 0; i < _w*(_h-1); i++)
				if (c_horz[i] == 1)
				{
					if (count == r)
						_horz[i] = (_horz[i] == s_wall) ? s_passage : s_wall;
					count++;
				}
		}
		
		delete[] c_vert;
		delete[] c_horz;
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
private:
	int _depth;
	bool _hasWall(int i, int j, int d)
	{
		switch((d+4)%4)
		{
			case 0: return i >= _w-1 || right(i, j) == s_wall;
			case 1: return j >= _h-1 || bottom(i, j) == s_wall;
			case 2: return i <= 0    || left(i, j) == s_wall;
			case 3: return j <= 0    || top(i, j) == s_wall;
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
	void _undefinedToWall()
	{
		for (int i = 0; i < (_w-1)*_h; i++)
			if (_vert[i] == s_undefined)
				_vert[i] = s_wall;
		for (int i = 0; i < _w*(_h-1); i++)
			if (_horz[i] == s_undefined)
				_horz[i] = s_wall;
	}
	void _recurse(int i, int j, bool *visited)
	{
		//print();
		//printf("visit %d, %d\n", i, j, i + _w*j);
		visited[i + _w*j] = true;
		for (;;)
		{
			int c = 0;
			if (!_hasWall(i, j, 0) && _notVisited(i+1, j, visited)) c++;
			if (!_hasWall(i, j, 1) && _notVisited(i, j+1, visited)) c++;
			if (!_hasWall(i, j, 2) && _notVisited(i-1, j, visited)) c++;
			if (!_hasWall(i, j, 3) && _notVisited(i, j-1, visited)) c++;
			if (c == 0)
				break;
			int r = rand() % c;
			//printf("      %d,%d %d, %d\n", i, j, c, r);
			if (!_hasWall(i, j, 0) && _notVisited(i+1, j, visited) && r-- == 0)
			{
				right(i, j) = s_passage;
				_recurse(i+1, j, visited);
			}
			else if (!_hasWall(i, j, 1) && _notVisited(i, j+1, visited) && r-- == 0)
			{
				bottom(i, j) = s_passage;
				_recurse(i, j+1, visited);
			}
			else if (!_hasWall(i, j, 2) && _notVisited(i-1, j, visited) && r-- == 0)
			{
				left(i, j) = s_passage;
				_recurse(i-1, j, visited);
				
			}
			else if (!_hasWall(i, j, 3) && _notVisited(i, j-1, visited) && r-- == 0)
			{
				top(i, j) = s_passage;
				_recurse(i, j-1, visited);
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
				top(i, j+k) = s_passage;
		}
		else if (h == 1)
		{
			for (int k = 1; k < w; k++)
				left(i+k, j) = s_passage;
		}
		else if (w < h || (w == h && (rand()%2 == 0)))
		{
			int h_r = h == 2 ? 1 :
					  h <= 4 ? 1 + rand()%(h-1) :
					  h <= 6 ? 2 + rand()%(h-3) :
							   3 + rand()%(h-5);
			int o = rand()%w;
			//printf("h_r = %d, o = %d\n", h_r, o);
			top(i + o, j + h_r) = s_passage;
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
			left(i + w_r, j + o) = s_passage;
			_split(i, j, w_r, h),
			_split(i+w_r, j, w-w_r , h);
		}
		_depth--;
	}

	void _fractal(int i, int j, int size, frac_type ft, int avoid_corner)
	{
		//printf("frac %d %d %d\n", i, j, size);
		if (   i + size <= 0 || i - size >= _w
		    || j + size <= 0 || j - size >= _h)
		    return;
		
		int d = -1;
		if (i <= 0)
		{
			if (j > 0 && j < _h)
				top(_left_range(i, size, ft), j) = s_passage;
		}
		else if (i >= _w)
		{
			if (j > 0 && j < _h)
				top(_right_range(i, size, ft), j) = s_passage;
		}
		else if (j <= 0)
		{
			left(i, _bottom_range(j, size, ft)) = s_passage;
		}
		else if (j >= _h)
		{
			left(i, _top_range(j, size, ft)) = s_passage;
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
			if (d != 0) top(_left_range(i, size, ft), j) = s_passage;
			if (d != 1) left(i, _bottom_range(j, size, ft)) = s_passage;
			if (d != 2) top(_right_range(i, size, ft), j) = s_passage;
			if (d != 3) left(i, _top_range(j, size, ft)) = s_passage;
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
	class _Cell
	{
	public:
		_Cell() : d(0), nr(1), prev(0) {}
		int d;
		long nr;
		class _Cell *prev;
	};
	void _calcStats(int* types, int* one, int* two_straight, int* two_turn, int* three, int* four)
	{
		for (int i = 0; i < 16; i++)
			types[i] = 0;
		for (int i = 0; i < _w; i++)
			for (int j = 0; j < _h; j++)
				types[  (_hasWall(i, j, 0) ? 0 : 1)
				      | (_hasWall(i, j, 1) ? 0 : 2)
				      | (_hasWall(i, j, 2) ? 0 : 4)
				      | (_hasWall(i, j, 3) ? 0 : 8)]++;
		*one = types[1] + types[2] + types[4] + types[8];
		*two_straight = types[1+4] + types[2 + 8];
		*two_turn = types[1 + 2] + types[2 + 4] + types[4 + 8] + types[8 + 1];
		*three = types[1 + 2 + 4] + types[2 + 4 + 8] + types [4 + 8 + 1] + types[8 + 1 + 2];
		*four = types[1 + 2 + 4 + 8];
	}
	void _calcDistances(long *dist)
	{
		_Cell *cells = new _Cell[_w*_h];
		for (int i = 0; i < _w; i++)
			for (int j = 0; j < _h; j++)
				cells[i + _w*j].d = 4 - _nrWalls(i, j);
		_Cell *prev = 0;
		for (iterator it(*this, 0, 0, 0); it.more(); it.next())
			if (it.turn() == 0)
			{
				_Cell *cell = &cells[it.i() + _w*it.j()];
				if (cell->d != 0)
				{
					int a_l = 1;
					for (_Cell* a = prev; a != 0; a = a->prev, a_l++)
					{
						dist[a_l] += a->nr;
						int b_l = 1;
						for (_Cell* b = cell->prev; b != 0; b = b->prev, b_l++)
							dist[a_l + b_l] += a->nr * b->nr;
					}
					_Cell* a = prev;
					_Cell* b = cell->prev;
					for (; a != 0 && b != 0; a = a->prev, b = b->prev)
						a->nr = b->nr = a->nr + b->nr;
					if (a != 0)
						cell->prev = prev;
					prev = (--cell->d == 0) ? cell : 0;
				}
			}
		delete[] cells;
	}
	int _w, _h;
	state *_vert, *_horz;
};

void dist_kind(Stat* stats, int* vec, int n, const char *name)
{
	for (int i = 0; i < n; i++)
		printf(" %5.2lf(%5.2lf)", 100*stats[vec[i]].avg(), 100*stats[vec[i]].stddev());
	double max_dist = 0;
	double sum_dist = 0;
	for (int i = 0; i < n-1; i++)
		for (int j = i+1; j < n; j++)
		{
			double dist = Stat::dist(stats[vec[i]], stats[vec[j]]);
			if (dist > max_dist)
				max_dist = dist;
			sum_dist += dist;
		}
	printf(" %s = %6.3lf %6.3lf", name, max_dist, sum_dist);
	printf(" %6.3lf |", sum_dist);
}

int main(int argc, char *argv[])
{
	srand(time(0));
	//Maze maze(30, 30);
	//maze.generateRecursive();
	//maze.removeCrosses();
	//maze.generateSplit();
	//maze.generateFractal(Maze::frac_regular);
	//maze.generateFractal(Maze::frac_reverse_random_orient_no_cross);
	//maze.generateFractal(Maze::frac_random_orient_no_cross);
	//maze.generateFractal(Maze::frac_random_orient);
	//maze.generateFractal(Maze::frac_all_random);
	//Maze maze2(6, 6);
	//maze2.generateRecursive();
	//maze.stamp(maze2);
	//maze.fix();
	//maze.generateRecursive();
	//maze.generateTrees();
	//maze.generateRandom();
	//maze.print();
	//maze.printStats();
	//maze.printAverageDist();
	//if (!maze.check())
	//	printf("Incorrect\n");
	//maze.svg("Maze.svg", 2, 8, "red", 1);
	//maze.dump();
	
/*	
	for (int t = 0; t < 10; t++)
	{
		printf("%d ", t);
		for (int size = 20; size <= 20; size += 10)
		{
			Stat stats[22];
			Stat times;
			for (int i = 0; i < 500; i++)
			{
				Maze maze(size, size);
				long start = clock();
				switch(t)
				{
					case 0:
						maze.generateRandom(); break;
					case 1:
						maze.generateWilson(); break;
						printf("x");
					case 2:
						maze.generateSplit(); break;
					case 3:
						maze.generateRecursive(); break;
					case 4:
						maze.generateTrees(); break;
					case 5:
					{
						Maze maze2(size/5, size/5);
						maze2.generateRecursive();
						maze.stamp(maze2);
						maze.fix();
					} break;
					case 6:
						maze.generateFractal(Maze::frac_reverse_random_orient_no_cross); break;
					case 7:
						maze.generateFractal(Maze::frac_random_orient_no_cross); break;
					case 8:
						maze.generateFractal(Maze::frac_random_orient); break;
					case 9:
						maze.generateFractal(Maze::frac_all_random); break;
				}
				if (!maze.check())
					printf("Error\n");
				times.add((clock() - start)/((double)size * size));
				maze.calcStats(stats);
			}
			//printf("%d: ", size);
			//for (int i = 1; i < 21; i++)
			//{
			//	printf(" %5.2lf(%5.2lf)", 100*stats[i].avg(), 100*stats[i].stddev());
			//}
			int ones[4] = { 1, 2, 4, 8 };
			dist_kind(stats, ones, 4, "ones");
			int two_corners[4] = { 1+2, 2+4, 4+8, 8+1 };
			dist_kind(stats, two_corners, 4, "two corners");
			int two_straight[2] = { 1+4, 2+8 };
			dist_kind(stats, two_straight, 2, "two straight");
			int three[4] = { 1+2+4, 2+4+8, 4+8+1, 8+1+2 };
			dist_kind(stats, three, 4, "three");
			printf(" %6.2lf(%5.2lf)", stats[21].avg(), stats[21].stddev());
			printf(" %6.2lf(%5.2lf)", times.avg(), times.stddev());
			printf("\n");
			//for (int i = 0; i < 4; i++)
			//	printf(" %5.2lf(%5.2lf)\n", 100*stats[ones[i]].avg(), 100*stats[ones[i]].stddev());
			//double max_dist = 0;
			//double sum_dist = 0;
			//for (int i = 0; i < 3; i++)
			//	for (int j = i+1; j < 4; j++)
			//	{
			//		double dist = Stat::dist(stats[ones[i]], stats[ones[j]]);
			//		if (dist > max_dist)
			//			max_dist = dist;
			//		sum_dist += dist;
			//	}
			//printf("ones dist = %lf %lf\n", max_dist, sum_dist);
		}
	}
*/
	long min_dist;
	int min_i = 0;
	for (int i = 1; ; i++)
	{
		srand(i);
		Maze maze2(6, 6);
		maze2.generateRecursive();
		Maze maze(30, 30);
		maze.stamp(maze2);
		maze.generateWilson();
		long dist = maze.calcDist();
		printf(" %ld\n", dist);
		if (min_i == 0 || dist < min_dist)
		{
			min_dist = dist;
			min_i = i;
		}
		if (i % 1000 == 0)
		{
			srand(min_i);
			Maze maze2(6, 6);
			maze2.generateRecursive();
			Maze maze(30, 30);
			maze.stamp(maze2);
			maze.generateWilson();
			printf("\nDist = %ld\n", maze.calcDist());
			maze.svg("Maze.svg", 2, 8, "red", 1);
			maze.svg("Maze2.svg", 5, 5, "red", 1);
		}
	}

}

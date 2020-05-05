// lemmings.cpp

#include "stencil1d.hpp"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <cstdlib>


// lemmings ///////////////////////////////////////////////////////////

class Lemming {
private:
	uint8_t type : 4;	// Lemming type
	uint8_t dir : 1;	// moving direction, not used with [s]
	uint8_t sub : 3;	// type-dependent subtype (counter, generator, ...)

	static constexpr int max_bomb = 7;
	static constexpr int bomber_sub = 6;
	enum e_from { FNone, FRight, FLeft, FBoth };

public:
	enum e_dir { Left, Right, Standing = Left };
	enum e_type { Empty, // [s]
		Zero,		// [m] dumb Lemming, just walking (l/r)
		Bomber,		// [m] carries the bomb, places it (after sub steps) and becomes Zero; he continues when the bomb is already placed
		Bomb,		// [s] kills Zeros
		Ammo,		// [s] Zero -> Bomber - sub=e_from
		Spawner,	// [s] [m] -> duplicates one approaching Zero (sub=1 -> gen)
		Wall,		// [s] all [m] bounce
		Crash,		// [s] just an animation
		Unknown, Type_count = Unknown };

	Lemming(e_type t = Empty, e_dir d = Standing, int s = 0) : type(t), dir(d), sub(s) {}
	static Lemming stencil(Lemming a1, Lemming a2, Lemming a3);
	std::string print() const;

private:
	inline static const std::string vis_type[Type_count] { ".", "O", "R", "B", "A", "S", "W", "*" };
	inline static const std::string vis_dir[] { "<", ">" };

	bool movable_type() const { return type == Zero || type == Bomber; }
	bool movable() const { return movable_type() || sub != 0; }
	e_dir standing_dir() const { return sub == FRight ? Left : Right; }		// sub -> dir, this must be Ammo
	bool is_effective_dir( e_dir d) const;	// if the field generates anything in a direction d
	Lemming approacher( e_dir d) const;		// generated Lemming in a direction d
	Lemming countdown() const;				// decrement of the Bomber's sub
};


Lemming Lemming::stencil(Lemming a1, Lemming a2, Lemming a3) {
	uint8_t approaching = ((a1.movable() && a1.is_effective_dir( Right)) ? FLeft : FNone) 
						+ ((a3.movable() && a3.is_effective_dir( Left)) ? FRight : FNone);
	switch (a2.type) {
		case Empty:
		case Crash:
			switch (approaching) {
			case FNone:		return Lemming{};
			case FRight:	return a3.approacher( Left).countdown();
			case FLeft:		return a1.approacher( Right).countdown();
			case FBoth:		return a1.type == a3.type ? Lemming { Spawner } : a1.type > a3.type ? a1 : a3;
			}
		case Wall:
			return a2;
		case Bomb:
			switch (approaching) {
			case FNone:		return Lemming{ Bomb };
			case FRight:	return a3.type == Bomber && a3.sub == 1 ? Lemming{ Bomb, Standing, FRight } : Lemming{ Crash };
			case FLeft:		return a1.type == Bomber && a1.sub == 1 ? Lemming{ Bomb, Standing, FLeft }  : Lemming{ Crash };
			case FBoth:		return Lemming{ Crash };
			}
		case Ammo:
			switch (approaching) {
			case FNone:		return Lemming{ Ammo};						// nulling sub
			default:		return Lemming{ Ammo, Standing, approaching };
			}
		case Spawner:
			if (a1.type == Spawner || a3.type == Spawner)
				return Lemming{};										// overgrowth prevention
			switch (approaching) {
			case FNone:		
			case FBoth:		return Lemming{ Spawner };					// nulling sub
			case FRight:
			case FLeft:		return Lemming{ Spawner, Standing, 1 };
			}
		default:														// movable
			switch (approaching) {
			case FNone:		
				if (a2.dir == Left && a1.type == Wall)	return Lemming{ static_cast<e_type>(a2.type), Right, a2.sub > 0 ? a2.sub - 1 : 0 };
				else if (a2.dir == Right && a3.type == Wall)	return Lemming{ static_cast<e_type>(a2.type), Left, a2.sub > 0 ? a2.sub - 1 : 0 };
				else		return Lemming{};							// someone is leaving -> Empty
			case FRight:	return a3.approacher( Left).countdown();	// someone is approaching
			case FLeft:		return a1.approacher( Right).countdown();
			case FBoth:		return Lemming{ Crash };
			}
	}

	return Lemming(Unknown);
}


bool Lemming::is_effective_dir( e_dir d) const {
	if (movable_type())		return dir == d;
	else switch (type) {
		case Ammo:
		case Bomb:			return standing_dir() == d;
		case Spawner:		return true;
		default:			return false;
	}
}

Lemming Lemming::approacher( e_dir d) const {
	if (movable_type())	return *this;
	else switch (type) {
		case Ammo:		return Lemming{ Bomber, d, bomber_sub };
		case Bomb:		return Lemming{ Zero, is_effective_dir( Right) ? Right : Left};
		case Spawner:	return Lemming{ Zero, d };
		default:		return Lemming{ Unknown };
	}
}

Lemming Lemming::countdown() const {
	switch (type) {
	case Bomber:
		if (sub > 1) return Lemming{ Bomber, static_cast<e_dir>( dir), sub - 1 };
		else		 return Lemming{ Bomb, Standing, dir == Left ? FRight : FLeft };	// activate the bomb and continue
	default:
		return *this;
	}
}


std::string Lemming::print() const {
	if (type == Empty)
		return " . ";
	if (type >= Unknown)
		return " ? ";
	if (! movable_type()) {
		return std::string{ " " } + vis_type[type] + (sub ? std::to_string( sub) : " ");
	}
	auto vt = vis_type[type];
	if (sub != 0)
		vt += std::to_string(sub);
	if (dir == Right)
		vt = vt + vis_dir[dir];
	else
		vt = vis_dir[dir] + vt;
	if (vt.length() < 3)
		vt += " ";
	return vt;
}


// tests //////////////////////////////////////////////////////////////

template< typename ET, typename PF>
inline void print(const circle<ET>& space, PF printer, std::size_t g, int lnw, std::size_t lim = 0)
{
	if (lnw)
		std::cout << std::setw(lnw) << g; //print number
	std::cout << std::setw(1) << "|";
	std::size_t j = 0;
	if (lim && 2 * lim < space.size())
	{
		for (; j < lim; ++j) //left side
		{
			printer(space.get(j));
		}
		std::cout << ":";
		j = space.size() - lim;
	}
	for (; j < space.size(); ++j) //right side
	{
		printer(space.get(j));
	}
	std::cout << "|" << std::endl;
}

void rule110(std::size_t xs, std::size_t max_gen, std::size_t gpt, int lnw = 0, std::size_t lim = 40)
{
	circle<bool> space(xs);

	// initial state
	for (std::size_t i = 0; i < space.size(); ++i)
		space.set(i, false);
	
	space.set(space.size() - 2, true);

	auto rule110_stencil = [](bool a1, bool a2, bool a3) {
		auto idx = ((unsigned)a1 << 2) + ((unsigned)a2 << 1) + (unsigned)a3;

		return (110 >> idx) & 1;
	};

	auto rule110_print = [](bool a) { std::cout << (a ? '#' : '.'); };

	std::size_t detailed = lnw ? std::min(lim, gpt - 1) : 0;
	for( std::size_t g = 0; g < max_gen; ++g) {


		for (std::size_t g2 = 0; g2 < detailed; ++g2) {
			print(space, rule110_print, g * gpt + g2, lnw, lim);
			space.run(rule110_stencil, 1);
		}
		if (lnw)
			print( space, rule110_print, g * gpt + detailed, lnw, lim);
		space.run(rule110_stencil, gpt - 2 * detailed);
		for (std::size_t g2 = 0; g2 < detailed; ++g2) {
			print(space, rule110_print, (g+1) * gpt - detailed + g2, lnw, lim);
			space.run(rule110_stencil, 1);
		}
	}
	print( space, rule110_print, max_gen * gpt, lnw, lim);
	std::cout << "----" << std::endl;
}

void lemmings(std::size_t xs, std::size_t max_gen, std::size_t gpt, int lnw = 0, std::size_t last = 0)
{
	circle<Lemming> lemworld(xs);

	// initial state
	lemworld.set(7, Lemming{ Lemming::Zero, Lemming::Right });
	lemworld.set(10, Lemming{ Lemming::Zero, Lemming::Right });
	lemworld.set(30, Lemming{ Lemming::Zero, Lemming::Left });
	lemworld.set(42, Lemming{ Lemming::Zero, Lemming::Left });
	lemworld.set(18, Lemming{ Lemming::Bomb });
	lemworld.set(60, Lemming{ Lemming::Ammo });
	lemworld.set(20, Lemming{ Lemming::Ammo });
	lemworld.set(32, Lemming{ Lemming::Wall });
	lemworld.set(38, Lemming{ Lemming::Spawner });

	if (xs > 1200) {
		lemworld.set(99,  Lemming{ Lemming::Zero, Lemming::Right });
		lemworld.set(430, Lemming{ Lemming::Zero, Lemming::Left });
		lemworld.set(120, Lemming{ Lemming::Ammo });
		lemworld.set(199, Lemming{ Lemming::Ammo });
		lemworld.set(222, Lemming{ Lemming::Ammo });
		lemworld.set(612, Lemming{ Lemming::Ammo });
		lemworld.set(838, Lemming{ Lemming::Ammo });
	}

	if (xs > 10000) {
		lemworld.set(7001, Lemming{ Lemming::Zero, Lemming::Right });
		lemworld.set(9001, Lemming{ Lemming::Zero, Lemming::Left });
		lemworld.set(4520, Lemming{ Lemming::Ammo });
		lemworld.set(9238, Lemming{ Lemming::Ammo });
		lemworld.set(5120, Lemming{ Lemming::Ammo });
		lemworld.set(2432, Lemming{ Lemming::Ammo });
	}
	//

	auto lemming_print = [](const Lemming& lem) {
		std::cout << lem.print();
	};

	for (std::size_t g = 0; g < max_gen; ++g)
	{
		if (lnw || (last > 0 && g > max_gen - last))
			print(lemworld, lemming_print, g, lnw, 0);
		// dbg:		else if (g % 1000 == 0) std::cout << ".";
		lemworld.run( Lemming::stencil, gpt);
	}
	print(lemworld, lemming_print, max_gen, lnw, 0);
	std::cout << "----" << std::endl;
}


// main ///////////////////////////////////////////////////////////////

int usage( char** argv) {
	std::cout << "Usage: Lemmings R|L size maxgen gpt [lnw]" << std::endl;
	std::cout << "(argv:";
	while (*argv)  std::cout << " [" << *argv++ << "]";
	std::cout << ")" << std::endl;
	return 8;
}

int main(int argc, char** argv)
{


//#define MYDEBUG_
#ifdef MYDEBUG_
	//rule110(31, 60, 1, 3);
	//rule110(27011, 2, 13504, 5);
	//lemmings(72, 128, 1, 3);
	//lemmings(1373, 47, 67);
	//lemmings( 10007, 7, 9973);
	//--rule110(44497, 2, 22247, 5);	// too long in recodex
#else
	if (argc < 5 || argc > 6)
		return usage( argv);
	std::size_t size = atoi(argv[2]);
	std::size_t maxgen = atoi(argv[3]);
	std::size_t gpt = atoi(argv[4]);
	int lnw = argc >= 6 ? atoi(argv[5]) : 0;

	switch (argv[1][0]) {
	case 'r':
	case 'R':
		rule110(size, maxgen, gpt, lnw);
		break;
	case 'l':
	case 'L':
		lemmings(size, maxgen, gpt, lnw);
		break;
	default:
		return usage( argv);
	}
#endif

	return 0;
}

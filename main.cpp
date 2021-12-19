#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <concepts>

#include "pipette/pipette.cpp"
#include "backend/SnekGame3D.hpp"

/*
$ HERE ARE FRONTEND PROTOCOL DETAILS :-

[backend started by user]
[backend pipes the  frontend]

backend (u8 max bits in one coord) >> frontend
backend (x,y,z world size (from 1,1,1 to x,y,z)) >> frontend

<repeat>
	frontend (key pressed) >> backend
	backend (u32 - no of points) >> frontend
	backend (x,y,z,x,y,z....) >> frontend
</repeat>
*/

int operator >> (const pipette::fifo& pipe, std::integral auto& T)
{
	return pipe.read(&T, sizeof(T));
}

int operator << (const pipette::fifo& pipe, const std::integral auto& T)
{
	return pipe.write(&T, sizeof(T));
}

template <typename T>
void operator << (const pipette::fifo& pipe, const Point3D<T>& pnt)
{
	pipe << pnt.x ; pipe << pnt.y ; pipe << pnt.z;
}

// SIGNAL HANDLERS //

void interrupt_handler(int signal)
{
  std::puts("\n\e[31;1m => Interrrupt Recieved, Exiting...\e[0m\n");
  std::exit(-1);
}

void segfault_handler(int signal)
{
  std::puts("\n\e[31;1m => Segmentation Fault, Exiting...\e[0m\n");
  std::exit(-4);
}

int main()
{
	std::signal(SIGINT, interrupt_handler);
	std::signal(SIGSEGV, segfault_handler);

	char key = 'F';
	int err_cnt = 0;
	uint32_t num_pnts = 1;
	
	using mint = uint8_t;
	SnekGame3D<mint> game(16,16,16);

	std::system("./Snek3D-Frontend &");
	
	pipette::fifo fin("/home/tmp_inb", 'r'); // recieve from frontend
	pipette::fifo fout("/home/tmp_outb", 'w'); // send to frontend
	
	auto cleaner = [&]()
	{
		std::puts("\n\e[33m => Cleaning Up Temporary FIFOs...\e[0m\n");
		remove("/home/tmp_inb");
		remove("/home/tmp_outb");
	};
	
	uint8_t gg = sizeof(mint) * 8u; // max bits per coord;
	fout << gg;
	fout << game.wrld; // max world size
	
	while (true)
	{
		if (!(fin >> key))
		{
			++err_cnt;
			std::printf("Error Reading Key... (%-2d errors)\n", err_cnt);
			
			if (err_cnt >= 20)
			{
				std::puts("\nToo Many Errors, Exiting...");
				cleaner(); return -3;
			}
			else continue;
		}

		usleep(1000*500);
		std::printf("key : %c\n", key);
		
		if (key == 'E')	// exit signal from GUI
		{
			cleaner();
			return 0;
		}
		
		if (!game.nextFrame(key))	// game loop
		{
			std::puts("\e[31;1m ---=== Game Over! ===--- \e[0m");
			cleaner(); return 0;
		}
		
		num_pnts = game.snek.size() + 1;
		fout << num_pnts;
		
		fout << game.food;
		for (const auto& piece : game.snek) fout << piece;
	}
}

/*
 *  EXIT CODES :-
 *  0 -> Normal Exit or Game Over
 * -1 -> Keyboard/System Interrupt
 * -2 -> Error Starting Frontend
 * -3 -> Too Many Errors in getting Key
 * -4 -> Segmentation Fault
 */

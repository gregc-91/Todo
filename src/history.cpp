#include "History.h"

#include <istream>
#include <fstream>

// todo: write functions to serialise and deserialise a command
//       x serialisation done
//       x test deserialisation done
// todo: write functions to generate inverse commands

History::History(std::string filename) :
	filename(filename)
{	
	std::ifstream file(filename);
	
	for(std::string line; getline(file, line); ) {
		
		// populate last_command from a serialised string
		//last_command(line);
		//getLine(file, line);
		
		// populate inverse_command from a serialised string
		//inverse_command(line);
	}
	file.close();
}
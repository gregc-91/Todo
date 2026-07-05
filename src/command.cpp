#include "command.h"

bool Command::changesTodo() const
{
	switch (commandType) {
	case Add:
	case Remove:
	case Doo:
	case Set:
	case Tidy:
		return true;
	default:
		return false;
	}
}

#include "util.h"

#include <fstream>

#include <sstream>
using std::stringstream;

#include <chrono>
#include <thread>

#include <unistd.h> // for terminal size detection
#include <sys/ioctl.h> // for terminal size detection

// change to toggle debug statements on and off
#define debug debug_off

std::chrono::high_resolution_clock::time_point programStartTime = std::chrono::high_resolution_clock::now();

int getTermWidth()
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.ws_col;
}

void logMessage(string source, MessageType type, string message)
{
	std::ostream * stream = &std::cerr;
	//if (type == MESSAGE_DEBUG)
		stream = &std::cout;
	
	string typeStr;
	switch (type)
	{
	case MESSAGE_DEBUG:
		typeStr = "debug";
		break;
	case MESSAGE_WARNING:
		typeStr = "WARNING";
		break;
	case MESSAGE_ASSERTION_FAILED:
		typeStr = "FATAL ASSERTION";
		break;
	case MESSAGE_FATAL_ERROR:
		typeStr = "FATAL ERROR";
		break;
	default:
		typeStr = "UNKNOWN MESSAGE TYPE";
	}
	
	*stream << typeStr + " (" + source + "): " + message << std::endl;
	
	if (type == MESSAGE_FATAL_ERROR || type == MESSAGE_ASSERTION_FAILED)
		exit(1);
}

bool loadFile(string filename, string& contents)
{
	std::fstream inFile;
	
	debug("attempting to open '" + filename + "'...");
	
	inFile.open(filename);
	
	if (!inFile.is_open())
	{
		debug("'" + filename + "' failed to open :(");
		return false;
	}
	else
	{
		debug("file opended, reading file...");
		
		stringstream strStream;
		strStream << inFile.rdbuf();	// read the file
		contents = strStream.str();	// str holds the content of the file
		inFile.close();
		
		debug("file reading done, '" + filename + "' closed");
		
		return true;
	}
}

void sleepForSeconds(double seconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds((int)(seconds * 1000)));
}

double timeSinceStart()
{
	return (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - programStartTime)).count();
}

int timeSinceStartMili()
{
	return timeSinceStart() * 1000;
}

/*
void MessageLogger::status(string msg)
{
	if (verbose)
	{
		important(msg);
	}
}

void MessageLogger::important(string msg)
{
	show(tag + ": " + msg);
}

void MessageLogger::show(string msg)
{
	cout << msg << endl;
}
*/


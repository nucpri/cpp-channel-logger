#pragma once

#include <fstream>
#include <string>
#include <source_location>
#include <vector>
#include <iostream>
#include <chrono>

// TODO: this needs to be scoped
using namespace std;

#ifdef _MSC_VER
#if _MSVC_LANG < 201703L
static_assert(false, "You need to use C++20 or newer to support CLog.");
static_assert(false, "Refer here on how to change it: https://github.com/nucpri/cpp-channel-logger/blob/main/README.md#changing-the-c-language-standard");
#endif // __cplusplus >= 201703L
#endif

// WORKAROUND, see: https://developercommunity.visualstudio.com/t/-cplusplus-macro-does-not-report-correct-c-version/1280944
// /Zc:__cplusplus option isn't a suitable solution in this case
// If not using MSVC
#ifndef _MSC_VER
#if __cplusplus < 201703L
static_assert(false, "You need to use C++20 or newer to support CLog.");
static_assert(false, "Refer here on how to change it: https://github.com/nucpri/cpp-channel-logger/blob/main/README.md#changing-the-c-language-standard");
#endif // __cplusplus >= 201703L
#endif

// TODO: Might not be the most optimal name, didn't think too long about it
struct ChannelConfig {
	uint32_t channels;
	uint32_t flags;
};

class CLog {
private:
	inline static source_location Location;

public:
	enum Channels
	{
		COUT = 1,
		CERR = 2,
		FILE = 4,
	};

	enum Flags
	{
		// Channel-specific flags for 'CERR'
		// ...
		// Channel-specific flags for 'FILE'
		FORCEOVERWRITE = 1,
		//someflag = 2
		//someflag = 4
		// ... etc.
	};

	enum class MessageFormats
	{
		Error = 1,
	};

	// TODO: Make ChannelConfigs scoped
	inline static const ChannelConfig Log = { (COUT | FILE), 0 };
	inline static const ChannelConfig LogOW = { (COUT | FILE), FORCEOVERWRITE };
	inline static const ChannelConfig Error = { (CERR | FILE), 0 };

	inline static string DefaultFilename = "log.txt";

	/// <summary>
	/// <para>Log to channel(s)</para>
	/// <para>‎ </para>
	/// <para>EXAMPLES</para>
	/// <para>Write to std::cerr and to a file:</para>
	/// <para>CLog::ToChannels("someMessage", CLog::CERR | CLog::FILE });</para>
	/// <para>‎ </para>
	/// <para>Write to a custom file:</para>
	/// <para>CLog::DefaultFilename = "myLogFile.txt";</para>
	/// <para>CLog::ToChannels("someMessage", CLog::FILE );</para>
	/// </summary>
	/// 
	/// <param name="Message"></param>
	/// <param name="Channels"></param>
	/// <param name="Flags"></param>
	/// <param name="Location"></param>
	static void ToChannels(string Message, uint32_t Channels, uint32_t Flags = {}, const source_location Location = source_location::current()) {
		CLog::Location = Location; // Provide location for helper functions, such as Format()
		string filename = DefaultFilename;

		// Logs to std::cerr, see: https://cplusplus.com/reference/iostream/cerr/
		if (Channels & CERR) {
			Message = Format(Message, (int)MessageFormats::Error);
			cerr << Message;
		}

		// Logs to std::cout
		if (Channels & COUT) cout << Message;

		// Logs into a file, location specified in 'DefaultFilename'
		if (Channels & FILE) {

			auto fstream_mode = std::ios_base::binary;

			// If a file exists, append.
			if (ifstream(filename)) fstream_mode = std::ios_base::app;

			// Unless 'FORCEOVERWRITE' is specified.
			if (Flags & FORCEOVERWRITE) fstream_mode = std::ios_base::binary;

			ofstream outfile;
			outfile.open(filename, fstream_mode);
			outfile << Message;
			outfile.close();
		}
	}

	/// <summary>
	/// <para>Log to channel(s) by using a 'ChannelConfig'</para>
	/// <para>‎ </para>
	/// <para>QUICK USAGE:</para>
	/// <para>CLog::ToChannels("Some message");</para>
	/// <para>‎</para>
	/// <para>Log an Error:</para>
	/// <para>CLog::ToChannels("Some message", CLog::Error);</para>
	/// </summary>
	static void ToChannels(string Message, ChannelConfig cc = Log, const source_location Location = source_location::current()) {
		// These lines make it easy to copy & paste changes between overloads
		uint32_t Channels = cc.channels;
		uint32_t Flags = cc.channels;
		//

		CLog::Location = Location; // Provide location for helper functions, such as Format()
		string filename = DefaultFilename;

		// Logs to std::cerr, see: https://cplusplus.com/reference/iostream/cerr/
		if (cc.channels & CERR) {
			Message = Format(Message, (int)MessageFormats::Error);
			cerr << Message;
		}

		// Logs to std::cout
		if (cc.channels & COUT) cout << Message;

		// Logs into a file, location specified in 'DefaultFilename'
		if (cc.channels & FILE) {

			auto fstream_mode = std::ios_base::binary;

			// If a file exists, append.
			if (ifstream(filename)) fstream_mode = std::ios_base::app;

			// Unless 'FORCEOVERWRITE' is specified.
			if (cc.flags & FORCEOVERWRITE) fstream_mode = std::ios_base::binary;

			ofstream outfile;
			outfile.open(filename, fstream_mode);
			outfile << Message;
			outfile.close();
		}
	}


	// Formats a 'Message' (defaults to ERROR format)
	// TODO: add useful comments
	static string Format(string Message, uint32_t Format = (int)MessageFormats::Error) {
		//TODO: Some method to introduce custom format easily without touching the class code
		string formatted_time = std::format("{0:%F_%T}", chrono::system_clock::now());

		if (Format & (int)MessageFormats::Error) {
			return formatted_time + "\nFile: " +
				Location.file_name() + "(" +
				to_string(Location.line()) + ":" +
				to_string(Location.column()) + ") \n" +
				Location.function_name() + ": " +
				Message + "\n\n";
		}

		return Message;
	}
};

#ifndef CLI_PARSER_HPP
#define CLI_PARSER_HPP

#include <stddef.h>
#include <stdint.h>

#include <functional>

namespace Cli {
enum class Key {
	// this Qt analogue. need own values and universal converter
	Home = 0x01000010, // cursor movement
	End = 0x01000011,
	Left = 0x01000012,
	Up = 0x01000013,
	Right = 0x01000014,
	Down = 0x01000015,
	//
	F1 = 0x01000030, // function keys
	F2 = 0x01000031,
	F3 = 0x01000032,
	F4 = 0x01000033,
	F5 = 0x01000034,
	F6 = 0x01000035,
	F7 = 0x01000036,
	F8 = 0x01000037,
	F9 = 0x01000038,
	F10 = 0x01000039,
	F11 = 0x0100003a,
	F12 = 0x0100003b,
};

/** PC-Style Function Keys */
enum class CsiKey {
	Up = 'A',	 // CUU
	Down = 'B',	 // CUD
	Right = 'C', // CUF
	Left = 'D',	 // CUB
	End = 'F',	 // CPL
	Home = 'H',	 // CUP

	// Enter = 'E', // CNL ?
	// Func_ = 'G', // CHA ?
};
/** */

// template for future parser
class Parser {
  private:
	/** Escape sequence character (ESC) */
	const char m_esc = '\x1b';

	/** Control Sequence Introducer (CSI) */
	const char m_csi[2] = {m_esc, '['};
	/** */

	/** Single shift 3 Sequence (SS3) */
	const char m_ss3[2] = {m_esc, 'O'};
	/** */

	using OnReadyCallback = std::function<int(const char *data, size_t size)>;
	using OnEventCallback = std::function<int(const char *data, size_t size)>;

	OnReadyCallback _on_ready_callback = nullptr;
	OnEventCallback _on_event_callback = nullptr;

  public:
	Parser() = default;
	~Parser() = default;

	void setOnReadyCallback(OnReadyCallback f);
	void setOnEventCallback(OnEventCallback f);

	int parseData(const char *data, size_t size);
	int processEventData(std::vector<char> &data, int key_event);
};
} // namespace Cli

#endif // CLI_PARSER_HPP

#ifndef CLI_PARSER_HPP
#define CLI_PARSER_HPP

#include <cstdint>
#include <stddef.h>
#include <stdint.h>

#include <functional>

namespace Cli {
enum class Key {
	// this Qt analogue. need own values or universal converter
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

/** VT100 Mode Single-character functions. Control characters */
enum class CtlCh : unsigned char {
	ENQ = 0x05, /* Return Terminal Status (ENQ  is Ctrl-E).  Default response is
				an empty string, but may be overridden by a resource
				answerbackString. */
	BEL = 0x07, /* Bell (BEL  is Ctrl-G). */
	BS = 0x08,	/* Backspace (BS  is Ctrl-H). */
	HT = 0x09,	/* Horizontal Tab (HTS  is Ctrl-I). */
	LF = 0x0a,	/* Line Feed or New Line (NL).  (LF  is Ctrl-J). */
	VT = 0x0b,	/* Vertical Tab (VT  is Ctrl-K).  This is treated the same as LF. */
	FF = 0x0c,	/* Form Feed or New Page (NP ).  (FF  is Ctrl-L).  FF  is treated
				the same as LF. */
	CR = 0x0d,	/* Carriage Return (CR  is Ctrl-M). */
	SO = 0x0e,	/* Switch to Alternate Character Set (Ctrl-N is Shift Out or
				LS1).  This invokes the G1 character set as GL.
				VT200 and up implement LS1. */
	SI = 0x0f,	/* Switch to Standard Character Set (Ctrl-O is Shift In or LS0).
				This invokes the G0 character set (the default) as GL.
				VT200 and up implement LS0. */
	CAN = 0x18, /* Cancel */
	SUB = 0x1a, /* Abort escape sequences */
	ESC = 0x1b, /* Starts an escape sequence */
	SP = 0x20,	/* Space. */
	DEL = 0x7f, /* Delete is ignored */
	CSI = 0x9b, /* Equivalent to ESC [ */
};

/** Controls beginning with ESC */

/** C1 (8-bit) Control Characters */
/** Controls beginning with ESC */
enum class CtlSq {
	IND = 'D',	 /* Index (IND  is 0x84). */
	NEL = 'E',	 /* Next Line (NEL  is 0x85). */
	HTS = 'H',	 /* Tab Set (HTS  is 0x88). */
	RI = 'M',	 /* Reverse Index (RI  is 0x8d). */
	SS2 = 'N',	 /* Single Shift Select of G2 Character Set (SS2  is 0x8e), VT220.
					This affects next character only. */
	SS3 = 'O',	 /* Single Shift Select of G3 Character Set(SS3 is 0x8f), VT220.
					This affects next character only. */
	DCS = 'P',	 /* Device Control String(DCS is 0x90). */
	SPA = 'V',	 /* Start of Guarded Area(SPA is 0x96). */
	EPA = 'W',	 /* End of Guarded Area(EPA is 0x97). */
	SOS = 'X',	 /* Start of String(SOS is 0x98). */
	DECID = 'Z', /* Return Terminal ID(DECID is 0x9a). Obsolete form of CSI c(DA). */
	CSI = '[',	 /* Control Sequence Introducer(CSI is 0x9b). */
	ST = '\\',	 /* String Terminator(ST is 0x9c). */
	OSC = ']',	 /* Operating System Command(OSC is 0x9d).
					 ESC ] R  Reset palette.
					 ESC ] P  Set palette, with parameter given in 7
						hexadecimal digits nrrggbb after the final P.
						Here n is the color (0–15), and rrggbb
						indicates the red/green/blue values (0–255). */
	PM = '^',	 /* Privacy Message(PM is 0x9e). */
	APC = '_',	 /* Application Program Command(APC is 0x9f). */

	//
	SP = ' ', /* ESC SP F  7-bit controls (S7C1T), VT220.  This tells the terminal to
							send C1 control characters as 7-bit sequences, e.g., its
							responses to queries.  DEC VT200 and up always accept 8-bit
							control sequences except when configured for VT100 mode.
				ESC SP G  8-bit controls (S8C1T), VT220.  This tells the terminal to
							send C1 control characters as 8-bit sequences, e.g., its
							responses to queries.  DEC VT200 and up always accept 8-bit
							control sequences except when configured for VT100 mode.

				ESC SP L  Set ANSI conformance level 1, ECMA-43.
				ESC SP M  Set ANSI conformance level 2, ECMA-43.
				ESC SP N  Set ANSI conformance level 3, ECMA-43. */

	DEC = '#', /* ESC # 3   DEC double-height line, top half (DECDHL), VT100.
				ESC # 4   DEC double-height line, bottom half (DECDHL), VT100.
				ESC # 5   DEC single-width line (DECSWL), VT100.
				ESC # 6   DEC double-width line (DECDWL), VT100.
				ESC # 8   DEC Screen Alignment Test (DECALN), VT100. */

	SEL = '%', /* ESC % @   Select default character set.  That is ISO 8859-1 (ISO 2022).
				ESC % G   Select UTF-8 character set, ISO 2022.
				ESC % 8   Select UTF-8 (obsolete) */

	G0 = '(', /* ESC (  Start sequence defining G0 character set (followed by one of B, 0, U, K, as below)
				ESC ( B  Select default (ISO/IEC 8859-1 mapping).
				ESC ( 0  Select VT100 graphics mapping.
				ESC ( U  Select null mapping - straight to character ROM.
				ESC ( K  Select user mapping - the map that is loaded by the utility mapscrn(8). */
	G1 = ')', /* ESC )  Start sequence defining G1 (followed by one of B, 0, U, K, as above). */

	/* ESC * C   Designate G2 Character Set, ISO 2022, VT220.
			The same character sets apply as for ESC ( C.

	ESC + C   Designate G3 Character Set, ISO 2022, VT220.
			The same character sets apply as for ESC ( C.

	ESC - C   Designate G1 Character Set, VT300.
			These controls apply only to 96-character sets.  Unlike the
			94-character sets, these can have different values than ASCII
			space and DEL for the mapping of 0x20 and 0x7f.  The valid
			final characters C for this control are:
				C = A  ⇒  ISO Latin-1 Supplemental, VT300.
				C = B  ⇒  ISO Latin-2 Supplemental, VT500.
				C = F  ⇒  ISO Greek Supplemental, VT500.
				C = H  ⇒  ISO Hebrew Supplemental, VT500.
				C = L  ⇒  ISO Latin-Cyrillic, VT500.
				C = M  ⇒  ISO Latin-5 Supplemental, VT500.

	ESC . C   Designate G2 Character Set, VT300.
			The same character sets apply as for ESC - C.

	ESC / C   Designate G3 Character Set, VT300.
			The same character sets apply as for ESC - C. */

	//
	DECBI = 6,	   /* Back Index (DECBI), VT420 and up. */
	DECSC = 7,	   /* Save Cursor (DECSC), VT100. */
	DECRC = 8,	   /* Restore Cursor (DECRC), VT100. */
	DECFI = 9,	   /* Forward Index (DECFI), VT420 and up. */
	DECKPAM = '=', /* Application Keypad (DECKPAM). */
	DECKPNM = '>', /* Normal Keypad (DECKPNM), VT100. */
	RIS = 'c',	   /* Reset. */
	ML = 'l',	   /* Memory Lock (per HP terminals).  Locks memory above the
					cursor. */
	MU = 'm',	   /* Memory Unlock (per HP terminals). */
	LS2 = 'n',	   /* Invoke the G2 Character Set as GL (LS2). */
	LS3 = 'o',	   /* Invoke the G3 Character Set as GL (LS3). */
	LS3R = '|',	   /* Invoke the G3 Character Set as GR (LS3R). */
	LS2R = '}',	   /* Invoke the G2 Character Set as GR (LS2R). */
	LS1R = '~',	   /* Invoke the G1 Character Set as GR (LS1R), VT100. */
};

/** ECMA-48 CSI sequences */

/** ECMA-48 CSI final character */
enum class CSISqFinal : char {
	ICH = '@',	   /* Insert the indicated # of blank characters. */
	CUU = 'A',	   /* Move cursor up the indicated # of rows. */
	CUD = 'B',	   /* Move cursor down the indicated # of rows. */
	CUF = 'C',	   /* Move cursor right the indicated # of columns. */
	CUB = 'D',	   /* Move cursor left the indicated # of columns. */
	CNL = 'E',	   /* Move cursor down the indicated # of rows, to column 1. */
	CPL = 'F',	   /* Move cursor up the indicated # of rows, to column 1. */
	CHA = 'G',	   /* Move cursor to indicated column in current row. */
	CUP = 'H',	   /* Move cursor to the indicated row, column (origin at 1,1). */
	ED = 'J',	   /* Erase display (default: from cursor to end of display).
					ESC [ 1 J: erase from start to cursor.
					ESC [ 2 J: erase whole display.
					ESC [ 3 J: erase whole display including scroll-back buffer (since Linux 3.0). */
	EL = 'K',	   /* Erase line (default: from cursor to end of line).
					ESC [ 1 K: erase from start of line to cursor.
					ESC [ 2 K: erase whole line. */
	IL = 'L',	   /* Insert the indicated # of blank lines. */
	DL = 'M',	   /* Delete the indicated # of lines. */
	DCH = 'P',	   /* Delete the indicated # of characters on current line. */
	ECH = 'X',	   /* Erase the indicated # of characters on current line. */
	HPR = 'a',	   /* Move cursor right the indicated # of columns. */
	DA = 'c',	   /* Answer ESC [ ? 6 c: "I am a VT102". */
	VPA = 'd',	   /* Move cursor to the indicated row, current column. */
	VPR = 'e',	   /* Move cursor down the indicated # of rows. */
	HVP = 'f',	   /* Move cursor to the indicated row, column. */
	TBC = 'g',	   /* Without parameter: clear tab stop at current position. */
				   /*  ESC [ 3 g: delete all tab stops. */
	SM = 'h',	   /* Set Mode (see below). */
	RM = 'l',	   /* Reset Mode (see below). */
	SGR = 'm',	   /* Set attributes (see below). */
	DSR = 'n',	   /* Status report (see below). */
	DECLL = 'q',   /* Set keyboard LEDs.
					ESC [ 0 q: clear all LEDs
					ESC [ 1 q: set Scroll Lock LED
					ESC [ 2 q: set Num Lock LED
					ESC [ 3 q: set Caps Lock LED */
	DECSTBM = 'r', /* Set scrolling region; parameters are top and bottom row. */
	SCOSC = 's',   /* Save cursor location. */
	SCORC = 'u',   /* Restore cursor location. */
	HPA = '`',	   /* Move cursor to indicated column in current row. */
};

/** */
class Parser {
  public:
	enum State : uint8_t {
		NONE = 0b0,
		ESC = 0b1 << 0,
		CSI = 0b1 << 1,
		OSC = 0b1 << 2,
		// others ...
		FINAL = 0b1 << 7,
	};

  private:
	const size_t _cs_buffer_size = 1024;
	const size_t _ns_buffer_size = 1024;

	/** Control Sequence Introducer (CSI) */
	const char m_csi[2] = {(char)CtlCh::ESC, (char)CtlSq::CSI};
	// const char m_csi[1] = {'\x9b'}; // or

	/** Operating System Command (OSC) */
	const char m_osc[2] = {(char)CtlCh::ESC, (char)CtlSq::OSC};
	// const char m_osc[1] = {'\x9d'}; // or

	/** VT100 console sequences not implemented on the Linux console */

	/** Single shift 3 Sequence (SS2) */
	const char m_ss2[2] = {(char)CtlCh::ESC, (char)CtlSq::SS2};
	// const char m_ss2[1] = {'\x8e'}; // or

	/** Single shift 3 Sequence (SS3) */
	const char m_ss3[2] = {(char)CtlCh::ESC, (char)CtlSq::SS3};
	// const char m_ss3[1] = {'\x8f'}; // or

	/** Device Control String (DCS) */
	const char m_dcs[2] = {(char)CtlCh::ESC, (char)CtlSq::DCS};
	// const char m_dcs[1] = {'\x90'}; // or

	std::vector<char> _cs_buf;
	std::vector<char> _ns_buf;
	uint8_t _state = State::NONE;

	using OnReadyCallback = std::function<int(const char *data, size_t size)>;
	using OnEventCallback = std::function<int(const char *data, size_t size)>;

	OnReadyCallback _on_ready_callback = nullptr;
	OnEventCallback _on_event_callback = nullptr;

  protected:
	int collect(const char c);

  public:
	Parser();
	~Parser() = default;

	void setOnReadyCallback(OnReadyCallback f);
	void setOnEventCallback(OnEventCallback f);

	int parseData(const char *data, size_t size);
	int processEventData(std::vector<char> &data, int key_event);
};
} // namespace Cli

#endif // CLI_PARSER_HPP

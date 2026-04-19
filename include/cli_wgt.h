/**
 * @file    cli_wgt.h
 * @authors y.sharapov, v.mineev
 * @date    14.06.2019
 */

#pragma once

#define OWN_SEQUENCES 0
#if OWN_SEQUENCES
#define SEQUENCES "\x1B[1;71H\x1B[3;9H"
#endif

#include <QDebug>
#include <QPlainTextEdit>
#include <QSerialPort>
#include <QTextBlock>
#include <QTextCodec>

#define CHARACTER_SET "UTF-8"
//"UTF-8"
//"Windows-1251"

/** Last character in CSI */
enum CSI_Final {
	None = '\0',
	CursorUp = 'A',
	CursorDown = 'B',
	CursorForward = 'C',
	CursorBack = 'D',
	CursorPosition = 'H',
	EraseInLine = 'K',
	EraseData = 'J',
	InsertLine = 'L',
	DeleteLine = 'M',
	DeleteCharacter = 'P',
	ScrollUplines = 'S',
	SelectGraphicRendition = 'm',
	SetScrollingRegion = 'r',
	SetMode = 'h',
	ResetMode = 'l'
};
/** */

enum CSI_Intermediate {
	DEC = '?',
	SET = '>',
};

enum C1_Control_Characters {
	ReverseIndex = 'M',
	ControlSequenceIntroducer = '[',
};

enum ControlsBeginning {
	SaveCursor = 7,
	RestoreCursor = 8,
	NormalKeypad = '>',
	ApplicationKeypad = '=',
	DesignateG0Character = '(',
};

enum _Mode {
	/** No modes **/
	NM = 0x00,
};

enum DEC_PrivateMode {
	/** Application / Normal Cursor Keys **/
	DECCKM = 0x01,
};

enum Mode {
	/** Keyboard Action Mode */
	AM = 0x02,
	/** Insert/replace Mode */
	IRM = 0x04,
	/** Send/Receive */
	SRM = 0x12,
	/** Automatic New line / Normal Linefeed */
	LNM = 0x20
};

/** PC-Style Function Keys */
enum Func_Key { Func_Up = 'A', Func_Down = 'B', Func_Right = 'C', Func_Left = 'D', Func_Home = 'H', Func_End = 'F' };
/** */

class KeySequence {
  public:
	explicit KeySequence() {
	}
	~KeySequence() {
	}

	void setMode(DEC_PrivateMode mode) {
		m_mode |= mode;
	}
	void setModes(int mode) {
		m_mode |= mode;
	}
	void resetMode(DEC_PrivateMode mode) {
		m_mode ^= mode;
	}
	void resetModes(int modes) {
		m_mode ^= modes;
	}
	int modes() {
		return m_mode;
	}

  private:
	int m_mode = NM;
};

class TextCursor : public QTextCursor {

  public:
	TextCursor() : QTextCursor() {
	}
	~TextCursor() {
	}

	TextCursor(QTextDocument *document) : QTextCursor(document) {
	}

	TextCursor &operator=(QTextCursor &&other) {
		//        if (this == &other) {
		//            return *this;
		//        }
		swap(other); // attention!!! using swap method
		return *this;
	}
	/** Print character with cursor mods */
	void insert(const QString &text, const QTextCharFormat &format) {
		// if(!(Mode::IRM & modes())) {
		if (!atBlockEnd()) {
			deleteChar();
		}
		//}

		// if(Cursor_Mode & modes()) {
		//..
		// }
		insertText(text, format);
	}

	void setMode(Mode mode) {
		m_mode |= mode;
	}
	void setModes(int modes) {
		m_mode |= modes;
	}
	void resetMode(Mode mode) {
		m_mode ^= mode;
	}
	void resetModes(int modes) {
		m_mode ^= modes;
	}
	int modes() {
		return m_mode;
	}

  private:
	int m_mode = NM;
};

class cli_wgt : public QPlainTextEdit {
	Q_OBJECT

  public:
	explicit cli_wgt(QWidget *parent = nullptr);
	~cli_wgt() {
		delete m_seq_key;
	}
	void setSerialPort(QSerialPort *port);

  public slots:
	void receiveData(const QByteArray &array);

  signals:
	void dataReceived(const QByteArray &array);

  protected:
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseDoubleClickEvent(QMouseEvent *e);
	virtual void contextMenuEvent(QContextMenuEvent *e);

  private:
	void readData();
	void writeData(const QByteArray &array);

	bool funcCollect(const uint8_t __c, TextCursor &cursor, QTextCharFormat &charFormat);
	std::vector<int> funcParams(const char *csiSequence);
	void csiFuncView(QByteArray &csiSequence);

	void keySequence(QByteArray &data, int key);

	void csiCUB(TextCursor &cursor, std::vector<int> params);
	void csiCUD(TextCursor &cursor, std::vector<int> params);
	void csiCUU(TextCursor &cursor, std::vector<int> params);
	void csiCUF(TextCursor &cursor, std::vector<int> params);
	void csiCUP(TextCursor &cursor, std::vector<int> params);
	void csiEL(TextCursor &cursor, std::vector<int> params);
	void csiED(TextCursor &cursor, std::vector<int> params);
	void csiIL(TextCursor &cursor, std::vector<int> params);
	void csiDL(TextCursor &cursor, std::vector<int> params);
	void csiDCH(TextCursor &cursor, std::vector<int> params);
	void csiSU(TextCursor &cursor, std::vector<int> params);
	void csiDECSTBM(TextCursor &cursor, std::vector<int> params);
	void csiSM(TextCursor &cursor, std::vector<int> params);
	void csiRM(TextCursor &cursor, std::vector<int> params);
	void csiSGR(TextCursor &cursor, QTextCharFormat &charFormat, std::vector<int> params);

	void csiDECSET(std::vector<int> params);
	void csiDECRST(std::vector<int> params);

	void imitRemovePositions(TextCursor &cursor);

	QSerialPort *m_pPort = nullptr;

	/** Esc character */
	const char m_esc = '\x1b';

	/** Control Sequence Introducer */
	const char *m_csi = "\x1b[";
	/** */

	/** SS3 Sequence */
	const char *m_ss3 = "\x1bO";
	/** */

	QTextCodec *m_codec;
	KeySequence *m_seq_key;
};

/**
 * @file    cli_wgt.cpp
 * @author  y.sharapov, v.mineev
 * @date    14.06.2019
 */

#include "cli_wgt.h"

#include <QScrollBar>

cli_wgt::cli_wgt(QWidget *parent) : QPlainTextEdit(parent), m_seq_key(new KeySequence()) {
	m_codec = QTextCodec::codecForName(CHARACTER_SET);
	document()->setMaximumBlockCount(2048);
	QPalette p = palette();
	p.setColor(QPalette::Base, Qt::black);
	//    p.setColor(QPalette::Text, QColor(42,220,130));
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);

	setTextCursor(TextCursor(document()));

	setFont(font);
	setPalette(p);
}

void cli_wgt::setSerialPort(QSerialPort *port) {
	m_pPort = port;
	connect(m_pPort, &QSerialPort::readyRead, this, &cli_wgt::readData);
}

void cli_wgt::receiveData(const QByteArray &array) {
	static QTextCharFormat charFormat;
	static TextCursor cursor;
	static std::string unicodebuf;
	qDebug() << array.toHex();
	for (int i = 0; i < array.count(); i++) {
		cursor = textCursor();
		uint8_t __uc = static_cast<uint8_t>(array.constData()[i]);
		bool sequenceCollecting = funcCollect(__uc, cursor, charFormat);

		// Keys switch
		switch (__uc) {
		case '\r': // CR imitation
			cursor.movePosition(TextCursor::StartOfBlock);
			break;
		case '\n': { // LF imitation
			csiCUD(cursor, std::vector<int>());
			break;
		}
		case 0x07:
			// bell
			break;
		case 0x08:
			// cursor.movePosition(TextCursor::PreviousCharacter);
			csiCUB(cursor, std::vector<int>());
			break;
		case 0x0f:
			// shift in
			break;
		case 0x0e:
			// shift out
			break;
		case 0x7f:
			// cursor.movePosition(TextCursor::PreviousCharacter);
			csiCUB(cursor, std::vector<int>());
			break;
		default:
			if (!sequenceCollecting) {
				// unicode stacking
				if (__uc >= 0x80) {
					unicodebuf.push_back(static_cast<char>(__uc));
				} else {
					if (!unicodebuf.empty()) {
						cursor.insert(unicodebuf.c_str(), charFormat);
						unicodebuf.clear();
					} else {
						// for Windows-1251-----v
						cursor.insert(m_codec->toUnicode(QByteArray(sizeof(__uc), static_cast<char>(__uc))), charFormat);
					}
				}
				// cursor.insert(m_codec->toUnicode(QByteArray(sizeof(__uc), static_cast<char>(__uc))), charFormat);
			}
			break;
		}
		setTextCursor(cursor);
	}

	//
	QScrollBar *sb = verticalScrollBar();
	sb->setValue(sb->maximum());
}

void cli_wgt::readData() {
#if OWN_SEQUENCES
	QByteArray array = SEQUENCES;
#else
	QByteArray array = m_pPort->readAll();
#endif

	receiveData(array);
}

bool cli_wgt::funcCollect(const uint8_t __c, TextCursor &cursor, QTextCharFormat &charFormat) {

	static std::string buf;
	static bool escFound = false;
	static bool csiFound = false;
	static bool csiIntermediateFound = false;
	static bool finalFound = false;

	auto lfReadySequence = [&]() -> void {
		finalFound = true;
		QByteArray dbg(buf.c_str());
		qDebug() << dbg;
	};

	if (finalFound) {
		escFound = false;
		csiFound = false;
		csiIntermediateFound = false;
		finalFound = false;
		buf.clear();
	}

	buf.push_back(static_cast<char>(__c));
	if (__c == m_esc) {
		escFound = true;
		buf.clear();
		return escFound;
	} else {
		if (!escFound) {
			buf.clear();
		}
	}

	if (escFound) {
		switch (__c) {
		case ControlSequenceIntroducer:
			csiFound = true;
			return escFound;
		case NormalKeypad:
			lfReadySequence();
			//
			break;
		case SaveCursor:
			lfReadySequence();
			//
			break;
		case RestoreCursor:
			lfReadySequence();
			//
			break;
		case ReverseIndex:
			lfReadySequence();
			//
			break;
		}

		/** CSI Found */
		if (csiFound) {
			switch (__c) {
			case CursorUp:
				lfReadySequence();
				csiCUU(cursor, funcParams(buf.c_str()));
				break;
			case CursorDown:
				lfReadySequence();
				csiCUD(cursor, funcParams(buf.c_str()));
				break;
			case CursorForward:
				lfReadySequence();
				csiCUF(cursor, funcParams(buf.c_str()));
				break;
			case CursorBack:
				lfReadySequence();
				csiCUB(cursor, funcParams(buf.c_str()));
				break;
			case CursorPosition:
				lfReadySequence();
				csiCUP(cursor, funcParams(buf.c_str()));
				break;
			case DeleteCharacter:
				lfReadySequence();
				csiDCH(cursor, funcParams(buf.c_str()));
				break;
			case EraseInLine:
				lfReadySequence();
				csiEL(cursor, funcParams(buf.c_str()));
				break;
			case EraseData:
				lfReadySequence();
				csiED(cursor, funcParams(buf.c_str()));
				break;
			case InsertLine:
				lfReadySequence();
				csiIL(cursor, funcParams(buf.c_str()));
				break;
			case DeleteLine:
				lfReadySequence();
				csiDL(cursor, funcParams(buf.c_str()));
				break;

			case SelectGraphicRendition:
				lfReadySequence();
				csiSGR(cursor, charFormat, funcParams(buf.c_str()));
				break;
			case ScrollUplines:
				lfReadySequence();
				csiSU(cursor, funcParams(buf.c_str()));
				break;
			case SetScrollingRegion:
				lfReadySequence();
				csiDECSTBM(cursor, funcParams(buf.c_str()));
				break;
			case SetMode:
				lfReadySequence();
				csiSM(cursor, funcParams(buf.c_str()));
				break;
			case ResetMode:
				lfReadySequence();
				csiRM(cursor, funcParams(buf.c_str()));
				break;

			/** Intermediate Test */
			case DEC:
				csiIntermediateFound = true;
				return escFound;
			case SET:
				csiIntermediateFound = true;
				return escFound;
			}
		}

		/** CSI Intermediate Found */
		if (csiIntermediateFound) {
			switch (__c) {
			case SetMode:
				lfReadySequence();
				csiDECSET(funcParams(buf.c_str()));
				break;
			case ResetMode:
				lfReadySequence();
				csiDECRST(funcParams(buf.c_str()));
				break;
			}
		}
	}
	return escFound;
}

// rewrite this more universaly
std::vector<int> cli_wgt::funcParams(const char *csiSequence) {
	char sep = ';';
	std::vector<int> params;
	if (csiSequence) {
		std::string buf;
		for (size_t i = 0; i < strlen(csiSequence) - 1; i++) {
			if (csiSequence[i] != sep) {
				if (isdigit(csiSequence[i])) {
					buf.push_back(csiSequence[i]);
				}
			} else {
				params.push_back(std::atoi(buf.c_str()));
				buf.clear();
			}
		}
		if (!buf.empty()) {
			params.push_back(std::atoi(buf.c_str()));
		}
	}
	//    if(params.empty()) {
	//        params.push_back(1); //default param equal 1
	//    }
	return params;
}

void cli_wgt::csiSGR(TextCursor &cursor, QTextCharFormat &charFormat, std::vector<int> params) {
	auto lfDefault = [&]() -> void {
		charFormat.clearBackground(); // or charFormat.clearBackground()
		charFormat.clearForeground(); // or charFormat.clearForeground()
		QFont font = charFormat.font();
		font.setFamily("Monospace");
		font.setWeight(QFont::Normal);
		font.setStyleHint(QFont::TypeWriter);
		charFormat.setFont(font);
	};

	if (!params.empty()) {
		for (int param : params) {
			switch (param) {
			case 0: {
				lfDefault();
				break;
			}
			case 1: {
				QFont font = charFormat.font();
				font.setFamily("Monospace");
				font.setWeight(QFont::Bold);
				font.setStyleHint(QFont::TypeWriter);
				charFormat.setFont(font);
				break;
			}
			case 7: {
				QBrush brushBackground = charFormat.background();
				QBrush brushForeground = charFormat.foreground();
				charFormat.setBackground(brushForeground);
				charFormat.setForeground(brushBackground);
				break;
			}
			/* Simple SGR realization
			Intensity 0       1     2       3    	4       5        	6    	7
			Normal    Black   Red 	Green 	Yellow 	Blue 	Magenta 	Cyan    White
			Bright    Black   Red 	Green 	Yellow 	Blue 	Magenta 	Cyan 	White
			*/
			case 30:
				charFormat.setForeground(Qt::black);
				break;
			case 31:
				charFormat.setForeground(Qt::red);
				break;
			case 32:
				charFormat.setForeground(Qt::green);
				break;
			case 33:
				charFormat.setForeground(Qt::yellow);
				break;
			case 34:
				charFormat.setForeground(Qt::blue);
				break;
			case 35:
				charFormat.setForeground(Qt::magenta);
				break;
			case 36:
				charFormat.setForeground(Qt::cyan);
				break;
			case 37:
				charFormat.setForeground(Qt::white);
				break;
			// Copy-paste?? Not good
			case 40:
				charFormat.setBackground(Qt::black);
				break;
			case 41:
				charFormat.setBackground(Qt::red);
				break;
			case 42:
				charFormat.setBackground(Qt::green);
				break;
			case 43:
				charFormat.setBackground(Qt::yellow);
				break;
			case 44:
				charFormat.setBackground(Qt::blue);
				break;
			case 45:
				charFormat.setBackground(Qt::magenta);
				break;
			case 46:
				charFormat.setBackground(Qt::cyan);
				break;
			case 47:
				charFormat.setBackground(Qt::white);
				break;

			default:
				break;
			}
		}
	} else {
		lfDefault();
	}

	cursor.setCharFormat(charFormat);
}

void cli_wgt::csiCUB(TextCursor &cursor, std::vector<int> params) {
	auto lfCUB = [&]() -> void {
		if (!cursor.atBlockStart()) {
			cursor.movePosition(TextCursor::PreviousCharacter, TextCursor::MoveAnchor);
		} /* else {
			 cursor.insertText(" ");
		 }*/
	};

	if (!params.empty()) {
		for (int i = 0; i < params[0]; i++)
			lfCUB();
	} else {
		lfCUB();
	}
}

void cli_wgt::csiCUD(TextCursor &cursor, std::vector<int> params) {
	auto lfCUD = [&]() -> void {
		int oldPositionInBlock = cursor.positionInBlock();
		bool moved = cursor.movePosition(TextCursor::NextBlock, TextCursor::MoveAnchor);
		if (!moved) {
			cursor.movePosition(TextCursor::EndOfBlock);
			cursor.insertBlock();
			// cursor.movePosition(TextCursor::NextBlock);
			int spCount = oldPositionInBlock - cursor.positionInBlock();
			for (int sp = 0; sp < spCount; sp++) {
				cursor.insertText(" ");
			}
		} else {
			for (int pos = 0; pos < oldPositionInBlock; pos++) {
				if (!cursor.atBlockEnd()) {
					cursor.movePosition(TextCursor::NextCharacter, TextCursor::MoveAnchor);
				} else {
					cursor.insertText(" ");
				}
			}
		}
	};

	if (!params.empty()) {
		for (int i = 0; i < params[0]; i++)
			lfCUD();
	} else {
		lfCUD();
	}
}

void cli_wgt::csiCUU(TextCursor &cursor, std::vector<int> params) {
	auto lfCUU = [&]() -> void {
		int oldPositionInBlock = cursor.positionInBlock();
		bool moved = cursor.movePosition(TextCursor::PreviousBlock, TextCursor::MoveAnchor);
		if (!moved) {
			cursor.movePosition(TextCursor::StartOfBlock);
			cursor.insertBlock();
			cursor.movePosition(TextCursor::PreviousBlock);
			int spCount = oldPositionInBlock - cursor.positionInBlock();
			for (int sp = 0; sp < spCount; sp++) {
				cursor.insertText(" ");
			}
		} else {
			for (int pos = 0; pos < oldPositionInBlock; pos++) {
				if (!cursor.atBlockEnd()) {
					cursor.movePosition(TextCursor::NextCharacter, TextCursor::MoveAnchor);
				} else {
					cursor.insertText(" ");
				}
			}
		}
	};

	if (!params.empty()) {
		for (int i = 0; i < params[0]; i++)
			lfCUU();
	} else {
		lfCUU();
	}
}

void cli_wgt::csiCUF(TextCursor &cursor, std::vector<int> params) {
	auto lfCUF = [&]() -> void {
		if (!cursor.atBlockEnd()) {
			cursor.movePosition(TextCursor::NextCharacter, TextCursor::MoveAnchor);
		} else {
			cursor.insertText(" ");
		}
	};

	if (!params.empty()) {
		for (int i = 0; i < params[0]; i++)
			lfCUF();
	} else {
		lfCUF();
	}
}

void cli_wgt::csiCUP(TextCursor &cursor, std::vector<int> params) {
	if (!params.empty()) {
		cursor.movePosition(TextCursor::Start, TextCursor::MoveAnchor);

		csiCUF(cursor, std::vector<int>(1, params[1] - 1));
		csiCUD(cursor, std::vector<int>(1, params[0] - 1));

	} else {
		cursor.movePosition(TextCursor::Start, TextCursor::MoveAnchor);
	}
}

void cli_wgt::csiDCH(TextCursor &cursor, std::vector<int> params) {
	if (!params.empty()) {
		cursor.movePosition(TextCursor::NextCharacter, TextCursor::KeepAnchor, params[0]);
		imitRemovePositions(cursor);
	} else {
		cursor.movePosition(TextCursor::NextCharacter, TextCursor::KeepAnchor);
		imitRemovePositions(cursor);
	}
}

void cli_wgt::csiSU(TextCursor &cursor, std::vector<int> params) {
	// implement this
}

void cli_wgt::csiDECSTBM(TextCursor &cursor, std::vector<int> params) {
	// implement this
}

void cli_wgt::csiSM(TextCursor &cursor, std::vector<int> params) {
	if (!params.empty()) {
		cursor.setModes(params[0]);
	}
}

void cli_wgt::csiRM(TextCursor &cursor, std::vector<int> params) {
	if (!params.empty()) {
		cursor.resetModes(params[0]);
	}
}

void cli_wgt::csiDECSET(std::vector<int> params) {
	if (!params.empty()) {
		m_seq_key->setModes(params[0]);
	}
}
void cli_wgt::csiDECRST(std::vector<int> params) {
	if (!params.empty()) {
		m_seq_key->resetModes(params[0]);
	}
}

void cli_wgt::csiEL(TextCursor &cursor, std::vector<int> params) {
	if (!params.empty()) {
		switch (params[0]) {
		case 0:
			cursor.movePosition(TextCursor::EndOfBlock, TextCursor::KeepAnchor);
			imitRemovePositions(cursor);
			break;
		case 1:
			cursor.movePosition(TextCursor::StartOfBlock, TextCursor::KeepAnchor);
			imitRemovePositions(cursor);
			break;
		case 2:
			cursor.movePosition(TextCursor::StartOfBlock, TextCursor::MoveAnchor);
			cursor.movePosition(TextCursor::EndOfBlock, TextCursor::KeepAnchor);
			imitRemovePositions(cursor);
			break;
		}
	} else {
		cursor.movePosition(TextCursor::EndOfBlock, TextCursor::KeepAnchor);
		imitRemovePositions(cursor);
	}
}

void cli_wgt::csiED(TextCursor &cursor, std::vector<int> params) {
	if (!params.empty()) {
		switch (params[0]) {
		case 0:
			cursor.movePosition(TextCursor::End, TextCursor::KeepAnchor);
			imitRemovePositions(cursor);
			break;
		case 1:
			cursor.movePosition(TextCursor::Start, TextCursor::KeepAnchor);
			imitRemovePositions(cursor);
			break;
		case 2:
			cursor.movePosition(TextCursor::Start, TextCursor::MoveAnchor);
			cursor.movePosition(TextCursor::End, TextCursor::KeepAnchor);
			imitRemovePositions(cursor);
			break;
		case 3:
			// reimplement for saved data
			cursor.movePosition(TextCursor::Start, TextCursor::MoveAnchor);
			cursor.movePosition(TextCursor::End, TextCursor::KeepAnchor);
			imitRemovePositions(cursor);
			break;
		}
	} else {
		cursor.movePosition(TextCursor::End, TextCursor::KeepAnchor);
		imitRemovePositions(cursor);
	}
}

void cli_wgt::csiIL(TextCursor &cursor, std::vector<int> params) {
	// implement this
}

void cli_wgt::csiDL(TextCursor &cursor, std::vector<int> params) {
	auto lfDL = [&]() {
		cursor.movePosition(TextCursor::StartOfBlock, TextCursor::MoveAnchor);
		cursor.movePosition(TextCursor::EndOfBlock, TextCursor::KeepAnchor);
		imitRemovePositions(cursor);
	};
	if (!params.empty()) {
		for (int i = 0; i < params[0]; i++)
			lfDL();
	} else {
		lfDL();
	}
}

void cli_wgt::imitRemovePositions(TextCursor &cursor) {
	// cursor.insertText(cursor.selectedText().fill(' '));
	cursor.removeSelectedText();
}

void cli_wgt::csiFuncView(QByteArray &csiSequence) {
	qDebug() << csiSequence << funcParams(csiSequence) << "\n";
}

void cli_wgt::keySequence(QByteArray &data, int key) {
	const char *seq_key;
	if (DECCKM & m_seq_key->modes()) {
		seq_key = m_ss3;
	} else {
		seq_key = m_csi;
	}

	switch (key) {
	case Qt::Key_Up:
		data.append(seq_key);
		data.append(Func_Up);
		break;
	case Qt::Key_Down:
		data.append(seq_key);
		data.append(Func_Down);
		break;
	case Qt::Key_Right:
		data.append(seq_key);
		data.append(Func_Right);
		break;
	case Qt::Key_Left:
		data.append(seq_key);
		data.append(Func_Left);
		break;
	case Qt::Key_Home:
		data.append(seq_key);
		data.append(Func_Home);
		break;
	case Qt::Key_End:
		data.append(seq_key);
		data.append(Func_End);
		break;

	case Qt::Key_F1:
		data.append(m_ss3);
		data.append('P');
		break;
	case Qt::Key_F2:
		data.append(m_ss3);
		data.append('Q');
		break;
	case Qt::Key_F3:
		data.append(m_ss3);
		data.append('R');
		break;
	case Qt::Key_F4:
		data.append(m_ss3);
		data.append('S');
		break;
	case Qt::Key_F5:
		data.append(m_csi);
		data.append("15~");
		break;
	case Qt::Key_F6:
		data.append(m_csi);
		data.append("17~");
		break;
	case Qt::Key_F7:
		data.append(m_csi);
		data.append("18~");
		break;
	case Qt::Key_F8:
		data.append(m_csi);
		data.append("19~");
		break;
	case Qt::Key_F9:
		data.append(m_csi);
		data.append("20~");
		break;
	case Qt::Key_F10:
		data.append(m_csi);
		data.append("21~");
		break;
	case Qt::Key_F11:
		data.append(m_csi);
		data.append("23~");
		break;
	case Qt::Key_F12:
		data.append(m_csi);
		data.append("24~");
		break;
	}
}

void cli_wgt::writeData(const QByteArray &array) {
	dataReceived(array);
	try {
#if OWN_SEQUENCES
		readData();
#else
		if (m_pPort == nullptr) {
			throw "Port is not Created";
		}
		if (!m_pPort->isOpen()) {
			throw "Port is not Open";
		}
		m_pPort->write(array);
#endif
	} catch (const char *msg) {
		qDebug() << msg;
	}
}

void cli_wgt::keyPressEvent(QKeyEvent *e) {
	QByteArray data = e->text().toLocal8Bit();

	keySequence(data, e->key());
	writeData(data);
}

void cli_wgt::mousePressEvent(QMouseEvent *e) {
	Q_UNUSED(e)
	// QPlainTextEdit::mousePressEvent(e);
	setFocus();
}

void cli_wgt::mouseDoubleClickEvent(QMouseEvent *e) {
	Q_UNUSED(e)
}

void cli_wgt::contextMenuEvent(QContextMenuEvent *e) {
	Q_UNUSED(e)
	// QPlainTextEdit::contextMenuEvent(e);
}

/**
 * @file    cli_wgt.cpp
 * @author  y.sharapov, v.mineev
 * @date    14.06.2019
 */

#include "cli_widget.h"

#include <QScrollBar>
#include <cstdint>
#include <qdir.h>
#include <qglobal.h>

CliWidget::CliWidget(QWidget *parent) : QPlainTextEdit(parent), m_seq_key(new KeySequence()) {
	m_codec = QTextCodec::codecForName(CHARACTER_SET);
	document()->setMaximumBlockCount(2048);

	this->undoAvailable(false);
    this->redoAvailable(false);
	this->copyAvailable(true);

	QPalette p = palette();
	p.setColor(QPalette::Base, Qt::black);
	//    p.setColor(QPalette::Text, QColor(42,220,130));

	QFont font("Monospace");
	font.setStyleHint(QFont::Monospace);
	font.setPointSize(8);

	setTextCursor(TextCursor(document()));
	setFont(font);
	setPalette(p);

	_cli_parser.setOnReadyCallback([ this ](const char *data, size_t size) {
		qDebug() << data;
		return 0;
	});

	_cli_parser.setOnEventCallback([ this ](const char *data, size_t size) {
		this->eventData(data, size);
		return 0;
	});
}

void CliWidget::processData(const QByteArray &array) {
	static QTextCharFormat charFormat;
	static TextCursor cursor;
	static std::string unicodebuf;

	qDebug() << array;

	_cli_parser.parseData(array.data(), array.size());

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

void onReadySequenceCallback(const char *buf, bool *is_final = nullptr) {
	if (is_final) {
		*is_final = true;
	}
	// qDebug() << buf;
}

bool CliWidget::funcCollect(const uint8_t __c, TextCursor &cursor, QTextCharFormat &charFormat) {

	static std::string buf;
	static bool escFound = false;
	static bool csiFound = false;
	static bool csiIntermediateFound = false;
	static bool finalFound = false;

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
			onReadySequenceCallback(buf.c_str(), &finalFound);
			//
			break;
		case SaveCursor:
			onReadySequenceCallback(buf.c_str(), &finalFound);
			//
			break;
		case RestoreCursor:
			onReadySequenceCallback(buf.c_str(), &finalFound);
			//
			break;
		case ReverseIndex:
			onReadySequenceCallback(buf.c_str(), &finalFound);
			//
			break;
		}

		/** CSI Found */
		if (csiFound) {
			switch (__c) {
			case CursorUp:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiCUU(cursor, funcParams(buf.c_str()));
				break;
			case CursorDown:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiCUD(cursor, funcParams(buf.c_str()));
				break;
			case CursorForward:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiCUF(cursor, funcParams(buf.c_str()));
				break;
			case CursorBack:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiCUB(cursor, funcParams(buf.c_str()));
				break;
			case CursorPosition:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiCUP(cursor, funcParams(buf.c_str()));
				break;
			case DeleteCharacter:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiDCH(cursor, funcParams(buf.c_str()));
				break;
			case EraseInLine:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiEL(cursor, funcParams(buf.c_str()));
				break;
			case EraseData:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiED(cursor, funcParams(buf.c_str()));
				break;
			case InsertLine:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiIL(cursor, funcParams(buf.c_str()));
				break;
			case DeleteLine:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiDL(cursor, funcParams(buf.c_str()));
				break;

			case SelectGraphicRendition:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiSGR(cursor, charFormat, funcParams(buf.c_str()));
				break;
			case ScrollUplines:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiSU(cursor, funcParams(buf.c_str()));
				break;
			case SetScrollingRegion:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiDECSTBM(cursor, funcParams(buf.c_str()));
				break;
			case SetMode:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiSM(cursor, funcParams(buf.c_str()));
				break;
			case ResetMode:
				onReadySequenceCallback(buf.c_str(), &finalFound);
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
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiDECSET(funcParams(buf.c_str()));
				break;
			case ResetMode:
				onReadySequenceCallback(buf.c_str(), &finalFound);
				csiDECRST(funcParams(buf.c_str()));
				break;
			}
		}
	}
	return escFound;
}

// rewrite this more universaly
std::vector<int> CliWidget::funcParams(const char *csiSequence) {
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

void CliWidget::csiSGR(TextCursor &cursor, QTextCharFormat &charFormat, std::vector<int> params) {
	auto lfDefault = [&]() -> void {
		charFormat.clearBackground(); // or charFormat.clearBackground()
		charFormat.clearForeground(); // or charFormat.clearForeground()
		QFont font = charFormat.font();
		font.setFamily("Monospace");
		font.setWeight(QFont::Normal);
		font.setPointSize(8);
		// font.setStyleHint(QFont::TypeWriter);
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
				// font.setStyleHint(QFont::TypeWriter);
				font.setPointSize(8);
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

void CliWidget::csiCUB(TextCursor &cursor, std::vector<int> params) {
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

void CliWidget::csiCUD(TextCursor &cursor, std::vector<int> params) {
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

void CliWidget::csiCUU(TextCursor &cursor, std::vector<int> params) {
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

void CliWidget::csiCUF(TextCursor &cursor, std::vector<int> params) {
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

void CliWidget::csiCUP(TextCursor &cursor, std::vector<int> params) {
	if (!params.empty()) {
		cursor.movePosition(TextCursor::Start, TextCursor::MoveAnchor);

		csiCUF(cursor, std::vector<int>(1, params[1] - 1));
		csiCUD(cursor, std::vector<int>(1, params[0] - 1));

	} else {
		cursor.movePosition(TextCursor::Start, TextCursor::MoveAnchor);
	}
}

void CliWidget::csiDCH(TextCursor &cursor, std::vector<int> params) {
	if (!params.empty()) {
		cursor.movePosition(TextCursor::NextCharacter, TextCursor::KeepAnchor, params[0]);
		imitRemovePositions(cursor);
	} else {
		cursor.movePosition(TextCursor::NextCharacter, TextCursor::KeepAnchor);
		imitRemovePositions(cursor);
	}
}

void CliWidget::csiSU(TextCursor &cursor, std::vector<int> params) {
	// implement this
}

void CliWidget::csiDECSTBM(TextCursor &cursor, std::vector<int> params) {
	// implement this
}

void CliWidget::csiSM(TextCursor &cursor, std::vector<int> params) {
	if (!params.empty()) {
		cursor.setModes(params[0]);
	}
}

void CliWidget::csiRM(TextCursor &cursor, std::vector<int> params) {
	if (!params.empty()) {
		cursor.resetModes(params[0]);
	}
}

void CliWidget::csiDECSET(std::vector<int> params) {
	if (!params.empty()) {
		m_seq_key->setModes(params[0]);
	}
}
void CliWidget::csiDECRST(std::vector<int> params) {
	if (!params.empty()) {
		m_seq_key->resetModes(params[0]);
	}
}

void CliWidget::csiEL(TextCursor &cursor, std::vector<int> params) {
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

void CliWidget::csiED(TextCursor &cursor, std::vector<int> params) {
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

void CliWidget::csiIL(TextCursor &cursor, std::vector<int> params) {
	// implement this
}

void CliWidget::csiDL(TextCursor &cursor, std::vector<int> params) {
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

void CliWidget::imitRemovePositions(TextCursor &cursor) {
	// cursor.insertText(cursor.selectedText().fill(' '));
	cursor.removeSelectedText();
}

void CliWidget::csiFuncView(QByteArray &csiSequence) {
	qDebug() << csiSequence << funcParams(csiSequence) << "\n";
}

void CliWidget::keyPressEvent(QKeyEvent *e) {
	QByteArray byte_array = e->text().toLocal8Bit();

	std::vector<char> data(byte_array.data(), byte_array.data() + byte_array.size());;
	_cli_parser.processEventData(data, e->key());
}

void CliWidget::mousePressEvent(QMouseEvent *e) {
	Q_UNUSED(e)
	// QPlainTextEdit::mousePressEvent(e);
	setFocus();
}

void CliWidget::mouseDoubleClickEvent(QMouseEvent *e) {
	Q_UNUSED(e)
}

void CliWidget::contextMenuEvent(QContextMenuEvent *e) {
	Q_UNUSED(e)
	// QPlainTextEdit::contextMenuEvent(e);
}

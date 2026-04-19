#include "terminal.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTextStream>

#include <DarkStyle.h>

#define _DbgPrint QTextStream(stdout)

QScopedPointer<QFile> m_logFile;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	a.setStyle(new DarkStyle);

	m_logFile.reset(new QFile(qApp->applicationDirPath() + "log.txt"));
	m_logFile.data()->open(QFile::WriteOnly | QFile::Text);

	qInstallMessageHandler(messageHandler);

	Terminal w;
	w.show();

	// setup icon
	QIcon cAppIcon("://icons/Alecive-Flatwoken-Apps-Terminal.ico");
	w.setWindowIcon(cAppIcon);
	w.setWindowTitle("Terminal");

	return a.exec();
}

// Реализация обработчика
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	QTextStream out(m_logFile.data());

	// out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
	switch (type) {
	case QtInfoMsg:
		out << "INF ";
		_DbgPrint << "INF ";
		break;
	case QtDebugMsg:
		out << "DBG ";
		_DbgPrint << "DBG ";
		break;
	case QtWarningMsg:
		out << "WRN ";
		_DbgPrint << "WRN ";
		break;
	case QtCriticalMsg:
		out << "CRT ";
		_DbgPrint << "CRT ";
		break;
	case QtFatalMsg:
		out << "FTL ";
		_DbgPrint << "FTL ";
		break;
	}
	out << context.category << ": " << msg << endl;
	out.flush();
	_DbgPrint << ": " << msg << endl;
}

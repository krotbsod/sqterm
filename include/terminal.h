/**
 * @file    terminal.h
 * @author  y.sharapov
 * @date    14.06.2019
 */

#pragma once

#include "cli_widget.h"
#include "settings.h"

#include <QDialog>
#include <QIntValidator>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class Terminal;
}

class Terminal : public QWidget {
	Q_OBJECT

  public:
	explicit Terminal(QWidget *parent = nullptr);
	~Terminal();

  private:
	Ui::Terminal *m_ui;

	QSerialPort *m_serial;
	Settings *m_settings;
	CliWidget *m_cli;

	void vPortSetup();
	bool bPortOpen();
	void vPortClose();
	void vScanPorts();

	QStringList listPortInfo;
  private slots:
	void readData();
	void writeData(const char *data, size_t size);

	void connectSerial();
	void save();
	void clear();
	void showSettings();

	void on_cbPort_activated(int index);
};

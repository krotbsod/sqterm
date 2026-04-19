/**
 * @file    terminal.h
 * @author  y.sharapov
 * @date    14.06.2019
 */

#pragma once

#include "cli_wgt.h"

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
	QSerialPort *m_port;
	cli_wgt *m_cli;

	void vPortSetup();
	bool bPortOpen();
	void vPortClose();
	void vScanPorts();

	QStringList listPortInfo;
  private slots:
	void vOnNewSerialData();

	void on_btnConnect_clicked();
	void on_btnSave_clicked();
	void on_btnClear_clicked();
	void on_cbPort_activated(int index);
};

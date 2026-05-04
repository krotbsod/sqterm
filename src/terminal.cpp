/**
 * @file    terminal.cpp
 * @author  y.sharapov
 * @date    14.06.2019
 */
#include "QFileDialog"
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <cstddef>
#include <exception>
#include <qdir.h>
#include <qfiledialog.h>
#include <qglobal.h>
#include <qpushbutton.h>
#include <qserialport.h>

#include "cli_widget.h"
#include "settings.h"
#include "terminal.h"
#include "ui_terminal.h"

Terminal::Terminal(QWidget *parent) : QWidget(parent), m_ui(new Ui::Terminal()) {
	m_ui->setupUi(this);
	m_serial = new QSerialPort(this);
	m_settings = new Settings(this);
	m_cli = new CliWidget(this);

	qDebug() << "Program is started";
	vPortSetup();

	connect(m_ui->btnConnect, &QPushButton::clicked, this, &Terminal::connectSerial);
	connect(m_ui->btnSave, &QPushButton::clicked, this, &Terminal::save);
	connect(m_ui->btnClear, &QPushButton::clicked, this, &Terminal::clear);
	connect(m_ui->btnConfig, &QPushButton::clicked, this, &Terminal::showSettings);

	m_ui->vblTerminal->addWidget(m_cli);
}

Terminal::~Terminal() {
	qDebug() << "Program is stopped";
	vPortClose();
	m_serial->deleteLater();
	delete m_ui;
}

bool Terminal::bPortOpen() {
	m_serial->setPortName((QString)(m_ui->cbPort->currentText()));

	if (m_serial->open(QIODevice::ReadWrite)) {
		m_serial->setBaudRate(static_cast<QSerialPort::BaudRate>(m_ui->cbBaudrate->itemData(m_ui->cbBaudrate->currentIndex()).toInt()));

		m_serial->setParity(QSerialPort::NoParity);
		m_serial->setDataBits(QSerialPort::Data8);
		m_serial->setStopBits(QSerialPort::TwoStop);
		m_serial->setFlowControl(QSerialPort::NoFlowControl);

		connect(m_serial, &QSerialPort::readyRead, this, &Terminal::readData);
		connect(m_cli, &CliWidget::eventData, this, &Terminal::writeData);

		qDebug() << "Port succesfully opened:" << m_serial->portName() << "on:" << m_ui->cbBaudrate->currentText();
		return true;
	}
	qCritical() << m_serial->errorString();

	return false;
}

void Terminal::vPortClose() {
	if (m_serial->isOpen()) {
		m_serial->clear();
		m_serial->close();
		disconnect(m_serial);

		qDebug() << "Port succesfully closed";
		return;
	} else {
		qDebug() << "Port is not open";
	}
}

void Terminal::vScanPorts() {
	qDebug() << "Searching for ports";

	if (m_ui->cbPort->count()) {
		m_ui->cbPort->clear();
	}

	for (QSerialPortInfo port : QSerialPortInfo::availablePorts())
		m_ui->cbPort->addItem(port.portName());

	const auto info = QSerialPortInfo::availablePorts();

	for (int i = 0; i < m_ui->cbPort->count(); i++) {
		if (info.at(i).description().contains("Bluetooth"))
			m_ui->cbPort->setItemIcon(i, QIcon("://icons/Alecive-Flatwoken-Apps-Bluetooth-Active.ico"));
		else if (info.at(i).manufacturer().contains("Стандартные порты"))
			m_ui->cbPort->setItemIcon(i, QIcon("://icons/Alecive-Flatwoken-Apps-Terminal.ico"));
		else
			m_ui->cbPort->setItemIcon(i, QIcon("://icons/Alecive-Flatwoken-Apps-Drive-Harddisk-Usb.ico"));
	}

	m_ui->cbPort->addItem("Refresh");
	m_ui->cbPort->setCurrentIndex(0);
}

void Terminal::vPortSetup() {
	vScanPorts();

	m_ui->cbBaudrate->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
	m_ui->cbBaudrate->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
	m_ui->cbBaudrate->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
	m_ui->cbBaudrate->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
	m_ui->cbBaudrate->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);

#if defined(DEBUG_MSG)
	QString description;
	QString manufacturer;
	QString serialNumber;
	const auto infos = QSerialPortInfo::availablePorts();
	for (const QSerialPortInfo &info : infos) {
		QStringList listPortInfo;
		description = info.description();
		manufacturer = info.manufacturer();
		serialNumber = info.serialNumber();
		qDebug() << "-------------------------------";
		qDebug() << "name - " << info.portName();
		qDebug() << "description - " << (!description.isEmpty() ? description : "");
		qDebug() << "manufact. - " << (!manufacturer.isEmpty() ? manufacturer : "");
		qDebug() << "number - " << (!serialNumber.isEmpty() ? serialNumber : "");
		qDebug() << "location - " << info.systemLocation();
		qDebug() << "vendor - " << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : "");
		qDebug() << "prod - " << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : "");
		qDebug() << "-------------------------------";
	}
#endif
}

void Terminal::readData() {
	// qDebug() << "Data on port";
	if (m_serial->isOpen()) {
		QByteArray data = m_serial->readAll();
		m_cli->processData(data);
	}
}

void Terminal::writeData(const char *data, size_t size) {
	if (m_serial->isOpen()) {
		m_serial->write(data, size);
	}
}

void Terminal::connectSerial() {
	if (bPortOpen()) {
		m_ui->btnConnect->setIcon(QIcon("://icons/cf_icon_usb2_white.svg"));
		// m_cli->setSerialPort(m_serial);
	} else {
		vPortClose();
		m_ui->btnConnect->setIcon(QIcon("://icons/cf_icon_usb1_white.svg"));
	}
}

void Terminal::save() {
	qDebug() << "Save to file";

	QDateTime curTime = QDateTime::currentDateTime();
	QString data = "LOG-";
	data.append(curTime.toString("yy.dd.mm-hh.mm.ss"));

	QString nomeFile = QFileDialog::getSaveFileName(this, tr("Save Log"), data, tr("File Name (*.txt);;C++ File (*.cpp *.h)"));
	if (nomeFile != "") {
		QFile file(nomeFile);

		if (file.open(QIODevice::ReadWrite)) {
			QTextStream stream(&file);
			stream << m_cli->toPlainText();

			file.flush();
			file.close();
		} else {
			QMessageBox::critical(this, tr("Error"), tr("I can't save the file"));
			return;
		}
	}
}

void Terminal::clear() {
	qDebug() << "Clear text";
	m_cli->clear();
}

void Terminal::showSettings() {
	m_settings->show();
}

void Terminal::on_cbPort_activated(int index) {
	if (index == m_ui->cbPort->count() - 1) {
		vScanPorts();
	}
}

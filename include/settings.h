/**
 * @file settings.h
 * @author v.mineev
 * @brief
 * @version 0.1
 * @date 2026-04-24
 */

#pragma once

#include <qdialog.h>
#include <qobjectdefs.h>
#include <qwidget.h>

namespace Ui {
class Settings;
}

class Settings : public QDialog {
	Q_OBJECT
  public:
	explicit Settings(QWidget *parent = nullptr);
	~Settings();

  private:
	Ui::Settings *m_ui;
};

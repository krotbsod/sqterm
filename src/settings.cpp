/**
 * @file settings.cpp
 * @author v.mineev
 * @brief 
 * @version 0.1
 * @date 2026-04-24
 */

#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) : QDialog(parent), m_ui(new Ui::Settings) {
    m_ui->setupUi(this);
}

Settings::~Settings() {
    delete m_ui;
}

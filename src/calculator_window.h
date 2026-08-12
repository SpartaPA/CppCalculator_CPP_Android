#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include "calculator_engine.h"

class CalculatorWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit CalculatorWindow(QWidget* parent=nullptr);
private:
    calc::CalculatorEngine engine;
    QLabel* expr;
    QLabel* result;
    QString text;
    int cursor=0;
    void click(const QString&);
    void render();
    void clearAll();
    QPushButton* makeButton(const QString&, const QString&);
};

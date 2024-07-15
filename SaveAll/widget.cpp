#include "widget.h"
#include "ui_widget.h"
#include <QSpinBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    SaveAll::load(this);
}

Widget::~Widget()
{
    SaveAll::save(this);
    delete ui;
}

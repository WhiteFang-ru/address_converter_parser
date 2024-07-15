Данный мини класс сохраняет в `QSettings` параметры объектов виде дерева (от родителя к детям...):  
Реализованы следующие виджеты/объекты
``` c++
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QToolButton>
#include <QSlider>
#include <QAction>
#include <QTimeEdit>
#include <QMainWindow>
//...
SAVE_ALL_REGISTER_WIDGET(QLineEdit, setText, text);
SAVE_ALL_REGISTER_WIDGET(QSpinBox, setValue, value);
SAVE_ALL_REGISTER_WIDGET(QDoubleSpinBox, setValue, value);
SAVE_ALL_REGISTER_WIDGET(QSlider, setValue, value);
SAVE_ALL_REGISTER_WIDGET(QCheckBox, setChecked, isChecked);
SAVE_ALL_REGISTER_WIDGET(QRadioButton, setChecked, isChecked);
SAVE_ALL_REGISTER_WIDGET(QComboBox, setCurrentIndex, currentIndex);
SAVE_ALL_REGISTER_WIDGET(QToolButton, setChecked, isChecked);
SAVE_ALL_REGISTER_WIDGET(QGroupBox, setChecked, isChecked);
SAVE_ALL_REGISTER_WIDGET(QGroupBox, setTitle, title);
SAVE_ALL_REGISTER_WIDGET(QAction, setChecked, isChecked);
SAVE_ALL_REGISTER_WIDGET(QMainWindow, restoreGeometry, saveGeometry);
SAVE_ALL_REGISTER_WIDGET_ARGS(QMainWindow, restoreState, saveState, 0);
```
Пример сохранения/загрузки виджетов/объектов:
``` c++
//метод сохранения
SaveAll::save(this); // this - главный объект того класса с которого нужно начать сохранение
//метот загрузки
SaveAll::load(this);  // this - главное объект того класса с которого нужно начать сохранение
//вторым параметром можно передать имя настроек, третим формат сохранения
//(QObject *obj, QString nameFile = defaultSettingsName(), QSettings::Format format = QSettings::Format::IniFormat)
```
Пример добавление нестандартного виджета:
``` c++
//регистрация нового виджета, нужно передать имя класса, и методы установки, получения
SAVE_ALL_REGISTER_WIDGET(QLineEdit, setText, text);
//для случаев, когда метод установить/получить имеет дополнительные поля
SAVE_ALL_REGISTER_WIDGET_ARGS(QMainWindow, restoreState, saveState, 0)
```
Пример исключения виджета:
``` c++
SAVE_ALL_UNREGISTER_WIDGET(QLineEdit)
```
Пример сохранения/загрузки переменных:
``` c++
// пример сохранения
int test = 5;
SAVE_ALL_SAVE(test);
// пример загрузки
SAVE_ALL_LOAD(test)
```
Для того, чтобы объект не сохранялся, objectName() должен содеражать "`SaveOff`" или "`_sof`".

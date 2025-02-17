#ifndef DATA_DIALOG_H
#define DATA_DIALOG_H

#include "dbc_parser/dbciterator.hpp"
#include "qabstractbutton.h"

#include <QBrush>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QString>
#include <QTreeWidget>
#include <QtCore>

#include <QColorDialog>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QApplication>
#include <QFont>
#include <QItemDelegate>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

namespace Ui {
class data_Dialog;
}

class data_Dialog : public QDialog {
  Q_OBJECT

  friend class Data;

public:
  explicit data_Dialog(QWidget *parent = nullptr);

  ~data_Dialog();

  void insertMessage(BusMessage &message);
  QTreeWidgetItem* insertSysVar(QString name, bool checked);
  void insertSerie(const QString &source, const QString &name, int count,
                   double factor = 1, double offset = 0);
  void hideEmptySeries();
  void showEmptySeries();
  void sortTreeItems(QTreeWidgetItem *parent = nullptr, int column = 0,
                     Qt::SortOrder order = Qt::AscendingOrder);
  Ui::data_Dialog *ui;

  QSettings *settings;
  QSettings *settingsTemplate;
  bool initOngoing = true;

  // QStringList axisTypes{"time", "engineering", "op. mode", "category"};

  QStringList axisUnits{"d", "A", "V", "°C", "%rh", "N", "n"};
  // QStringList axisPositions{"left", "right", "bottom", "top"};
  // QString::fromLatin1("°C").toUtf8()
  QMap<QString, Qt::AlignmentFlag> axisPositions = {{"left", Qt::AlignLeft},
                                                    {"right", Qt::AlignRight},
                                                    {"bottom", Qt::AlignBottom},
                                                    {"top", Qt::AlignTop}};

  QMap<QString, QString> axisTypes{{"time", "%.1Td"},
                                   {"engineering", "%.1R"},
                                   {"plain", "%d"},
                                   {"op. mode", "%0.0fOpM|"},
                                   {"category", "category"}};

  QList<QColor> chartColors = {
      QColor(255, 87, 51),   // Vivid Red-Orange
      QColor(51, 255, 87),   // Vivid Green
      QColor(51, 87, 255),   // Vivid Blue
      QColor(241, 196, 15),  // Vivid Yellow
      QColor(142, 68, 173),  // Vivid Purple
      QColor(52, 152, 219),  // Soft Blue
      QColor(46, 204, 113),  // Soft Green
      QColor(231, 76, 60),   // Soft Red
      QColor(255, 165, 0),   // Orange
      QColor(22, 160, 133),  // Dark Cyan
      QColor(39, 174, 96),   // Dark Green
      QColor(41, 128, 185),  // Blue
      QColor(142, 68, 173),  // Purple
      QColor(243, 156, 18),  // Orange
      QColor(211, 84, 0),    // Pumpkin Orange
      QColor(192, 57, 43),   // Dark Red
      QColor(255, 192, 203), // Pink
      QColor(255, 105, 180), // Hot Pink
      QColor(241, 148, 138), // Soft Pink
      QColor(187, 143, 206), // Soft Purple
      QColor(125, 206, 160), // Soft Green
      QColor(247, 220, 111), // Light Yellow
      QColor(69, 179, 157),  // Sea Green
      QColor(65, 105, 225),  // Royal Blue
      QColor(215, 189, 226), // Light Purple
      QColor(210, 77, 87),   // Strong Red
      QColor(125, 60, 152),  // Deep Purple
      QColor(82, 190, 128),  // Medium Green
      QColor(255, 127, 80),  // Coral
      QColor(255, 215, 0),   // Gold
      QColor(30, 144, 255),  // Dodger Blue
      QColor(123, 104, 238), // Medium Slate Blue
      QColor(0, 206, 209),   // Dark Turquoise
      QColor(255, 99, 71),   // Tomato
      QColor(144, 238, 144)  // Light Green
  };

protected:
  //    bool eventFilter(QObject *watched, QEvent *event) override;

private:
  QTreeWidgetItem *addTreeChild(QTreeWidgetItem *parent, QString name,
                                QString description);
  QTreeWidgetItem *addMiddleNode(QTreeWidgetItem *parent, QString name);

  QTreeWidgetItem *getRootItem(QString name);
  // int getNestingLevel(QTreeWidgetItem *item);
  void updateChildren(QTreeWidgetItem *item, Qt::CheckState state);
  Qt::CheckState aggregateChildStates(QTreeWidgetItem *parent);
  QStringList splitSysvarName(const QString &str);
  // QTreeWidgetItem *getItemByName(QTreeWidgetItem *rootItem, const QString
  // &name);
  QStringList getTreeItems(QTreeWidgetItem *treeItem);
  // void saveTreeState(QTreeWidget *tree, const QString &key);
  // void loadTreeState(QTreeWidget *tree, const QString &key);
  void saveAxes(QSettings *setting);
  void loadAxes(QSettings* settings = nullptr);
  void loadSeriesTable(QSettings* settings = nullptr);
  void saveSeriesTable(QSettings *setting);
  void addAddRowButton(int row);
  void insertAxis(QString name, QString type, QString unit, QColor color,
                  QString position);
  int tableContains(QTableWidget *table, int column, const QString &text);

  void setItemPropertiesBasedOnColor(QTableWidgetItem *item, QColor color);
  int longestCommonSubstring(const QString &s1, const QString &s2);
  bool serieSelectionClicked = false;

  // Helper functions for insertSerie
  QTableWidgetItem* getOrCreateItem(int row, int col);
  QTableWidgetItem* createTableItem(const QString& value, bool editable = false);
  QString findMatchingAxis(const QString& name);

private slots:
  void treeWidgetItemChanged(QTreeWidgetItem *item, int column);
  void resizeTreeWidgetColumns();

  void dropEvent(QDropEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;

  void buttonPressed(QAbstractButton *button);
  void handleAddRowButtonClicked();
  void deleteAxis(QPushButton *btn);
  void onAxisDoubleClicked(QTableWidgetItem *item);
  void onSerieDoubleClicked(QTableWidgetItem *item);
  void filterDataTree(QString term);

signals:

  void filesDropped(QList<QFileInfo> fileList);
  void saveClick();
  void loadClick();
  void addRowButtonClicked();

  void signalCheckStateUpdated(qint32 msgId, QString signalName,
                               bool boolCheckState);
  void sysVarCheckStateUpdated(QString varName, bool boolCheckState);
  void cosmeticReset();
  void dataReset();
  // void axisChanged(QString axisName, QString axisType, QString axisUnit,
  // QColor color, QString axisPosition);


    void saveSettings();
    void reloadDataFile();
};

class CustomSortProxyModel : public QSortFilterProxyModel {
protected:
  bool lessThan(const QModelIndex &left,
                const QModelIndex &right) const override {
    // Your custom sorting logic here. For example:
    QString leftData = sourceModel()->data(left).toString();
    QString rightData = sourceModel()->data(right).toString();

    int leftIndex;
    int rightIndex;

    if ((!leftData.contains("[")) || (!rightData.contains("[")))
      return leftData < rightData;

    int startPos = leftData.lastIndexOf('[') + 1;
    int endPos = leftData.lastIndexOf(']');
    leftIndex = leftData.mid(startPos, endPos - startPos).toInt();

    startPos = leftData.lastIndexOf('[') + 1;
    endPos = leftData.lastIndexOf(']');
    rightIndex = leftData.mid(startPos, endPos - startPos).toInt();

    // qDebug() << "Sort:" << (leftIndex < rightIndex);
    return leftIndex < rightIndex;
  }
};

class NoSelectionColorDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  NoSelectionColorDelegate(QObject *parent = nullptr)
      : QStyledItemDelegate(parent) {}

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    QStyleOptionViewItem modifiedOption(option);

    // qDebug() << " void paint(QPainter *painter, const QStyleOptionViewItem
    // &option, const QModelIndex &index) "
    //             "const override";

    if (option.state & QStyle::State_Selected) {
      modifiedOption.state ^= QStyle::State_Selected;
    }

    QStyledItemDelegate::paint(painter, modifiedOption, index);
  }
};

class ComboBoxItemDelegate : public QItemDelegate {
public:
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    QStyleOptionViewItem opt = option;

    QFont font = QFont("Arial", 16, QFont::Bold);
    // QFont font               = opt.font;
    // font.setBold(true);
    // font.setPointSize(font.pointSize() + 2);
    opt.font = font;
    opt.fontMetrics = QFontMetrics(font);

    QItemDelegate::paint(painter, opt, index);
  }

  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override {
    QSize originalSize = QItemDelegate::sizeHint(option, index);

    // Increase the height by 10 pixels; keep the original width.
    return QSize(originalSize.width(), originalSize.height() + 10);
  }
};

class CustomComboBox : public QComboBox {
public:
  explicit CustomComboBox(QWidget *parent = nullptr) : QComboBox(parent) {}

  void showPopup() override {
    QComboBox::showPopup();

    // Get the popup view and calculate the width needed for all items
    QAbstractItemView *view = this->view();
    int maxWidth = 0;
    for (int i = 0; i < count(); ++i) {
      int itemWidth = view->sizeHintForIndex(model()->index(i, 0)).width();
      maxWidth = qMax(maxWidth, itemWidth);
    }

    // Set the popup width
    view->setMinimumWidth(maxWidth);
  }
};

class CheckBoxDelegate : public QStyledItemDelegate {
public:
  CheckBoxDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

protected:
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

    QStyleOptionButton checkboxOption;
    QRect checkboxRect = QApplication::style()->subElementRect(
        QStyle::SE_CheckBoxIndicator, &checkboxOption);
    checkboxOption.rect = option.rect;
    // checkboxOption.rect.setLeft(option.rect.x() + option.rect.width() / 2 -
    //                             checkboxRect.width() / 2); // Center checkbox
    checkboxOption.state = QStyle::State_Enabled |
                           (checked ? QStyle::State_On : QStyle::State_Off);

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxOption,
                                       painter);
  }
};

#endif // DATA_DIALOG_H

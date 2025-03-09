#include "data_dialog.h"
#include "qevent.h"
#include "qmenu.h"
#include "ui_data_dialog.h"
#include <QRegularExpression>
#include <QSet>

#include <QStandardItem>
#include <QStandardItemModel>

data_Dialog::data_Dialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::data_Dialog)
{

  setAcceptDrops(true);

  ui->setupUi(this);
  ui->treeWidget->setColumnCount(2);
  ui->axesTable->verticalHeader()->hide();
  ui->seriesTable->verticalHeader()->hide();

  this->settings = nullptr;
  this->settingsTemplate = nullptr;

  ui->buttonBox->button(QDialogButtonBox::Save)->setText("Save settings");
  ui->buttonBox->button(QDialogButtonBox::Retry)->setText("Reload data file");

  ui->treeWidget->setHeaderLabels(QStringList() << "Name" << "Data Type");

  // Set column widths
  ui->treeWidget->setColumnWidth(0, 250);
  ui->treeWidget->setColumnWidth(1, 150);

  connect(ui->treeWidget, &QTreeWidget::itemChanged, this,
          &data_Dialog::treeWidgetItemChanged);
  connect(ui->treeWidget, &QTreeWidget::itemExpanded, this,
          &data_Dialog::resizeTreeWidgetColumns);
  connect(ui->treeWidget, &QTreeWidget::itemCollapsed, this,
          &data_Dialog::resizeTreeWidgetColumns);
  connect(ui->buttonBox, &QDialogButtonBox::clicked, this,
          &data_Dialog::buttonPressed);

  void treeWidgetItemCollapsed(QTreeWidgetItem * item);
  // void treeWidgetItemExpanded(QTreeWidgetItem * item);

  ui->treeWidget->setMidLineWidth(3);
  ui->progressBar->hide();

  // set treeWidget stylesheet
  {
    // Debug resource loading
    QFile file(":/images/png/stylesheet-vline.png");
    if (file.exists()) {
      qDebug() << "Resource file found: :/images/png/stylesheet-vline.png";
    } else {
      qDebug() << "ERROR: Resource file not found: :/images/png/stylesheet-vline.png";
      // List all available resources
      qDebug() << "Available resources:";
      QDirIterator it(":", QDirIterator::Subdirectories);
      while (it.hasNext()) {
        qDebug() << it.next();
      }
    }
    
    ui->treeWidget->setStyleSheet(
        "QTreeView::branch:has-siblings:!adjoins-item {"
        "    border-image: url(:/images/png/stylesheet-vline.png) 0;"
        "}"
        "QTreeView::branch:has-siblings:adjoins-item {"
        "    border-image: url(:/images/png/stylesheet-branch-more.png) 0;"
        "}"
        "QTreeView::branch:!has-children:!has-siblings:adjoins-item {"
        "    border-image: url(:/images/png/stylesheet-branch-end.png) 0;"
        "}"
        "QTreeView::branch:has-children:!has-siblings:closed,"
        "    QTreeView::branch:closed:has-children:has-siblings {"
        "    border-image: none;"
        "    image: url(:/images/png/stylesheet-branch-closed.png);"
        "}"
        "QTreeView::branch:open:has-children:!has-siblings,"
        "    QTreeView::branch:open:has-children:has-siblings  {"
        "    border-image: none;"
        "    image: url(:/images/png/stylesheet-branch-open.png);"
        "}");
  }

  addAddRowButton(0);

  // Set header color
  ui->axesTable->horizontalHeader()->setStyleSheet(
      "QHeaderView::section{ background-color: rgb(0, 150, 150);  color: "
      "rgb(255, 255, 255); font-weight: bold;}");

  ui->seriesTable->horizontalHeader()->setStyleSheet(
      "QHeaderView::section{ background-color: rgb(0, 150, 150);  color: "
      "rgb(255, 255, 255); font-weight: bold; }");
  qDebug() << ui->seriesTable->horizontalHeader()->styleSheet();

  ui->axesTable->setItemDelegateForColumn(3,
                                          new NoSelectionColorDelegate(this));

  insertAxis("Duration", "time", "s", Qt::black, "bottom");

  connect(this, SIGNAL(addRowButtonClicked()), this,
          SLOT(handleAddRowButtonClicked()));
  connect(ui->axesTable, &QTableWidget::itemDoubleClicked, this,
          &data_Dialog::onAxisDoubleClicked);
  connect(ui->seriesTable, &QTableWidget::itemDoubleClicked, this,
          &data_Dialog::onSerieDoubleClicked);
  //    connect(qApp, &QApplication::focusChanged, this,
  //    &data_Dialog::handleFocusChanged);

  connect(ui->axesTable, &QTableWidget::itemChanged, this,
          [this](QTableWidgetItem *item)
          {
            ui->axesTable->resizeColumnToContents(item->column());
            // emit cosmeticReset();
          });

  connect(
      ui->seriesTable, &QTableWidget::itemChanged, this,
      [this](QTableWidgetItem *item)
      {
        if (item->column() == 0)
        {
          if (item->checkState() == Qt::Checked)
          {
            // qDebug() << "checked"
            //          << ui->seriesTable->item(item->row(), 2)->text();
            item->setBackground(Qt::green);
          }
          else
          {
            item->setBackground(Qt::white);
          }
        }
        else if (item->column() == 5) // symbol, 6 color
        {
          if ((ui->seriesTable->item(item->row(), 6)) &&
              (ui->seriesTable->item(item->row(), 6)
                   ->background()
                   .color()
                   .alpha() != 255))
          {
            item->setForeground(
                QBrush(ui->seriesTable->item(item->row(), 6)->background()));
            // qDebug() << "din changeevent" <<
            // QBrush(ui->seriesTable->item(item->row(), 6)->background());
          }
        }
        else if (item->column() == 6) // color, 5 symbol
        {
          if ((!item->text().contains("white")) &&
              (!item->text().contains("#ffffff")))
          {
            ui->seriesTable->item(item->row(), 5)
                ->setForeground(QBrush(item->background()));
          }
        }
        // if initOngoing is false and the item column is between 3 and 6 emit
        // cosmeticReset()
        //  if (!initOngoing && item->column() >= 3 && item->column() <= 6)
        //  {
        //    emit cosmeticReset();
        //  }
      },
      Qt::QueuedConnection);

  connect(ui->seriesTable, &QTableWidget::itemPressed, this,
          [](QTableWidgetItem *item)
          {
            if (item->column() == 0)
            {
              item->setCheckState(item->checkState() == Qt::Checked
                                      ? Qt::Unchecked
                                      : Qt::Checked);
            }
          });

  connect(
      ui->seriesTable, &QTableWidget::customContextMenuRequested, this,
      [this](const QPoint &pos)
      {
        QMenu menu;
        QAction *checkAction = menu.addAction("Check");
        QAction *uncheckAction = menu.addAction("Uncheck");
        QAction *toggleAction = menu.addAction("Toggle check state");
        QAction *selectAxis = menu.addAction("Set Y axis");
        QAction *deleteSelectedSeries = menu.addAction("Delete selection");
        QAction *showEmpty = menu.addAction("Show empty series");

        connect(deleteSelectedSeries, &QAction::triggered, this, [&, this]()
                {
          for (auto item : ui->seriesTable->selectedItems()) {
            ui->seriesTable->removeRow(item->row());
          } });

        connect(selectAxis, &QAction::triggered, this, [&, this]()
                {
          auto comboBox = new CustomComboBox(ui->seriesTable);

          QMetaObject::Connection activatedConnection;

          auto axesTable = this->ui->axesTable;
          QStringList axesList;
          int rowCount = axesTable->rowCount();
          for (int i = 0; i < rowCount - 1; ++i) // Exclude the last row
          {
            QTableWidgetItem *item = axesTable->item(i, 0);
            if (item && !item->text().isEmpty()) {
              if (axesTable->item(i, 1) &&
                  axesTable->item(i, 1)->text() == "time")
                continue;
              axesList.append(item->text());
            }
          }
          comboBox->addItems(axesList);

          comboBox->installEventFilter(this);

          comboBox->setGeometry(pos.x(), pos.y(), comboBox->width(),
                                comboBox->height());

          comboBox->show();
          comboBox->showPopup();

          activatedConnection =
              connect(comboBox, QOverload<int>::of(&QComboBox::activated), this,
                      [=, this]() {   // int index
                        if (comboBox) // Check if comboBox is still valid
                        {

                          for (auto item : ui->seriesTable->selectedItems()) {
                            if (item->column() != 4)
                              continue;
                            qDebug() << "action set axis";
                            item->setText(comboBox->currentText());
                            item->setTextAlignment(Qt::AlignCenter);
                          }
                          ui->seriesTable->resizeColumnsToContents();
                          comboBox->deleteLater();
                        }
                      }); });

        connect(checkAction, &QAction::triggered, this, [this]()
                {
          for (auto item : ui->seriesTable->selectedItems()) {
            if (item->column() > 1)
              continue;
            ui->seriesTable->item(item->row(), 0)->setCheckState(Qt::Checked);
          } });

        connect(uncheckAction, &QAction::triggered, this, [this]()
                {
          for (auto item : ui->seriesTable->selectedItems()) {
            if (item->column() > 1)
              continue;
            ui->seriesTable->item(item->row(), 0)->setCheckState(Qt::Unchecked);
          } });

        connect(toggleAction, &QAction::triggered, this, [this]()
                {
          for (auto item : ui->seriesTable->selectedItems()) {
            if (item->column() > 1)
              continue;
            ui->seriesTable->item(item->row(), 0)
                ->setCheckState(
                    (ui->seriesTable->item(item->row(), 0)->checkState() ==
                     Qt::Checked)
                        ? Qt::Unchecked
                        : Qt::Checked);
          } });
        connect(showEmpty, &QAction::triggered, this,
                [this]()
                { showEmptySeries(); });

        menu.exec(ui->seriesTable->viewport()->mapToGlobal(pos));
        return;
      });

  // CheckBoxDelegate *delegate = new CheckBoxDelegate(ui->seriesTable);
  // ui->seriesTable->setItemDelegateForColumn(0, delegate);
  ui->seriesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

  ui->seriesTable->setSortingEnabled(true);

  connect(ui->dataFilter, &QLineEdit::textEdited, this,
          &data_Dialog::filterDataTree);
}

data_Dialog::~data_Dialog() {
    if (settings) {
        settings->sync();
        delete settings;
    }
    if (settingsTemplate) {
        settingsTemplate->sync();
        delete settingsTemplate;
    }
    delete ui;
}

void data_Dialog::sortTreeItems(QTreeWidgetItem *parent, int column,
                                Qt::SortOrder order)
{
  QCollator collator;
  collator.setNumericMode(true);

  // Function to sort children of a given parent item
  std::function<void(QTreeWidgetItem *)> sortChildren;
  sortChildren = [&](QTreeWidgetItem *parentItem)
  {
    std::vector<QTreeWidgetItem *> children;
    const int childCount = parentItem ? parentItem->childCount()
                                      : ui->treeWidget->topLevelItemCount();

    // Collect children
    for (int i = 0; i < childCount; ++i)
    {
      children.push_back(parentItem ? parentItem->child(i)
                                    : ui->treeWidget->topLevelItem(i));
    }

    // Sort using QCollator
    std::sort(
        children.begin(), children.end(),
        [&](QTreeWidgetItem *a, QTreeWidgetItem *b)
        {
          return order == Qt::AscendingOrder
                     ? collator.compare(a->text(column), b->text(column)) < 0
                     : collator.compare(a->text(column), b->text(column)) > 0;
        });

    // Reinsert items in sorted order
    for (auto *child : children)
    {
      if (parentItem)
      {
        parentItem->removeChild(child);
        parentItem->addChild(child);
      }
      else
      {
        ui->treeWidget->takeTopLevelItem(
            ui->treeWidget->indexOfTopLevelItem(child));
        ui->treeWidget->addTopLevelItem(child);
      }
      // Sort grandchildren
      sortChildren(child);
    }
  };

  if (parent)
  {
    sortChildren(parent);
  }
  else
  {
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i)
    {
      sortChildren(ui->treeWidget->topLevelItem(i));
    }
  }
}

void data_Dialog::filterDataTree(QString term)
{
  // qDebug() << "filter term:" << term;

  // Container to track items that match the search criteria.
  QSet<QTreeWidgetItem *> matchedItems;

  // Function to recursively search for matching items and populate
  // matchedItems.
  std::function<bool(QTreeWidgetItem *)> findMatches =
      [&](QTreeWidgetItem *item) -> bool
  {
    bool matchFound = item->text(0).contains(term, Qt::CaseInsensitive);
    for (int i = 0; i < item->childCount(); ++i)
    {
      matchFound = findMatches(item->child(i)) || matchFound;
    }
    if (matchFound)
    {
      // Track the item and all its ancestors.
      QTreeWidgetItem *parent = item;
      while (parent != nullptr)
      {
        matchedItems.insert(parent);
        parent = parent->parent();
      }
    }
    return matchFound;
  };

  // Function to apply visibility based on matchedItems.
  std::function<void(QTreeWidgetItem *)> applyVisibility =
      [&](QTreeWidgetItem *item)
  {
    if (!matchedItems.contains(item))
    {
      if (item->parent() != nullptr)
      {
        if (!matchedItems.contains(item->parent()))
          item->setHidden(true);
        if (!item->parent()->text(0).contains(term, Qt::CaseInsensitive))
          item->setHidden(true);
      }
      else
      {
        item->setHidden(true);
      }
    }

    // Expand if item is in matchedItems and has children.
    if (matchedItems.contains(item) && item->childCount() > 0)
    {
      // set all children to visible
      for (int i = 0; i < item->childCount(); ++i)
      {
        item->child(i)->setHidden(false);
      }
      item->setExpanded(true);
    }

    for (int i = 0; i < item->childCount(); ++i)
    {
      applyVisibility(item->child(i));
    }
  };

  if (term.isEmpty())
  {
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it)
    {
      QTreeWidgetItem *item = *it;
      item->setHidden(false);
      if (item->checkState(0) != Qt::Checked)
      {
        item->setExpanded(false);
      }
      else
      {
        item->setExpanded(true);
        QTreeWidgetItem *parent = item->parent();
        while (parent)
        {
          parent->setExpanded(true);
          parent = parent->parent();
        }
      }
      ++it;
    }
    return;
  }

  // Step 1: Find all matches.
  QTreeWidgetItemIterator it(ui->treeWidget);
  while (*it)
  {
    findMatches(*it);
    ++it;
  }

  // Step 2: Apply visibility based on the matches found.
  it = QTreeWidgetItemIterator(ui->treeWidget); // Reset the iterator.
  while (*it)
  {
    applyVisibility(*it);
    ++it;
  }
}

int data_Dialog::tableContains(QTableWidget *table, int column,
                               const QString &text)
{
  for (int row = 0; row < table->rowCount(); ++row)
  {
    QTableWidgetItem *item = table->item(row, column);
    if (item && item->text() == text)
    {
      return row;
    }
  }
  return -1; // Text not found
}

void data_Dialog::hideEmptySeries()
{
  // hide all rows in ui->seriesTable that have the column 2 empty
  for (int i = 0; i < ui->seriesTable->rowCount(); i++)
  {
    if (ui->seriesTable->item(i, 2) && ui->seriesTable->item(i, 2)->text().isEmpty())
    {
      ui->seriesTable->hideRow(i);
    }
  }
}

void data_Dialog::showEmptySeries()
{
  // hide all rows in ui->seriesTable that have the column 2 empty
  for (int i = 0; i < ui->seriesTable->rowCount(); i++)
  {
    ui->seriesTable->showRow(i);
  }
}

void data_Dialog::setItemPropertiesBasedOnColor(QTableWidgetItem *item,
                                                QColor color)
{
  if (color.isValid())
  {
    item->setBackground(QBrush(color));
    if (color.alpha() != 255)
    {
      item->setForeground(QBrush(color));
    }

    qreal luminance =
        0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue();

    if (luminance < 128)
    {
      item->setForeground(QBrush(Qt::white));
    }
    else
    {
      item->setForeground(QBrush(Qt::black));
    }

    item->setText(color.name());
    item->setTextAlignment(Qt::AlignCenter);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);

    for (const auto &colorName : QColor::colorNames())
    {
      QColor tempColor(colorName);
      if (tempColor == color)
      {
        item->setText(colorName);
        break;
      }
    }
  }
}

void data_Dialog::insertAxis(QString name, QString type, QString unit,
                             QColor color, QString position)
{
  int lastRow = ui->axesTable->rowCount() - 1;
  // Remove the last row (which contains the button)
  if (lastRow >= 0)
  {
    ui->axesTable->removeRow(lastRow);
  } /*else {
    lastRow = 0;
  }*/

  // Insert new row with default columns
  int newRow = ui->axesTable->rowCount();
  // qDebug() << "int newRow = ui->axesTable->rowCount();" <<
  // ui->axesTable->rowCount();
  ui->axesTable->insertRow(newRow);

  QTableWidgetItem *newItem = new QTableWidgetItem(name);
  ui->axesTable->setItem(newRow, 0, newItem);
  newItem->setTextAlignment(Qt::AlignCenter);

  newItem = new QTableWidgetItem(type);
  ui->axesTable->setItem(newRow, 1, newItem);
  newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
  newItem->setTextAlignment(Qt::AlignCenter);

  newItem = new QTableWidgetItem(unit);
  ui->axesTable->setItem(newRow, 2, newItem);
  newItem->setTextAlignment(Qt::AlignCenter);

  newItem = new QTableWidgetItem(color.name());
  ui->axesTable->setItem(newRow, 3, newItem);

  setItemPropertiesBasedOnColor(newItem, color);

  newItem = new QTableWidgetItem(position);
  ui->axesTable->setItem(newRow, 4, newItem);
  newItem->setTextAlignment(Qt::AlignCenter);
  newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);

  // Add delete button to last column, if the current row is not the X axis
  // (time)
  if (newRow > 0)
  {
    QPushButton *deleteBtn = new QPushButton("Delete", this);
    connect(deleteBtn, &QPushButton::clicked, this,
            [this, deleteBtn]()
            { deleteAxis(deleteBtn); });
    ui->axesTable->setCellWidget(newRow, ui->axesTable->columnCount() - 1,
                                 deleteBtn);
  }

  // Move button to the last row
  addAddRowButton(ui->axesTable->rowCount());
  QApplication::processEvents();

  QList<int> sizes = ui->v_splitter->sizes();
  // qDebug() << "sizes" << sizes << ui->axesTable->height();

  for (int i = 0; i < ui->axesTable->columnCount(); ++i)
  {
    ui->axesTable->resizeColumnToContents(i);
  }

  if (sizes[0] < 50)
    return;

  sizes[0] = ui->axesTable->height(); // Assuming the QTableWidget is the first
                                      // widget managed by the
  ui->v_splitter->setSizes(sizes);
}

void data_Dialog::onSerieDoubleClicked(QTableWidgetItem *item)
{
  int column = item->column();
  // int row = item->row();
  QString headerText = ui->seriesTable->horizontalHeaderItem(column)->text();
  qDebug() << "onSerieDoubleClicked" << item->column() << headerText;

  if (headerText == "Axis" || headerText == "Symbol")
  {
    QRect rect = ui->seriesTable->visualItemRect(item);
    int headerHeight = ui->seriesTable->horizontalHeader()->height();
    rect.translate(0, headerHeight); // Translate down by the header's height
    QPoint topLeft = ui->seriesTable->mapToGlobal(
        rect.topLeft()); // Convert to global coordinates
    topLeft = ui->seriesTable->mapFromGlobal(
        topLeft); // Convert back to widget coordinates
    auto comboBox = new CustomComboBox(ui->seriesTable);

    QMetaObject::Connection activatedConnection;

    if (headerText == "Axis")
    {
      auto axesTable = this->ui->axesTable;
      QStringList axesList;
      int rowCount = axesTable->rowCount();
      for (int i = 0; i < rowCount - 1; ++i) // Exclude the last row
      {
        QTableWidgetItem *item = axesTable->item(i, 0);
        if (item && !item->text().isEmpty())
        {
          if (axesTable->item(i, 1) && axesTable->item(i, 1)->text() == "time")
            continue;
          axesList.append(item->text());
        }
      }
      comboBox->addItems(axesList);

      comboBox->installEventFilter(this);
      rect.setWidth(rect.width() + 20);
      comboBox->setGeometry(
          QRect(topLeft, rect.size())); // Set position and size

      comboBox->show();
      comboBox->showPopup();

      auto activatedConnection =
          connect(comboBox, QOverload<int>::of(&QComboBox::activated), this,
                  [=, this]()
                  {
                    if (comboBox) // Check if comboBox is still valid
                    {
                      item->setText(comboBox->currentText());
                      item->setTextAlignment(Qt::AlignCenter);
                      ui->seriesTable->resizeColumnsToContents();
                      comboBox->deleteLater();
                    }
                  });
    }
    else if (headerText == "Symbol")
    {
      QStringList items;
      items << "•"
            << "─"
            << "×"
            << "#"
            << "§"
            << "*"
            << "☼"
            << "↑"
            << "↓"
            << "↨"
            << "¤"
            << "Θ"
            << "√"
            << "♦"
            << "■"
            << "▲"
            << "▼";

      comboBox->setItemDelegate(new ComboBoxItemDelegate());
      comboBox->addItems(items);

      comboBox->addItems(items);
      comboBox->installEventFilter(this);
      comboBox->setGeometry(
          QRect(topLeft, rect.size())); // Set position and size
      comboBox->show();
      comboBox->showPopup();

      auto activatedConnection =
          connect(comboBox, QOverload<int>::of(&QComboBox::activated), this,
                  [=, this]()
                  {
                    if (comboBox) // Check if comboBox is still valid
                    {
                      item->setText(comboBox->currentText());
                      item->setTextAlignment(Qt::AlignCenter);
                      QFont font = QFont("Arial", 16, QFont::Bold);

                      // font.setBold(true);
                      // font.setPointSize(font.pointSize() + 2);
                      item->setFont(font);
                      ui->seriesTable->resizeColumnsToContents();
                      comboBox->deleteLater();
                    }
                  });
    }

    // Listen for when another cell is selected
    auto itemChangedConnection = connect(
        ui->seriesTable, &QTableWidget::currentItemChanged, this,
        [=](QTableWidgetItem *current, QTableWidgetItem *previous)
        {
          Q_UNUSED(previous);
          if (current != item &&
              comboBox) // Another cell is clicked and comboBox is valid
          {
            disconnect(activatedConnection); // Disconnect activated signal
            comboBox->deleteLater();         // Safely delete the comboBox
          }
        });

    // Make sure to disconnect the itemChanged signal once the comboBox is
    // destroyed
    connect(comboBox, &QObject::destroyed,
            [=]()
            { disconnect(itemChangedConnection); });
  }
  else if (headerText ==
           "Color") //&& row != (ui->seriesTable->rowCount() - 1)
  {
    QColor currentColor = item->background().color();
    int h, s, v, a;
    currentColor.getHsv(&h, &s, &v, &a);          // Extract the HSV values
    currentColor = QColor::fromHsv(h, s, 128, a); // Set V to 128

    QColor newColor =
        QColorDialog::getColor(currentColor, this, "Choose a color");

    setItemPropertiesBasedOnColor(item, newColor);
  }
}

void data_Dialog::onAxisDoubleClicked(QTableWidgetItem *item)
{
  int column = item->column();
  int row = item->row();
  QString headerText = ui->axesTable->horizontalHeaderItem(column)->text();

  if (headerText == "Color" && row != (ui->axesTable->rowCount() - 1))
  {
    QColor currentColor = item->background().color();
    int h, s, v, a;
    currentColor.getHsv(&h, &s, &v, &a);          // Extract the HSV values
    currentColor = QColor::fromHsv(h, s, 128, a); // Set V to 128

    QColor newColor =
        QColorDialog::getColor(currentColor, this, "Choose a color");

    setItemPropertiesBasedOnColor(item, newColor);
  }
  else if (column > 0 && column < ui->axesTable->columnCount() - 1 &&
           row !=
               (ui->axesTable->rowCount() -
                1)) // Assuming the column index for the text drop-down is 2
  {
    QRect rect = ui->axesTable->visualItemRect(item);
    int headerHeight = ui->axesTable->horizontalHeader()->height();
    rect.translate(0, headerHeight); // Translate down by the header's height
    QPoint topLeft = ui->axesTable->mapToGlobal(
        rect.topLeft()); // Convert to global coordinates
    topLeft = ui->axesTable->mapFromGlobal(
        topLeft); // Convert back to widget coordinates
    auto comboBox = new CustomComboBox(ui->axesTable);

    if (headerText == "Unit")
      comboBox->addItems(axisUnits);
    else if (headerText == "Type")
      comboBox->addItems(axisTypes.keys());
    else if (headerText == "Position")
      comboBox->addItems(axisPositions.keys());

    comboBox->installEventFilter(this);
    comboBox->setGeometry(QRect(topLeft, rect.size())); // Set position and size
    comboBox->show();
    comboBox->showPopup();

    auto activatedConnection = connect(
        comboBox, QOverload<int>::of(&QComboBox::activated), this, [=, this]()
        {
          if (comboBox) // Check if comboBox is still valid
          {
            item->setText(comboBox->currentText());
            item->setTextAlignment(Qt::AlignCenter);
            ui->axesTable->resizeColumnsToContents();
            comboBox->deleteLater();
          } });

    // Listen for when another cell is selected
    auto itemChangedConnection = connect(
        ui->axesTable, &QTableWidget::currentItemChanged, this,
        [=](QTableWidgetItem *current, QTableWidgetItem *previous)
        {
          Q_UNUSED(previous);
          if (current != item &&
              comboBox) // Another cell is clicked and comboBox is valid
          {
            disconnect(activatedConnection); // Disconnect activated signal
            comboBox->deleteLater();         // Safely delete the comboBox
          }
        });

    // Make sure to disconnect the itemChanged signal once the comboBox is
    // destroyed
    connect(comboBox, &QObject::destroyed,
            [=]()
            { disconnect(itemChangedConnection); });
  }
}

void data_Dialog::addAddRowButton(int row)
{
  row = ui->axesTable->rowCount();
  ui->axesTable->insertRow(row);

  QPushButton *btn = new QPushButton("New Axis", this);
  connect(btn, &QPushButton::clicked, this,
          [this]()
          { emit addRowButtonClicked(); });
  ui->axesTable->setCellWidget(row, 0, btn);

  QList<int> sizes;
  int sum;

  int headerHeight = ui->seriesTable->horizontalHeader()->height();

  int totalHeight = headerHeight + 10;
  for (int i = 0; i < ui->axesTable->rowCount(); ++i)
  {
    totalHeight += ui->axesTable->rowHeight(i);
  }

  sizes = ui->v_splitter->sizes();
  sum = sizes[0] + sizes[1];
  if (sizes[0] < 50)
    return;

  sizes[0] = totalHeight;
  sizes[1] = sum - totalHeight;
  ui->v_splitter->setSizes(sizes);
  ui->v_splitter->update();
}

void data_Dialog::handleAddRowButtonClicked()
{
  insertAxis("NewAxis" + QString::number(ui->axesTable->rowCount()), "plain",
             "unit", Qt::black, "right");
  return;
}

void data_Dialog::deleteAxis(QPushButton *btn)
{
  // qDebug() << "int data_Dialog::deleteAxis(QPushButton *btn)";
  for (int i = 1; i < ui->axesTable->rowCount(); ++i)
  {
    if (ui->axesTable->cellWidget(i, ui->axesTable->columnCount() - 1) == btn)
    {
      ui->axesTable->removeRow(i);
    }
  }
  for (int i = 0; i < ui->axesTable->columnCount(); ++i)
  {
    ui->axesTable->resizeColumnToContents(i);
  }
}

QString stringBetween(const QString &source, const QString &start,
                      const QString &end)
{
  int startIndex = source.indexOf(start);
  if (startIndex == -1)
  {
    return QString();
  }
  startIndex += start.length(); // move to the end of the 'start' substring

  int endIndex = source.indexOf(end, startIndex);
  if (endIndex == -1)
  {
    return QString();
  }

  return source.mid(startIndex, endIndex - startIndex);
}

void data_Dialog::dropEvent(QDropEvent *event)
{

  QList<QFileInfo> dropFileList;

  for (auto url : event->mimeData()->urls())
  {
    QString filePath;
    if (!url.toString().contains("///"))
    {
      filePath = url.toString().remove(0, 5);
    }
    else
    {
      filePath = url.path(QUrl::FullyDecoded).remove(0, 1);
    }

    QFileInfo fi(filePath);
    if (fi.exists() and fi.size() > 0 and
        (fi.suffix().contains("csv", Qt::CaseInsensitive) or
         fi.suffix().contains("zs2", Qt::CaseInsensitive) or
         fi.suffix().contains("txt", Qt::CaseInsensitive) or
         fi.suffix().contains("dbc", Qt::CaseInsensitive) or
         fi.suffix().contains("blf", Qt::CaseInsensitive) or
         fi.suffix().contains("ini", Qt::CaseInsensitive) or
         fi.suffix().contains("ldf", Qt::CaseInsensitive)))
    {

      qDebug() << "Valid File dropped:" << fi.baseName();

      dropFileList << fi;
    }
    else
    {
      qDebug() << "the file does not contain a recognized suffix!" << fi;
    }
  }

  event->acceptProposedAction();

  emit filesDropped(dropFileList);
}

void data_Dialog::buttonPressed(QAbstractButton *button)
{
  if (button == ui->buttonBox->button(QDialogButtonBox::Save)) {
    // Save settings
    emit saveSettings();
    emit saveClick();
  } else if (button == ui->buttonBox->button(QDialogButtonBox::Retry)) {
    // Reload data file
    emit reloadDataFile();
    emit loadClick();
  } else if (button == ui->buttonBox->button(QDialogButtonBox::Apply)) {
    // Apply settings
    emit cosmeticReset();
  }
}

QTreeWidgetItem *data_Dialog::getRootItem(QString name)
{
  // Iterate through the root items of the QTreeWidget
  for (int i = 0; i < this->ui->treeWidget->topLevelItemCount(); ++i)
  {
    QTreeWidgetItem *item = this->ui->treeWidget->topLevelItem(i);

    // Check if the text in column 0 matches the given name
    if (item->text(0) == name)
    {
      return item; // Return the matching item
    }
  }

  // If no matching item was found, create a new root item and return it
  QTreeWidgetItem *newItem = new QTreeWidgetItem(this->ui->treeWidget);
  newItem->setText(0, name);

  return newItem;
}

void data_Dialog::dragEnterEvent(QDragEnterEvent *event)
{
  event->acceptProposedAction();
}

void data_Dialog::updateChildren(QTreeWidgetItem *item, Qt::CheckState state)
{
  for (int i = 0; i < item->childCount(); ++i)
  {
    QTreeWidgetItem *child = item->child(i);
    child->setCheckState(0, state);
    updateChildren(child, state); // Recursive update for grandchildren
  }

  if (item->childCount() == 0)
  {
    auto boolCheckState = (item->checkState(0) == Qt::Checked ? true : false);
    auto type = item->data(0, Qt::UserRole + 2).value<int>();

    if (type == 1)
    {
      auto msgId = item->data(0, Qt::UserRole + 1).value<qint32>();
      emit signalCheckStateUpdated(msgId, item->text(0), boolCheckState);
    }
    else if (type == 2)
    {
      auto varName = item->data(0, Qt::UserRole + 1).value<QString>();
      //            qDebug() << Qt::endl
      //                     << "File:" << __FILE__ << "Line:" << __LINE__
      //                     << "End item check state changed by signal from
      //                     above. SysVar:" << varName;

      emit sysVarCheckStateUpdated(varName, boolCheckState);
    }
  }
}

Qt::CheckState data_Dialog::aggregateChildStates(QTreeWidgetItem *parent)
{
  int checked = 0, unchecked = 0, total = parent->childCount();

  for (int i = 0; i < total; ++i)
  {
    QTreeWidgetItem *child = parent->child(i);
    Qt::CheckState state = child->checkState(0);

    if (state == Qt::Checked)
      ++checked;
    else if (state == Qt::Unchecked)
      ++unchecked;
  }

  if (checked == total)
    return Qt::Checked;
  else if (unchecked == total)
    return Qt::Unchecked;
  else
    return Qt::PartiallyChecked;
}

void data_Dialog::treeWidgetItemChanged(QTreeWidgetItem *item, int column)
{
  Q_UNUSED(column)
  ui->treeWidget->blockSignals(true);

  // Updating children if a parent node is changed
  if (item->childCount() > 0)
  {
    Qt::CheckState state = item->checkState(0);
    updateChildren(item, state);
  }

  // Update parents if a child node is changed
  if (item->parent())
  {
    QTreeWidgetItem *parent = item->parent();
    auto checkState = aggregateChildStates(parent);

    parent->setCheckState(0, checkState);

    //        qDebug() << "first parent propagation" << parent->text(0);

    // Potentially update grandparents or higher
    while (parent->parent())
    {
      //            qDebug() << "multi parent propagation" << parent->text(0);

      parent = parent->parent();
      parent->setCheckState(0, aggregateChildStates(parent));
    }
  }

  if (item->childCount() == 0)
  {

    auto boolCheckState = (item->checkState(0) == Qt::Checked ? true : false);
    auto type = item->data(0, Qt::UserRole + 2).value<int>();
    //        qDebug() << "File:" << __FILE__ << "Line:" << __LINE__
    //                 << "End item check state changed by signal. type:" <<
    //                 type;

    if (type == 1)
    {
      auto msgId = item->data(0, Qt::UserRole + 1).value<qint32>();
      emit signalCheckStateUpdated(msgId, item->text(0), boolCheckState);
    }
    else if (type == 2)
    {

      auto varName = item->data(0, Qt::UserRole + 1).value<QString>();
      //            qDebug() << "File:" << __FILE__ << "Line:" << __LINE__
      //                     << "End item check state changed by signal.
      //                     SysVar:" << varName;

      emit sysVarCheckStateUpdated(varName, boolCheckState);
    }
  }

  ui->treeWidget->blockSignals(false);
}

void data_Dialog::insertMessage(BusMessage &message)
{

  //    ui->treeWidget->blockSignals(true);
  auto rootItem = getRootItem("Bus");

  auto messageParent = addMiddleNode(
      rootItem, message.getName() + " (id " +
                    QString::number(message.getId() & 0x7FFFFFFFU) + ")");

  for (auto &sig : message.getSignals())
  {
    auto sigTreeItem =
        addTreeChild(messageParent, sig.getName(), sig.getUnit());
    sigTreeItem->setCheckState(0, sig.checked ? Qt::Checked : Qt::Unchecked);
    sigTreeItem->setData(0, Qt::UserRole + 1,
                         QVariant::fromValue(message.getId()));
    sigTreeItem->setData(0, Qt::UserRole + 2, QVariant::fromValue(1));
  }
}

QTreeWidgetItem *data_Dialog::addMiddleNode(QTreeWidgetItem *parent,
                                            QString name)
{
  QTreeWidgetItem *treeItem = new QTreeWidgetItem();
  //    treeItem->setCheckState(0, Qt::Unchecked); // Qt::Checked

  treeItem->setText(0, name);
  parent->addChild(treeItem);
  return treeItem;
}

QTreeWidgetItem *data_Dialog::addTreeChild(QTreeWidgetItem *parent,
                                           QString name, QString description)
{
  QTreeWidgetItem *treeItem = new QTreeWidgetItem();
  treeItem->setText(0, name);
  treeItem->setText(1, description);
  parent->addChild(treeItem);
  return treeItem;
}

void data_Dialog::resizeTreeWidgetColumns()
{
  // Resize columns to fit content
  ui->treeWidget->resizeColumnToContents(0);
  
  // Ensure minimum width for data type column
  int typeColumnWidth = ui->treeWidget->columnWidth(1);
  if (typeColumnWidth < 150) {
    ui->treeWidget->setColumnWidth(1, 150);
  }
}

// Restored original implementation for SysVars
QTreeWidgetItem *data_Dialog::insertSysVar(QString SysVarString, bool checked) {
  auto parent = getRootItem("SysVar");
  auto SysVar = splitSysvarName(SysVarString);
  SysVar.takeFirst();

  // qDebug() << "data_Dialog::insertSysVar" << SysVar;

  QTreeWidgetItem *currentParent = parent;
  QTreeWidgetItem *lastItem = nullptr;

  for (const QString &value : SysVar) {
    bool itemFound = false;

    // Iterate through child items to find the matching node.
    for (int i = 0; i < currentParent->childCount(); ++i) {
      QTreeWidgetItem *child = currentParent->child(i);
      if (child->text(0) == value) {
        currentParent = child;
        lastItem = child;
        itemFound = true;
        break;
      }
    }

    // If the item is not found, create and insert it.
    if (!itemFound) {
      // qDebug() << "insertSysVar inserting new sysvar in tree!";
      QTreeWidgetItem *newItem = new QTreeWidgetItem();
      newItem->setText(0, value);
      newItem->setCheckState(0, (checked ? Qt::Checked : Qt::Unchecked));
      newItem->setData(0, Qt::UserRole + 1, QVariant::fromValue(SysVarString));
      newItem->setData(0, Qt::UserRole + 2, QVariant::fromValue(2));

      currentParent->addChild(newItem);
      currentParent = newItem;
      lastItem = newItem;
    }
  }
  
  return lastItem;
}

// New function for CSV variables
QTreeWidgetItem *data_Dialog::insertCSVVar(QString name, bool checked, DataType dataType)
{
  QTreeWidgetItem *root = getRootItem("CSV");
  if (!root)
  {
    root = new QTreeWidgetItem(ui->treeWidget);
    root->setText(0, "CSV");
    root->setFlags(root->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
    root->setCheckState(0, Qt::Unchecked);
  }

  QTreeWidgetItem *item = new QTreeWidgetItem(root);
  item->setText(0, name.replace("CSV::", ""));
  item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
  item->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);

  // Add the same data as in the original implementation
  QString fullName = "CSV::" + item->text(0);
  item->setData(0, Qt::UserRole + 1, QVariant::fromValue(fullName));
  item->setData(0, Qt::UserRole + 2, QVariant::fromValue(2));
  
  // Store the data type
  item->setData(0, Qt::UserRole + 3, QVariant::fromValue(static_cast<int>(dataType)));
  
  qDebug() << "insertCSVVar: Creating item for" << fullName << "with data type:" << static_cast<int>(dataType);

  // Create a combo box for data type selection in column 1
  QComboBox *typeCombo = new QComboBox();
  typeCombo->addItem("Numeric", static_cast<int>(DataType::Numeric));
  typeCombo->addItem("Operating Mode", static_cast<int>(DataType::OperatingMode));
  typeCombo->addItem("Error", static_cast<int>(DataType::Error));
  typeCombo->addItem("Date/Time", static_cast<int>(DataType::DateTime));
  typeCombo->addItem("Not Used", static_cast<int>(DataType::NotUsed));
  typeCombo->addItem("Unknown", static_cast<int>(DataType::Unknown));
  
  // Set the current selection based on the data type
  int index = typeCombo->findData(static_cast<int>(dataType));
  if (index != -1) {
    typeCombo->setCurrentIndex(index);
    qDebug() << "insertCSVVar: Setting combo box index to" << index << "for data type:" << static_cast<int>(dataType);
  } else {
    qDebug() << "insertCSVVar: Could not find index for data type:" << static_cast<int>(dataType);
    typeCombo->setCurrentIndex(typeCombo->findData(static_cast<int>(DataType::Unknown)));
  }
  
  // Connect the combo box signal to handle changes
  connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
          [this, item, fullName](int index) {
              QComboBox *combo = qobject_cast<QComboBox*>(sender());
              if (combo) {
                  DataType newType = static_cast<DataType>(combo->itemData(index).toInt());
                  item->setData(0, Qt::UserRole + 3, QVariant::fromValue(static_cast<int>(newType)));
                  qDebug() << "Combo box changed: Setting data type for" << fullName << "to" << static_cast<int>(newType);
                  
                  // Ensure the signal is emitted with the correct parameters
                  qDebug() << "Emitting dataTypeChanged signal for" << fullName << "with type" << static_cast<int>(newType);
                  emit dataTypeChanged(fullName, newType);
              }
          });
  
  ui->treeWidget->setItemWidget(item, 1, typeCombo);

  return item;
}

// Overload for backward compatibility
QTreeWidgetItem *data_Dialog::insertCSVVar(QString name, bool checked)
{
  return insertCSVVar(name, checked, DataType::Unknown);
}

void data_Dialog::insertSerie(const QString &source, const QString &name,
                            int count, double factor, double offset) {
    qDebug() << "Starting insertSerie:" << name << "count:" << count;
    
    if (!ui || !ui->seriesTable) {
        qDebug() << "Error: Series table not initialized";
        return;
    }

    // Find existing row
    auto foundRow = tableContains(ui->seriesTable, 9, name);
    qDebug() << "Found existing row:" << foundRow;

    if (foundRow > -1) {
        // Update existing row
        try {
            QTableWidgetItem* countItem = getOrCreateItem(foundRow, 2);
            QTableWidgetItem* sourceItem = getOrCreateItem(foundRow, 10);
            QTableWidgetItem* factorItem = getOrCreateItem(foundRow, 7);
            QTableWidgetItem* offsetItem = getOrCreateItem(foundRow, 8);

            countItem->setText(QString::number(count));
            sourceItem->setText(source);
            factorItem->setText(QString::number(factor));
            offsetItem->setText(QString::number(offset));
            
            qDebug() << "Updated existing row" << foundRow;
            return;
        } catch (const std::exception& e) {
            qDebug() << "Error updating existing row:" << e.what();
        }
    }

    // Create new row
    static int colorIndex = 0;
    qDebug() << "Creating new row for" << name;
    
    int newRow = ui->seriesTable->rowCount();
    ui->seriesTable->insertRow(newRow);

    // Create checkbox
    QTableWidgetItem* checkboxItem = new QTableWidgetItem();
    checkboxItem->setCheckState(Qt::Unchecked);
    checkboxItem->setTextAlignment(Qt::AlignCenter);
    ui->seriesTable->setItem(newRow, 0, checkboxItem);

    // Process name
    QString channel = name.contains("(ch_") ? 
        stringBetween(name, "(ch_", ")") : 
        name.mid(0, name.indexOf("::"));
    
    QString caption = name;
    if (name.contains("SysVar")) {
        auto nameSplit = name.split("::");
        if (nameSplit.count() >= 3) {
            caption = nameSplit.at(nameSplit.count() - 2) + "::" + nameSplit.last();
        } else {
            caption = name.mid(name.lastIndexOf("::") + 2);
        }
    }

    // Find matching axis
    QString axisName = findMatchingAxis(name);

    // Insert all columns
    ui->seriesTable->setItem(newRow, 1, createTableItem(channel));
    ui->seriesTable->setItem(newRow, 2, createTableItem(QString::number(count)));
    ui->seriesTable->setItem(newRow, 3, createTableItem(caption, true));
    ui->seriesTable->setItem(newRow, 4, createTableItem(axisName));
    
    auto symbolItem = createTableItem("•");
    symbolItem->setFont(QFont("Arial", 16, QFont::Bold));
    ui->seriesTable->setItem(newRow, 5, symbolItem);

    auto colorItem = createTableItem("none");
    setItemPropertiesBasedOnColor(colorItem, chartColors.at(colorIndex++ % chartColors.size()));
    ui->seriesTable->setItem(newRow, 6, colorItem);

    ui->seriesTable->setItem(newRow, 7, createTableItem(QString::number(factor)));
    ui->seriesTable->setItem(newRow, 8, createTableItem(QString::number(offset)));
    ui->seriesTable->setItem(newRow, 9, createTableItem(name));
    ui->seriesTable->setItem(newRow, 10, createTableItem(source));

    ui->seriesTable->resizeColumnsToContents();
    qDebug() << "Completed inserting new row" << newRow;
}

QString data_Dialog::findMatchingAxis(const QString& name) {
    int maxCommonSubstring = 0;
    QString axisName = "none";
    
    for (int row = 0; row < ui->axesTable->rowCount(); ++row) {
        QTableWidgetItem* axisItem = ui->axesTable->item(row, 0);
        if (axisItem) {
            int currSubstr = longestCommonSubstring(axisItem->text(), name);
            if (currSubstr > maxCommonSubstring) {
                maxCommonSubstring = currSubstr;
                axisName = axisItem->text();
            }
        }
    }
    
    return maxCommonSubstring >= 4 ? axisName : "none";
}

QStringList data_Dialog::splitSysvarName(const QString &str)
{
  QStringList list;
  QStringList temp = str.split("::");

  for (int i = 0; i < temp.size(); ++i)
  {
    // Special handling for the last element
    if (i == temp.size() - 1)
    {
      int openBracketIndex = temp[i].indexOf('[');
      int closeBracketIndex = temp[i].indexOf(']');

      if (openBracketIndex != -1 && closeBracketIndex != -1)
      {
        QString beforeBracket = temp[i].left(openBracketIndex);
        QString insideBracket = temp[i].mid(
            openBracketIndex, closeBracketIndex - openBracketIndex + 1);

        if (!beforeBracket.isEmpty())
        {
          list << beforeBracket;
        }

        if (!insideBracket.isEmpty())
        {
          list << insideBracket;
        }
      }
      else
      {
        list << temp[i];
      }
    }
    else
    {
      list << temp[i];
    }
  }
  return list;
}

QStringList data_Dialog::getTreeItems(QTreeWidgetItem *treeItem)
{
  QStringList list;
  if (!treeItem)
    return list;

  // Function to traverse the tree recursively.
  std::function<void(QTreeWidgetItem *, QString)> traverseTree =
      [&](QTreeWidgetItem *node, QString acc)
  {
    if (!node)
      return;

    if (node->text(0).contains("["))
      acc.chop(2);

    acc += node->text(0);

    if (node->childCount() == 0)
    {
      list << acc;
      return;
    }

    acc += "::";

    for (int i = 0; i < node->childCount(); ++i)
    {
      traverseTree(node->child(i), acc);
    }
  };

  // Start the traversal from the given tree item.
  traverseTree(treeItem, "");

  return list;
}

void data_Dialog::saveAxes(QSettings *setting)
{
  QTableWidget *uiTable = ui->axesTable;
  // QSettings settings("../settings/settings.ini", QSettings::IniFormat);

  int rowCount = uiTable->rowCount() - 1; // Exclude the last row
  int colCount = uiTable->columnCount();

  while (!setting->group().isEmpty())
    setting->endGroup();

  setting->beginGroup("Axes");
  setting->remove("");
  setting->setValue("rowCount", rowCount);
  setting->setValue("colCount", colCount);

  for (int i = 0; i < rowCount; ++i)
  {
    for (int j = 0; j < colCount; ++j)
    {
      if (j == 3)
      {
        QColor bgColor = uiTable->item(i, j)->background().color();
        QString colorStr = QString::number(bgColor.red()) + "," +
                           QString::number(bgColor.green()) + "," +
                           QString::number(bgColor.blue());

        setting->setValue(QString("cell_%1_%2").arg(i).arg(j), colorStr);
      }
      else if (j == 0 || j == 1 || j == 2 || j == 4)
      {
        setting->setValue(QString("cell_%1_%2").arg(i).arg(j),
                          uiTable->item(i, j)->text());
      }
    }
  }

  setting->endGroup();
}

void data_Dialog::loadAxes(QSettings *settings)
{
  // Use the provided settings if available, otherwise use the default settings
  QSettings *settingsToUse = settings ? settings : this->settings;
  QTableWidget *uiTable = ui->axesTable;

  while (!settingsToUse->group().isEmpty())
    settingsToUse->endGroup();

  settingsToUse->beginGroup("Axes");

  int rowCount = settingsToUse->value("rowCount", 0).toInt();
  int colCount = settingsToUse->value("colCount", 0).toInt();

  if (rowCount < 2)
  {
    settingsToUse->endGroup();
    return;
  }

  uiTable->clearContents();
  uiTable->setRowCount(0);

  for (int i = 0; i < rowCount; ++i)
  {
    QString name, type, unit, position;
    QColor bgColor;

    for (int j = 0; j < colCount; ++j)
    {
      QString key = QString("cell_%1_%2").arg(i).arg(j);
      QString defaultValue;
      QStringList colorParts;

      switch (j)
      {
      case 0:
        defaultValue = "Axis1";
        name = settingsToUse->value(key, defaultValue).toString();
        break;

      case 1:
        defaultValue = "engineering";
        type = settingsToUse->value(key, defaultValue).toString();
        break;

      case 2:
        defaultValue = "unit";
        unit = settingsToUse->value(key, defaultValue).toString();
        break;

      case 3:
        defaultValue = "255,255,255";
        colorParts = settingsToUse->value(key, defaultValue).toString().split(',');
        bgColor = QColor(colorParts[0].toInt(), colorParts[1].toInt(),
                         colorParts[2].toInt());
        break;

      case 4:
        defaultValue = "right";
        position = settingsToUse->value(key, defaultValue).toString();
        break;

      default:
        continue;
      }
    }
    insertAxis(name, type, unit, bgColor, position);
  }

  settingsToUse->endGroup();
}

void data_Dialog::saveSeriesTable(QSettings *setting)
{
  // QSettings settings("../settings/settings.ini", QSettings::IniFormat);

  while (!setting->group().isEmpty())
    setting->endGroup();

  setting->beginGroup("SeriesTable");
  setting->remove("");
  // Save row and column counts
  setting->setValue("rowCount", ui->seriesTable->rowCount());
  setting->setValue("colCount", ui->seriesTable->columnCount());

  for (int i = 0; i < ui->seriesTable->rowCount(); ++i)
  {
    for (int j = 0; j < ui->seriesTable->columnCount(); ++j)
    {
      // Skip columns 2 and 10
      if (j == 2 || j == 10)
        continue;

      if (j == 0)
      {
        QTableWidgetItem *item = ui->seriesTable->item(i, j);
        if (item)
        {
          setting->setValue(QString("cell_checkstate_%1_%2").arg(i).arg(j),
                            item->checkState());
        }
        continue;
      }

      QTableWidgetItem *item = ui->seriesTable->item(i, j);
      if (item)
      {
        // Save text
        setting->setValue(QString("cell_%1_%2").arg(i).arg(j), item->text());

        // For column 6, also save background color
        if (j == 6)
        {
          QColor bgColor = item->background().color();
          // qDebug() << "  QColor bgColor   = item->background().color();" <<
          // bgColor << bgColor.alpha()
          //          << bgColor.red();

          if ((bgColor.alpha() == 255) && (bgColor.red() == 0) &&
              (bgColor.green() == 0))
            bgColor = Qt::white;

          QString colorStr = QString::number(bgColor.red()) + "," +
                             QString::number(bgColor.green()) + "," +
                             QString::number(bgColor.blue());

          setting->setValue(QString("cell_bgcolor_%1_%2").arg(i).arg(j),
                            colorStr);
        }
      }
    }
  }

  setting->endGroup();
}

void data_Dialog::loadSeriesTable(QSettings *settings)
{
  // Use the provided settings if available, otherwise use the default settings
  QSettings *settingsToUse = settings ? settings : this->settings;

  while (!settingsToUse->group().isEmpty())
    settingsToUse->endGroup();

  settingsToUse->beginGroup("SeriesTable");

  int rowCount = settingsToUse->value("rowCount", 0).toInt();
  int colCount = settingsToUse->value("colCount", 0).toInt();
  QTableWidgetItem *symbolItem;

  if (rowCount < 1)
    return;

  // reset the table
  ui->seriesTable->clearContents();
  ui->seriesTable->setRowCount(0);

  // set row count as in the settings
  ui->seriesTable->setRowCount(rowCount);

  for (int i = 0; i < rowCount; ++i)
  {
    for (int j = 0; j < colCount; ++j)
    {
      // Skip columns 2 and 10
      if (j == 2 || j == 10)
        continue;

      QString value =
          settingsToUse->value(QString("cell_%1_%2").arg(i).arg(j), "").toString();

      // Initialize count to 0 for column 2 and leave column 10 empty
      if (j == 2)
      {
        value = "0";
      }
      else if (j == 10)
      {
        value = "";
      }

      if (j == 0)
      {
        QTableWidgetItem *checkboxItem = new QTableWidgetItem();
        checkboxItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        Qt::CheckState checkState = static_cast<Qt::CheckState>(
            settingsToUse->value(QString("cell_checkstate_%1_%2").arg(i).arg(j),
                                 Qt::Unchecked)
                .toInt());
        checkboxItem->setCheckState(checkState);
        ui->seriesTable->setItem(i, j, checkboxItem);
        continue;
      }

      QTableWidgetItem *newItem = new QTableWidgetItem(value);
      ui->seriesTable->setItem(i, j, newItem);
      if (j == 5)
      {
        newItem->setFont(QFont("Arial", 16, QFont::Bold));
        newItem->setTextAlignment(Qt::AlignCenter);
        symbolItem = newItem;
      }

      // For column 6, also restore background color
      if (j == 6)
      {
        QStringList colorParts = settingsToUse->value(QString("cell_bgcolor_%1_%2").arg(i).arg(j), "#FFFFFF")
                .toString()
                .split(',');
        QColor color = QColor(colorParts[0].toInt(), colorParts[1].toInt(),
                              colorParts[2].toInt());

        setItemPropertiesBasedOnColor(newItem, color);

        if (color.isValid() && color.alpha() != 255)
        {
          symbolItem->setForeground(QBrush(color));
        }
      }
    }
  }

  ui->seriesTable->resizeColumnsToContents();

  settingsToUse->endGroup();
}

int data_Dialog::longestCommonSubstring(const QString &s1, const QString &s2)
{
  int max_len = 0; // Length of the longest common substring

  int len1 = s1.size();
  int len2 = s2.size();

  // Create a 2D vector to store the length of the longest common suffixes
  QVector<QVector<int>> dp(len1 + 1, QVector<int>(len2 + 1, 0));

  // Build the dp array from bottom up
  for (int i = 1; i <= len1; ++i)
  {
    for (int j = 1; j <= len2; ++j)
    {
      if (s1[i - 1] == s2[j - 1])
      {
        dp[i][j] = dp[i - 1][j - 1] + 1;
        max_len = std::max(max_len, dp[i][j]);
      }
      else
      {
        dp[i][j] = 0; // Reset if characters do not match
      }
    }
  }

  return max_len;
}

QTableWidgetItem* data_Dialog::getOrCreateItem(int row, int col) {
    QTableWidgetItem* item = ui->seriesTable->item(row, col);
    if (!item) {
        item = new QTableWidgetItem();
        ui->seriesTable->setItem(row, col, item);
    }
    return item;
}

QTableWidgetItem* data_Dialog::createTableItem(const QString& value, bool editable) {
    QTableWidgetItem* item = new QTableWidgetItem(value);
    if (!editable) {
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    }
    item->setTextAlignment(Qt::AlignCenter);
    return item;
}

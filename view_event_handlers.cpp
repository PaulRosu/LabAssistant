//make sure to only include this file once
// #pragma once
#include "view.h"
// #include "view_utilities.cpp"

extern bool calloutDragging;
extern bool labViewEval;
extern dataLoader *tempDataLoader;

void View::handleParsingProgress(int percent, QString name)
{
    // Static local variables
    static QMap<QString, QProgressBar *> progressBarMap;

    QGridLayout *layout = dynamic_cast<QGridLayout *>(parentWidget()->layout());
    if (!layout) {
        qDebug() << "Error: Parent widget's layout is not a QGridLayout";
        return;
    }

    if (!progressBarMap.contains(name))
    {
        // Create a new progress bar.
        QProgressBar *newProgressBar = new QProgressBar;
        newProgressBar->setMinimum(0);
        newProgressBar->setMaximum(100);
        newProgressBar->setAccessibleName(name);

        // Add the progress bar to the parent's layout.
        // Assuming you want to add it to the next row in the first column.
        int newRow = layout->rowCount();
        layout->addWidget(newProgressBar, newRow, 0, 1, 1);

        // Store the progress bar in the map.
        progressBarMap[name] = newProgressBar;
    }

    // Update the progress bar.
    QProgressBar *progressBar = progressBarMap[name];
    progressBar->setValue(percent);

    if (percent < 1)
    {
        progressBar->hide();               // Explicitly hide the widget
        layout->removeWidget(progressBar); // Remove it from the layout
        progressBarMap.remove(name);       // Remove it from the map
        progressBar->deleteLater();        // Schedule it for deletion

        // layout->invalidate();     // Invalidate the layout
        // layout->update();         // Update the layout
        // parentWidget()->update(); // Update the parent widget
    }
}

void View::proc_rangeChanged(qreal min, qreal max)
{
    if (this->userInteracted)
    {
        //           qDebug() << sender()->objectName () << min << max;
        emit rangeChanged(min, max, sender()->objectName());
    }
}

void View::dragEnterEvent(QDragEnterEvent *event)
{
    //    if (event->mimeData()->hasFormat("text/plain"))
    event->acceptProposedAction();
}

void View::dragMoveEvent(QDragMoveEvent *event)
{
    Q_UNUSED(event)

    // Overriding default dragMoveEvent in order to accept drops without items under them
}

void View::dropEvent(QDropEvent *event)
{
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

        qDebug() << "File File dropped:" << fi.baseName() << fi.suffix() << fi << url;

        //        if (fi.exists () and
        //                (fi.suffix().contains ("csv")
        //                 or fi.suffix().contains ("CSV")
        //                 or fi.suffix().contains ("zs2")
        //                 or fi.suffix().contains ("ZS2")
        //                 or fi.suffix().contains ("txt")))
        //        {

        qDebug() << "File dropped in view:" << fi.baseName();
        emit fileDropped(filePath);
        //        }
    }

    event->acceptProposedAction();
}

void View::handleOpMRangeChanged(qreal min, qreal max)
{
    Q_UNUSED(min)
    Q_UNUSED(max)

    axis_opmode->setRange(0.9, 3.1);
}

void View::handleTime2RangeChanged(qreal min, qreal max)
{
    Q_UNUSED(min)
    Q_UNUSED(max)

    //            qreal max2 = axisX2->max ()  - axisX2->min ();
    //    axisX2->setMin (0);
    //    axisX2->setMax (max2);

    //    if (max2 <= (1.0/60.0/60.0/24.0)){
    //        this->axisX2->setLabelFormat ("%4.0Tms");
    ////        qDebug() << "switched to ms" << max2;

    //    }else if (max2 <= (5.0/60.0/24.0)){
    //        this->axisX2->setLabelFormat ("%4.1Ts");

    //    }else if (max2 <= (2.0/24.0)){
    //        this->axisX2->setLabelFormat ("%4.1Tm");

    //    }else if (max2 <= (2)){
    //        this->axisX2->setLabelFormat ("%4.1Th");

    //    }else if (max2 > (2)){
    //        this->axisX2->setLabelFormat ("%4.1Td");

    //    }
}

void View::handleTimeRangeChanged(qreal min, qreal max)
{
    if (!(axisX2 == nullptr))
    {
        qreal max2 = max - min;
        axisX2->setMin(0);
        axisX2->setMax(max2);

        if (max2 <= (10.0 / 60.0 / 60.0 / 24.0))
            this->axisX2->setLabelFormat("%4.0Tms");

        else if (max2 <= (5.0 / 60.0 / 24.0))
            this->axisX2->setLabelFormat("%4.1Ts");

        else if (max2 <= (2.0 / 24.0))
            this->axisX2->setLabelFormat("%4.1Tm");

        else if (max2 <= (2))
            this->axisX2->setLabelFormat("%4.1Th");

        else if (max2 > (2))
            this->axisX2->setLabelFormat("%4.1Td");
    }

    if (this->axisX->labelFormat().contains("hh"))
        return;
    if (this->axisX->labelFormat().contains("dd"))
        return;

    if (max <= (10.0 / 60.0 / 60.0 / 24.0))
        this->axisX->setLabelFormat("%4.0Tms");

    else if (max <= (2.0 / 60.0 / 24.0))
        this->axisX->setLabelFormat("%4.1Ts");

    else if (max <= (2.0 / 24.0))
        this->axisX->setLabelFormat("%4.1Tm");

    else if (max <= (2))
        this->axisX->setLabelFormat("%4.1Th");

    else if (max > (2))
        this->axisX->setLabelFormat("%4.1Td");

    // qDebug() << "this->axisX->labelFormat ()" << this->axisX->labelFormat ();
}

void View::handleSerieClick(QPointF point)
{
    Q_UNUSED(point)

       qDebug() << "Serie clicked:" << sender()->objectName () << point <<  m_chart->mapToPosition (point);

    //    rband_origin = m_chart->mapToPosition (point).toPoint ();
    //    auto event = QMouseEvent (QEvent::MouseButtonPress, m_chart->mapToPosition (point),
    //                              Qt::LeftButton, Qt::AllButtons, Qt::NoModifier);

    //    this->mousePressEvent (&event);

    //    rband_origin = m_chart->mapToPosition (point).toPoint ();
    //    m_lastMousePos = m_chart->mapToPosition (point);
}

// void View::handleSerieRelease(QPointF point)
//{
//     released_from_item = true;
//     auto event = QMouseEvent(QEvent::MouseButtonPress, m_chart->mapToPosition(point), Qt::LeftButton, Qt::AllButtons,
//                              Qt::NoModifier);

//    this->mouseReleaseEvent(&event);
//}

void View::handleSerieRelease(QPointF point)
{
    released_from_item  = true;
  qDebug() << "Serie released:" << sender()->objectName () << point <<  m_chart->mapToPosition (point);

    QPointF mappedPoint = m_chart->mapToPosition(point);
    QMouseEvent event(QEvent::MouseButtonRelease, mappedPoint, mappedPoint, mappedPoint, Qt::LeftButton, Qt::LeftButton,
                      Qt::NoModifier);

    this->mouseReleaseEvent(&event);
}

void View::keyPressEvent(QKeyEvent *event)
{
    //    qDebug () << event->text() << " key pressed!";
    this->userInteracted = true;

    if (event->text() == "\r")
        m_chart->setTitle(m_chart->title());

    //    if(event->text() == "r")  axisX2->setTickType (QValueAxis::TicksDynamic);

    else if (event->text() == "\u0013")
    {
        qDebug() << "You pressed ctrl+s, exporting current view as png! " << this->serieName;
        savePNG(this->serieName);
        emit this->saved();
    }

    else if (event->text() == "\u0003")
    {

        for (auto legend_item : m_chart->legend()->markers())
        {
            if (!legend_item->series()->isVisible())
            {
                legend_item->setVisible(false);
            }
        }

        m_chart->legend()->layout()->invalidate();
        this->update();
        QApplication::processEvents();

        QPixmap p               = this->grab(m_chart->rect().toAlignedRect().marginsRemoved(m_chart->margins() + 10));

        QOpenGLWidget *glWidget = View::findChild<QOpenGLWidget *>();
        if (glWidget)
        {
            QPainter painter(&p);
            //            QPoint d = glWidget->mapToGlobal(QPoint()) - View::mapToGlobal(QPoint());
            //            painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            //            painter.drawImage(d, glWidget->grabFramebuffer());
            painter.end();
        }
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setPixmap(p);

        for (auto legend_item : m_chart->legend()->markers())
        {
            legend_item->setVisible(true);
        }

        m_chart->legend()->layout()->invalidate();
        this->update();
        QApplication::processEvents();
    }

    else if (event->text() == "\u001B")
    {
        if ((rubberBand != nullptr) and (rubberBand->isVisible()))
            rubberBand->hide();
    }

    else if (event->key() == Qt::Key_Left)
    {
        auto min    = axisX->min();
        auto max    = axisX->max();
        auto offset = max - min;
        offset      = offset / 2;

        axisX->setRange(min - offset, max - offset);
        View::m_chart->update();
        axisX->dumpObjectTree();
    }

    else if (event->key() == Qt::Key_Right)
    {
        auto min    = axisX->min();
        auto max    = axisX->max();
        auto offset = max - min;
        offset      = offset / 2;

        axisX->setRange(min + offset, max + offset);
        View::m_chart->update();
    }

    else if (event->text() == "\u007F")
    {
        qDebug() << "You pressed Delete, deleting selected callouts!";
        for (auto serie : m_chart->series())
        {
            for (int i = 0; i < intPoints[serie->name()].size(); i++)
            {
                if (intPoints[serie->name()].at(i)->selected)
                {
                    Callout *todelete = intPoints[serie->name()].at(i);
                    intPoints[serie->name()].removeAt(i);
                    delete todelete;
                }
            }
        }
    }

    /*

     if( event->text() == "\u0012")
         {
             qDebug () << "You pressed ctrl+R, resetting zoom";
             m_chart->axes (Qt::Horizontal).first ()->setMin (0);
             m_chart->axes (Qt::Vertical).first ()->setMin (0);
             m_chart->axes (Qt::Vertical).first ()->setMax(origYMax);
             m_chart->axes (Qt::Horizontal).first ()->setMax(origXMax);
             for  (auto serie : m_chart->series ())
             {
                 if (serie->isVisible ())
                     for ( Callout* callout : intPoints[serie->name()]) callout->updateGeometry();
             }
         }





      if( event->text() == "x")
      {
          qDebug () << "You pressed x, zoom x!";


         QRectF r = QRectF(m_chart->plotArea().left(),
                           m_chart->plotArea().top(),
                           m_chart->plotArea().width()  / 1.1,
                           m_chart->plotArea().height());

          m_chart->zoomIn(r);
      }

     if( event->text() == "z")
     {
         qDebug () << "You pressed x, zoom x!";


          QRectF r = QRectF(m_chart->plotArea().left(),
                            m_chart->plotArea().top(),
                            m_chart->plotArea().width()  / 0.9,
                            m_chart->plotArea().height());

         m_chart->zoomIn(r);
     }

      if( event->text() == "s")
      {
          qDebug () << "You pressed s, zoom s!";


         QRectF r = QRectF(m_chart->plotArea().left(),
                           m_chart->plotArea().top(),
                           m_chart->plotArea().width(),
                           m_chart->plotArea().height()/ 1.1) ;

          m_chart->zoomIn(r);
      }
      if( event->text() == "a")
      {
          qDebug () << "You pressed x, zoom x!";


         QRectF r = QRectF(m_chart->plotArea().left(),
                           m_chart->plotArea().top(),
                           m_chart->plotArea().width(),
                           m_chart->plotArea().height()/ 0.9) ;

          m_chart->zoomIn(r);
      }


     if( event->text() == "t")
     {
         qDebug () << "You pressed t, toggling all markers!";

          for ( auto marker : m_chart->legend ()->markers ())
          {
              handleMarkerToggle(marker);
          }
      }

     if( event->text() == "l") {
         axisX2->setVisible(! axisX2->isVisible());
     }


          for (auto axis : m_chart->axes())
          {
              QValueAxis* v_axis =  qobject_cast<QValueAxis*> (axis);
              v_axis->applyNiceNumbers();
              //                   double niceR = niceTickRange( v_axis->max() - v_axis->min(), v_axis->tickCount());
              //                    qDebug() << v_axis->labelFormat() << niceR;

             //                    v_axis->setRange(niceR *(round(v_axis->min() / niceR)), niceR *(round(1 +
     v_axis->max() / niceR)));

              //
          }
      }

     */
    //    if( event->text() == "n")
    //    {
    //        for (auto axis : m_chart->axes())
    //        {
    //            QValueAxis* v_axis =  qobject_cast<QValueAxis*> (axis);
    ////            v_axis->applyNiceNumbers();
    //            double niceR = niceTickRange( v_axis->max() - v_axis->min(), v_axis->tickCount());
    ////                                qDebug() << v_axis->labelFormat() << niceR;

    //            v_axis->setRange(niceR *(round(v_axis->min() / niceR)), niceR *(round(1 + v_axis->max() / niceR)));

    //            //
    //        }
    //    }

    if (event->text() == "\u0014")
    {
        //        qDebug () << "You pressed ctrl+t, deleting all markers!";

        //        //        qDeleteAll(intPoints);

        //        for  (auto serie : m_chart->series ())
        //        {
        //            for ( int i = 0; i <  intPoints[serie->name()].size(); i++)
        //            {
        //                //                Callout* todelete = intPoints[serie->name()].at(i);

        //                delete intPoints[serie->name()].at(i);
        //                intPoints[serie->name()].removeAt(i);
        //                //                delete  todelete;
        //            }
        //        }

        //        for  (auto serie : m_chart->series ())
        //        {

        for (auto legendmarker : m_chart->legend()->markers())
        {

            handleMarkerToggle(legendmarker);
        }
        // hideUnusedAxes(m_chart);
    }

    if (event->text() == "\b")
    {
        for (auto serie : m_chart->series())
        {
            if (!serie->isVisible())
                continue;
            for (Callout *callout : intPoints[serie->name()])
            {
                callout->setVisible(!callout->isVisible());
                callout->supressed = !callout->supressed;
            }
        }
    }
    if (event->text() == "\u0011")
    {
        QDesktopServices::openUrl("file:" + QCoreApplication::applicationDirPath() + "\\help\\help.html");
    }
    QGraphicsView::keyPressEvent(event);
}

void View::wheelEvent(QWheelEvent *event)
{
    this->userInteracted = true;

    if (event->angleDelta().y() < 0)
        emit mouseEvent("up", true);
    if (event->angleDelta().y() > 0)
        emit mouseEvent("down", true);

    if ((event->buttons() & Qt::MiddleButton))
    {
        if (event->angleDelta().y() < 0)
            emit mouseEvent("up", false);
        if (event->angleDelta().y() > 0)
            emit mouseEvent("down", false);
        return;
    }

    if (event->position().y() > m_chart->plotArea().height() + m_chart->plotArea().top() or
        event->position().y() < m_chart->plotArea().top())
    {

        QGraphicsView::wheelEvent(event);

        qreal factor;
        if (event->angleDelta().y() > 0)
            factor = 0.9;
        else
            factor = 1.1;

        int ticknr       = abs(event->angleDelta().y()) / 120;
        QPointF mousePos = mapFromGlobal(QCursor::pos());

        QRectF r         = m_chart->plotArea();

        for (int i = 0; i < ticknr; i++)
        {
            r.setWidth(factor * r.width());
        }

        auto temp_r = r;
        temp_r.moveCenter(mousePos);

        r.moveLeft(temp_r.left());
        m_chart->zoomIn(r);

        QPointF delta = m_chart->plotArea().center() - mousePos;
        m_chart->scroll(delta.x(), 0);

        updateCallouts();

        View::m_chart->update();
        View::update();
        QApplication::processEvents();
        event->accept();

        if (event->angleDelta().y() < 0)
            emit mouseEvent("up", false);
        if (event->angleDelta().y() > 0)
            emit mouseEvent("down", false);

        return;
    }
    else if ((event->position().x() < m_chart->plotArea().x()) or
             event->position().x() > (m_chart->plotArea().width() + m_chart->plotArea().x()))
    {

        if (m_chart->legend()->alignment() == Qt::AlignRight)
            if (event->position().x() > m_chart->legend()->x())
            {
                if (event->angleDelta().y() < 0)
                    emit mouseEvent("up", false);
                if (event->angleDelta().y() > 0)
                    emit mouseEvent("down", false);
                return;
            }

        QGraphicsView::wheelEvent(event);

        qreal factor;
        if (event->angleDelta().y() > 0)
            factor = 0.9;
        else
            factor = 1.1;

        int ticknr       = abs(event->angleDelta().y()) / 120;
        QPointF mousePos = mapFromGlobal(QCursor::pos());

        QRectF r         = m_chart->plotArea();

        for (int i = 0; i < ticknr; i++)
        {
            r.setHeight(factor * r.height());
        }

        auto temp_r = r;
        temp_r.moveCenter(mousePos);

        r.moveTop(temp_r.top());
        m_chart->zoomIn(r);

        QPointF delta = m_chart->plotArea().center() - mousePos;
        m_chart->scroll(0, -delta.y());

        updateCallouts();

        View::m_chart->update();
        View::update();
        QApplication::processEvents();
        event->accept();

        if (event->angleDelta().y() < 0)
            emit mouseEvent("up", false);
        if (event->angleDelta().y() > 0)
            emit mouseEvent("down", false);
        return;
    }

    if (event->position().x() < m_chart->plotArea().x() or event->position().y() < m_chart->plotArea().top() or
        event->position().x() > m_chart->plotArea().width() + m_chart->plotArea().x() or
        event->position().y() > m_chart->plotArea().height() + m_chart->plotArea().top())
    {
        QGraphicsView::wheelEvent(event);
        event->ignore();
        if (event->angleDelta().y() < 0)
            emit mouseEvent("up", false);
        if (event->angleDelta().y() > 0)
            emit mouseEvent("down", false);
        return;
    }
    // QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QGraphicsView::wheelEvent(event);

    qreal factor;
    if (event->angleDelta().y() > 0)
        factor = 1.1;
    else
        factor = 0.9;

    int ticknr       = abs(event->angleDelta().y()) / 120;
    QPointF mousePos = mapFromGlobal(QCursor::pos());

    QRectF r = QRectF(m_chart->plotArea().left(), m_chart->plotArea().top(), m_chart->plotArea().width() / factor,
                      m_chart->plotArea().height() / factor);

    for (int i = 0; i < ticknr - 1; i++)
    {
        //        qDebug() << "in bucla";
        r = QRectF(r.left(), r.top(), r.width() / factor, r.height() / factor);
    }

    r.moveCenter(mousePos);

    m_chart->zoomIn(r);
    QPointF delta = m_chart->plotArea().center() - mousePos;
    m_chart->scroll(delta.x(), -delta.y());

    updateCallouts();

    View::m_chart->update();
    View::update();
    QApplication::processEvents();
    //    qDebug() << "Scroll" << event->angleDelta().y() << abs(event->angleDelta().y()) /120;
    event->accept();
    // QApplication::restoreOverrideCursor();

    if (event->angleDelta().y() < 0)
        emit mouseEvent("up", false);
    if (event->angleDelta().y() > 0)
        emit mouseEvent("down", false);
}

void View::resizeEvent(QResizeEvent *event)
{
    if (this->objectName() == "")
        return;
    if (m_chart == nullptr)
        return;
    // return;
    if (scene())
    {
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
        if (this->m_chart && event)
        {
            this->m_chart->resize(event->size());
        }
        m_coordX->setPos(m_chart->size().width() / 2 - 50, m_chart->size().height() - 20);
        m_coordY->setPos(m_chart->size().width() / 2 + 50, m_chart->size().height() - 20);

        updateCallouts();

        View::update();
    }
    //    View::m_chart->update();
    QGraphicsView::resizeEvent(event);
    //    QApplication::processEvents ();
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    // long long in = 0;
    if (event->buttons() & Qt::RightButton)
        emit mouseEvent("right", true);
    if (event->buttons() & Qt::LeftButton)
        emit mouseEvent("left", true);
    if (event->buttons() & Qt::MiddleButton)
        emit mouseEvent("middle", true);

    this->userInteracted = true;
    if (event->pos().x() < m_chart->plotArea().x() or event->pos().y() < m_chart->plotArea().top() or
        event->pos().x() > m_chart->plotArea().width() + m_chart->plotArea().x() or
        event->pos().y() > m_chart->plotArea().height() + m_chart->plotArea().top())
    {
        QGraphicsView::mouseMoveEvent(event);
        event->ignore();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);

    //    char force[40];
    //    char travel[40];
    //    char position[40];
    //    char position2[40];
    //    char temp[40];
    //    bool has_position = false;
    //    bool has_position2 = false;
    //    bool has_temp = false;
    //    sprintf(force, "%.3f",  event->pos().y ());
    //    sprintf(travel, "%.3f",  event->pos().x ());

    //    for (auto serie : m_chart->series ()){
    //        if(serie->name ().contains ("Capa_1")){
    //            qDebug() << "serie->name ().contains (Capa_1)" << serie->name ();
    //            QScatterSeries* s_serie = qobject_cast<QScatterSeries*>(serie);
    //            for (auto p_point : s_serie->pointsVector ()){
    ////                qDebug() << p_point << event->pos();
    //                if(p_point.x() >= m_chart->mapToValue(event->pos()).x()){
    //                    sprintf(position, "%.1f", p_point.y());
    ////                    qDebug() << "p_point.y()" << p_point.y();
    //                    has_position = true;
    //                    break;
    //                }
    //            }
    //        } else if(serie->name ().contains ("Capa_2")){
    //            qDebug() <<  "serie->name ().contains (Capa_2)" << serie->name ();
    //            QScatterSeries* s_serie = qobject_cast<QScatterSeries*>(serie);
    //            for (auto p_point : s_serie->pointsVector ()){
    //                if(p_point.x() >= m_chart->mapToValue(event->pos()).x()){
    //                    sprintf(position2, "%.1f", p_point.y());
    ////                    qDebug() << "p_point.y()" << p_point.y();
    //                    has_position2 = true;
    //                    break;
    //                }
    //            }
    //        }
    //        else if(serie->name ().contains ("Temp")){
    //            qDebug() <<  "serie->name ().contains (Temp)" << serie->name ();
    //            QLineSeries* s_serie = qobject_cast<QLineSeries*>(serie);
    //            for (auto p_point : s_serie->pointsVector ()){
    //                if(p_point.x() >= m_chart->mapToValue(event->pos()).x()){
    //                    sprintf(temp, "%.1f", p_point.y());
    ////                        qDebug() << "p_point.y()" << p_point.y();
    //                    has_temp = true;
    //                    break;
    //                }
    //            }
    //        }

    //        if (has_position and has_position2) break;//and has_temp
    //    }

    //    qDebug() << "position" << position << "position2" << position2 << has_position << has_position2;

    m_coordX->setText(QString("X: %1").arg(m_chart->mapToValue(event->pos()).x())); // *** T
    m_coordY->setText(QString("Y: %1").arg(m_chart->mapToValue(event->pos()).y()));

    //    m_coordY->setText(QString("Temp: %1 Capa_1: %2 Capa2: %3").arg
    //                      (m_chart->mapToValue(event->pos()).y()).arg (position).arg (position2));

    //    m_coordX->setText(QString("Time: %1").arg(m_chart->mapToValue(event->pos()).x()));// *** T
    //    m_coordY->setText(QString("Temp: %1 Capa_1: %2 Capa2: %3").arg(m_chart->mapToValue(event->pos()).y()).arg
    //    (position).arg (position2));

    if ((event->buttons() & Qt::RightButton) and (event->buttons() & Qt::LeftButton) and !calloutDragging)
    { // pan Y

        setCursor(QCursor(Qt::SizeAllCursor));
        auto dPos = event->pos() - m_lastMousePos;

        m_chart->scroll(-dPos.x(), dPos.y());

        updateCallouts();
        event->accept();
        View::update();
    }
    else if ((event->buttons() & Qt::LeftButton) and !calloutDragging)
    { // Pan x

        //      in = QDateTime::currentMSecsSinceEpoch();

        setCursor(QCursor(Qt::SizeHorCursor));

        //        qDebug() << "mouse move with left pressed";

        auto dPos = event->pos() - m_lastMousePos;
        //            qDebug() << "moved:" << dPos;
        m_chart->scroll(-dPos.x(), 0);

        updateCallouts();
        event->accept();

        m_lastMousePos = event->pos();
    }
    else if ((event->buttons() & Qt::MiddleButton) and !calloutDragging)
    {

        setCursor(QCursor(Qt::CrossCursor));

        if (rubberBand == nullptr)
        {
            rband_origin = event->pos();
            rubberBand   = new QRubberBand(QRubberBand::Rectangle, this);
            rubberBand->setGeometry(QRect(rband_origin, QSize()));
            rubberBand->show();
        }

        if (!rubberBand->isVisible())
        {
            rband_origin = event->pos();

            QPalette pal;
            auto temp_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
            rubberBand->setPalette(temp_rubberBand->palette());
            delete temp_rubberBand;
            if (cutMode)
                pal.setBrush(QPalette::Highlight, QBrush(Qt::red));
            rubberBand->setPalette(pal);

            rubberBand->show();
        }
        if (!(rubberBand == nullptr) and rubberBand->isVisible())
        {
            rubberBand->setGeometry(QRect(rband_origin, event->pos()).normalized());
        }

        updateCallouts();
        event->accept();
        View::update();

        m_lastMousePos = event->pos();
    }
    else if ((event->buttons() & Qt::RightButton) and !calloutDragging)
    { // pan Y
        setCursor(QCursor(Qt::SizeVerCursor));
        auto dPos = event->pos() - m_lastMousePos;
        m_chart->scroll(0, dPos.y());
        updateCallouts();
        event->accept();
        View::update();
    }
    else
    {
        if (!(rubberBand == nullptr) and rubberBand->isVisible())
            rubberBand->hide();
    }

    // if(in > 0)       qDebug() << "pan x before update time:" << QDateTime::currentMSecsSinceEpoch() - in;

    m_lastMousePos = event->pos();

    this->m_chart->update();

    //    if(in > 0)       qDebug() << "pan x after update time:" << QDateTime::currentMSecsSinceEpoch() - in;
    QApplication::processEvents();

    //         if(in > 0)       qDebug() << "pan x time:" << QDateTime::currentMSecsSinceEpoch() - in;

    //    QGraphicsView::mouseMoveEvent(event);
}

void View::mousePressEvent(QMouseEvent *event)
{

    if (event->button() == Qt::RightButton)
        emit mouseEvent("right", true);
    if (event->button() == Qt::LeftButton)
        emit mouseEvent("left", true);
    if (event->button() == Qt::MiddleButton)
        emit mouseEvent("middle", true);

    this->userInteracted = true;
    rmb_clicked          = false;
    mid_clicked          = false;
    QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
    QApplication::restoreOverrideCursor();

    if (event->button() == Qt::RightButton)
        rmb_clicked = true;
    //    if (event->button() == Qt::MidButton) mid_clicked = true;
    if (event->button() == Qt::MiddleButton)
        mid_clicked = true;

    if (event->pos().x() < m_chart->plotArea().x() or event->pos().y() < m_chart->plotArea().top() or
        event->pos().x() > m_chart->plotArea().width() + m_chart->plotArea().x() or
        event->pos().y() > m_chart->plotArea().height() + m_chart->plotArea().top())
    {
        if (event->button() == Qt::RightButton)
            rmb_clicked = true;

        QGraphicsView::mousePressEvent(event);

        return;
    }

    if (event->button() == Qt::LeftButton)
    {
      qDebug() << "left mouse press event in chart area" << this->cursor ();
        this->setCursor(QCursor(Qt::SizeHorCursor));
      qDebug() << "after setting cursor to size" << this->cursor () << this;
        m_lastMousePos = event->pos();
    }
    else if (event->button() == Qt::RightButton)
    {
        setCursor(QCursor(Qt::SizeVerCursor));
        m_lastMousePos = event->pos();
    }

    if ((event->buttons() & Qt::RightButton) and (event->buttons() & Qt::LeftButton))
    { // pan Y
        setCursor(QCursor(Qt::SizeAllCursor));
        m_lastMousePos = event->pos();
    }

    else if (event->button() == Qt::MiddleButton)
    {
        rband_origin = event->pos();
        if (rubberBand == nullptr)
            rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
        rubberBand->setGeometry(QRect(rband_origin, QSize()));

        if (cutMode)
        {
            QPalette pal;
            pal.setBrush(QPalette::Highlight, QBrush(Qt::red));
            rubberBand->setPalette(pal);
        }
        else
        {
            auto temp_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
            rubberBand->setPalette(temp_rubberBand->palette());
            delete temp_rubberBand;
        }

        rubberBand->show();
    }

    View::m_chart->update();
    QGraphicsView::mousePressEvent(event);
    //    if (calloutDragging) qDebug() << "Dragging:" << calloutDragging;
}

void View::mouseReleaseEvent(QMouseEvent *event)
{

    this->m_chart->update();
    QApplication::processEvents();

    //    if (event->button() == Qt::RightButton) emit mouseEvent("right", false);
    //    if (event->button() == Qt::LeftButton) emit mouseEvent("left", false);
    //    if (event->button() == Qt::MiddleButton) emit mouseEvent("middle", false);

    if (QGuiApplication::mouseButtons() != Qt::RightButton)
        emit mouseEvent("right", false);
    if (QGuiApplication::mouseButtons() != Qt::MiddleButton)
        emit mouseEvent("left", false);
    if (QGuiApplication::mouseButtons() != Qt::LeftButton)
        emit mouseEvent("middle", false);

    //    QApplication::restoreOverrideCursor();
    unsetCursor();

    this->userInteracted = true;

    //    qDebug () << "released" << event->button();

    if (!(rubberBand == nullptr) and rubberBand->isVisible())
    {
        rubberBand->hide();

        if (rubberBand->geometry().width() < 10)
            return;

        if (cutMode)
        {
            QPointF fp = m_chart->mapToValue(rubberBand->geometry().topLeft());
            QPointF tp = m_chart->mapToValue(rubberBand->geometry().bottomRight());

            if (fp.x() < 0)
                fp.setX(0);
            if (tp.x() > 0)
            {

                cutGraph(fp.x(), tp.x());

                emit I_cut_my_graph(fp.x(), tp.x());
            }
            updateCallouts();
        }
        else
        {
            m_chart->zoomIn(rubberBand->geometry());
            updateCallouts();
        }
    }

    if (released_from_item)
    {
        released_from_item = false;
        return;
    }

    // QPointF scenePoint                     = mapToScene(event->pos());
    // QList<QGraphicsItem *> itemsUnderMouse = scene()->items(scenePoint);
    // QGraphicsItem *lastChildItem           = nullptr;
    // for (QGraphicsItem *item : itemsUnderMouse)
    // {

    //     if (item->childItems().isEmpty())
    //     {

    //         if (item->type() == 6)
    //         {

    //             qDebug() << item->parentItem()->type() << item << item->parentObject();

    //             for (auto &axis : m_chart->axes())
    //             {
    //                 qDebug() << "axis" << axis << axis->children();
    //             }
    //         }
    //     }
    // }

    QGraphicsView::mouseReleaseEvent(event);
}

void View::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        emit mouseEvent("right", true);
    if (event->button() == Qt::LeftButton)
        emit mouseEvent("left", true);
    if (event->button() == Qt::MiddleButton)
        emit mouseEvent("middle", true);

    // QGraphicsView::mouseDoubleClickEvent(event);
    // editable title and axis labels event propagation bug workaround Paul Rosu
    QPointF scenePoint                     = mapToScene(event->pos());
    QList<QGraphicsItem *> itemsUnderMouse = scene()->items(scenePoint);

    QGraphicsItem *lastChildItem           = nullptr;
    for (QGraphicsItem *item : itemsUnderMouse)
    {
        if (item->childItems().isEmpty())
        {
            lastChildItem = item;

            if (!lastChildItem->toGraphicsObject())
                continue;

            if (qgraphicsitem_cast<QGraphicsTextItem *>(lastChildItem) == nullptr)
                continue;

            auto objName   = qgraphicsitem_cast<QGraphicsTextItem *>(lastChildItem)->objectName().toLocal8Bit();
            auto className = lastChildItem->toGraphicsObject()->metaObject()->className();

            if (((strcmp(objName, "ChartTitle") == 0)) || strcmp(className, "ValueAxisLabel") == 0)
            {
                auto textItem = dynamic_cast<QGraphicsTextItem *>(lastChildItem);
                textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
                textItem->setFocus(Qt::MouseFocusReason);
                QTextCursor cursor = textItem->textCursor();
                cursor.select(QTextCursor::Document);
                textItem->setTextCursor(cursor);
                break;
            }
        }
    }
}

void View::actionShowDataManager()
{

    qDebug() << "void View::actionShowDataManager()";

    data.DataManager->show();
}

#include "view.h"
// #include "view_utilities.cpp"

extern bool calloutDragging;
extern bool labViewEval;

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

    // axis_opmode->setRange(0.9, 3.1);
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
    // qDebug() << "Serie released:" << sender()->objectName () << point <<  m_chart->mapToPosition (point);

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

    // qDebug() << "void View::actionShowDataManager()";

    data.DataManager->show();
}

void View::applyNiceNumbers() {
  // if ((axis_current != nullptr) and (axis_current->isVisible()))
  //     axis_current->applyNiceNumbers();
  // if ((axis_sleep_current != nullptr) and (axis_sleep_current->isVisible()))
  //     axis_sleep_current->applyNiceNumbers();
  // if ((axis_temperature != nullptr) and (axis_temperature->isVisible()))
  //     axis_temperature->applyNiceNumbers();
  // if ((axis_humidity != nullptr) and (axis_humidity->isVisible()))
  //     axis_humidity->applyNiceNumbers();
  // if ((axis_voltage != nullptr) and (axis_voltage->isVisible()))
  //     axis_voltage->applyNiceNumbers();

  // if (axis_travel != nullptr)
  //     axis_travel->applyNiceNumbers();
  // if (axis_force != nullptr)
  //     axis_force->applyNiceNumbers();

  // for (auto ax : m_chart->axes())
  // {
  //     QValueAxis *axis = qobject_cast<QValueAxis *>(ax);
  //     if (axis->objectName() == "axis_HV_voltage")
  //         axis->applyNiceNumbers();
  //     if (axis->objectName() == "axis_HV_current")
  //         axis->applyNiceNumbers();
  // }

  for (QAbstractAxis *axis : m_chart->axes()) {
    if (axis->orientation() == Qt::Vertical) {
      auto valueAxis = qobject_cast<QValueAxis *>(axis);
      // if (valueAxis->isVisible())
      valueAxis->applyNiceNumbers();
    }
  }

  updateCallouts();
  this->m_chart->update();
  QApplication::processEvents();
}

void View::tenTicks() {
  for (auto abstract_axis : m_chart->axes()) {
    QValueAxis *axis = qobject_cast<QValueAxis *>(abstract_axis);
    axis->setTickCount(11);
  }
  this->m_chart->update();
  QApplication::processEvents();
}

void View::setScaleAxis(bool state) {
  if (axisX2 != nullptr) {
    axisX2->setVisible(state);
  }
  this->m_chart->update();
  QApplication::processEvents();
}

void View::exportAction() {

  // if (this->is_durability) {
  //   exportDurability();
  //   return;
  // }

  // if (this->is_robot) {
  //   exportRobot();
  //   return;
  // }

  bool ok;
  QFileInfo fi(serieName);
  QString text = QInputDialog::getText(this, tr("Export current view"),
                                       tr("Picture name:"), QLineEdit::Normal,
                                       fi.baseName(), &ok);
  if (ok && !text.isEmpty())
    savePNG(fi.absolutePath() + "/" + text);
}


void View::cutGraph(double start, double stop) {
  if (!cutMode)
    return;

  if (stop > 0) {

    QPointF temp_point;
    QVector<QPointF> new_vector;
    QVector<QPointF> m_points_vector;

    // qDebug() << "stop" << stop;

    // if ((temperature != nullptr) and temperature->points().size() > 5) {

    //   if (stop > temperature->points().last().x())
    //     stop = temperature->points().last().x();
    // } else if ((OM != nullptr) and (OM->points().size() > 1)) {

    //   if (stop > OM->points().last().x())
    //     stop = OM->points().last().x();
    // }

    // qDebug() << "stop" << stop;

    for (auto m_serie : m_chart->series()) {

      auto m_serie_xy = qobject_cast<QXYSeries *>(m_serie);
      m_points_vector = m_serie_xy->points();

      for (auto m_point : m_points_vector) {
        if (m_point.x() < 0)
          continue;
        if (m_point.x() > start) {

          if (m_point.x() > stop) {
            temp_point = m_point;
            temp_point.setX(m_point.x() - (stop - start));
            new_vector.append(temp_point);
          }
        } else {
          new_vector.append(m_point);
        }
      }

      m_serie_xy->replace(new_vector);
      new_vector.clear();
      m_points_vector.clear();

      if (m_serie_xy->property("my_type") == "error") {
        if (m_serie_xy->points().size() == 0) {
          m_chart->removeSeries(m_serie_xy);
          m_serie_xy->deleteLater();
        } else {
          m_serie_xy->setName(m_serie_xy->name().section('(', 0, 0) + "(" +
                              QString::number(m_serie_xy->points().size()) +
                              ")");
        }
      }
    }

    // if (axisX != nullptr)
    // {
    //     auto init_X_Max = this->initialScale.value(axisX).y();
    //     this->initialScale.insert(axisX, QPointF(0, init_X_Max - (stop -
    //     start)));
    // }

    // if (axis_travel != nullptr)
    // {
    //     auto init_X_Max = this->initialScale.value(axis_travel).y();
    //     this->initialScale.insert(axis_travel, QPointF(0, init_X_Max -
    //     (stop - start)));
    // }

    for (auto axis : m_chart->axes(Qt::Horizontal)) {
      QValueAxis *valueAxis = qobject_cast<QValueAxis *>(axis);
      if (valueAxis->objectName() == "axisX2")
        continue;
      auto init_X_Max = this->initialScale.value(valueAxis).y();
      this->initialScale.insert(valueAxis,
                                QPointF(0, init_X_Max - (stop - start)));
    }

    new_vector.clear();
    m_points_vector.clear();
    new_vector.squeeze();
    m_points_vector.squeeze();

    m_chart->update();

    updateCallouts();

    View::m_chart->update();
    View::update();
    QApplication::processEvents();
  }
}


void showStatistics(const QVector<QPointF> *serie, QString name) {
  if (!serie || serie->isEmpty())
    return;

  // Calculate statistics
  double average = 0, min = serie->at(0).y(), max = serie->at(0).y(), sum = 0;
  for (const QPointF &point : *serie) {
    double y = point.y();
    min = std::min(min, y);
    max = std::max(max, y);
    sum += y;
  }
  average = sum / serie->size();

  double variance = 0;
  for (const QPointF &point : *serie) {
    double diff = point.y() - average;
    variance += diff * diff;
  }
  variance /= serie->size();
  double stddev = std::sqrt(variance);

  // Copy statistics to clipboard
  QString statsString =
      QString("Min: %1\nMax: %2\nAverage: %3\nStandard Deviation: %4")
          .arg(min)
          .arg(max)
          .arg(average)
          .arg(stddev);
  QGuiApplication::clipboard()->setText(statsString);

  // Setup Dialog
  QDialog dialog;
  dialog.setWindowTitle(name + " - Statistics & Distribution");
  dialog.resize(1024, 768); // Set initial dialog size
  QVBoxLayout *layout = new QVBoxLayout(&dialog);

  // Customize font for statistics text
  QFont statsFont;
  statsFont.setBold(true);
  statsFont.setPointSize(12);

  // Text for statistics with customized font
  QLabel *statsLabel = new QLabel(statsString);
  statsLabel->setFont(statsFont);
  layout->addWidget(statsLabel);

  // Setup chart
  QChart *chart = new QChart();
  QBarSeries *series = new QBarSeries();

  // Histogram calculation with percentage
  int numBins = 10;
  QVector<double> bins(numBins, 0);
  double binWidth = (max - min) / numBins;
  int totalPoints = serie->size();

  for (const QPointF &point : *serie) {
    int binIndex =
        std::min(static_cast<int>((point.y() - min) / binWidth), numBins - 1);
    bins[binIndex]++;
  }

  QBarSet *set = new QBarSet("Distribution");
  for (double bin : bins) {
    double percent = (bin / totalPoints) * 100;
    *set << percent;
  }
  series->append(set);

  chart->addSeries(series);
  chart->createDefaultAxes();
  chart->legend()->hide();

  // Customize axes
  QFont labelFont;
  labelFont.setBold(true);
  labelFont.setPointSize(12);

  QBarCategoryAxis *categoryAxis = new QBarCategoryAxis();
  for (int i = 0; i < numBins; ++i) {
    categoryAxis->append(QString("[%1, %2)")
                             .arg(min + i * binWidth)
                             .arg(min + (i + 1) * binWidth));
  }
  categoryAxis->setTitleText("Bin Range");
  categoryAxis->setTitleFont(labelFont);
  chart->addAxis(categoryAxis, Qt::AlignBottom);
  series->attachAxis(categoryAxis);

  QValueAxis *axisY = new QValueAxis();
  axisY->setRange(0, 100);
  axisY->setTitleText("Percentage (%)");
  axisY->setTitleFont(labelFont);
  chart->addAxis(axisY, Qt::AlignLeft);
  series->attachAxis(axisY);

  QChartView *chartView = new QChartView(chart);
  chartView->setMinimumSize(640, 480);
  layout->addWidget(chartView);

  dialog.exec();
}




void View::handleMarkerClicked() {
  // qDebug() << "handleMarkerClicked";

  this->userInteracted = true;
  QLegendMarker *marker = qobject_cast<QLegendMarker *>(sender());
  Q_ASSERT(marker);

  //    qDebug() << "marker clicked!";

  switch (marker->type()) {
  case QLegendMarker::LegendMarkerTypeXY: {

    if (mid_clicked) {
      auto serie = qobject_cast<QXYSeries *>(marker->series())->points();
      showStatistics(&serie, marker->series()->name());
      // double average = 0;
      // double min = serie.at(0).y();
      // double max = serie.at(0).y();
      // double sum = 0;

      // for (QPointF point : serie) {
      //   min > point.y() ? min = point.y() : min;
      //   max < point.y() ? max = point.y() : max;
      //   sum += point.y();
      // }
      // average = sum / serie.size();

      // // Second pass to calculate the variance
      // double variance = 0;
      // for (const QPointF &point : serie) {
      //   double diff = point.y() - average;
      //   variance += diff * diff;
      // }
      // variance = variance / serie.size();
      // double stddev = std::sqrt(variance);

      // QString unit = "";
      // QString sname = marker->series()->name();
      // if ((sname.contains("[A]")) or (sname.contains("Curr")) or
      //     (sname.contains("curr")) or (sname.contains("ström")) or
      //     (sname.contains("Strom")) or (sname.contains("strom")) or
      //     (sname.contains("Ström")))
      //   unit = "A";

      // if ((sname.contains("[V]")) or (sname.contains("Volt")) or
      //     (sname.contains("volt")) or (sname.contains("spann")) or
      //     (sname.contains("Spann")))
      //   unit = "V";

      // if ((sname.contains("[°C]")) or (sname.contains("Tempe")) or
      //     (sname.contains("tempe")) or (sname.contains("[C]")))
      //   unit = "°C";

      // if ((sname.contains("[%rh]")) or (sname.contains("Humid")) or
      //     (sname.contains("humid")) or (sname.contains("Feuch")) or
      //     (sname.contains("feuch")) or (sname.contains("[%]")))
      //   unit = "%rh";

      // QString text = "Measurements: " + QString::number(serie.size()) +
      //                "\nMin: " + engineering_Format(min) + unit +
      //                "\nMax: " + engineering_Format(max) + unit +
      //                "\nAvg: " + engineering_Format(average) + unit +
      //                "\nStdDev: " + QString::number(stddev);

      // //            qDebug() << text;

      // QMessageBox::information(this, sname + tr(" statistics:"), text);
      // QClipboard *clipboard = QApplication::clipboard();
      // clipboard->setText(marker->series()->name() + tr(" statistics:\n") +
      //                    text);
      break;
    } else if (rmb_clicked) {

      QString sname = qobject_cast<QXYSeries *>(marker->series())->name();
      QColor scolor = qobject_cast<QXYSeries *>(marker->series())->color();

      if (qobject_cast<QXYSeries *>(marker->series())->name() ==
          "Operating Mode")
        return;

      if (qobject_cast<QXYSeries *>(marker->series())->brush().style() ==
          Qt::TexturePattern) {
        auto size = qobject_cast<QXYSeries *>(marker->series())
                        ->brush()
                        .textureImage()
                        .size();
        auto image =
            qobject_cast<QXYSeries *>(marker->series())->brush().textureImage();
        bool found = false;
        for (int x = 0; x < size.width(); x++) {
          for (int y = 0; y < size.height(); y++) {
            if (image.pixelColor(x, y).alpha() == 255) {
              // qDebug() << "tried to get color" << image.pixelColor (x, y) <<
              // image.pixelColor (x, y).alpha ();
              scolor = image.pixelColor(x, y);
              found = true;
              break;
            }
          }
          if (found)
            break;
        }
      }

      QColor color =
          QColorDialog::getColor(scolor, this, "Select " + sname + " Color");

      qDebug() << sname;

      if (color.isValid()) {
        if (QXYSeries *lineseries =
                qobject_cast<QXYSeries *>(marker->series())) {
          if (lineseries->brush().style() == Qt::TexturePattern) // error
          {

            bool ok;
            QStringList items;
            items << tr("×") << tr("#") << tr("§") << tr("*") << tr("☼")
                  << tr("↑") << tr("↓") << tr("↨") << tr("¤") << tr("Θ")
                  << tr("√") << tr("•") << tr("♦") << tr("■") << tr("▲")
                  << tr("▼");

            QInputDialog symbolInput;
            symbolInput.setParent(nullptr);
            symbolInput.setWindowTitle(sname); // sname + tr(" error symbol")
            symbolInput.setLabelText(tr("Symbol:")); // Choose/insert symbol:
            symbolInput.setInputMode(QInputDialog::TextInput);
            symbolInput.setComboBoxItems(items);
            symbolInput.setFont(QFont("Arial", 18, QFont::Bold));
            symbolInput.setComboBoxEditable(true);
            ok = symbolInput.exec();
            QString text = symbolInput.textValue();
            if (ok && !text.isEmpty())
              text = text.at(0);
            else
              text = "x";

            int size = QInputDialog::getInt(this, tr("Error Symbol size"),
                                            sname + " error symbol size:", 20,
                                            4, 60, 1, &ok);
            if (ok)
              ;
            else
              size = 20;

            const auto mytest = text;
            auto font = QFont("Arial", size, QFont::Bold);
            auto fm = QFontMetrics(font);
            auto rectangle = fm.boundingRect(text);
            auto w = fm.horizontalAdvance(text);
            auto h = rectangle.height();
            qDebug() << rectangle;

            auto s = w > h ? w : h;

            QImage star(s, s, QImage::Format_ARGB32);
            star.fill(Qt::transparent);
            QPainter painter(&star);
            painter.setPen(color);
            painter.setFont(font);
            painter.drawText(star.rect(), Qt::AlignCenter, text);
            painter.end();

            lineseries->setBrush(star);
            lineseries->setPen(QColor(Qt::transparent));

            lineseries->setProperty("my_symbol", QVariant(text));
            lineseries->setProperty("my_symbol_size", QVariant(size));
            lineseries->setProperty("sprite", QVariant(star));

            QColor my_color = color;
            my_color.setAlpha(200);
            lineseries->setProperty("my_color", QVariant(my_color.rgba()));

            static_cast<QScatterSeries *>(lineseries)->setMarkerSize(s);
          } else // other
          {
            if (color != QColor(0, 0, 0)) {
              lineseries->setColor(color);
              lineseries->setBrush(color);
            } else {
              lineseries->setColor(QColor(65, 65, 65));
              lineseries->setBrush(QColor(65, 65, 65));
            }
          }

          /* replaced with universal solution below
          //also color the axes if applies
          if (lineseries->name () == "Temperature")
          {
              this->axis_temperature->setLinePenColor (color);
              this->axis_temperature->setLabelsColor (color);
              //               qobject_cast<QLineSeries *>(sender
          ())->setUseOpenGL (false);
          }
          else if (lineseries->name () == "Current active")
          {
              this->axis_current->setLinePenColor (color);
              this->axis_current->setLabelsColor (color);
              //                    qobject_cast<QScatterSeries *>(sender
          ())->setUseOpenGL (false);
          }
          else if (lineseries->name () == "Current sleep")
          {
              this->axis_sleep_current->setLinePenColor (color);
              this->axis_sleep_current->setLabelsColor (color);
              //                        qobject_cast<QScatterSeries *>(sender
          ())->setUseOpenGL (false);
          }
          else if (lineseries->name () == "Humidity")
          {
              this->axis_humidity->setLinePenColor (color);
              this->axis_humidity->setLabelsColor (color);
          }
          else if (lineseries->name () == "Voltage")
          {
              this->axis_voltage->setLinePenColor (color);
              this->axis_voltage->setLabelsColor (color);
          }
          */

          for (auto attachedaxis : lineseries->attachedAxes()) {
            if (attachedaxis->alignment() != Qt::AlignBottom) {
              attachedaxis->setLinePenColor(color);
              attachedaxis->setLabelsColor(color);
            }
          }
        }
        m_chart->legend()->update();
      }
      emit propertyChanged();
      break;
    }

    // Toggle visibility of series
    marker->series()->setVisible(!marker->series()->isVisible());
    if (marker->label().contains("(")) {
      emit markerToggled(marker->label().section('(', 0, 0));
    } else {
      emit markerToggled(marker->label());
    }

    // Turn legend marker back to visible, since hiding series also hides the
    // marker and we don't want it to happen now.
    marker->setVisible(true);

    // Dim the marker, if series is not visible
    qreal alpha = 1.0;

    // qDebug() << "serie name:" << marker->series()->name ();
    if (!marker->series()->isVisible()) // hidden
    {
      // dim the marker label
      alpha = 0.5;

      // hide the callouts
      for (Callout *callout : intPoints[marker->series()->name()]) {
        if (!callout->supressed)
          callout->hide();
      }

      //--> hide the corresponding normal MFU axis
      // if (marker->series()->name() == "Operating Mode")
      //   axis_opmode->setVisible(false);
      // else if (marker->series()->name() == "Current sleep")
      //   axis_sleep_current->setVisible(false);
      // else if (marker->series()->name() == "Current active")
      //   axis_current->setVisible(false);
      // else if (marker->series()->name() == "Humidity")
      //   axis_humidity->setVisible(false);
      // else if (marker->series()->name() == "Temperature")
      //   axis_temperature->setVisible(false);
      //<--
      // else {
        //-->if this is the last MFU AS serie on axis, hide the axis
        bool voltage_visible = false;
        bool current_visible = false;
        for (auto my_marker : m_chart->legend()->markers()) {
          if (my_marker->series()->property("my_type") == "current")
            if (my_marker->series()->isVisible())
              current_visible = true;

          if (my_marker->series()->property("my_type") == "voltage")
            if (my_marker->series()->isVisible())
              voltage_visible = true;
        }
        // if (axis_current != nullptr)
        //   axis_current->setVisible(current_visible);
        // if (axis_voltage != nullptr)
        //   axis_voltage->setVisible(voltage_visible);
        //<--
      // }
    } else // visible
    {
      // show callouts
      for (Callout *callout : intPoints[marker->series()->name()]) {
        if (!callout->supressed)
          callout->show();
      }

      //--> show the corresponding normal MFU axis
      // if (marker->series()->name() == "Operating Mode")
      //   axis_opmode->setVisible(true);
      // else if (marker->series()->name() == "Current sleep")
      //   axis_sleep_current->setVisible(true);
      // else if (marker->series()->name() == "Current active")
      //   axis_current->setVisible(true);
      // else if (marker->series()->name() == "Humidity")
      //   axis_humidity->setVisible(true);
      // else if (marker->series()->name() == "Temperature")
      //   axis_temperature->setVisible(true);
      //<--
      // else {
        //--> if any MFU AS serie is visible, show it's corresponding axis
        bool voltage_visible = false;
        bool current_visible = false;
        for (auto my_marker : m_chart->legend()->markers()) {

          if (my_marker->series()->property("my_type") == "current")
            if (my_marker->series()->isVisible())
              current_visible = true;

          if (my_marker->series()->property("my_type") == "voltage")
            if (my_marker->series()->isVisible())
              voltage_visible = true;
        }
        // if (axis_current != nullptr)
        //   axis_current->setVisible(current_visible);
        // if (axis_voltage != nullptr)
        //   axis_voltage->setVisible(voltage_visible);
        //<--
      // }
    }

    updateCallouts();

    QColor color;
    QBrush brush = marker->labelBrush();
    color = brush.color();
    color.setAlphaF(alpha);
    brush.setColor(color);
    marker->setLabelBrush(brush);

    brush = marker->brush();
    color = brush.color();
    color.setAlphaF(alpha);
    brush.setColor(color);
    marker->setBrush(brush);

    QPen pen = marker->pen();
    color = pen.color();
    color.setAlphaF(alpha);
    pen.setColor(color);
    marker->setPen(pen);

    // Start new logic for axis hide/show
    QAbstractSeries *series = marker->series();
    QList<QAbstractAxis *> axes = series->attachedAxes();
    QList<QAbstractSeries *> seriesList = m_chart->series();
    QValueAxis *yAxis = nullptr;
    for (QAbstractAxis *axis : axes) {
      if (axis->orientation() == Qt::Vertical) {
        yAxis = qobject_cast<QValueAxis *>(axis);
        break;
      }
    }

    if (yAxis != nullptr) {
      bool hasVisibleSeries = false;
      for (QAbstractSeries *otherSeries : seriesList) {
        if (otherSeries == series)
          continue;
        if (!otherSeries->isVisible())
          continue;

        QList<QAbstractAxis *> otherAxes = otherSeries->attachedAxes();
        if (otherAxes.contains(yAxis)) {
          hasVisibleSeries = true;
          break;
        }
      }
      yAxis->setVisible(hasVisibleSeries || series->isVisible());
    }
    // End new logic for axis hide/show

    View::update();
    break;
  }
  default: {
    qDebug() << "Unknown marker type";
    break;
  }
  }
}

void View::toggleLegendMarker(QString name, bool state) {
  qDebug() << "TGM";
  QLegendMarker *marker = nullptr;

  for (auto m_marker : m_chart->legend()->markers()) {
    if (m_marker->label().contains(name)) {
      marker = m_marker;
      break;
    }
  }

  if (marker == nullptr)
    return;

  if (marker->series()->isVisible() == state)
    return;

  // Toggle visibility of series
  marker->series()->setVisible(!marker->series()->isVisible());

  // Turn legend marker back to visible, since hiding series also hides the
  // marker and we don't want it to happen now.
  marker->setVisible(true);

  // Dim the marker, if series is not visible
  qreal alpha = 1.0;

  // qDebug() << "serie name:" << marker->series()->name ();
  if (!marker->series()->isVisible()) // hidden
  {
    // dim the marker label
    alpha = 0.5;

    // hide the callouts
    for (Callout *callout : intPoints[marker->series()->name()]) {
      callout->hide();
    }

    // //--> hide the corresponding normal MFU axis
    // if (marker->series()->name() == "Operating Mode")
    //   axis_opmode->setVisible(false);
    // else if (marker->series()->name() == "Current sleep")
    //   axis_sleep_current->setVisible(false);
    // else if (marker->series()->name() == "Current active")
    //   axis_current->setVisible(false);
    // else if (marker->series()->name() == "Humidity")
    //   axis_humidity->setVisible(false);
    // else if (marker->series()->name() == "Temperature")
    //   axis_temperature->setVisible(false);
    // //<--
    // else {
      //-->if this is the last MFU AS serie on axis, hide the axis
      bool voltage_visible = false;
      bool current_visible = false;
      for (auto my_marker : m_chart->legend()->markers()) {
        if (my_marker->series()->property("my_type") == "current")
          if (my_marker->series()->isVisible())
            current_visible = true;

        if (my_marker->series()->property("my_type") == "voltage")
          if (my_marker->series()->isVisible())
            voltage_visible = true;
      }
      // if (axis_current != nullptr)
      //   axis_current->setVisible(current_visible);
      // if (axis_voltage != nullptr)
      //   axis_voltage->setVisible(voltage_visible);
      //<--
    // }
  } else // visible
  {
    // show callouts
    for (Callout *callout : intPoints[marker->series()->name()]) {
      callout->show();
    }

    // //--> show the corresponding normal MFU axis
    // if (marker->series()->name() == "Operating Mode")
    //   axis_opmode->setVisible(true);
    // else if (marker->series()->name() == "Current sleep")
    //   axis_sleep_current->setVisible(true);
    // else if (marker->series()->name() == "Current active")
    //   axis_current->setVisible(true);
    // else if (marker->series()->name() == "Humidity")
    //   axis_humidity->setVisible(true);
    // else if (marker->series()->name() == "Temperature")
    //   axis_temperature->setVisible(true);
    // //<--
    // else {
      //--> if any MFU AS serie is visible, show it's corresponding axis
      bool voltage_visible = false;
      bool current_visible = false;
      for (auto my_marker : m_chart->legend()->markers()) {

        if (my_marker->series()->property("my_type") == "current")
          if (my_marker->series()->isVisible())
            current_visible = true;

        if (my_marker->series()->property("my_type") == "voltage")
          if (my_marker->series()->isVisible())
            voltage_visible = true;
      }
      // if (axis_current != nullptr)
      //   axis_current->setVisible(current_visible);
      // if (axis_voltage != nullptr)
      //   axis_voltage->setVisible(voltage_visible);
      //<--
    // }
  }

  updateCallouts();

  QColor color;
  QBrush brush = marker->labelBrush();
  color = brush.color();
  color.setAlphaF(alpha);
  brush.setColor(color);
  marker->setLabelBrush(brush);

  brush = marker->brush();
  color = brush.color();
  color.setAlphaF(alpha);
  brush.setColor(color);
  marker->setBrush(brush);

  QPen pen = marker->pen();
  color = pen.color();
  color.setAlphaF(alpha);
  pen.setColor(color);
  marker->setPen(pen);

  View::update();
}

void View::handleMarkerToggle(QLegendMarker *marker) {
  //    QLegendMarker* marker = qobject_cast<QLegendMarker*> (sender());
  //    Q_ASSERT(marker);

  // switch (marker->type())

  // {
  // case QLegendMarker::LegendMarkerTypeXY: {

  //   // Toggle visibility of series
  //   marker->series()->setVisible(!marker->series()->isVisible());

  //   // Turn legend marker back to visible, since hiding series also hides the
  //   // marker and we don't want it to happen now.
  //   marker->setVisible(true);

  //   // Dim the marker, if series is not visible
  //   qreal alpha = 1.0;

  //   if (!marker->series()->isVisible()) {

  //     alpha = 0.5;

  //     for (Callout *callout : intPoints[marker->series()->name()]) {
  //       callout->hide();
  //     }

  //     if (marker->series()->name() == "Operating Mode")
  //       axis_opmode->setVisible(false);
  //     else if (marker->series()->name() == "Current sleep")
  //       axis_sleep_current->setVisible(false);
  //     else if (marker->series()->name() == "Current active")
  //       axis_current->setVisible(false);
  //     else if (marker->series()->name() == "Humidity")
  //       axis_humidity->setVisible(false);
  //     else if (marker->series()->name() == "Temperature")
  //       axis_temperature->setVisible(false);
  //   } else {
  //     for (Callout *callout : intPoints[marker->series()->name()]) {
  //       callout->show();
  //     }

  //     if (marker->series()->name() == "Operating Mode")
  //       axis_opmode->setVisible(true);
  //     else if (marker->series()->name() == "Current sleep")
  //       axis_sleep_current->setVisible(true);
  //     else if (marker->series()->name() == "Current active")
  //       axis_current->setVisible(true);
  //     else if (marker->series()->name() == "Humidity")
  //       axis_humidity->setVisible(true);
  //     else if (marker->series()->name() == "Temperature")
  //       axis_temperature->setVisible(true);
  //   }

  //   for (auto serie : m_chart->series()) {
  //     if (serie->isVisible())
  //       for (Callout *callout : intPoints[serie->name()])
  //         callout->updateGeometry();
  //   }

  //   QColor color;
  //   QBrush brush = marker->labelBrush();
  //   color = brush.color();
  //   color.setAlphaF(alpha);
  //   brush.setColor(color);
  //   marker->setLabelBrush(brush);

  //   brush = marker->brush();
  //   color = brush.color();
  //   color.setAlphaF(alpha);
  //   brush.setColor(color);
  //   marker->setBrush(brush);

  //   QPen pen = marker->pen();
  //   color = pen.color();
  //   color.setAlphaF(alpha);
  //   pen.setColor(color);
  //   marker->setPen(pen);
  //   View::update();
  //   break;
  // }
  // default: {
  //   qDebug() << "Unknown marker type";
  //   break;
  // }
  // }
}

void View::savePNG(QString name) {

  for (auto legend_item : m_chart->legend()->markers()) {
    if (!legend_item->series()->isVisible()) {
      legend_item->setVisible(false);
    }
  }

  m_chart->legend()->layout()->invalidate();
  this->update();
  QApplication::processEvents();

  QPixmap p = this->grab(
      m_chart->rect().toAlignedRect().marginsRemoved(m_chart->margins() + 10));
  QOpenGLWidget *glWidget = View::findChild<QOpenGLWidget *>();
  if (glWidget) {
    QPainter painter(&p);
    //                    QPoint d = glWidget->mapToGlobal(QPoint()) -
    //                    View::mapToGlobal(QPoint());
    //                    painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    //                    painter.drawImage(d, glWidget->grabFramebuffer());

    painter.end();
  }
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setPixmap(p);

  qDebug() << name;
  p.save(name + "_" + QString::number(exportNr) + ".png", "PNG");
  exportNr += 1;

  for (auto legend_item : m_chart->legend()->markers()) {
    legend_item->setVisible(true);
  }

  m_chart->legend()->layout()->invalidate();
  this->update();
  QApplication::processEvents();
}

void View::adjustAxisRange(QValueAxis *axis) {
  if (axis == nullptr)
    return;
  // if (!axis->isVisible())
  //     return;
  // if (axis->orientation () == Qt::Horizontal)
  //     return;
  if (axis->objectName() == "axis_dura_signals")
    return;

  QString format = axis->labelFormat();

  // qDebug() << "initial:" << axis->objectName() << "axis format" << format
  //          << "min-max" << axis->min() << axis->max();

  // if (axis->min() < -10000000) {
  //   axis->setMin(-10000000);
  // }
  // if (axis->max() > 10000000) {
  //   axis->setMax(100000000);
  // }

  if (axis->min() == axis->max()) {
    axis->setMin(axis->min() - 1);
    axis->setMax(axis->min() + 1);
  }

  qDebug() << axis->objectName() << "axis format" << format << "min-max"
           << axis->min() << axis->max();

  if (format.contains("RA") or format.contains("RV")) {

    if (axis->max() < 0)
      ; // axis->setMax (1)
    else if (axis->max() < 0.00001)
      axis->setMax(0.00001);
    else if (axis->max() < 0.00001)
      axis->setMax(0.00001);
    else if (axis->max() < 0.00005)
      axis->setMax(0.00005);
    else if (axis->max() < 0.0001)
      axis->setMax(0.0001);
    else if (axis->max() < 0.0002)
      axis->setMax(0.0002);
    else if (axis->max() < 0.0003)
      axis->setMax(0.0003);
    else if (axis->max() < 0.0004)
      axis->setMax(0.0004);
    else if (axis->max() < 0.0005)
      axis->setMax(0.0005);
    else if (axis->max() < 0.0008)
      axis->setMax(0.0008);
    else if (axis->max() < 0.001)
      axis->setMax(0.001);
    else if (axis->max() < 0.01)
      axis->setMax(0.01);
    else if (axis->max() < 0.03)
      axis->setMax(0.03);
    else if (axis->max() < 0.05)
      axis->setMax(0.05);
    else if (axis->max() < 0.08)
      axis->setMax(0.08);
    else if (axis->max() < 0.1)
      axis->setMax(0.1);
    else if (axis->max() < 0.13)
      axis->setMax(0.13);
    else if (axis->max() < 0.15)
      axis->setMax(0.15);
    else if (axis->max() < 0.2)
      axis->setMax(0.2);
    else if (axis->max() < 0.3)
      axis->setMax(0.3);
    else if (axis->max() < 0.5)
      axis->setMax(0.5);
    else if (axis->max() < 0.8)
      axis->setMax(0.8);
    else if (axis->max() < 1)
      axis->setMax(1);
    else if (axis->max() < 1.3)
      axis->setMax(1.3);
    else if (axis->max() < 1.5)
      axis->setMax(1.5);
    else if (axis->max() < 2)
      axis->setMax(2);
    else if (axis->max() < 5)
      axis->setMax(5);
    else if (axis->max() < 10)
      axis->setMax(10);
    else if (axis->max() < 15)
      axis->setMax(15);
    else if (axis->max() < 20)
      axis->setMax(20);
    else if (axis->max() < 25)
      axis->setMax(25);
    else if (axis->max() < 30)
      axis->setMax(30);
    else if (axis->max() < 35)
      axis->setMax(35);
    else if (axis->max() < 40)
      axis->setMax(40);
    else if (axis->max() < 50)
      axis->setMax(50);
    else if (axis->max() < 60)
      axis->setMax(60);

    if (axis->min() < -35)
      ; // axis->setMin (0)
    else if ((axis->min() < -30))
      axis->setMin(-35);
    else if ((axis->min() < -20))
      axis->setMin(-30);
    else if ((axis->min() < -15))
      axis->setMin(-20);
    else if ((axis->min() < -10))
      axis->setMin(-15);
    else if ((axis->min() < -5))
      axis->setMin(-10);
    else if ((axis->min() < -1))
      axis->setMin(-5);
    else if ((axis->min() < -0.5))
      axis->setMin(-1);
    else if ((axis->min() < -0.1))
      axis->setMin(-0.5);
    else if ((axis->min() < -0.05))
      axis->setMin(-0.1);
    else if ((axis->min() < -0.01))
      axis->setMin(-0.05);
    else if ((axis->min() < -0.005))
      axis->setMin(-0.01);
    else if ((axis->min() < -0.001))
      axis->setMin(-0.005);
    else if ((axis->min() < -0.0005))
      axis->setMin(-0.001);
    else if ((axis->min() < 0))
      axis->setMin(-0.0001);
    else if ((axis->min() < 0.000005))
      axis->setMin(0);
    else if ((axis->min() < 0.00001))
      axis->setMin(0);
    else if ((axis->min() < 0.00003))
      axis->setMin(0.00001);
    else if ((axis->min() < 0.00005))
      axis->setMin(0.00003);
    else if ((axis->min() < 0.00008))
      axis->setMin(0.00005);
    else if ((axis->min() < 0.0001))
      axis->setMin(0.00008);
    else if ((axis->min() < 0.0002))
      axis->setMin(0.0001);
    else if ((axis->min() < 0.0003))
      axis->setMin(0.0002);
    else if ((axis->min() < 0.0005))
      axis->setMin(0.0003);
    else if ((axis->min() < 0.0008))
      axis->setMin(0.0005);
    else if ((axis->min() < 0.001))
      axis->setMin(0.0008);
    else if ((axis->min() < 0.002))
      axis->setMin(0.001);
    else if ((axis->min() < 0.003))
      axis->setMin(0.002);
    else if ((axis->min() < 0.005))
      axis->setMin(0.003);
    else if ((axis->min() < 0.01))
      axis->setMin(0.005);
    else if ((axis->min() < 0.02))
      axis->setMin(0.01);
    else if ((axis->min() < 0.03))
      axis->setMin(0.02);
    else if ((axis->min() < 0.05))
      axis->setMin(0.03);
    else if ((axis->min() < 0.08))
      axis->setMin(0.05);
    else if ((axis->min() < 0.1))
      axis->setMin(0.08);
    else if ((axis->min() < 0.15))
      axis->setMin(0.1);
    else if ((axis->min() < 0.5))
      axis->setMin(0.1);
    else if ((axis->min() < 1))
      axis->setMin(0);
    else if ((axis->min() < 5))
      axis->setMin(0);
    else if ((axis->min() < 10))
      axis->setMin(5);
    else if ((axis->min() < 15))
      axis->setMin(10);
    else if ((axis->min() < 20))
      axis->setMin(15);
    else if ((axis->min() < 25))
      axis->setMin(20);
    else if ((axis->min() < 30))
      axis->setMin(25);
    else if ((axis->min() < 35))
      axis->setMin(30);
    else if ((axis->min() < 40))
      axis->setMin(35);
    else if ((axis->min() < 45))
      axis->setMin(40);
    else if ((axis->min() < 50))
      axis->setMin(45);
    else if ((axis->min() < 55))
      axis->setMin(50);

    if (axis->min() == axis->max())
      axis->setMin(axis->min() - axis->min());

    qDebug() << "adjusted min-max" << axis->min() << axis->max();

    axis->setVisible(true);
  }

  else if (format.contains("%rh") or format.contains("C")) {

    if (axis->max() > 150)
      axis->setMax(axis->max() +
                   (axis->max() * 0.1)); // should not happen, do nothing;
    else if (axis->max() > 120)
      axis->setMax(150);
    else if (axis->max() > 100)
      axis->setMax(120);
    else if (axis->max() > 85)
      axis->setMax(100);
    else if (axis->max() > 60)
      axis->setMax(85);
    else if (axis->max() > 40)
      axis->setMax(60);
    else if (axis->max() > 20)
      axis->setMax(40);
    else if (axis->max() > 0)
      axis->setMax(20);
    else if (axis->max() > -10)
      axis->setMax(0);
    else if (axis->max() > -20)
      axis->setMax(-10);
    else if (axis->max() > -40)
      axis->setMax(-20);
    else if (axis->max() > -50)
      axis->setMax(-40);
    else if (axis->max() > -80)
      axis->setMax(-50);

    if (axis->min() < -80) {
      // qDebug() << "axis min set here:" << axis->min()
      //          << axis->min() + (axis->min() * 0.1);
      axis->setMin(axis->min() + (axis->min() * 0.1));
    } else if (axis->min() < -50)
      axis->setMin(-80);
    else if (axis->min() < -30)
      axis->setMin(-50);
    else if (axis->min() < -20)
      axis->setMin(-30);
    else if (axis->min() < -10)
      axis->setMin(-20);
    else if (axis->min() < 0)
      axis->setMin(-10);
    else if (axis->min() < 10)
      axis->setMin(0);
    else if (axis->min() < 20)
      axis->setMin(10);
    else if (axis->min() < 40)
      axis->setMin(20);
    else if (axis->min() < 60)
      axis->setMin(40);
    else if (axis->min() < 80)
      axis->setMin(60);
    else if (axis->min() < 100)
      axis->setMin(80);
    else if (axis->min() < 120)
      axis->setMin(100);
    else if (axis->min() < 150)
      axis->setMin(120);

    qDebug() << "adjusted min-max" << axis->min() << axis->max();
  } else if (axis->orientation() == Qt::Vertical) {
    // Axes with less-specific formatting

    if (axis->max() < 1) {
      axis->setMax(3); // Extend the maximum upwards
    } else if (axis->max() < 10) {
      axis->setMax(axis->max() + 2); // Add a margin above
    } else {
      // For larger values, add a 10% margin
      axis->setMax(axis->max() * 1.1);
    }

    if (axis->min() > 0) {
      axis->setMin(-1); // Extend minimum downwards
    } else if (axis->min() > -10) {
      axis->setMin(axis->min() - 2); // Add a margin below
    } else {
      // For smaller values, add a 10% margin
      axis->setMin(axis->min() * 0.9);
    }
  }

  if (axis->orientation() == Qt::Vertical) {
    axis->applyNiceNumbers();
    int desiredTickCount = std::max(1, (int)abs(axis->max() - axis->min()) + 1);
    int maxAllowedTicks = 11;
    desiredTickCount = std::min(desiredTickCount, maxAllowedTicks);
    axis->setTickCount(desiredTickCount);
    axis->setVisible(true);
  }
  this->initialScale.insert(axis, QPointF(axis->min(), axis->max()));

  return;
}

void View::actionOpmEdit() {
  // bool ok;
  // if (axis_opmode == nullptr)
  //   return;

  // auto current_format = axis_opmode->labelFormat().split("|");
  // QString new_format = current_format.at(0);

  // QString OpM_C =
  //     QInputDialog::getText(this, tr("Change OpM Name"), tr("OpM C name:"),
  //                           QLineEdit::Normal, current_format.at(1), &ok);
  // if (ok && !OpM_C.isEmpty())
  //   new_format += "|" + OpM_C;
  // else
  //   new_format += "|" + current_format.at(1);

  // QFileInfo fi(fileNameArg);
  // QString OpM_D =
  //     QInputDialog::getText(this, tr("Change OpM Name"), tr("OpM D name:"),
  //                           QLineEdit::Normal, current_format.at(2), &ok);
  // if (ok && !OpM_D.isEmpty())
  //   new_format += "|" + OpM_D;
  // else
  //   new_format += "|" + current_format.at(2);

  // QString OpM_E =
  //     QInputDialog::getText(this, tr("Change OpM Name"), tr("OpM E name:"),
  //                           QLineEdit::Normal, current_format.at(3), &ok);
  // if (ok && !OpM_E.isEmpty())
  //   new_format += "|" + OpM_E;
  // else
  //   new_format += "|" + current_format.at(3);

  // axis_opmode->setLabelFormat(new_format.toUtf8());

  // emit propertyChanged();
}

void View::actionTitleEdit() {
  bool ok;
  QFileInfo fi(fileNameArg);
  QString text =
      QInputDialog::getText(this, tr("Change chart title"), tr("Chart title:"),
                            QLineEdit::Normal, m_chart->title(), &ok);
  if (ok && !text.isEmpty())
    m_chart->setTitle(text);
}

double engineering_value(double value) {

  value = qAbs(value);

  if (value >= 3000000) {
    value /= 1000000;
  } else if (value >= 3000) {
    value /= 1000;
  } else if (value >= 3) {
    // No unit needed
  } else if (value >= 0.003) {
    value *= 1000;
  } else if (value >= 0.000003) {
    value *= 1000000;
  } else {
    value *= 1000000000;
  }

  return value;
}



void View::adjustAxisFormat(QValueAxis *axis, qreal min, qreal max) {
  int ticks = axis->tickCount();
  if (ticks < 2)
    return;

  min = engineering_value(min);
  max = engineering_value(max);

  double interval = (max - min) / (ticks - 1);

  // Compute minimal decimals ensuring unique tick labels
  int decimals = 0;
  bool uniqueLabels = false;
  while (!uniqueLabels && decimals < 10) {
    uniqueLabels = true;
    QString prev = QString::number(min, 'f', decimals);
    for (int i = 1; i < ticks; ++i) {
      double value = min + i * interval;
      QString current = QString::number(value, 'f', decimals);
      if (current == prev) {
        uniqueLabels = false;
        break;
      }
      prev = current;
    }
    if (!uniqueLabels)
      ++decimals;
  }

  // Build a new label format string that preserves the engineering marker 'R'
  // and the unit 'A' Note: "%%" escapes a literal '%' in the QString format.
  QString formatStr = QString("4.%1RA ").arg(decimals);
  formatStr = "%" + formatStr;
  // qDebug() << "Axis format:" << formatStr;
  axis->setLabelFormat(formatStr);
}
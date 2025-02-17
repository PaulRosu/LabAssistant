/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "callout.h"
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QMouseEvent>
#include <QApplication>
#include <QtCharts/QChart>
#include <QtMath>
#include <QDebug>

#include <QtCharts>

 bool calloutDragging;

Callout::Callout(QChart *chart):
    QGraphicsItem(chart),
    m_chart(chart)

{
}

QRectF Callout::boundingRect() const
{
    QPointF anchor = mapFromParent(m_chart->mapToPosition(m_anchor));
    QRectF rect;
    rect.setLeft(qMin(m_rect.left(), anchor.x()));
    rect.setRight(qMax(m_rect.right(), anchor.x()));
    rect.setTop(qMin(m_rect.top(), anchor.y()));
    rect.setBottom(qMax(m_rect.bottom(), anchor.y()));
    return rect;
}

void Callout::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    QPainterPath path;
    path.addRoundedRect(m_rect, 5, 5);

    QPointF anchor = mapFromParent(m_chart->mapToPosition(m_anchor));

    // qDebug() << "Entered paint - anchor:" << anchor << "m_anchor:" << m_anchor << "m_rect:" << m_rect;

    if (!m_rect.contains(anchor)) {
        QPointF point1, point2;

        // establish the position of the anchor point in relation to m_rect
        bool above = anchor.y() <= m_rect.top();
        bool aboveCenter = anchor.y() > m_rect.top() && anchor.y() <= m_rect.center().y();
        bool belowCenter = anchor.y() > m_rect.center().y() && anchor.y() <= m_rect.bottom();
        bool below = anchor.y() > m_rect.bottom();

        bool onLeft = anchor.x() <= m_rect.left();
        bool leftOfCenter = anchor.x() > m_rect.left() && anchor.x() <= m_rect.center().x();
        bool rightOfCenter = anchor.x() > m_rect.center().x() && anchor.x() <= m_rect.right();
        bool onRight = anchor.x() > m_rect.right();

        // get the nearest m_rect corner.
        qreal x = (onRight + rightOfCenter) * m_rect.width();
        qreal y = (below + belowCenter) * m_rect.height();
        bool cornerCase = (above && onLeft) || (above && onRight) || (below && onLeft) || (below && onRight);
        bool vertical = qAbs(anchor.x() - x) > qAbs(anchor.y() - y);

        qreal x1 = x + leftOfCenter * 10 - rightOfCenter * 20 + cornerCase * !vertical * (onLeft * 10 - onRight * 20);
        qreal y1 = y + aboveCenter * 10 - belowCenter * 20 + cornerCase * vertical * (above * 10 - below * 20);;
        point1.setX(x1);
        point1.setY(y1);

        qreal x2 = x + leftOfCenter * 20 - rightOfCenter * 10 + cornerCase * !vertical * (onLeft * 20 - onRight * 10);;
        qreal y2 = y + aboveCenter * 20 - belowCenter * 10 + cornerCase * vertical * (above * 20 - below * 10);;
        point2.setX(x2);
        point2.setY(y2);

        path.moveTo(point1);
        path.lineTo(anchor);
        path.lineTo(point2);
        path = path.simplified();
    }



    if (this->selected)
    {
        painter->setBrush(QColor(0, 153, 153));
        //painter->setBrush(
        //QBrush mybrush =  painter->brush();
        //mybrush.setStyle(Qt::LinearGradientPattern);
        //painter->setBrush(mybrush);
        //painter->setBrush(Qt::NoBrush);
        painter->setPen(Qt::white);

    }
    else{
        painter->setBrush(QColor(255, 255, 255));
    }


    painter->drawPath(path);

    painter->setBrush(QColor(255, 255, 255));


    painter->setFont(m_font);
    painter->drawText(m_textRect, m_text);

    painter->setBrush(color);
    painter->drawEllipse (anchor, 5.0, 5.0);

}

void Callout::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
//      qDebug() << "event" << event;
    if (event->button() == Qt::LeftButton)
    {
    calloutDragging = true;
        QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));
        //m_lastMousePos = event->pos();
    }

    if (event->type () == QEvent::GraphicsSceneMouseDoubleClick){
         this->selected = !this->selected;
         qDebug() << "Selected";
         this->update ();
    }

    event->setAccepted(true);
}

void Callout::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QApplication::restoreOverrideCursor();
//    tooltip_moving = false;
    calloutDragging = false;
    event->setAccepted(true);
}

void Callout::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton){
    calloutDragging = true;
        setPos(mapToParent(event->pos() - event->buttonDownPos(Qt::LeftButton)));
        setOffset (m_offset + event->pos() - event->buttonDownPos(Qt::LeftButton));
        event->setAccepted(true);
    } else {
        event->setAccepted(false);
    }
}

void Callout::setText(const QString &text)
{
    m_text = text;
    QFontMetrics metrics(m_font);
    m_textRect = metrics.boundingRect(QRect(0, 0, 150, 150), Qt::AlignLeft, m_text);
    m_textRect.translate(5, 5);
    prepareGeometryChange();
    m_rect = m_textRect.adjusted(-5, -5, 5, 5);
}

void Callout::setAnchor(QPointF point)
{
    m_anchor = point;
}

void Callout::setOffset(QPointF point)
{
    m_offset = point;
}

void Callout::updateGeometry(bool init)
{
    if (this->supressed) return;
    //dynamic_cast<QValueAxis*>(m_chart->axes (Qt::Vertical).first ())->min ()
    if (m_chart->mapToPosition(m_anchor).x () < m_chart->plotArea ().x () or
            m_chart->mapToPosition(m_anchor).y () < m_chart->plotArea ().top () or
            m_chart->mapToPosition(m_anchor).x () > m_chart->plotArea ().width () + m_chart->plotArea ().x () or
            m_chart->mapToPosition(m_anchor).y () > m_chart->plotArea ().height () + m_chart->plotArea ().top ()) {
       if(!init) this->hide ();
       this->in_view = false;
    }else{
       if(!init) this->show ();
       this->in_view = true;
        prepareGeometryChange();
        QPointF offset;
        offset = m_offset;

        if ((m_chart->mapToPosition(m_anchor) + m_offset).y () + this->m_rect.height ()
                > m_chart->plotArea ().height () + m_chart->plotArea ().top ())
        {
            offset.setY (m_chart->plotArea ().height ()
                         + m_chart->plotArea ().y ()
                         - this->m_rect.height ()
                         - m_chart->mapToPosition(m_anchor).y ()
                         );
        }

        if ((m_chart->mapToPosition(m_anchor) + m_offset).x () < m_chart->plotArea ().x ())
        {
            //qDebug() << "before:" << offset.x () << m_chart->mapToPosition(m_anchor).x () <<  m_chart->plotArea ().x ();
            offset.setX (m_chart->plotArea ().x () - m_chart->mapToPosition(m_anchor).x ());
            // qDebug() << "after:" << offset.x () << m_chart->mapToPosition(m_anchor).x () <<  m_chart->plotArea ().x ();
        }

        if ((m_chart->mapToPosition(m_anchor) + m_offset).y () < m_chart->plotArea ().top ())
        {
            offset.setY (m_chart->plotArea ().top () - m_chart->mapToPosition(m_anchor).y ());
        }

        if ((m_chart->mapToPosition(m_anchor) + m_offset).x () + this->m_rect.width ()
                > m_chart->plotArea ().width () + m_chart->plotArea ().x ())
        {
            offset.setX (m_chart->plotArea ().width ()
                         + m_chart->plotArea ().x ()
                         - this->m_rect.width ()
                         - m_chart->mapToPosition(m_anchor).x ()
                         );
        }

        setPos(m_chart->mapToPosition(m_anchor) + offset);
    }
}

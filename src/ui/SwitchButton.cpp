#include "SwitchButton.h"
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QMouseEvent>

SwitchButton::SwitchButton(QWidget *parent)
    : QWidget(parent)
    , m_checked(false)
    , m_position(0.0)
{
    setFixedSize(51, 31);
    setCursor(Qt::PointingHandCursor);
}

QSize SwitchButton::sizeHint() const
{
    return QSize(51, 31);
}

void SwitchButton::setChecked(bool checked)
{
    if (m_checked == checked) return;
    m_checked = checked;
    animateToggle();
    emit toggled(m_checked);
}

void SwitchButton::setPosition(qreal pos)
{
    m_position = pos;
    update();
}

void SwitchButton::animateToggle()
{
    QPropertyAnimation *anim = new QPropertyAnimation(this, "position", this);
    anim->setDuration(200);
    anim->setStartValue(m_position);
    anim->setEndValue(m_checked ? 1.0 : 0.0);
    anim->setEasingCurve(QEasingCurve::InOutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SwitchButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    qreal w = width();
    qreal h = height();
    qreal radius = h / 2.0;

    // Background track
    QColor trackColor;
    if (m_position > 0.5) {
        // Interpolate from gray to green
        int r = 76 + (int)((52 - 76) * (m_position - 0.5) * 2);
        int g = 76 + (int)((199 - 76) * (m_position - 0.5) * 2);
        int b = 76 + (int)((89 - 76) * (m_position - 0.5) * 2);
        trackColor = QColor(r, g, b);
    } else {
        trackColor = QColor(200, 200, 200);
    }

    // Smoothly interpolate
    QColor offColor(200, 200, 200);
    QColor onColor(52, 199, 89);  // iOS green
    int r = offColor.red() + (int)((onColor.red() - offColor.red()) * m_position);
    int g = offColor.green() + (int)((onColor.green() - offColor.green()) * m_position);
    int b = offColor.blue() + (int)((onColor.blue() - offColor.blue()) * m_position);
    trackColor = QColor(r, g, b);

    QPainterPath trackPath;
    trackPath.addRoundedRect(0, 0, w, h, radius, radius);
    p.fillPath(trackPath, trackColor);

    // Thumb (white circle)
    qreal thumbDiameter = h - 4;
    qreal thumbX = 2 + m_position * (w - thumbDiameter - 4);
    qreal thumbY = 2;

    // Thumb shadow
    p.setPen(Qt::NoPen);
    QColor shadowColor(0, 0, 0, 30);
    p.setBrush(shadowColor);
    p.drawEllipse(QRectF(thumbX, thumbY + 1, thumbDiameter, thumbDiameter));

    // Thumb
    p.setBrush(Qt::white);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QRectF(thumbX, thumbY, thumbDiameter, thumbDiameter));
}

void SwitchButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void SwitchButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (rect().contains(event->pos())) {
        setChecked(!m_checked);
    }
}

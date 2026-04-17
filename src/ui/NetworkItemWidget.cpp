#include "NetworkItemWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFontDatabase>
#include <QSvgRenderer>
#include <QSvgWidget>
#include <QPixmap>
#include <QIcon>

NetworkItemWidget::NetworkItemWidget(const WifiNetwork &network, QWidget *parent)
    : QWidget(parent)
    , m_network(network)
    , m_hovered(false)
{
    setupUi();
    updateDisplay();
}

void NetworkItemWidget::setupUi()
{
    setFixedHeight(52);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 12, 0);
    layout->setSpacing(8);

    // Checkmark for connected network
    m_checkLabel = new QLabel(this);
    m_checkLabel->setFixedWidth(24);
    m_checkLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_checkLabel);

    // SSID label
    m_ssidLabel = new QLabel(this);
    m_ssidLabel->setStyleSheet("font-size: 16px; color: #1c1c1e;");
    layout->addWidget(m_ssidLabel, 1);

    // Lock icon
    m_lockLabel = new QLabel(this);
    m_lockLabel->setFixedWidth(20);
    m_lockLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_lockLabel);

    // Signal strength icon
    m_signalLabel = new QLabel(this);
    m_signalLabel->setFixedWidth(24);
    m_signalLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_signalLabel);

    // Info button
    m_infoButton = new QPushButton(this);
    m_infoButton->setFixedSize(28, 28);
    m_infoButton->setCursor(Qt::PointingHandCursor);
    m_infoButton->setStyleSheet(
        "QPushButton {"
        "  border: 2px solid #007AFF;"
        "  border-radius: 14px;"
        "  background: transparent;"
        "  color: #007AFF;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  padding: 0px;"
        "}"
        "QPushButton:hover {"
        "  background: #007AFF;"
        "  color: white;"
        "}"
    );
    m_infoButton->setText("i");
    layout->addWidget(m_infoButton);

    connect(m_infoButton, &QPushButton::clicked, this, [this]() {
        emit infoRequested(m_network);
    });
}

void NetworkItemWidget::updateNetwork(const WifiNetwork &network)
{
    m_network = network;
    updateDisplay();
}

void NetworkItemWidget::updateDisplay()
{
    // Checkmark (SVG icon)
    if (m_network.isConnected()) {
        QPixmap checkPix = loadSvgIcon(":/icons/checkmark", QSize(18, 18));
        m_checkLabel->setPixmap(checkPix);
    } else {
        m_checkLabel->setPixmap(QPixmap());
        m_checkLabel->setText("");
    }

    // SSID
    m_ssidLabel->setText(m_network.ssid());

    // Lock (SVG icon)
    if (m_network.isSecured()) {
        QPixmap lockPix = loadSvgIcon(":/icons/lock", QSize(16, 16));
        m_lockLabel->setPixmap(lockPix);
    } else {
        m_lockLabel->setPixmap(QPixmap());
        m_lockLabel->setText("");
    }

    // Signal strength (SVG icon)
    int level = m_network.signalLevel();
    QString signalIcon = QString(":/icons/wifi_%1").arg(level);
    QPixmap sigPix = loadSvgIcon(signalIcon, QSize(22, 22));
    m_signalLabel->setPixmap(sigPix);
}

QString NetworkItemWidget::signalIconText() const
{
    return QString(); // Now using SVG icons
}

QPixmap NetworkItemWidget::loadSvgIcon(const QString &path, const QSize &size) const
{
    QSvgRenderer renderer(path);
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    return pixmap;
}

void NetworkItemWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_hovered) {
        p.fillRect(rect(), QColor(0, 0, 0, 10));
    }

    // Bottom separator line
    p.setPen(QPen(QColor(220, 220, 225), 0.5));
    p.drawLine(16, height() - 1, width() - 16, height() - 1);
}

void NetworkItemWidget::enterEvent(QEvent *)
{
    m_hovered = true;
    update();
}

void NetworkItemWidget::leaveEvent(QEvent *)
{
    m_hovered = false;
    update();
}

void NetworkItemWidget::mousePressEvent(QMouseEvent *event)
{
    // Don't trigger connect when clicking info button
    QWidget *child = childAt(event->pos());
    if (child == m_infoButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    QWidget::mousePressEvent(event);
}

void NetworkItemWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget *child = childAt(event->pos());
    if (child == m_infoButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    if (rect().contains(event->pos())) {
        emit connectRequested(m_network);
    }
}

#include "NetworkListWidget.h"
#include "NetworkItemWidget.h"
#include <QFrame>
#include <QTimer>

NetworkListWidget::NetworkListWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void NetworkListWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical {"
        "  background: transparent;"
        "  width: 6px;"
        "  margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #c7c7cc;"
        "  border-radius: 3px;"
        "  min-height: 30px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
    );

    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet("background: transparent;");
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);
    m_contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea);
}

void NetworkListWidget::setNetworks(const QList<WifiNetwork> &networks)
{
    rebuildList(networks);
}

void NetworkListWidget::setScanning(bool scanning)
{
    // The scanning indicator is managed in rebuildList
    Q_UNUSED(scanning);
}

void NetworkListWidget::clear()
{
    // Remove all items
    for (auto *item : m_items) {
        m_contentLayout->removeWidget(item);
        item->deleteLater();
    }
    m_items.clear();

    // Remove other widgets
    QLayoutItem *child;
    while ((child = m_contentLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
    m_contentLayout->addStretch();
}

void NetworkListWidget::rebuildList(const QList<WifiNetwork> &networks)
{
    clear();

    // Separate connected and other networks
    QList<WifiNetwork> connected;
    QList<WifiNetwork> others;

    for (const WifiNetwork &net : networks) {
        if (net.isConnected()) {
            connected.append(net);
        } else {
            others.append(net);
        }
    }

    int insertIndex = 0;

    // Connected network(s)
    for (const WifiNetwork &net : connected) {
        NetworkItemWidget *item = new NetworkItemWidget(net, m_contentWidget);
        connect(item, &NetworkItemWidget::connectRequested,
                this, &NetworkListWidget::connectRequested);
        connect(item, &NetworkItemWidget::infoRequested,
                this, &NetworkListWidget::infoRequested);
        m_contentLayout->insertWidget(insertIndex++, item);
        m_items.append(item);
    }

    // "Other Networks" header
    if (!others.isEmpty()) {
        // Spacer
        QWidget *spacer = new QWidget(m_contentWidget);
        spacer->setFixedHeight(24);
        m_contentLayout->insertWidget(insertIndex++, spacer);

        // Section header
        QWidget *headerWidget = new QWidget(m_contentWidget);
        QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
        headerLayout->setContentsMargins(16, 0, 16, 8);
        headerLayout->setSpacing(8);

        m_otherNetworksLabel = new QLabel(tr("Other Networks"), headerWidget);
        m_otherNetworksLabel->setStyleSheet(
            "font-size: 13px; font-weight: 600; color: #007AFF;"
        );
        headerLayout->addWidget(m_otherNetworksLabel);

        // Scanning indicator (spinning dots using timer)
        m_scanningIndicator = new QLabel("⟳", headerWidget);
        m_scanningIndicator->setStyleSheet("font-size: 14px; color: #8e8e93;");
        headerLayout->addWidget(m_scanningIndicator);

        headerLayout->addStretch();
        m_contentLayout->insertWidget(insertIndex++, headerWidget);

        // Other networks card background
        QWidget *cardWidget = new QWidget(m_contentWidget);
        // cardWidget->setStyleSheet(
        //     "background-color: white;"
        //     "border-radius: 10px;"
        //     "margin: 0px 16px;"
        // );
        QVBoxLayout *cardLayout = new QVBoxLayout(cardWidget);
        cardLayout->setContentsMargins(0, 4, 0, 4);
        cardLayout->setSpacing(0);

        for (const WifiNetwork &net : others) {
            NetworkItemWidget *item = new NetworkItemWidget(net, cardWidget);
            connect(item, &NetworkItemWidget::connectRequested,
                    this, &NetworkListWidget::connectRequested);
            connect(item, &NetworkItemWidget::infoRequested,
                    this, &NetworkListWidget::infoRequested);
            cardLayout->addWidget(item);
            m_items.append(item);
        }

        m_contentLayout->insertWidget(insertIndex++, cardWidget);
    }
}

#include "NetworkInfoDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

NetworkInfoDialog::NetworkInfoDialog(const WifiNetwork &network, QWidget *parent)
    : QDialog(parent)
{
    setupUi(network);
}

void NetworkInfoDialog::setupUi(const WifiNetwork &network)
{
    setWindowTitle(network.ssid());
    setFixedWidth(380);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setStyleSheet(
        "QDialog {"
        "  background-color: #F2F2F7;"
        "}"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 16);
    mainLayout->setSpacing(0);

    // Header
    QWidget *headerWidget = new QWidget(this);
    headerWidget->setStyleSheet("background-color: white; border-bottom: 1px solid #E5E5EA;");
    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 20, 20, 16);
    headerLayout->setSpacing(4);

    QLabel *titleLabel = new QLabel(network.ssid(), headerWidget);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: 700; color: #1c1c1e;");

    QLabel *statusLabel = new QLabel(
        network.isConnected() ? tr("Connected") : tr("Not Connected"),
        headerWidget
    );
    statusLabel->setStyleSheet(
        QString("font-size: 13px; color: %1;")
            .arg(network.isConnected() ? "#34C759" : "#8e8e93")
    );

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(statusLabel);
    mainLayout->addWidget(headerWidget);

    // Network Info Section
    mainLayout->addWidget(createSectionHeader(tr("NETWORK INFO")));

    QWidget *infoCard = new QWidget(this);
    infoCard->setStyleSheet(
        "background-color: white;"
        "border-radius: 10px;"
        "margin: 0px 16px;"
    );
    QVBoxLayout *infoLayout = new QVBoxLayout(infoCard);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(0);

    infoLayout->addWidget(createInfoRow(tr("SSID"), network.ssid()));
    infoLayout->addWidget(createInfoRow(tr("BSSID"), network.bssid().isEmpty() ? "-" : network.bssid()));
    infoLayout->addWidget(createInfoRow(tr("Security"),
        network.security().isEmpty() ? tr("Open") : network.security()));
    infoLayout->addWidget(createInfoRow(tr("Signal Strength"),
        QString("%1 dBm (%2%)").arg(network.signalDbm()).arg(network.signalPercent())));
    infoLayout->addWidget(createInfoRow(tr("Frequency"),
        network.frequency() > 0 ? QString("%1 GHz").arg(network.frequency(), 0, 'f', 3) : "-"));
    infoLayout->addWidget(createInfoRow(tr("Channel"),
        network.channel() > 0 ? QString::number(network.channel()) : "-"));

    mainLayout->addWidget(infoCard);

    // IP Configuration section (only for connected networks)
    if (network.isConnected()) {
        mainLayout->addWidget(createSectionHeader(tr("IP CONFIGURATION")));

        QWidget *ipCard = new QWidget(this);
        ipCard->setStyleSheet(
            "background-color: white;"
            "border-radius: 10px;"
            "margin: 0px 16px;"
        );
        QVBoxLayout *ipLayout = new QVBoxLayout(ipCard);
        ipLayout->setContentsMargins(0, 0, 0, 0);
        ipLayout->setSpacing(0);

        ipLayout->addWidget(createInfoRow(tr("IP Address"),
            network.ipAddress().isEmpty() ? "-" : network.ipAddress()));
        ipLayout->addWidget(createInfoRow(tr("Subnet Mask"),
            network.subnetMask().isEmpty() ? "-" : network.subnetMask()));
        ipLayout->addWidget(createInfoRow(tr("Gateway"),
            network.gateway().isEmpty() ? "-" : network.gateway()));
        ipLayout->addWidget(createInfoRow(tr("DNS"),
            network.dns().isEmpty() ? "-" : network.dns()));

        mainLayout->addWidget(ipCard);
    }

    mainLayout->addStretch();

    // Close button
    QPushButton *closeBtn = new QPushButton(tr("Close"), this);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 12px;"
        "  margin: 8px 16px;"
        "  font-size: 15px;"
        "  font-weight: 600;"
        "  color: white;"
        "  background-color: #007AFF;"
        "}"
        "QPushButton:hover {"
        "  background-color: #0056CC;"
        "}"
    );
    mainLayout->addWidget(closeBtn);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    adjustSize();
    setMinimumHeight(qMin(height(), 600));
}

QWidget* NetworkInfoDialog::createInfoRow(const QString &label, const QString &value)
{
    QWidget *row = new QWidget();
    row->setFixedHeight(44);
    row->setStyleSheet("background: transparent;");

    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(16, 0, 16, 0);

    QLabel *labelWidget = new QLabel(label, row);
    labelWidget->setStyleSheet("font-size: 13px; color: #8e8e93;");

    QLabel *valueWidget = new QLabel(value, row);
    valueWidget->setStyleSheet("font-size: 14px; color: #1c1c1e;");
    valueWidget->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(labelWidget);
    layout->addStretch();
    layout->addWidget(valueWidget);

    return row;
}

QWidget* NetworkInfoDialog::createSectionHeader(const QString &title)
{
    QWidget *header = new QWidget();
    header->setFixedHeight(40);

    QHBoxLayout *layout = new QHBoxLayout(header);
    layout->setContentsMargins(32, 16, 16, 4);

    QLabel *label = new QLabel(title, header);
    label->setStyleSheet("font-size: 12px; color: #8e8e93; letter-spacing: 0.5px;");

    layout->addWidget(label);
    layout->addStretch();

    return header;
}

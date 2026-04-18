#include "MainWindow.h"
#include "SwitchButton.h"
#include "NetworkListWidget.h"
#include "PasswordDialog.h"
#include "NetworkInfoDialog.h"
#include "../core/WifiManager.h"
#include "../core/WifiNetwork.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QSvgRenderer>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    applyStyles();

    // Create WiFi manager (business logic)
    m_wifiManager = new WifiManager(this);

    // Connect signals
    connect(m_toggleSwitch, &SwitchButton::toggled,
            this, &MainWindow::onWifiToggled);

    connect(m_wifiManager, &WifiManager::wifiEnabledChanged, this, [this](bool enabled) {
        m_toggleSwitch->blockSignals(true);
        m_toggleSwitch->setChecked(enabled);
        m_toggleSwitch->blockSignals(false);
        if (!enabled) {
            m_networkList->clear();
            m_statusLabel->setText(tr("Wi-Fi is turned off"));
            m_statusLabel->setVisible(true);
        } else {
            m_statusLabel->setVisible(false);
        }
    });

    connect(m_wifiManager, &WifiManager::networksUpdated,
            this, &MainWindow::onNetworksUpdated);

    connect(m_wifiManager, &WifiManager::connectionChanged,
            this, &MainWindow::onConnectionChanged);

    connect(m_wifiManager, &WifiManager::errorOccurred,
            this, &MainWindow::onErrorOccurred);

    connect(m_wifiManager, &WifiManager::scanStarted,
            this, &MainWindow::onScanStarted);

    connect(m_networkList, &NetworkListWidget::connectRequested,
            this, &MainWindow::onConnectRequested);

    connect(m_networkList, &NetworkListWidget::infoRequested,
            this, &MainWindow::onInfoRequested);

    // Initialize toggle state
    m_toggleSwitch->blockSignals(true);
    m_toggleSwitch->setChecked(m_wifiManager->isWifiEnabled());
    m_toggleSwitch->blockSignals(false);

    if (!m_wifiManager->isWifiEnabled()) {
        m_statusLabel->setText(tr("Wi-Fi is turned off"));
        m_statusLabel->setVisible(true);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    //setFixedSize(460, 800);
    setMinimumSize(420, 840);
    setWindowTitle(tr("Wi-Fi Settings"));

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    // Main card container
    QWidget *cardWidget = new QWidget(this);
    cardWidget->setObjectName("mainCard");
    QVBoxLayout *mainLayout = new QVBoxLayout(cardWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ========== Header Section ==========
    QWidget *headerWidget = new QWidget(cardWidget);
    headerWidget->setObjectName("headerWidget");
    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 24, 20, 16);
    headerLayout->setSpacing(8);

    // WiFi icon (large blue rounded square with SVG)
    m_wifiIconLabel = new QLabel(headerWidget);
    m_wifiIconLabel->setFixedSize(64, 64);
    m_wifiIconLabel->setAlignment(Qt::AlignCenter);
    // Render SVG icon
    QSvgRenderer svgRenderer(QString(":/icons/wifi_icon"));
    QPixmap iconPix(64, 64);
    iconPix.fill(Qt::transparent);
    QPainter iconPainter(&iconPix);
    svgRenderer.render(&iconPainter);
    iconPainter.end();
    m_wifiIconLabel->setPixmap(iconPix);
    headerLayout->addWidget(m_wifiIconLabel);

    headerLayout->addSpacing(8);

    // Title
    m_titleLabel = new QLabel(tr("WLAN"), headerWidget);
    m_titleLabel->setObjectName("titleLabel");
    headerLayout->addWidget(m_titleLabel);

    // Subtitle
    m_subtitleLabel = new QLabel(
        tr("Connect to Wi-Fi, view available networks, and manage network connections and nearby hotspot settings."),
        headerWidget
    );
    m_subtitleLabel->setObjectName("subtitleLabel");
    m_subtitleLabel->setWordWrap(true);
    headerLayout->addWidget(m_subtitleLabel);

    mainLayout->addWidget(headerWidget);

    // Separator
    QFrame *sep1 = new QFrame(cardWidget);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("background-color: #E5E5EA; max-height: 1px; border: none;");
    mainLayout->addWidget(sep1);

    // ========== Toggle Section ==========
    m_toggleRow = new QWidget(cardWidget);
    m_toggleRow->setObjectName("toggleRow");
    m_toggleRow->setFixedHeight(56);
    QHBoxLayout *toggleLayout = new QHBoxLayout(m_toggleRow);
    toggleLayout->setContentsMargins(20, 0, 20, 0);

    m_toggleLabel = new QLabel(tr("Wi-Fi"), m_toggleRow);
    m_toggleLabel->setObjectName("toggleLabel");

    m_toggleSwitch = new SwitchButton(m_toggleRow);

    toggleLayout->addWidget(m_toggleLabel);
    toggleLayout->addStretch();
    toggleLayout->addWidget(m_toggleSwitch);

    mainLayout->addWidget(m_toggleRow);

    // Separator
    QFrame *sep2 = new QFrame(cardWidget);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("background-color: #E5E5EA; max-height: 1px; border: none; margin: 0px 16px;");
    mainLayout->addWidget(sep2);

    // ========== Network List ==========
    m_networkList = new NetworkListWidget(cardWidget);
    mainLayout->addWidget(m_networkList, 1);

    // ========== Status Label (for when WiFi is off) ==========
    m_statusLabel = new QLabel(cardWidget);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setVisible(false);
    mainLayout->addWidget(m_statusLabel);

    outerLayout->addWidget(cardWidget);
}

void MainWindow::applyStyles()
{
    setStyleSheet(
        // Main window background
        "MainWindow {"
        "  background-color: #F2F2F7;"
        "}"

        // Main card
        "#mainCard {"
        "  background-color: #F2F2F7;"
        "  border-radius: 16px;"
        "}"

        // Header
        "#headerWidget {"
        "  background-color: white;"
        "  border-top-left-radius: 16px;"
        "  border-top-right-radius: 16px;"
        "}"

        // Title
        "#titleLabel {"
        "  font-size: 26px;"
        "  font-weight: 700;"
        "  color: #1c1c1e;"
        "}"

        // Subtitle
        "#subtitleLabel {"
        "  font-size: 13px;"
        "  color: #8e8e93;"
        "  line-height: 18px;"
        "}"

        // Toggle row
        "#toggleRow {"
        "  background-color: white;"
        "}"

        "#toggleLabel {"
        "  font-size: 16px;"
        "  font-weight: 500;"
        "  color: #1c1c1e;"
        "}"

        // Status label
        "#statusLabel {"
        "  font-size: 15px;"
        "  color: #8e8e93;"
        "  padding: 40px;"
        "}"
    );
}

void MainWindow::onWifiToggled(bool enabled)
{
    m_wifiManager->setWifiEnabled(enabled);
}

void MainWindow::onNetworksUpdated(const QList<WifiNetwork> &networks)
{
    m_networkList->setNetworks(networks);
    m_statusLabel->setVisible(networks.isEmpty() && m_wifiManager->isWifiEnabled());
    if (networks.isEmpty() && m_wifiManager->isWifiEnabled()) {
        m_statusLabel->setText(tr("Searching for networks..."));
    }
}

void MainWindow::onConnectRequested(const WifiNetwork &networkRef)
{
    WifiNetwork network = networkRef;

    if (network.isConnected()) {
        // Already connected — offer to disconnect
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Disconnect"),
            tr("Disconnect from \"%1\"?").arg(network.ssid()),
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply == QMessageBox::Yes) {
            m_wifiManager->disconnectFromNetwork();
        }
        return;
    }

    if (network.isSecured()) {
        PasswordDialog dlg(network.ssid(), this);
        if (dlg.exec() == QDialog::Accepted) {
            QString password = dlg.password();
            if (!password.isEmpty()) {
                m_wifiManager->connectToNetwork(network.ssid(), password);
            }
        }
    } else {
        // Open network — connect directly
        m_wifiManager->connectToNetwork(network.ssid());
    }
}

void MainWindow::onInfoRequested(const WifiNetwork &network)
{
    NetworkInfoDialog dlg(network, this);
    dlg.exec();
}

void MainWindow::onConnectionChanged(const QString &ssid, bool connected)
{
    Q_UNUSED(ssid);
    if (connected) {
        m_statusLabel->setVisible(false);
    }
}

void MainWindow::onErrorOccurred(const QString &message)
{
    QMessageBox::warning(this, tr("WiFi Error"), message);
}

void MainWindow::onScanStarted()
{
    m_networkList->setScanning(true);
}

#include "WifiManager.h"
#include "WifiBackend.h"
#include <QDebug>

WifiManager::WifiManager(QObject *parent)
    : QObject(parent)
    , m_backend(new WifiBackend(this))
    , m_scanTimer(new QTimer(this))
    , m_statusTimer(new QTimer(this))
    , m_wifiEnabled(false)
    , m_scanning(false)
{
    // Detect wireless interface
    m_interface = m_backend->detectInterface();
    if (m_interface.isEmpty()) {
        qWarning() << "No wireless interface detected!";
    } else {
        qDebug() << "Using wireless interface:" << m_interface;
        m_wifiEnabled = m_backend->isInterfaceUp(m_interface);
    }

    // Setup scan timer (every 15 seconds)
    m_scanTimer->setInterval(15000);
    connect(m_scanTimer, &QTimer::timeout, this, &WifiManager::onScanTimer);

    // Setup status timer (every 5 seconds)
    m_statusTimer->setInterval(5000);
    connect(m_statusTimer, &QTimer::timeout, this, &WifiManager::updateConnectionStatus);

    if (m_wifiEnabled && !m_interface.isEmpty()) {
        m_scanTimer->start();
        m_statusTimer->start();
        // Initial scan
        QTimer::singleShot(500, this, &WifiManager::requestScan);
    }
}

WifiManager::~WifiManager()
{
}

const WifiNetwork* WifiManager::connectedNetwork() const
{
    for (const WifiNetwork &net : m_networks) {
        if (net.isConnected()) {
            return &net;
        }
    }
    return nullptr;
}

void WifiManager::setWifiEnabled(bool enabled)
{
    if (m_interface.isEmpty()) {
        emit errorOccurred(tr("No wireless interface found"));
        return;
    }

    if (m_wifiEnabled == enabled) return;

    bool success = m_backend->setInterfaceUp(m_interface, enabled);
    if (!success) {
        emit errorOccurred(tr("Failed to %1 WiFi").arg(enabled ? tr("enable") : tr("disable")));
        return;
    }

    m_wifiEnabled = enabled;
    emit wifiEnabledChanged(enabled);

    if (enabled) {
        m_scanTimer->start();
        m_statusTimer->start();
        QTimer::singleShot(2000, this, &WifiManager::requestScan);
    } else {
        m_scanTimer->stop();
        m_statusTimer->stop();
        m_networks.clear();
        m_currentSsid.clear();
        emit networksUpdated(m_networks);
        emit connectionChanged(QString(), false);
    }
}

void WifiManager::requestScan()
{
    if (!m_wifiEnabled || m_interface.isEmpty()) return;

    emit scanStarted();
    doScan();
}

void WifiManager::onScanTimer()
{
    if (!m_wifiEnabled || m_scanning) return;
    doScan();
}

void WifiManager::doScan()
{
    if (m_scanning) return;
    m_scanning = true;

    QList<WifiNetwork> scanned = m_backend->scanNetworks(m_interface);

    // Check connection status and merge
    QMap<QString, QString> status = m_backend->getConnectionStatus(m_interface);
    QString connectedSsid;
    if (status.value("wpa_state") == "COMPLETED") {
        connectedSsid = status.value("ssid");
    }

    for (int i = 0; i < scanned.size(); ++i) {
        if (scanned[i].ssid() == connectedSsid) {
            scanned[i].setConnected(true);
            // Get IP info for connected network
            WifiBackend::IpInfo ipInfo = m_backend->getIpInfo(m_interface);
            scanned[i].setIpAddress(ipInfo.ipAddress);
            scanned[i].setSubnetMask(ipInfo.subnetMask);
            scanned[i].setGateway(ipInfo.gateway);
            scanned[i].setDns(ipInfo.dns);
        }
    }

    // Sort: connected first, then by signal strength
    std::sort(scanned.begin(), scanned.end(), [](const WifiNetwork &a, const WifiNetwork &b) {
        if (a.isConnected() != b.isConnected()) return a.isConnected();
        return a.signalDbm() > b.signalDbm();
    });

    m_networks = scanned;

    if (m_currentSsid != connectedSsid) {
        QString oldSsid = m_currentSsid;
        m_currentSsid = connectedSsid;
        emit connectionChanged(m_currentSsid, !m_currentSsid.isEmpty());
    }

    m_scanning = false;
    emit networksUpdated(m_networks);
}

void WifiManager::updateConnectionStatus()
{
    if (!m_wifiEnabled || m_interface.isEmpty()) return;

    QMap<QString, QString> status = m_backend->getConnectionStatus(m_interface);
    QString connectedSsid;
    if (status.value("wpa_state") == "COMPLETED") {
        connectedSsid = status.value("ssid");
    }

    if (m_currentSsid != connectedSsid) {
        m_currentSsid = connectedSsid;
        // Update connected flag in network list
        mergeConnectionInfo();
        emit connectionChanged(m_currentSsid, !m_currentSsid.isEmpty());
        emit networksUpdated(m_networks);
    }
}

void WifiManager::mergeConnectionInfo()
{
    for (int i = 0; i < m_networks.size(); ++i) {
        bool isConn = (m_networks[i].ssid() == m_currentSsid);
        m_networks[i].setConnected(isConn);
        if (isConn) {
            WifiBackend::IpInfo ipInfo = m_backend->getIpInfo(m_interface);
            m_networks[i].setIpAddress(ipInfo.ipAddress);
            m_networks[i].setSubnetMask(ipInfo.subnetMask);
            m_networks[i].setGateway(ipInfo.gateway);
            m_networks[i].setDns(ipInfo.dns);
        }
    }
}

void WifiManager::connectToNetwork(const QString &ssid, const QString &password)
{
    if (m_interface.isEmpty()) {
        emit errorOccurred(tr("No wireless interface found"));
        return;
    }

    emit connectingToNetwork(ssid);

    bool success = m_backend->connectToNetwork(m_interface, ssid, password);
    if (!success) {
        emit errorOccurred(tr("Failed to connect to %1").arg(ssid));
        return;
    }

    // Wait and check connection status
    QTimer::singleShot(3000, this, [this, ssid]() {
        updateConnectionStatus();
        if (m_currentSsid != ssid) {
            // Try again after more time
            QTimer::singleShot(5000, this, [this, ssid]() {
                updateConnectionStatus();
                if (m_currentSsid != ssid) {
                    emit errorOccurred(tr("Connection to %1 may have failed. Check password and try again.").arg(ssid));
                }
            });
        }
        requestScan();
    });
}

void WifiManager::disconnectFromNetwork()
{
    if (m_interface.isEmpty()) return;

    bool success = m_backend->disconnectNetwork(m_interface);
    if (success) {
        m_currentSsid.clear();
        mergeConnectionInfo();
        emit connectionChanged(QString(), false);
        emit networksUpdated(m_networks);
    }
}

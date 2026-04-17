#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <QObject>
#include <QTimer>
#include <QList>
#include "WifiNetwork.h"

class WifiBackend;

/**
 * @brief High-level WiFi manager providing a clean API for the UI layer.
 *
 * Manages periodic scanning, connection state, and exposes signals
 * for the UI to react to. No UI dependencies.
 */
class WifiManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool wifiEnabled READ isWifiEnabled WRITE setWifiEnabled NOTIFY wifiEnabledChanged)
    Q_PROPERTY(QString currentSsid READ currentSsid NOTIFY connectionChanged)

public:
    explicit WifiManager(QObject *parent = nullptr);
    ~WifiManager();

    bool isWifiEnabled() const { return m_wifiEnabled; }
    QString currentSsid() const { return m_currentSsid; }
    QString interfaceName() const { return m_interface; }
    QList<WifiNetwork> networks() const { return m_networks; }

    /**
     * @brief Get the currently connected network (if any).
     * @return Pointer to connected network or nullptr.
     */
    const WifiNetwork* connectedNetwork() const;

public slots:
    /**
     * @brief Enable or disable WiFi.
     */
    void setWifiEnabled(bool enabled);

    /**
     * @brief Trigger a manual WiFi scan.
     */
    void requestScan();

    /**
     * @brief Connect to a specific network.
     * @param ssid Network SSID.
     * @param password Network password.
     */
    void connectToNetwork(const QString &ssid, const QString &password = QString());

    /**
     * @brief Disconnect from the current network.
     */
    void disconnectFromNetwork();

signals:
    void wifiEnabledChanged(bool enabled);
    void scanStarted();
    void networksUpdated(const QList<WifiNetwork> &networks);
    void connectionChanged(const QString &ssid, bool connected);
    void errorOccurred(const QString &message);
    void connectingToNetwork(const QString &ssid);

private slots:
    void onScanTimer();
    void updateConnectionStatus();

private:
    WifiBackend *m_backend;
    QTimer *m_scanTimer;
    QTimer *m_statusTimer;
    QString m_interface;
    QList<WifiNetwork> m_networks;
    QString m_currentSsid;
    bool m_wifiEnabled;
    bool m_scanning;

    void doScan();
    void mergeConnectionInfo();
};

#endif // WIFIMANAGER_H

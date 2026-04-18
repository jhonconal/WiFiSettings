#ifndef WIFIBACKEND_H
#define WIFIBACKEND_H

#include <QObject>
#include <QList>
#include <QProcess>
#include "WifiNetwork.h"

/**
 * @brief Low-level WiFi command execution backend.
 *
 * Wraps system commands (iwlist, wpa_cli) to perform
 * WiFi operations. No UI dependencies.
 */
class WifiBackend : public QObject
{
    Q_OBJECT

public:
    explicit WifiBackend(QObject *parent = nullptr);

    /**
     * @brief Auto-detect the wireless interface name.
     * @return Interface name (e.g., "wlan0") or empty string if none found.
     */
    QString detectInterface() const;

    /**
     * @brief Scan for available WiFi networks.
     * @param interface Wireless interface name.
     * @return List of discovered networks.
     */
    QList<WifiNetwork> scanNetworks(const QString &interface);

    /**
     * @brief Get current connection status from wpa_supplicant.
     * @param interface Wireless interface name.
     * @return Map of status key-value pairs (ssid, bssid, ip_address, etc.)
     */
    QMap<QString, QString> getConnectionStatus(const QString &interface);

    /**
     * @brief Connect to a WiFi network.
     * @param interface Wireless interface name.
     * @param ssid Network SSID.
     * @param password Network password (empty for open networks).
     * @return true if connection command was issued successfully.
     */
    bool connectToNetwork(const QString &interface, const QString &ssid,
                          const QString &password);

    /**
     * @brief Disconnect from current WiFi network.
     * @param interface Wireless interface name.
     * @return true if disconnect command succeeded.
     */
    bool disconnectNetwork(const QString &interface);

    /**
     * @brief Enable or disable the wireless interface.
     */
    bool setInterfaceUp(const QString &interface, bool up);

    /**
     * @brief Check if WiFi interface is up.
     */
    bool isInterfaceUp(const QString &interface) const;

    /**
     * @brief Get IP configuration for an interface.
     */
    struct IpInfo {
        QString ipAddress;
        QString subnetMask;
        QString gateway;
        QString dns;
    };
    IpInfo getIpInfo(const QString &interface) const;

private:
    /**
     * @brief Execute a system command and return stdout.
     */
    QString executeCommand(const QString &command, const QStringList &args, int timeoutMs = 10000) const;

    /**
     * @brief Parse iwlist scan output into a list of WifiNetwork objects.
     */
    QList<WifiNetwork> parseIwlistOutput(const QString &output);

    /**
     * @brief Parse nmcli terse scan output into a list of WifiNetwork objects.
     */
    QList<WifiNetwork> parseNmcliOutput(const QString &output);

    /**
     * @brief Parse wpa_cli status output into a key-value map.
     */
    QMap<QString, QString> parseWpaStatus(const QString &output);
};

#endif // WIFIBACKEND_H

#ifndef WIFINETWORK_H
#define WIFINETWORK_H

#include <QString>
#include <QMetaType>

/**
 * @brief Data model representing a single WiFi network.
 *
 * This is a pure data class with no UI dependencies.
 * It holds all information about a discovered WiFi network.
 */
class WifiNetwork
{
public:
    WifiNetwork();
    WifiNetwork(const QString &ssid, const QString &bssid, int signalDbm,
                double frequency, int channel, const QString &security,
                bool connected = false);

    // --- Accessors ---
    QString ssid() const { return m_ssid; }
    QString bssid() const { return m_bssid; }
    int signalDbm() const { return m_signalDbm; }
    int signalPercent() const;
    double frequency() const { return m_frequency; }
    int channel() const { return m_channel; }
    QString security() const { return m_security; }
    bool isSecured() const { return !m_security.isEmpty() && m_security != "Open"; }
    bool isConnected() const { return m_connected; }

    // --- Mutators ---
    void setSsid(const QString &ssid) { m_ssid = ssid; }
    void setBssid(const QString &bssid) { m_bssid = bssid; }
    void setSignalDbm(int dbm) { m_signalDbm = dbm; }
    void setFrequency(double freq) { m_frequency = freq; }
    void setChannel(int ch) { m_channel = ch; }
    void setSecurity(const QString &sec) { m_security = sec; }
    void setConnected(bool connected) { m_connected = connected; }

    // --- Connected network extra info ---
    QString ipAddress() const { return m_ipAddress; }
    QString subnetMask() const { return m_subnetMask; }
    QString gateway() const { return m_gateway; }
    QString dns() const { return m_dns; }

    void setIpAddress(const QString &ip) { m_ipAddress = ip; }
    void setSubnetMask(const QString &mask) { m_subnetMask = mask; }
    void setGateway(const QString &gw) { m_gateway = gw; }
    void setDns(const QString &dns) { m_dns = dns; }

    /**
     * @brief Returns signal strength level 0-3 for icon selection.
     */
    int signalLevel() const;

    bool operator==(const WifiNetwork &other) const;

private:
    QString m_ssid;
    QString m_bssid;
    int m_signalDbm;
    double m_frequency;
    int m_channel;
    QString m_security;
    bool m_connected;

    // Extra info for connected network
    QString m_ipAddress;
    QString m_subnetMask;
    QString m_gateway;
    QString m_dns;
};

Q_DECLARE_METATYPE(WifiNetwork)

#endif // WIFINETWORK_H

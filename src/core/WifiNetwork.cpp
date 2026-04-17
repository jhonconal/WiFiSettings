#include "WifiNetwork.h"

WifiNetwork::WifiNetwork()
    : m_signalDbm(-100)
    , m_frequency(0.0)
    , m_channel(0)
    , m_connected(false)
{
}

WifiNetwork::WifiNetwork(const QString &ssid, const QString &bssid, int signalDbm,
                         double frequency, int channel, const QString &security,
                         bool connected)
    : m_ssid(ssid)
    , m_bssid(bssid)
    , m_signalDbm(signalDbm)
    , m_frequency(frequency)
    , m_channel(channel)
    , m_security(security)
    , m_connected(connected)
{
}

int WifiNetwork::signalPercent() const
{
    // Convert dBm to percentage (typical range: -100 to -30 dBm)
    if (m_signalDbm >= -30) return 100;
    if (m_signalDbm <= -100) return 0;
    return 2 * (m_signalDbm + 100);
}

int WifiNetwork::signalLevel() const
{
    int pct = signalPercent();
    if (pct >= 75) return 3;  // Excellent
    if (pct >= 50) return 2;  // Good
    if (pct >= 25) return 1;  // Fair
    return 0;                  // Weak
}

bool WifiNetwork::operator==(const WifiNetwork &other) const
{
    return m_bssid == other.m_bssid && m_ssid == other.m_ssid;
}

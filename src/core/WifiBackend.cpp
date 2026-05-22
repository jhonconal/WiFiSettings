#include "WifiBackend.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <QStandardPaths>


WifiBackend::WifiBackend(QObject *parent)
    : QObject(parent)
{
}

QString WifiBackend::detectInterface() const
{
    QDir sysNetDir("/sys/class/net");
    QStringList ifaces = sysNetDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &iface : ifaces) {
        QString wirelessPath = QString("/sys/class/net/%1/wireless").arg(iface);
        if (QDir(wirelessPath).exists()) {
            return iface;
        }
        // Also check phy80211
        QString phy80211Path = QString("/sys/class/net/%1/phy80211").arg(iface);
        if (QDir(phy80211Path).exists()) {
            return iface;
        }
    }

    // Fallback: try common names
    QStringList common = {"wlan0", "wlp2s0", "wlp3s0", "wifi0","wlxe84e066dc4bb"};
    for (const QString &name : common) {
        if (QFile::exists(QString("/sys/class/net/%1").arg(name))) {
            return name;
        }
    }

    return QString();
}

QString WifiBackend::executeCommand(const QString &command, const QStringList &args, int timeoutMs) const
{
    QString cmd = command;
    QStringList actualArgs = args;

    QProcess process;
    
    // Instead of blocking with process.waitForFinished, use a nested event loop
    // so the application UI remains responsive and doesn't freeze.
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(timeoutMs);
    timer.setSingleShot(true);
    
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    connect(&process, &QProcess::errorOccurred, &loop, &QEventLoop::quit);
    
    process.start(cmd, actualArgs);
    timer.start();
    loop.exec();

    if (process.state() == QProcess::Running) {
        qWarning() << "Command timed out:" << cmd << actualArgs;
        process.kill();
        process.waitForFinished(1000);
        return QString();
    }

    if (process.exitCode() != 0) {
        QString errOutput = process.readAllStandardError();
        qWarning() << "Command failed:" << cmd << actualArgs << "Error:" << errOutput;
    }

    return QString::fromUtf8(process.readAllStandardOutput());
}

QList<WifiNetwork> WifiBackend::scanNetworks(const QString &interface)
{
    bool hasNmcli = !QStandardPaths::findExecutable("nmcli").isEmpty();
    if (hasNmcli) {
        // Try to request a fresh scan from NetworkManager. We don't worry about failures 
        // here because if it fails (e.g. scanning too frequently), it's fine to use cached list.
        executeCommand("nmcli", {"dev", "wifi", "rescan", "ifname", interface}, 5000);

        // Get the parsed list from NetworkManager
        QString nmOutput = executeCommand("nmcli", {"-t", "-f", "IN-USE,SSID,BSSID,SIGNAL,FREQ,CHAN,SECURITY", "dev", "wifi", "list", "ifname", interface}, 10000);
        
        if (!nmOutput.isEmpty() && !nmOutput.contains("Error:", Qt::CaseInsensitive)) {
            return parseNmcliOutput(nmOutput);
        }
    }

    // Fallback to iwlist if nmcli is not available
    QString output = executeCommand("iwlist", {interface, "scan"}, 15000);
    return parseIwlistOutput(output);
}

QList<WifiNetwork> WifiBackend::parseNmcliOutput(const QString &output)
{
    QList<WifiNetwork> networks;
    QStringList lines = output.split('\n', QString::SkipEmptyParts);
    
    for (const QString &line : lines) {
        QStringList parts;
        QString current;
        // nmcli escapes colons with '\:' in terse mode
        for (int i = 0; i < line.length(); ++i) {
            if (line[i] == '\\' && i + 1 < line.length() && line[i+1] == ':') {
                current.append(':');
                i++;
            } else if (line[i] == ':') {
                parts.append(current);
                current.clear();
            } else {
                current.append(line[i]);
            }
        }
        parts.append(current);
        
        if (parts.size() < 7) continue;
        
        WifiNetwork net;
        net.setSsid(parts[1]);
        if (net.ssid().isEmpty() || net.ssid() == "--") continue;
        
        net.setBssid(parts[2]);
        
        int percent = parts[3].toInt();
        int dbm = (percent / 2) - 100;
        net.setSignalDbm(dbm);
        
        QString freqStr = parts[4];
        freqStr.remove(" MHz", Qt::CaseInsensitive); // nmcli outputs '2412 MHz'
        net.setFrequency(freqStr.trimmed().toDouble() / 1000.0);

        net.setChannel(parts[5].toInt());
        
        QString sec = parts[6];
        if (sec.isEmpty() || sec == "--") {
            net.setSecurity("Open");
        } else {
            net.setSecurity(sec);
        }
        
        // De-duplicate: keep strongest signal
        bool found = false;
        for (int j = 0; j < networks.size(); ++j) {
            if (networks[j].ssid() == net.ssid()) {
                if (net.signalDbm() > networks[j].signalDbm()) {
                    networks[j] = net;
                }
                found = true;
                break;
            }
        }
        if (!found) {
            networks.append(net);
        }
    }
    
    return networks;
}

QList<WifiNetwork> WifiBackend::parseIwlistOutput(const QString &output)
{
    QList<WifiNetwork> networks;
    if (output.isEmpty()) return networks;

    // Split by "Cell " to get individual network blocks
    QStringList cells = output.split(QRegularExpression("Cell \\d+ - "));

    for (int i = 1; i < cells.size(); ++i) {
        const QString &cell = cells[i];
        WifiNetwork net;

        // Parse BSSID (Address)
        QRegularExpression reAddr("Address:\\s*([0-9A-Fa-f:]+)");
        auto matchAddr = reAddr.match(cell);
        if (matchAddr.hasMatch()) {
            net.setBssid(matchAddr.captured(1));
        }

        // Parse SSID
        QRegularExpression reSsid("ESSID:\"([^\"]*)\"");
        auto matchSsid = reSsid.match(cell);
        if (matchSsid.hasMatch()) {
            QString rawSsid = matchSsid.captured(1);
            QByteArray decodedBytes;
            for (int k = 0; k < rawSsid.length(); ) {
                if (rawSsid[k] == '\\' && k + 3 < rawSsid.length() && rawSsid[k+1] == 'x') {
                    bool ok;
                    int byteVal = rawSsid.mid(k + 2, 2).toInt(&ok, 16);
                    if (ok) {
                        decodedBytes.append(static_cast<char>(byteVal));
                        k += 4;
                        continue;
                    }
                } else if (rawSsid[k] == '\\' && k + 1 < rawSsid.length()) {
                    decodedBytes.append(rawSsid.at(k+1).toLatin1());
                    k += 2;
                    continue;
                }
                
                int nextEscape = rawSsid.indexOf('\\', k);
                if (nextEscape == -1) {
                    decodedBytes.append(rawSsid.mid(k).toUtf8());
                    break;
                } else {
                    decodedBytes.append(rawSsid.mid(k, nextEscape - k).toUtf8());
                    k = nextEscape;
                }
            }
            net.setSsid(QString::fromUtf8(decodedBytes));
        }

        // Skip hidden SSIDs
        if (net.ssid().isEmpty()) continue;

        // Parse signal level
        QRegularExpression reSig("Signal level[=:]\\s*(-?\\d+)\\s*dBm");
        auto matchSig = reSig.match(cell);
        if (matchSig.hasMatch()) {
            net.setSignalDbm(matchSig.captured(1).toInt());
        } else {
            // Try quality format: Quality=XX/100
            QRegularExpression reQual("Quality[=:]\\s*(\\d+)/(\\d+)");
            auto matchQual = reQual.match(cell);
            if (matchQual.hasMatch()) {
                int quality = matchQual.captured(1).toInt();
                int max = matchQual.captured(2).toInt();
                // Convert to approximate dBm
                int dbm = -100 + (quality * 70 / (max > 0 ? max : 100));
                net.setSignalDbm(dbm);
            }
        }

        // Parse frequency
        QRegularExpression reFreq("Frequency:\\s*([\\d.]+)\\s*GHz\\s*\\(Channel\\s*(\\d+)\\)");
        auto matchFreq = reFreq.match(cell);
        if (matchFreq.hasMatch()) {
            net.setFrequency(matchFreq.captured(1).toDouble());
            net.setChannel(matchFreq.captured(2).toInt());
        } else {
            // Try alternative format
            QRegularExpression reFreq2("Frequency:\\s*([\\d.]+)\\s*GHz");
            auto matchFreq2 = reFreq2.match(cell);
            if (matchFreq2.hasMatch()) {
                net.setFrequency(matchFreq2.captured(1).toDouble());
            }
            QRegularExpression reChan("Channel:\\s*(\\d+)");
            auto matchChan = reChan.match(cell);
            if (matchChan.hasMatch()) {
                net.setChannel(matchChan.captured(1).toInt());
            }
        }

        // Parse security
        if (cell.contains("WPA2")) {
            net.setSecurity("WPA2");
        } else if (cell.contains("WPA")) {
            net.setSecurity("WPA");
        } else if (cell.contains("WEP")) {
            net.setSecurity("WEP");
        } else {
            // Check encryption key
            QRegularExpression reEnc("Encryption key:\\s*(on|off)");
            auto matchEnc = reEnc.match(cell);
            if (matchEnc.hasMatch() && matchEnc.captured(1) == "off") {
                net.setSecurity("Open");
            } else {
                net.setSecurity("WPA2");  // Default assumption
            }
        }

        // De-duplicate: keep the one with strongest signal
        bool found = false;
        for (int j = 0; j < networks.size(); ++j) {
            if (networks[j].ssid() == net.ssid()) {
                if (net.signalDbm() > networks[j].signalDbm()) {
                    networks[j] = net;
                }
                found = true;
                break;
            }
        }
        if (!found) {
            networks.append(net);
        }
    }

    return networks;
}

QMap<QString, QString> WifiBackend::getConnectionStatus(const QString &interface)
{
    QString output = executeCommand("wpa_cli", {"-i", interface, "status"});
    return parseWpaStatus(output);
}

QMap<QString, QString> WifiBackend::parseWpaStatus(const QString &output)
{
    QMap<QString, QString> status;
    if (output.isEmpty()) return status;

    QStringList lines = output.split('\n', QString::SkipEmptyParts);
    for (const QString &line : lines) {
        int eqPos = line.indexOf('=');
        if (eqPos > 0) {
            QString key = line.left(eqPos).trimmed();
            QString value = line.mid(eqPos + 1).trimmed();
            status[key] = value;
        }
    }
    return status;
}

bool WifiBackend::connectToNetwork(const QString &interface, const QString &ssid,
                                   const QString &password)
{
    // Step 1: Add a new network
    QString addResult = executeCommand("wpa_cli", {"-i", interface, "add_network"});

    QString networkId = addResult.trimmed();
    // The last line should be the network ID (a number)
    QStringList lines = networkId.split('\n', QString::SkipEmptyParts);
    if (lines.isEmpty()) return false;
    networkId = lines.last().trimmed();

    bool ok;
    networkId.toInt(&ok);
    if (!ok) {
        qWarning() << "Failed to add network, got:" << addResult;
        return false;
    }

    // Step 2: Set SSID
    QString setSSID = executeCommand("wpa_cli",
        {"-i", interface, "set_network", networkId, "ssid", QString("\"%1\"").arg(ssid)});

    // Step 3: Set PSK or key_mgmt for open networks
    if (password.isEmpty()) {
        executeCommand("wpa_cli",
            {"-i", interface, "set_network", networkId, "key_mgmt", "NONE"});
    } else {
        executeCommand("wpa_cli",
            {"-i", interface, "set_network", networkId, "psk", QString("\"%1\"").arg(password)});
    }

    // Step 4: Enable & select the network
    executeCommand("wpa_cli", {"-i", interface, "select_network", networkId});
    executeCommand("wpa_cli", {"-i", interface, "enable_network", networkId});

    // Step 5: Save configuration
    executeCommand("wpa_cli", {"-i", interface, "save_config"});

    // Step 6: Request DHCP
    QString dhcpCmd;
    if (!QStandardPaths::findExecutable("dhclient").isEmpty()) {
        dhcpCmd = "dhclient";
    } else if (!QStandardPaths::findExecutable("udhcpc").isEmpty()) {
        dhcpCmd = "udhcpc";
    } else if (!QStandardPaths::findExecutable("dhcpcd").isEmpty()) {
        dhcpCmd = "dhcpcd";
    }

    if (dhcpCmd == "dhclient") {
        executeCommand("dhclient", {interface}, 15000);
    } else if (dhcpCmd == "udhcpc") {
        executeCommand("udhcpc", {"-i", interface, "-n"}, 15000);
    } else if (dhcpCmd == "dhcpcd") {
        executeCommand("dhcpcd", {interface}, 15000);
    } else {
        qWarning() << "No supported DHCP client (dhclient, udhcpc, dhcpcd) found in PATH.";
    }

    return true;
}

bool WifiBackend::disconnectNetwork(const QString &interface)
{
    QString result = executeCommand("wpa_cli", {"-i", interface, "disconnect"});
    return result.contains("OK");
}

bool WifiBackend::setInterfaceUp(const QString &interface, bool up)
{
    QString action = up ? "up" : "down";
    executeCommand("ip", {"link", "set", interface, action});
    return true;  // ip link doesn't always produce output on success
}

bool WifiBackend::isInterfaceUp(const QString &interface) const
{
    QString flagsPath = QString("/sys/class/net/%1/flags").arg(interface);
    QFile file(flagsPath);
    if (file.open(QIODevice::ReadOnly)) {
        QString flags = file.readAll().trimmed();
        bool ok;
        uint flagVal = flags.toUInt(&ok, 16);
        if (ok) {
            return (flagVal & 0x1) != 0;  // IFF_UP
        }
    }

    // Fallback: check operstate
    QString statePath = QString("/sys/class/net/%1/operstate").arg(interface);
    QFile stateFile(statePath);
    if (stateFile.open(QIODevice::ReadOnly)) {
        QString state = stateFile.readAll().trimmed();
        return state != "down";
    }

    return false;
}

WifiBackend::IpInfo WifiBackend::getIpInfo(const QString &interface) const
{
    IpInfo info;

    // Get IP address and subnet using 'ip addr show'
    QProcess proc;
    proc.start("ip", {"addr", "show", interface});
    proc.waitForFinished(5000);
    QString output = QString::fromUtf8(proc.readAllStandardOutput());

    QRegularExpression reInet("inet\\s+(\\d+\\.\\d+\\.\\d+\\.\\d+)/(\\d+)");
    auto match = reInet.match(output);
    if (match.hasMatch()) {
        info.ipAddress = match.captured(1);
        int prefix = match.captured(2).toInt();
        // Convert CIDR to netmask
        uint32_t mask = prefix > 0 ? (~uint32_t(0)) << (32 - prefix) : 0;
        info.subnetMask = QString("%1.%2.%3.%4")
            .arg((mask >> 24) & 0xFF)
            .arg((mask >> 16) & 0xFF)
            .arg((mask >> 8) & 0xFF)
            .arg(mask & 0xFF);
    }

    // Get gateway from 'ip route'
    proc.start("ip", {"route", "show", "default"});
    proc.waitForFinished(5000);
    output = QString::fromUtf8(proc.readAllStandardOutput());

    QRegularExpression reGw("default via (\\d+\\.\\d+\\.\\d+\\.\\d+)");
    match = reGw.match(output);
    if (match.hasMatch()) {
        info.gateway = match.captured(1);
    }

    // Get DNS from /etc/resolv.conf
    QFile dnsFile("/etc/resolv.conf");
    if (dnsFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&dnsFile);
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            if (line.startsWith("nameserver")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                if (parts.size() >= 2) {
                    if (info.dns.isEmpty()) {
                        info.dns = parts[1];
                    } else {
                        info.dns += ", " + parts[1];
                    }
                }
            }
        }
    }

    return info;
}

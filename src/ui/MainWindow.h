#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "../core/WifiNetwork.h"

class WifiManager;
class SwitchButton;
class NetworkListWidget;

/**
 * @brief Main application window assembling all WiFi Settings UI.
 *
 * Matches the iOS-style mockup with header, toggle, and network list.
 */
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onWifiToggled(bool enabled);
    void onNetworksUpdated(const QList<WifiNetwork> &networks);
    void onConnectRequested(const WifiNetwork &network);
    void onInfoRequested(const WifiNetwork &network);
    void onConnectionChanged(const QString &ssid, bool connected);
    void onErrorOccurred(const QString &message);
    void onScanStarted();

private:
    WifiManager *m_wifiManager;

    // Header
    QLabel *m_wifiIconLabel;
    QLabel *m_titleLabel;
    QLabel *m_subtitleLabel;

    // Toggle
    QWidget *m_toggleRow;
    QLabel *m_toggleLabel;
    SwitchButton *m_toggleSwitch;

    // Network list
    NetworkListWidget *m_networkList;

    // Status
    QLabel *m_statusLabel;

    void setupUi();
    void applyStyles();
};

#endif // MAINWINDOW_H

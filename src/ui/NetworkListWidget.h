#ifndef NETWORKLISTWIDGET_H
#define NETWORKLISTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QList>
#include "../core/WifiNetwork.h"

class NetworkItemWidget;

/**
 * @brief Scrollable list widget displaying WiFi networks.
 *
 * Divided into "Connected" and "Other Networks" sections.
 */
class NetworkListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkListWidget(QWidget *parent = nullptr);

    void setNetworks(const QList<WifiNetwork> &networks);
    void setScanning(bool scanning);
    void clear();

signals:
    void connectRequested(const WifiNetwork &network);
    void infoRequested(const WifiNetwork &network);

private:
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;
    QLabel *m_otherNetworksLabel;
    QLabel *m_scanningIndicator;
    QList<NetworkItemWidget*> m_items;

    void setupUi();
    void rebuildList(const QList<WifiNetwork> &networks);
};

#endif // NETWORKLISTWIDGET_H

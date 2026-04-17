#ifndef NETWORKITEMWIDGET_H
#define NETWORKITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPixmap>
#include "../core/WifiNetwork.h"

/**
 * @brief Widget representing a single WiFi network row.
 *
 * Shows: [checkmark] SSID [lock] [signal] [info]
 * Clicking the SSID area triggers connection.
 * Clicking info (ⓘ) shows network details.
 */
class NetworkItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkItemWidget(const WifiNetwork &network, QWidget *parent = nullptr);

    void updateNetwork(const WifiNetwork &network);
    WifiNetwork network() const { return m_network; }

signals:
    void connectRequested(const WifiNetwork &network);
    void infoRequested(const WifiNetwork &network);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    WifiNetwork m_network;
    QLabel *m_checkLabel;
    QLabel *m_ssidLabel;
    QLabel *m_lockLabel;
    QLabel *m_signalLabel;
    QPushButton *m_infoButton;
    bool m_hovered;

    void setupUi();
    void updateDisplay();
    QString signalIconText() const;
    QPixmap loadSvgIcon(const QString &path, const QSize &size) const;
};

#endif // NETWORKITEMWIDGET_H

#ifndef NETWORKINFODIALOG_H
#define NETWORKINFODIALOG_H

#include <QDialog>
#include "../core/WifiNetwork.h"

/**
 * @brief Dialog showing detailed information about a WiFi network.
 *
 * For connected networks, also shows IP configuration.
 */
class NetworkInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkInfoDialog(const WifiNetwork &network, QWidget *parent = nullptr);

private:
    void setupUi(const WifiNetwork &network);
    QWidget* createInfoRow(const QString &label, const QString &value);
    QWidget* createSectionHeader(const QString &title);
};

#endif // NETWORKINFODIALOG_H
